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
        # Use common ARMv8-M isolation implementation
        ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${PLATFORM_DIR}/ext/common/tfm_interrupts.c>
)

# Platform include directories
target_include_directories(platform_s
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    PRIVATE
        ${PLATFORM_DIR}/ext/common
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
        # Use default TF-M BL2 with platform FSP BSP (no OFS regions)
        target_sources(bl2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6m4.c
        )

        target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${PLATFORM_DIR}/ext/common/gcc/tfm_common_bl2.ld>
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

############################ Install ###########################################

# Install platform headers for TF-M
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
    DESTINATION ${TFM_INSTALL_PATH}/interface/include
)
