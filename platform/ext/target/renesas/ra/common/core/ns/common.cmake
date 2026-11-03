#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Shared RA platform_ns build logic, included from each board's exported
# ns/CMakeLists.txt. The board file must set FSP_SOC_TARGET (e.g. ra8p1) and
# FSP_BOARD_TARGET (e.g. ra8p1_ek) before including this file; everything
# else resolves through TARGET_PLATFORM_PATH (the board dir) and
# RENESAS_RA_COMMON_SOURCE_DIR (ra/common, set by cpuarch.cmake/config.cmake).

add_library(platform_ns)

target_compile_definitions(platform_ns
    PRIVATE
        DOMAIN_NS=1
)

target_sources(platform_ns
    PRIVATE
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Device/Source/startup.c
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/CMSIS_Driver/Driver_USART.c
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/ns/tfm_platform_ns_init.c
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_ioctl_ns_api.c
)

target_include_directories(platform_ns
    PUBLIC
        ${TARGET_PLATFORM_PATH}
        ${TARGET_PLATFORM_PATH}/CMSIS_Driver/Config
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/cmsis/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/cmsis/Include/m-profile
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/common
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Native_Driver
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/inc
        ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/partition
)

# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/partition
)

# Include NS FSP library (reuses S-side FSP clone, includes the s_ns_*.cmake settings
# and the SoC's fsp_post_config.cmake, whose fsp_library_config defs apply only to
# the fsp_library sources)
include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/fsp/fsp_ns_lib.cmake)

# Link platform_ns against platform_region_defs to get memory region defines
target_link_libraries(platform_ns
    PUBLIC
        platform_region_defs
        fsp_library
    INTERFACE
        # FSP objects go straight onto the NS executable's link line
        $<TARGET_OBJECTS:fsp_library>
)

target_compile_definitions(platform_ns
    PUBLIC
        $<$<BOOL:${PLATFORM_DEFAULT_CRYPTO_KEYS}>:PLATFORM_DEFAULT_CRYPTO_KEYS>
)

#===================== NS image signing (BL2) ===============================#

# The S and NS images are separate MCUboot images and are never merged: the NS
# image can be in another flash (RA_CPU0_NS_EXTERNAL / RA_CPU1_NS_EXTERNAL) and
# is programmed on its own. TF-M's default signing in the installed spe
# CMakeLists.txt (PLATFORM_DEFAULT_IMAGE_SIGNING) assembles a combined
# tfm_s_ns_signed image with the CPU0 NS offset, and has no option to skip only
# that step, so it is turned off (spe_config sets it in the spe directory scope,
# which is read after this directory) and the NS image is signed here with the
# layout and keys exported by the secure build. The S image is signed by the
# secure build.
#
# CPU0 (TrustZone) is MCUboot image 1. CPU1 (mailbox) is image 2: it is signed
# with the S key (the default provisioning stores the S key hash in
# BL2_ROTPK_2) and its own layout, installed by the secure build. The _ns.o
# suffix of the layouts makes the signing wrapper record the image as non-secure
# for measured boot.
if(BL2 AND MCUBOOT_IMAGE_NUMBER GREATER 1)
    set(PLATFORM_DEFAULT_IMAGE_SIGNING OFF PARENT_SCOPE)

    set(_ra_signing_dir ${CMAKE_CURRENT_SOURCE_DIR}/../image_signing)

    if(RA_NS_INTERFACE_MAILBOX)
        set(_ra_ns_layout       ${_ra_signing_dir}/layout_files/signing_layout_cpu1_ns.o)
        set(_ra_ns_key          ${MCUBOOT_KEY_S})
        set(_ra_ns_version      ${MCUBOOT_IMAGE_VERSION_CPU1_NS})
        set(_ra_ns_counter      ${MCUBOOT_SECURITY_COUNTER_CPU1_NS})
        set(_ra_ns_extra_args)
    else()
        set(_ra_ns_layout       ${_ra_signing_dir}/layout_files/signing_layout_ns.o)
        set(_ra_ns_key          ${MCUBOOT_KEY_NS})
        set(_ra_ns_version      ${MCUBOOT_IMAGE_VERSION_NS})
        set(_ra_ns_counter      ${MCUBOOT_SECURITY_COUNTER_NS})
        # The NS image depends on the S image (minimum version)
        set(_ra_ns_extra_args
            -d \"\(0, ${MCUBOOT_S_IMAGE_MIN_VER}\)\"
            $<$<BOOL:${MCUBOOT_ENC_IMAGES}>:-E${MCUBOOT_KEY_ENC_NS}>
        )
        if(MCUBOOT_IMAGE_BINDING)
            list(APPEND _ra_ns_extra_args
                --extra-tlv "${TLV_METADATA}"
                --extra-tlv "${TLV_BINDING_TAG}"
            )
        endif()
        if(MCUBOOT_BUILTIN_KEY)
            list(APPEND _ra_ns_extra_args --psa-key-ids ${TFM_NS_KEY_ID})
        endif()
    endif()

    set(_ra_ns_wrapper_args
        --version ${_ra_ns_version}
        --layout ${_ra_ns_layout}
        --key ${_ra_ns_key}
        --public-key-format $<IF:$<BOOL:${MCUBOOT_HW_KEY}>,full,hash>
        --align ${MCUBOOT_ALIGN_VAL}
        --pad
        --pad-header
        -H ${BL2_HEADER_SIZE}
        -s ${_ra_ns_counter}
        -L ${MCUBOOT_ENC_KEY_LEN}
        ${_ra_ns_extra_args}
        $<$<STREQUAL:${MCUBOOT_UPGRADE_STRATEGY},OVERWRITE_ONLY>:--overwrite-only>
        $<$<BOOL:${MCUBOOT_CONFIRM_IMAGE}>:--confirm>
        $<$<BOOL:${MCUBOOT_MEASURED_BOOT}>:--measured-boot-record>
        $<TARGET_FILE_DIR:${NS_TARGET_NAME}>/${NS_TARGET_NAME}.bin
        ${CMAKE_BINARY_DIR}/bin/${NS_TARGET_NAME}_signed.bin
    )

    add_custom_target(${NS_TARGET_NAME}_signed_bin ALL
        SOURCES ${CMAKE_BINARY_DIR}/bin/${NS_TARGET_NAME}_signed.bin
    )
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/${NS_TARGET_NAME}_signed.bin
        DEPENDS ${NS_TARGET_NAME}_bin
        DEPENDS ${_ra_ns_layout}
        WORKING_DIRECTORY ${_ra_signing_dir}/scripts
        COMMAND mcuboot_imagesign_wrapper ${_ra_ns_wrapper_args}
    )

    add_convert_to_offset_hex_target(${NS_TARGET_NAME}_signed
        INPUT_FILE          ${CMAKE_BINARY_DIR}/bin/${NS_TARGET_NAME}_signed.bin
        OFFSET_MACRO_NAME   NS_CODE_START
        INCLUDE_BL2_HEADER  TRUE
        MIRROR_FILE         $<TARGET_PROPERTY:${NS_TARGET_NAME},LINK_DEPENDS>
        OUTPUT_FILE         ${CMAKE_BINARY_DIR}/bin/${NS_TARGET_NAME}_signed.hex
    )
endif()
