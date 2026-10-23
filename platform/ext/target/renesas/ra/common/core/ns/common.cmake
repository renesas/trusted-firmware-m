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

# Include NS FSP library (reuses S-side FSP clone, includes s_ns_memory.cmake
# which sets FLASH/RAM cache variables and compile defs on fsp_library)
include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/fsp/fsp_ns_lib.cmake)

# Link platform_ns against platform_region_defs to get memory region defines
target_link_libraries(platform_ns
    PUBLIC
        platform_region_defs
        fsp_library
)

target_compile_definitions(platform_ns
    PUBLIC
        $<$<BOOL:${PLATFORM_DEFAULT_CRYPTO_KEYS}>:PLATFORM_DEFAULT_CRYPTO_KEYS>
)
