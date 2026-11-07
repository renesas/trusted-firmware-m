#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# BL2 (MCUboot) target configuration for the Renesas RA platforms.
#
# Included from common.cmake when BL2 is ON, after the FSP library
# (fsp_library, FSP_LIBS_CORE_PATH) and the generic platform_bl2 / bl2 targets
# exist. It adds the platform sources, the FSP integration and the linker script
# to those targets.

# BL2 is an FSP flat project: it runs only in the secure state, has no
# non-secure world and no CMSE entry functions, so it is built without
# _RA_TZ_SECURE and without the CMSE compiler flag. In a flat build the FSP
# start-up skips R_BSP_SecurityInit (no SAR / SAU partitioning) and only puts
# the peripherals (PSARB..E) and the port pins (PMSAR) in secure mode, which is
# all BL2 needs. The SAU is left disabled with ALLNS set (core startup.c), so
# the IDAU alone attributes addresses by alias. The TF-M secure image does the
# full security init with its real configuration after BL2 jumps to it.

# fsp_library_config holds the FSP memory map macros (fsp_library_config
# applies them to the FSP sources only), fsp_library the build selectors. Drop
# the secure-build selector so the FSP sources compiled into platform_bl2 are a
# flat build.
get_target_property(_ra_fsp_defs fsp_library_config INTERFACE_COMPILE_DEFINITIONS)
get_target_property(_ra_fsp_build_defs fsp_library INTERFACE_COMPILE_DEFINITIONS)
set(RA_BL2_FSP_DEFINITIONS ${_ra_fsp_defs} ${_ra_fsp_build_defs})
list(REMOVE_ITEM RA_BL2_FSP_DEFINITIONS _RA_TZ_SECURE)
list(REMOVE_DUPLICATES RA_BL2_FSP_DEFINITIONS)

# .ram_from_flash holds MRAM programming code that must run from SRAM, so
# the RAM load segment is intentionally executable.
target_link_options(platform_bl2
    PUBLIC
    ${BL2_LINKER_CP_OPTION}
    $<$<C_COMPILER_ID:GNU>:-Wl,--no-warn-rwx-segments>
)

target_sources(platform_bl2
    PRIVATE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/CMSIS_Driver/Driver_USART.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/CMSIS_Driver/Driver_Flash.c
)

# The FSP sources are added to platform_bl2 only (not by linking
# fsp_library, which would also apply the secure-build selector and the CMSE
# flag and pull every FSP source into the signing_layout_* object
# libraries that link platform_bl2). hal_warmstart.c is excluded: it is linked
# directly into bl2 below.
get_target_property(_ra_fsp_sources fsp_library SOURCES)
list(FILTER _ra_fsp_sources EXCLUDE REGEX "/hal_warmstart\\.c$")
get_target_property(_ra_fsp_includes fsp_library INCLUDE_DIRECTORIES)
target_sources(platform_bl2 PRIVATE ${_ra_fsp_sources})
target_include_directories(platform_bl2 PUBLIC ${_ra_fsp_includes})
target_compile_definitions(platform_bl2 PUBLIC ${RA_BL2_FSP_DEFINITIONS})
target_link_libraries(platform_bl2 PUBLIC cmsis)

# hal_warmstart.c holds the strong R_BSP_WarmStart that configures the pins
# (UART TX/RX) after the clocks are up. FSP system.c has a weak empty
# R_BSP_WarmStart, so if this object only sat inside the platform_bl2
# static library the linker would never extract it and BL2 would run with
# unconfigured pins (no UART log). Link it directly into the bl2 image.
target_sources(bl2
    PRIVATE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Device/Source/startup.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/hal_warmstart.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/bl2/flash_map_bl2.c
)

# startup.c is the only BL2 source built with the CMSE flag: it programs the
# SAU, which CMSIS declares only for CMSE builds (__ARM_FEATURE_CMSE == 3). It
# defines no cmse_nonsecure_entry function, so no CMSE veneer is generated.
# Not for the rest of BL2: with __ARM_FEATURE_CMSE == 3, BSP_CMSE_NONSECURE_ENTRY
# turns FSP functions into entry functions. startup.c tells BL2 (program the
# SAU, no stack sealing) from the TF-M secure image by CONFIG_TFM_USE_TRUSTZONE,
# defined for the secure image only. TARGET_DIRECTORY: bl2 is created in
# another directory, and tfm_s (which also builds startup.c) is not affected.
set_source_files_properties(${RENESAS_RA_COMMON_SOURCE_DIR}/core/Device/Source/startup.c
    TARGET_DIRECTORY bl2
    PROPERTIES COMPILE_OPTIONS "${COMPILER_CMSE_FLAG}"
)

# Signing layout of the CPU1 NS image (MCUboot image 2, flash area 4). The
# generic TF-M layouts only cover the S and CPU0 NS images. The layout is
# installed next to them for the CPU1 NS build, which signs its image with it
# (core/ns/common.cmake). The _ns.o suffix makes the signing wrapper record the
# image as non-secure for measured boot.
if(MCUBOOT_IMAGE_NUMBER EQUAL 3)
    set(IMAGE_TYPE "CPU1_NS_IMAGE")
    set(FLASH_AREA_NUM 4)
    configure_file(${CMAKE_SOURCE_DIR}/bl2/ext/mcuboot/signing_layout.c.in
                   ${CMAKE_CURRENT_BINARY_DIR}/signing_layout_cpu1_ns.c @ONLY)

    add_library(signing_layout_cpu1_ns OBJECT ${CMAKE_CURRENT_BINARY_DIR}/signing_layout_cpu1_ns.c)
    target_compile_options(signing_layout_cpu1_ns
        PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang>:-E\;-xc>
            $<$<C_COMPILER_ID:ARMClang>:-E\;-xc>
            $<$<C_COMPILER_ID:IAR>:--preprocess=ns\;$<TARGET_OBJECTS:signing_layout_cpu1_ns>>
    )
    target_compile_definitions(signing_layout_cpu1_ns
        PRIVATE
            BL2
            MCUBOOT_IMAGE_NUMBER=${MCUBOOT_IMAGE_NUMBER}
    )
    target_link_libraries(signing_layout_cpu1_ns
        PRIVATE
            platform_bl2
    )

    if(PLATFORM_DEFAULT_IMAGE_SIGNING)
        add_custom_target(signed_images_cpu1_ns ALL DEPENDS signing_layout_cpu1_ns)
        install(FILES $<TARGET_OBJECTS:signing_layout_cpu1_ns>
                DESTINATION ${INSTALL_IMAGE_SIGNING_DIR}/layout_files
                RENAME signing_layout_cpu1_ns.o)
    endif()
endif()

target_add_scatter_file(bl2
    $<$<C_COMPILER_ID:ARMClang>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/armclang/tfm_common_bl2.sct>
    $<$<C_COMPILER_ID:GNU>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/gcc/tfm_common_bl2.ld>
    $<$<C_COMPILER_ID:Clang>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/atfe/tfm_common_bl2.ld>
)
