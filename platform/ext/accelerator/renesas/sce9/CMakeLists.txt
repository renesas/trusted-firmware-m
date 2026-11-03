#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Renesas SCE9 crypto accelerator - CRYPTO_HW_ACCELERATOR_TYPE=renesas/sce9.
#
# SCE9 parts: RA6M4, RA6M5, RA6E1, RA4M2/3 ... RSIP-E50D parts use the sibling
# renesas/rsip_e50d; its CMakeLists.txt header gives the reason for two directories (D050).
#
# Model: the legacy *_ALT interface (TF-M's LEGACY_DRIVER_API_ENABLED, as CC312 uses it).
# FSP's rm_psa_crypto ships SCE9 forks of Mbed TLS library modules. The core set is compiled
# here into TF-M's own Mbed TLS, from the secure e2 project. Mbed TLS 3.6 only; see
# mbedtls_accelerator_config.h.
#
# The secure crypto partition gets the full set; BL2 gets SHA-256 for its image hash.
#-------------------------------------------------------------------------------

if(NOT FSP_S_APP_DIR)
    message(FATAL_ERROR
        "renesas/sce9: FSP_S_APP_DIR is not set. The SCE9 ALT sources are taken from the "
        "secure e2 project's rm_psa_crypto module.")
endif()

set(SCE9_RM_PSA_CRYPTO_DIR "${FSP_S_APP_DIR}/ra/fsp/src/rm_psa_crypto")
if(NOT EXISTS "${SCE9_RM_PSA_CRYPTO_DIR}")
    message(FATAL_ERROR
        "renesas/sce9: ${SCE9_RM_PSA_CRYPTO_DIR} not found.\n"
        "The accelerator compiles FSP's SCE9 *_ALT sources from the secure project. Add the "
        "Crypto stack (rm_psa_crypto) to it in e2 studio, regenerate, and build once - or "
        "configure with -DCRYPTO_HW_ACCELERATOR=OFF to keep ciphers in software.")
endif()

############################ BL2 - SHA-256 #####################################

if(BL2)
    # SHA-256 only; bl2_mbedtls_accelerator_config.h explains why the P-256 verify stays on
    # p256-m. The platform's boot_platform_post_init() brings the engine up before MCUboot
    # hashes the first image; boot_hal_bl2.c's crypto_hw_accelerator_init() resolves to the
    # empty stub in bl2_main.c.
    #
    # CRYPTO_HW_ACCELERATOR must be set on bl2_crypto_config itself, not only inherited from
    # tfm_config: mcuboot-mbedtls-cfg.h includes the accelerator config only under it, and
    # bl2_crypto does not link tfm_config. Without it, the library builds software sha256.c
    # with the native context while bootutil, which sees the define, uses FSP's
    # mbedtls_sha256_context. That is a struct-layout mismatch across translation units, not
    # only a lost speed-up.
    target_compile_definitions(bl2_crypto_config
        INTERFACE
            CRYPTO_HW_ACCELERATOR
            MBEDTLS_ACCELERATOR_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/bl2_mbedtls_accelerator_config.h"
            MBEDTLS_ACCELERATOR_PSA_CRYPTO_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/bl2_crypto_accelerator_config.h"
    )

    add_library(bl2_crypto_hw STATIC)

    target_sources(bl2_crypto_hw
        PRIVATE
            ${SCE9_RM_PSA_CRYPTO_DIR}/sha256_alt.c
            ${SCE9_RM_PSA_CRYPTO_DIR}/sha256_alt_process.c
    )

    # bl2_crypto brings the BL2 Mbed TLS config and library/ (PUBLIC there); fsp_sce_bl2
    # brings HW_SCE_* and the FSP include paths for the bootloader role (flat, no TZ defines).
    target_link_libraries(bl2_crypto_hw
        PRIVATE
            bl2_crypto
            fsp_sce_bl2
    )

    target_compile_options(bl2_crypto_hw
        PRIVATE
            ${BL2_COMPILER_CP_FLAG}
    )

    target_link_libraries(bl2_crypto
        PUBLIC
            bl2_crypto_hw
    )

    # Every file compiled against the BL2 Mbed TLS config now includes sce9_fsp_cfg.h
    # (bsp_api.h) and, through mbedtls/sha256.h, FSP's sha256_alt.h, which includes the
    # internal "common.h". It therefore needs these paths and the FSP defines.
    # ${MBEDCRYPTO_PATH}/include supplies "psa/crypto_accel_driver.h", which FSP's
    # psa_crypto_driver_wrappers.h includes and which exists only in the crypto library's own
    # include tree. TF-M's interface/include is added before it, so BL2 still takes its
    # psa/*.h from there.
    target_include_directories(bl2_crypto_config
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${SCE9_RM_PSA_CRYPTO_DIR}/inc
            ${MBEDCRYPTO_PATH}/library
            ${MBEDCRYPTO_PATH}/include
            $<TARGET_PROPERTY:fsp_sce_bl2,INTERFACE_INCLUDE_DIRECTORIES>
    )
    target_compile_definitions(bl2_crypto_config
        INTERFACE
            $<TARGET_PROPERTY:fsp_sce_bl2,INTERFACE_COMPILE_DEFINITIONS>
    )
