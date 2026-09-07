#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

############################ FSP RASC Project Paths ############################
#
# These variables allow using RASC-generated FSP projects for BL2, Secure, and
# Non-Secure components. When set, the corresponding FSP project's TF-M
# integration layer will be used instead of the default TF-M implementation.
#
# Each FSP project must have a tfm_integration/CMakeLists_tfm.cmake file.
#
# Example CMake configuration:
#   -DFSP_BL2_APP_DIR=C:/path/to/FSP_Project_ra6m4_bl2
#   -DFSP_S_APP_DIR=C:/path/to/FSP_Project_ra6m4_s_rtos
#   -DFSP_NS_APP_DIR=C:/path/to/FSP_Project_ra6m4_ns_rtos

set(FSP_BL2_APP_DIR "" CACHE PATH "Path to RASC-generated FSP BL2 (MCUboot) project")
set(FSP_S_APP_DIR "" CACHE PATH "Path to RASC-generated FSP Secure project")
set(FSP_NS_APP_DIR "" CACHE PATH "Path to RASC-generated FSP Non-Secure application")

############################ FSP Integration ###################################

# Include FSP modular libraries (self-contained in platform directory)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/fsp_bsp.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/fsp_uart.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/fsp_flash.cmake)

############################ Platform Configuration ############################

# Platform-specific compile definitions
target_compile_definitions(platform_s
    PRIVATE
        TFM_PLATFORM_RA6M4=1
        _RA_ORDINAL=1
        _RA_TZ_SECURE=1
)

# Enable CMSE for TrustZone
target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

# Link FSP modules to platform_s
target_link_libraries(platform_s
    PUBLIC
        fsp_bsp
        fsp_uart
        fsp_flash
)

############################ Platform Sources ##################################

