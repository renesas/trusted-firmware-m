#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

find_package(Python3)

project("Bootloader" VERSION 0.1.0 LANGUAGES C ASM)

############################### BL2_CRYPTO_CONFIG ##############################

if(NOT ${MCUBOOT_SIGNATURE_TYPE} STREQUAL "")
    string(REGEX MATCH "[0-9]*$" SIG_LEN ${MCUBOOT_SIGNATURE_TYPE})
    string(REGEX MATCH "^[A-Z]*" SIG_TYPE ${MCUBOOT_SIGNATURE_TYPE})
endif()

set(is_ec_signature  "$<STREQUAL:${SIG_TYPE},EC>")
set(is_rsa_signature "$<STREQUAL:${SIG_TYPE},RSA>")
# ML-DSA-87 doesn't fit the <TYPE><LEN> parsing above (SIG_TYPE = "ML"),
# so compare against the full signature type string.
set(is_mldsa87_signature "$<STREQUAL:${MCUBOOT_SIGNATURE_TYPE},ML-DSA-87>")

add_library(bl2_crypto_config INTERFACE)

target_compile_definitions(bl2_crypto_config
    INTERFACE
        TF_PSA_CRYPTO_CONFIG_FILE="${MCUBOOT_PSA_CRYPTO_CONFIG_FILEPATH}"
        # The config files have conditional includes based on these four definitions
        $<${is_rsa_signature}:MCUBOOT_SIGN_RSA>
        $<${is_rsa_signature}:MCUBOOT_SIGN_RSA_LEN=${SIG_LEN}>
        $<${is_ec_signature}:MCUBOOT_SIGN_EC${SIG_LEN}>
        $<$<BOOL:${MCUBOOT_USE_PSA_CRYPTO}>:MCUBOOT_USE_PSA_CRYPTO>
        $<$<BOOL:${MCUBOOT_IMAGE_BINDING}>:MCUBOOT_IMAGE_BINDING>
        $<$<BOOL:${MCUBOOT_ENC_IMAGES}>:MCUBOOT_ENC_IMAGES>
        # ML-DSA-87 is verified in pure mode (must match imgtool --pure)
        $<${is_mldsa87_signature}:MCUBOOT_SIGN_PURE>
)

target_include_directories(bl2_crypto_config
    INTERFACE
        ${CMAKE_SOURCE_DIR}/interface/include
        # Workaround: needed for private headers in drivers/builtin/include/mbedtls/private
        # which are included by some of the TF-PSA-Crypto interface headers.
        ${TF_PSA_CRYPTO_PATH}/drivers/builtin/include
)

# Check if the p256m driver is enabled in the config file, as that will require
# to build some 3rd party specific source code in addition to TF-PSA-Crypto source.
# Note that 0 means SUCCESS here, 1 means FAILURE
set(MBEDTLS_P256M_NOT_FOUND 1)
execute_process(COMMAND
    ${Python3_EXECUTABLE}
    ${TF_PSA_CRYPTO_PATH}/scripts/config.py -f "${MCUBOOT_PSA_CRYPTO_CONFIG_FILEPATH}" get MBEDTLS_PSA_P256M_DRIVER_ENABLED
    RESULT_VARIABLE MBEDTLS_P256M_NOT_FOUND)

# If the enablement is conditional, the script would still mark it as found
if (${MBEDTLS_P256M_NOT_FOUND} EQUAL 0)
    set(MBEDTLS_P256M_ENABLED true)
else()
    set(MBEDTLS_P256M_ENABLED false)
endif()

############################### BL2_CRYPTO #####################################

set(is_384_bit_curve "$<STREQUAL:${SIG_LEN},384>")
set(is_256_bit_curve "$<STREQUAL:${SIG_LEN},256>")
set(build_sha_384    "$<AND:${is_ec_signature},${is_384_bit_curve}>")
set(build_sha_256    "$<NOT:${build_sha_384}>")
set(build_p256m      "$<IF:$<BOOL:${MBEDTLS_P256M_ENABLED}>,$<AND:${is_ec_signature},${is_256_bit_curve}>,0>")