endif()

############################ Secure crypto partition ###########################

if(TFM_PARTITION_CRYPTO)
    # FSP's SCE9 set: the generic cipher layer, AES, GCM, CMAC, SHA-256, ECC and RSA. Each
    # *_alt.c replaces the matching Mbed TLS library file; each *_process.c holds its SCE9
    # calls. Adding a module means adding its pair here and its MBEDTLS_*_ALT in
    # mbedtls_accelerator_config.h.
    #
    # ccm_alt.c and ccm_alt_process.c are deliberately absent: FSP's SCE9 CCM caps associated
    # data at 110 B, and Protected Storage exceeds it (D043, mbedtls_accelerator_config.h).
    #
    # cipher_alt.c is required with the rest. It supplies the block chunking the AES ALT
    # assumes (whole blocks only, partial blocks cached in the cipher context) and issues the
    # engine session FinalSub at finish. Without it, upstream cipher.c hands partial blocks
    # straight to the engine and never closes the session.
    set(SCE9_ALT_SOURCES
        cipher_alt.c
        aes_alt.c           aes_alt_process.c
        gcm_alt.c           gcm_alt_process.c
        cmac_alt.c
        sha256_alt.c        sha256_alt_process.c
        ecp_alt.c           ecp_alt_process.c
        ecp_curves_alt.c
        ecdsa_alt.c         ecdsa_alt_process.c
        rsa_alt.c           rsa_alt_process.c
    )
    list(TRANSFORM SCE9_ALT_SOURCES PREPEND "${SCE9_RM_PSA_CRYPTO_DIR}/")

    target_sources(crypto_service_crypto_hw
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/crypto_hw.c
            ${SCE9_ALT_SOURCES}
    )

    target_include_directories(crypto_service_crypto_hw
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${SCE9_RM_PSA_CRYPTO_DIR}/inc
        PRIVATE
            # The ALT sources are library files: they include Mbed TLS internals by bare
            # name ("common.h", "bn_mul.h", "bignum_internal.h", ...).
            ${MBEDCRYPTO_PATH}/library
    )

    # r_sce provides the HW_SCE_* procedures; platform_s provides ra_sce_init().
    target_link_libraries(crypto_service_crypto_hw
        PRIVATE
            crypto_service_mbedcrypto
            platform_s
            fsp_sce_s
    )

    target_link_libraries(crypto_service_mbedcrypto
        PUBLIC
            crypto_service_crypto_hw
    )

    # LEGACY_DRIVER_API_ENABLED makes crypto_init.c call crypto_hw_accelerator_init() before
    # psa_crypto_init().
    target_compile_definitions(crypto_service_mbedcrypto
        PUBLIC
            LEGACY_DRIVER_API_ENABLED
    )

    # Every file compiled against the Mbed TLS config now sees mbedtls_accelerator_config.h,
    # which includes bsp_api.h and makes mbedtls/*.h pull in the FSP *_alt.h context
    # definitions. It therefore needs this directory, rm_psa_crypto/inc and the FSP/SCE include
    # paths and defines. They are copied from fsp_sce_s rather than linked, so the FSP library
    # is not pulled into every consumer of the config.
    #
    # BUILD_INTERFACE: both config targets are exported to the installed api_ns tree, and
    # CMake refuses source-tree paths in an exported interface.
    target_include_directories(psa_crypto_library_config
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${SCE9_RM_PSA_CRYPTO_DIR}/inc>
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:fsp_sce_s,INTERFACE_INCLUDE_DIRECTORIES>>
            # FSP's *_alt.h headers include Mbed TLS's internal "common.h" (sha256_alt.h
            # does), and mbedtls/<module>.h includes the *_alt.h once the ALT is defined -
            # so every consumer of the public headers needs the library directory too.
            $<BUILD_INTERFACE:${MBEDCRYPTO_PATH}/library>
    )
    target_compile_definitions(psa_crypto_library_config
        INTERFACE
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:fsp_sce_s,INTERFACE_COMPILE_DEFINITIONS>>
    )

    target_include_directories(psa_crypto_config
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
endif()
