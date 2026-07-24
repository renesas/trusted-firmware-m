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
#   -DFSP_BL2_APP_DIR=C:/path/to/FSP_Project_ra6e1_bl2
#   -DFSP_S_APP_DIR=C:/path/to/FSP_Project_ra6e1_s_rtos
#   -DFSP_NS_APP_DIR=C:/path/to/FSP_Project_ra6e1_ns_rtos

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
        TFM_PLATFORM_RA6E1=1
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
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c
        # Use common ARMv8-M isolation implementation
        ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${PLATFORM_DIR}/ext/common/tfm_interrupts.c>
        # SEGGER RTT stdout backend (replaces common uart_stdout.c when RA6E1_STDOUT_RTT=ON)
        $<$<BOOL:${RA6E1_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c>
        $<$<BOOL:${RA6E1_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c>
)

# Platform include directories
target_include_directories(platform_s
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    PRIVATE
        ${PLATFORM_DIR}/ext/common
        $<$<BOOL:${RA6E1_STDOUT_RTT}>:${CMAKE_CURRENT_SOURCE_DIR}/rtt>
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

        # Use TF-M's OWN startup + linker (they define the TF-M BL2 symbols the boot HAL
        # needs: Image$$ER_DATA$$Base, ARM_LIB_STACK/HEAP, `end`). FSP's fsp.ld is for a
        # standalone bootloader and lacks these. We still consume the RA6E1 BSP from the
        # RASC project (fsp_bsp_bl2). OFS: bl2_option_setting.c emits the words, ra6e1_bl2.ld
        # places each in its own MEMORY region (per-word segments, safe - DESIGN.md 8.4).
        target_sources(bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c
                ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
                # Provides g_main_stack + g_init_info (copy/zero lists) that FSP's system.c
                # needs, without pulling in FSP's fsp.ld-coupled startup.c. (See fsp_bsp for S.)
                ${CMAKE_CURRENT_SOURCE_DIR}/bsp_init_stub.c
        )

        target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_bl2.ld>
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
                # UART console only when RTT is OFF (flat RA6E1 BL2 project has no r_sci_uart)
                $<$<NOT:$<BOOL:${RA6E1_STDOUT_RTT}>>:${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c>
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
        # bl2_option_setting.c emits the RA6E1 OFS words (.option_setting_*) that RASC
        # configured (BSP_CFG_OPTION_SETTING_*), into the bl2 executable (not a lib, or
        # the linker wouldn't pull it in). SAFETY: ra6e1_bl2.ld places each word in its
        # OWN exactly-sized MEMORY region (like FSP's fsp_gen.ld) so each becomes a
        # separate PT_LOAD segment - NO gap-filled span. The earlier all-in-one-region
        # layout let ld coalesce them into a zero-filled segment that programmed
        # PBPS=0 and bricked two boards. See DESIGN.md 8.4. Keep the per-region linker.
        target_sources(bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c
                ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
                # Provides g_main_stack + g_init_info (copy/zero lists) that FSP's system.c
                # needs, without pulling in FSP's fsp.ld-coupled startup.c. (See fsp_bsp for S.)
                ${CMAKE_CURRENT_SOURCE_DIR}/bsp_init_stub.c
        )

        # OFS brick guard, run automatically on EVERY build: fail the build if the
        # option-setting words landed in one gap-filled PT_LOAD segment (the layout
        # that programmed PBPS=0 and bricked two boards). Discrete per-word segments
        # pass. A custom target in ALL that depends on bl2 (bl2 is defined in another
        # directory, so add_custom_command TARGET POST_BUILD can't be used here).
        # See DESIGN.md 8.4 and check_ofs.py.
        find_program(PYTHON_EXECUTABLE NAMES python3 python)
        if(PYTHON_EXECUTABLE)
            add_custom_target(bl2_ofs_guard ALL
                COMMAND ${CMAKE_COMMAND} -E env READELF=${CMAKE_READELF}
                        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/check_ofs.py $<TARGET_FILE:bl2>
                COMMENT "OFS brick guard: checking bl2 for a spanning option-setting segment")
            add_dependencies(bl2_ofs_guard bl2)
        else()
            message(WARNING "python not found - OFS brick guard (check_ofs.py) will NOT run on bl2 builds")
        endif()

        # DEBUG (brick investigation, DESIGN.md 8.4): spin at the very start of BL2
        # main() before any bootloader work, so a freshly programmed board can be
        # checked (read FAWMON/FSPR over J-Link) for survival of flash+startup without
        # running MCUboot. Toggle: -DRA6E1_BL2_HALT_AT_MAIN=ON. Default OFF.
        if(RA6E1_BL2_HALT_AT_MAIN)
            message(WARNING "RA6E1_BL2_HALT_AT_MAIN=ON - BL2 will spin at main(); not a bootable image")
            target_compile_definitions(bl2 PRIVATE BL2_HALT_AT_MAIN=1)
        endif()

        # BL2 uses a platform-owned linker script = a copy of TF-M's tfm_common_bl2.ld
        # with the .ram_noinit fix (FCLK/SystemCoreClock, DESIGN.md 8.1). GNU ld INSERT
        # cannot augment a -T main script from a second -T fragment, so this is a
        # documented copy - keep in sync with TF-M on version bumps (the ONLY forked
        # linker; the secure/NS side stays on TF-M's generated linker via region_defs.h
        # macros). NO image (BL2/secure/NS) carries OFS.
        target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_bl2.ld>
            $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
            $<$<C_COMPILER_ID:Clang>:${PLATFORM_DIR}/ext/common/llvm/tfm_common_bl2.ld>
        )

        # Platform library for BL2
        target_sources(platform_bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
                ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
                # UART console only when RTT is OFF (flat RA6E1 BL2 project has no r_sci_uart)
                $<$<NOT:$<BOOL:${RA6E1_STDOUT_RTT}>>:${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c>
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
    if(RA6E1_STDOUT_RTT)
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

# Non-secure platform configuration is in ns/CMakeLists.txt (not yet ported for RA6E1;
# BL2 bring-up first). Guard on existence so a BL2-only scaffold configures.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ns)
    add_subdirectory(ns)
endif()

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
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ns_app)
    # Use built-in test application
    add_subdirectory(ns_app)
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