# Reject unsupported ML-DSA variants and spellings to prevent BL2 from
# silently building without ML-DSA support. Only "ML-DSA-87" is recognized.
if(MCUBOOT_SIGNATURE_TYPE MATCHES "^ML-?DSA"
   AND NOT MCUBOOT_SIGNATURE_TYPE STREQUAL "ML-DSA-87")
    message(FATAL_ERROR
        "Unsupported MCUBOOT_SIGNATURE_TYPE: "
        "${MCUBOOT_SIGNATURE_TYPE}. Only ML-DSA-87 is supported "
        "for ML-DSA signatures."
    )
endif()

# ML-DSA-87: BL2 doesn't use the TF-PSA-Crypto PQCP CMake target, which belongs
# to the SPE Crypto partition and is built with the SPE crypto config. Build the
# same driver sources here with bl2_crypto_config instead. They provide
# tf_psa_crypto_mldsa_verify_message(), called by psa_verify_message() in
# thin_psa_crypto_core.c.
# As in the PQCP target, only the top-level src/*.c wrappers are compiled; the
# mldsa-native implementation files are pulled in by those wrappers through
# #include and must not be compiled separately.
set(BL2_PQCP_SRC)
if(MCUBOOT_SIGNATURE_TYPE STREQUAL "ML-DSA-87")
    file(GLOB BL2_PQCP_SRC CONFIGURE_DEPENDS
        "${TF_PSA_CRYPTO_PATH}/drivers/pqcp/src/*.c"
    )
    if(NOT BL2_PQCP_SRC)
        message(FATAL_ERROR "ML-DSA-87 requires the PQCP driver, but no sources were "
                            "found in ${TF_PSA_CRYPTO_PATH}/drivers/pqcp/src")
    endif()
endif()

