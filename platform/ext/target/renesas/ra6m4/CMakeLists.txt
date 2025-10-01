#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_isolation.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_interrupts.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
)

# Platform include directories
target_include_directories(platform_s
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
)

############################ BL2 Configuration #################################

if(BL2)
    target_sources(bl2
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
            ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
            ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    )

    target_link_libraries(bl2
        PUBLIC
            fsp_bsp
            fsp_flash
    )

    target_include_directories(bl2
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

############################ Linker Scripts ####################################

# Note: TF-M will use its own linker script generation, but we provide
# the memory region definitions via region_defs.h

# If custom linker script is needed:
# target_link_options(platform_s
#     PRIVATE
#         -T${CMAKE_CURRENT_SOURCE_DIR}/linker_scripts/tfm_s.ld
# )

############################ Install ###########################################

# Install platform headers for TF-M
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
    DESTINATION ${TFM_INSTALL_PATH}/interface/include
)