# Platform-specific TF-M HAL implementations
target_sources(platform_s
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_peripherals_def.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6m4.c
        ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c
        # Use common ARMv8-M isolation implementation
        ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${PLATFORM_DIR}/ext/common/tfm_interrupts.c>
        # SEGGER RTT stdout backend (replaces common uart_stdout.c when RA6M4_STDOUT_RTT=ON)
        $<$<BOOL:${RA6M4_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c>
        $<$<BOOL:${RA6M4_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c>
)

# Platform include directories
target_include_directories(platform_s
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    PRIVATE
        ${PLATFORM_DIR}/ext/common
        $<$<BOOL:${RA6M4_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt>
        # attest_hal.c uses tfm_strnlen() (declared here, defined in tfm_sprt runtime lib)
        ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/include
)

# Add platform directory to region_defs target so linker scripts can find region_defs.h
target_include_directories(platform_region_defs
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

############################ BL2 Configuration #################################

if(BL2)
    if(FSP_BL2_APP_DIR)
        # Use external FSP BL2 project with OFS regions and RASC-generated configuration
        message(STATUS "Using external FSP BL2 application: ${FSP_BL2_APP_DIR}")

        if(NOT EXISTS "${FSP_BL2_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake")
            message(FATAL_ERROR
                "TF-M integration file not found in FSP BL2 application.\n"
                "Expected: ${FSP_BL2_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake\n"
                "Make sure the FSP project has TF-M integration files."
            )
        endif()

        # Include the FSP BL2 project's TF-M integration layer
        include(${FSP_BL2_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake)

        # BL2 uses FSP startup with OFS regions
        target_sources(bl2
            PRIVATE
                # FSP startup provides Reset_Handler, vector table, and OFS data
                ${FSP_BL2_APP_DIR}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c
                ${FSP_BL2_APP_DIR}/ra/fsp/src/bsp/mcu/ra6m4/bsp_linker.c
        )

        # Use FSP linker script (includes OFS memory regions)
        target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${FSP_BL2_APP_DIR}/script/fsp.ld>
            $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
            $<$<C_COMPILER_ID:Clang>:${PLATFORM_DIR}/ext/common/llvm/tfm_common_bl2.ld>
        )

        # Add FSP BL2 project directory for linker script includes (memory_regions.ld, etc.)
        target_link_options(bl2
            PRIVATE
                -L${FSP_BL2_APP_DIR}
        )

        # Platform library for BL2 uses FSP BL2 BSP
        target_sources(platform_bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
                ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c
        )

        target_link_libraries(platform_bl2
            PUBLIC
                fsp_bsp_bl2
                fsp_flash_bl2
        )

        target_include_directories(platform_bl2
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
                ${FSP_BL2_INCLUDE_DIRS}
            PRIVATE
                ${PLATFORM_DIR}/ext/common
        )
    else()
        # Use default TF-M BL2 with platform FSP BSP.
        # bl2_option_setting.c (OFS) is compiled straight into the bl2 executable,
        # NOT a static lib: nothing references the OFS symbols, so from an archive
        # the linker would never pull the object in (KEEP only helps once linked).
        #
        # SAFETY: this is only sound because ra6m4_bl2.ld gives each option-setting
        # group its OWN MEMORY region, so ld emits one PT_LOAD per group and cannot
        # zero-fill the FCU config bytes (incl. FSPR) in the gaps. Emitting these
        # sections without per-group regions permanently bricks the part - it cost
        # two EK-RA6M4 boards on 2026-07-21. If you touch the OFS placement, verify
        # with `arm-none-eabi-readelf -l bin/bl2.axf` (13 small LOAD segments, never
        # one 0x1CC-byte span); an srec diff will NOT show the fill. Rationale in
        # region_defs.h, OPTION_SETTING_* block.
        target_sources(bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6m4.c
                ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
        )

        # DEBUG (brick investigation, DESIGN.md 8.4): spin at the very start of BL2
        # main() before any bootloader work, so a freshly programmed board can be
        # checked (read FAWMON/FSPR over J-Link) for survival of flash+startup without
        # running MCUboot. Toggle: -DRA6M4_BL2_HALT_AT_MAIN=ON. Default OFF.
        if(RA6M4_BL2_HALT_AT_MAIN)
            message(WARNING "RA6M4_BL2_HALT_AT_MAIN=ON - BL2 will spin at main(); not a bootable image")
            target_compile_definitions(bl2 PRIVATE BL2_HALT_AT_MAIN=1)
        endif()

        # BL2 uses a platform-owned linker script = a copy of TF-M's tfm_common_bl2.ld
        # PLUS the .ram_noinit fix (FCLK/SystemCoreClock, DESIGN.md 8.1) and the RA6M4
        # OFS option-setting regions (BL2-only; see bl2_option_setting.c). GNU ld
        # INSERT cannot augment a -T main script from a second -T fragment, so this is
        # a documented copy - keep in sync with TF-M on version bumps (the ONLY forked
        # linker; the secure/NS side stays on TF-M's generated linker via region_defs.h
        # macros). Secure/NS images are MCUboot-signed/contiguous and carry NO OFS.
        target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6m4_bl2.ld>
            $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
            $<$<C_COMPILER_ID:Clang>:${PLATFORM_DIR}/ext/common/llvm/tfm_common_bl2.ld>
        )

        # Platform library for BL2
        target_sources(platform_bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
                ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c
        )

        target_link_libraries(platform_bl2
            PUBLIC
                fsp_bsp
                fsp_flash
                fsp_uart
        )

        target_include_directories(platform_bl2
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
            PRIVATE
                ${PLATFORM_DIR}/ext/common
        )
    endif()

    # Enable CMSE for BL2 platform library
    target_compile_options(platform_bl2
        PUBLIC
            ${COMPILER_CMSE_FLAG}
    )

    # SEGGER RTT stdout backend for BL2 (MCUboot logs over J-Link) when selected.
    # platform_bl2 is defined in both the external and default BL2 branches above.
    if(RA6M4_STDOUT_RTT)
        target_sources(platform_bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
                ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c
        )
        target_include_directories(platform_bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/rtt
        )
    endif()
endif()
############################ Linker Scripts ####################################

# TF-M generates linker scripts based on our region_defs.h
# We need to explicitly add the generated script to the build
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:ARMClang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/armclang/tfm_isolation_s.sct>
    $<$<C_COMPILER_ID:GNU>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/gcc/tfm_isolation_s.ld>
    $<$<C_COMPILER_ID:IAR>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/iar/tfm_isolation_s.icf>
    $<$<C_COMPILER_ID:Clang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/llvm/tfm_isolation_s.ld>
)

# __STACK_SEAL alias is provided in tfm_hal_platform.c


############################ Non-Secure Configuration ##########################

# Non-secure platform configuration is in ns/CMakeLists.txt
# This gets included automatically by TF-M build system when building NS side
add_subdirectory(ns)

if(FSP_NS_APP_DIR)
    # Use external FSP NS application
    message(STATUS "Using external FSP NS application: ${FSP_NS_APP_DIR}")

    if(NOT EXISTS "${FSP_NS_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake")
        message(FATAL_ERROR
            "TF-M integration file not found in FSP NS application.\n"
            "Expected: ${FSP_NS_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake\n"
            "Make sure the FSP project has TF-M integration files."
        )
    endif()

    # Include the FSP NS application's TF-M integration layer
    include(${FSP_NS_APP_DIR}/tfm_integration/CMakeLists_tfm.cmake)
else()
    # Use built-in test application
    add_subdirectory(ns_app)
endif()

#-------------------------------------------------------------------------------
# OFS brick guard (post-link)
#
# This is the part that was actually destroyed. On 2026-07-21 two EK-RA6M4 boards were
# permanently bricked by a bl2 image whose thirteen option-setting words had been coalesced
# by ld into ONE PT_LOAD spanning 0x0100A100-0x0100A284, with the 368 bytes between them
# zero-filled. A debugger flashes by program header, so it wrote 0x00000000 over the FCU
# configuration in those gaps - including the FSPR permanence word - and the parts were
# gone. DESIGN.md 8.4.
#
# The defence is in ra6m4_bl2.ld: thirteen DISCRETE MEMORY regions, one per option word,
# each with its own '> REGION', which forces one tiny PT_LOAD per word. check_ofs.py
# asserts that on the linked ELF - the only artifact where the mistake is visible. It does
# NOT appear in the srec, because the zero fill lives in the program header and objcopy
# emits from sections, so the srec-diff approach cannot catch a regression here.
#
# Hard failure, no opt-out switch: silent, instant, unrecoverable. tfm_s is checked too and
# must report CLEAN - it carries no OFS, by design, since imgtool needs a contiguous payload.
#
# The script is shared with ra6e1 from renesas/common; the option-setting map is byte
# identical on both parts.
#-------------------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # Scoped per directory; this one has never called it (bl2/ and spe-CMakeLists do their
    # own). Not REQUIRED - a missing interpreter should surface as this guard failing, not
    # as an aborted configure.
    find_package(Python3 COMPONENTS Interpreter)

    # A custom TARGET, not add_custom_command(TARGET ... POST_BUILD): that form is
    # directory-scoped, and tfm_s and bl2 are created in secure_fw/ and bl2/, not here.
    function(ra6m4_add_ofs_check target)
        add_custom_target(ra6m4_ofs_check_${target} ALL
            COMMAND ${CMAKE_COMMAND} -E env READELF=${CMAKE_READELF}
                    ${Python3_EXECUTABLE}
                    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../common/check_ofs.py
                    $<TARGET_FILE:${target}>
            DEPENDS $<TARGET_FILE:${target}>
            VERBATIM
            COMMENT "RA6M4 [${target}]: OFS brick guard")
        add_dependencies(ra6m4_ofs_check_${target} ${target})
    endfunction()

    ra6m4_add_ofs_check(tfm_s)
    if(BL2)
        ra6m4_add_ofs_check(bl2)
    endif()
endif()

############################ Install ###########################################

# Install platform headers for TF-M
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
    DESTINATION ${TFM_INSTALL_PATH}/interface/include
)