list(APPEND BL2_CRYPTO_SRC
    $<$<OR:$<BOOL:${MCUBOOT_USE_PSA_CRYPTO}>,$<BOOL:${MCUBOOT_USE_MBED_TLS}>>:${CMAKE_SOURCE_DIR}/lib/ext/thin-psa-crypto-core/thin_psa_crypto_core.c>
    ${BL2_PQCP_SRC}
    ${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/aes.c
    ${TF_PSA_CRYPTO_PATH}/platform/platform.c
    ${TF_PSA_CRYPTO_PATH}/platform/platform_util.c
    ${TF_PSA_CRYPTO_PATH}/platform/memory_buffer_alloc.c
    ${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/psa_crypto_hash.c
    $<${build_sha_256}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/sha256.c>
    $<${build_sha_384}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/sha512.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/psa_crypto_ecp.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/core/psa_util.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/ecp.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/ecp_curves.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/ecdsa.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/bignum.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/bignum_core.c>
    $<$<AND:${is_ec_signature},$<NOT:${build_p256m}>>:${TF_PSA_CRYPTO_PATH}/utilities/constant_time.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/psa_crypto_rsa.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/core/psa_util.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/rsa.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/rsa_alt_helpers.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/bignum.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/bignum_core.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/psa_util_internal.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/utilities/constant_time.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/utilities/asn1parse.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/utilities/asn1write.c>
    $<${is_rsa_signature}:${TF_PSA_CRYPTO_PATH}/extras/md.c>
    $<$<AND:${is_ec_signature},${build_p256m}>:${TF_PSA_CRYPTO_PATH}/drivers/p256-m/p256-m_driver_entrypoints.c>
    $<$<AND:${is_ec_signature},${build_p256m}>:${TF_PSA_CRYPTO_PATH}/drivers/p256-m/p256-m/p256-m.c>
    $<$<BOOL:${MCUBOOT_ENCRYPT_KW}>:${TF_PSA_CRYPTO_PATH}/drivers/builtin/src/nist_kw.c>
    $<$<BOOL:${MCUBOOT_ENCRYPT_KW}>:${TF_PSA_CRYPTO_PATH}/utilities/constant_time.c>
    # Include __weak stub implementation, serving as a fallback for random number generation in case no other RNG is provided.
    $<$<NOT:$<BOOL:${CRYPTO_HW_ACCELERATOR}>>:src/psa_stub_rng.c>
)

add_library(bl2_crypto STATIC ${BL2_CRYPTO_SRC})

target_compile_definitions(bl2_crypto
    PRIVATE
        $<$<BOOL:${MCUBOOT_ROTPK_SIGN_POLICY}>:MCUBOOT_ROTPK_SIGN_POLICY>
        MAX_KEYS_PER_IMAGE=${MCUBOOT_ROTPK_MAX_KEYS_PER_IMAGE}
)

if(MCUBOOT_SIGNATURE_TYPE STREQUAL "ML-DSA-87")
    target_include_directories(bl2_crypto
        PUBLIC
            # PQCP driver headers: when ML-DSA is enabled, crypto_struct.h
            # includes tf-psa-crypto/private/crypto_struct_pqcp.h. psa/crypto.h
            # is used by BL2 and bootutil, so this path is propagated to every
            # target linking bl2_crypto. TF-PSA-Crypto adds the same path in
            # drivers/CMakeLists.txt, which BL2 doesn't use.
            ${TF_PSA_CRYPTO_PATH}/drivers/pqcp/include
        PRIVATE
            # Same private include directories as drivers/pqcp/CMakeLists.txt;
            # only needed to compile BL2_PQCP_SRC.
            ${TF_PSA_CRYPTO_PATH}/drivers/pqcp/src
            ${TF_PSA_CRYPTO_PATH}/drivers/pqcp/mldsa-native/mldsa
    )
endif()

target_include_directories(bl2_crypto
    PUBLIC
        ${TF_PSA_CRYPTO_PATH}/core
        ${TF_PSA_CRYPTO_PATH}/include
        ${TF_PSA_CRYPTO_PATH}/dispatch
        ${TF_PSA_CRYPTO_PATH}/utilities
        ${TF_PSA_CRYPTO_PATH}/drivers/builtin/include
        ${TF_PSA_CRYPTO_PATH}/drivers/builtin/src
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/mcuboot/include
)

target_compile_options(bl2_crypto
    PRIVATE
        ${BL2_COMPILER_CP_FLAG}
)

target_link_libraries(bl2_crypto
    PUBLIC
        bootutil
        bl2_crypto_config
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
# IAR only allows 1 optimization option for the compilation unit
if (${BUILD_TYPE_LOWER} STREQUAL "debug" AND NOT ${CMAKE_C_COMPILER_ID} STREQUAL "IAR")
    set_source_files_properties(${BL2_CRYPTO_SRC}
        PROPERTIES COMPILE_FLAGS -Os
    )
endif()

############################### BL2 ############################################

add_executable(bl2
    src/flash_map.c
    $<$<C_COMPILER_ID:GNU,Clang>:
        ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/crt_start.c
        ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/crt_memcmp.c
        ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/crt_memmove.c
        ${CMAKE_SOURCE_DIR}/secure_fw/shared/crt_memset.c
        ${CMAKE_SOURCE_DIR}/secure_fw/shared/crt_memcpy.c
    >
    $<$<BOOL:${DEFAULT_MCUBOOT_SECURITY_COUNTERS}>:src/security_cnt.c>
    $<$<BOOL:${DEFAULT_MCUBOOT_FLASH_MAP}>:src/default_flash_map.c>
    $<$<BOOL:${MCUBOOT_DATA_SHARING}>:src/shared_data.c>
    $<$<BOOL:${PLATFORM_DEFAULT_PROVISIONING}>:src/provisioning.c>
    $<$<C_COMPILER_ID:GNU>:${CMAKE_SOURCE_DIR}/platform/ext/common/syscalls_stub.c>
)

if (${CONFIG_TFM_INCLUDE_STDLIBC})
    target_sources(bl2 PRIVATE src/crt_exit.c)
else()
    target_sources(bl2 PRIVATE src/mbedcrypto_stubs.c)
endif()

set_target_properties(bl2
    PROPERTIES
        SUFFIX ".axf"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ADDITIONAL_CLEAN_FILES "${CMAKE_BINARY_DIR}/generated;${CMAKE_BINARY_DIR}/bin/bl2.map"
)

add_subdirectory(ext/mcuboot)

target_include_directories(bl2
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${MCUBOOT_PATH}/boot/bootutil/src>
)

target_link_libraries(bl2
    PRIVATE
        tfm_boot_status
        $<$<BOOL:${TEST_BL2}>:mcuboot_tests>
    PUBLIC
        bl2_crypto
)

target_compile_options(bl2
    PRIVATE
        ${BL2_COMPILER_CP_FLAG}
)

target_link_options(bl2
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:LINKER:-Map=${CMAKE_BINARY_DIR}/bin/bl2.map>
        $<$<C_COMPILER_ID:ARMClang>:--map>
        $<$<C_COMPILER_ID:IAR>:--map\;${CMAKE_BINARY_DIR}/bin/bl2.map>
        ${BL2_LINKER_CP_OPTION}
)

target_compile_definitions(bl2
    PRIVATE
        $<$<BOOL:${DEFAULT_MCUBOOT_FLASH_MAP}>:DEFAULT_MCUBOOT_FLASH_MAP>
        $<$<BOOL:${PLATFORM_PSA_ADAC_SECURE_DEBUG}>:PLATFORM_PSA_ADAC_SECURE_DEBUG>
        $<$<BOOL:${PSA_ADAC_AS_TFM_RUNTIME_SERVICE}>:PSA_ADAC_AS_TFM_RUNTIME_SERVICE>
        $<$<BOOL:${TEST_BL2}>:TEST_BL2>
        $<$<BOOL:${TFM_PARTITION_FIRMWARE_UPDATE}>:TFM_PARTITION_FIRMWARE_UPDATE>
        $<$<AND:$<BOOL:${CONFIG_TFM_BOOT_STORE_MEASUREMENTS}>,$<NOT:$<BOOL:${CONFIG_TFM_BOOT_STORE_ENCODED_MEASUREMENTS}>>>:TFM_MEASURED_BOOT_API>
        $<$<BOOL:${MCUBOOT_BUILTIN_KEY}>:MCUBOOT_BUILTIN_KEY>
        $<$<BOOL:${MCUBOOT_ENC_IMAGES}>:MCUBOOT_ENC_IMAGES>
)

target_link_libraries(bl2 PRIVATE bl2_build_flags)

add_convert_to_bin_target(bl2)

############################### BOOT HAL # #####################################

add_library(bl2_hal INTERFACE)

target_include_directories(bl2_hal
    INTERFACE
        include
)

############################### CODE SHARING ###################################

if (TFM_CODE_SHARING)
    target_share_symbols(bl2 ${CMAKE_CURRENT_SOURCE_DIR}/bl2_shared_symbols.txt)

    if (NOT EXISTS ${TF_PSA_CRYPTO_PATH}/drivers/code_share.c)
        message(FATAL_ERROR "File ${TF_PSA_CRYPTO_PATH}/drivers/code_share.c does not exist.
        Have the patch ${CMAKE_SOURCE_DIR}/lib/ext/tf-psa-crypto/0002-Enable-crypto-code-sharing-between-independent-binar.patch
        been applied to the tf-psa-crypto repo at ${TF_PSA_CRYPTO_PATH}?
        Hint: The command might be `cd ${TF_PSA_CRYPTO_PATH} && git apply ${CMAKE_SOURCE_DIR}/lib/ext/tf-psa-crypto/*.patch`")
    endif()
endif()
