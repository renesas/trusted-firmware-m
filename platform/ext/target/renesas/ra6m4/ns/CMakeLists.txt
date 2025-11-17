#-------------------------------------------------------------------------------
# Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#
# Non-Secure Platform Configuration for RA6M4
#-------------------------------------------------------------------------------

# This file configures the non-secure platform library for RA6M4
# It provides CMSIS drivers and FSP integration for the NS side

cmake_policy(SET CMP0076 NEW)

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# Platform library for non-secure side
add_library(platform_ns STATIC)

# Include FSP modular libraries
include(${PLATFORM_DIR}/cmake/modules/fsp_bsp.cmake)
include(${PLATFORM_DIR}/cmake/modules/fsp_uart.cmake)

# Add NS-side CMSIS drivers
target_sources(platform_ns
    PRIVATE
        ${PLATFORM_DIR}/cmsis_drivers/Driver_USART.c
)

# Link FSP modules
target_link_libraries(platform_ns
    PUBLIC
        fsp_bsp
        fsp_uart
)

# Include directories
target_include_directories(platform_ns
    PUBLIC
        ${PLATFORM_DIR}
        ${PLATFORM_DIR}/cmsis_drivers
        ${CMAKE_SOURCE_DIR}/platform/ext/cmsis/Include
)

# Compile definitions for non-secure side
target_compile_definitions(platform_ns
    PUBLIC
        DOMAIN_NS=1
        __DOMAIN_NS=1
)
