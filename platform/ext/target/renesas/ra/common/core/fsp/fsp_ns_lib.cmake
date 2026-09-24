#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------

# NS-side FSP library. Reuses the same FSP source tree as the S side

# Create target for FSP library to be linked to platform_ns code.
add_library(fsp_library INTERFACE)

# Renesas RA FSP library definitions
set(FSP_CMSIS_DEVICE_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/cmsis/Device)
set(FSP_COMMON_BSP_PATH   ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/all)
set(FSP_TARGET_BSP_PATH   ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/${FSP_SOC_TARGET})
set(FSP_BOARD_BSP_PATH    ${RENESAS_RA_FSP_LIB_PATH}/ra/board)
set(FSP_LIBS_CORE_PATH    ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/fsp)

# Include security configuration for RA platform code
include(${FSP_LIBS_CORE_PATH}/s_ns_memory.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_interrupt.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_peripheral.cmake)

# Mark as non-secure build for RA platform code
target_compile_definitions(fsp_library
    INTERFACE
    _RA_TZ_NONSECURE
    _RA_BOOT_IMAGE
    _RA_CORE=CPU0
)

target_include_directories(fsp_library
    INTERFACE
    ${FSP_CMSIS_DEVICE_PATH}/RENESAS/Include
    ${FSP_COMMON_BSP_PATH}
    ${FSP_TARGET_BSP_PATH}
    ${FSP_BOARD_BSP_PATH}
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/inc
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/inc/api
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/inc/instances
    ${FSP_LIBS_CORE_PATH}
    ${FSP_LIBS_CORE_PATH}/ra_cfg
    ${FSP_LIBS_CORE_PATH}/ra_cfg/bsp
    ${FSP_LIBS_CORE_PATH}/ra_gen
    ${TARGET_PLATFORM_PATH}/CMSIS_Driver/Config
)

# FSP BSP sources
target_sources(fsp_library
    INTERFACE
    ${FSP_CMSIS_DEVICE_PATH}/RENESAS/Source/system.c
    ${FSP_COMMON_BSP_PATH}/bsp_clocks.c
    ${FSP_COMMON_BSP_PATH}/bsp_common.c
    ${FSP_COMMON_BSP_PATH}/bsp_delay.c
    ${FSP_COMMON_BSP_PATH}/bsp_irq.c
    ${FSP_COMMON_BSP_PATH}/bsp_register_protection.c
    ${FSP_COMMON_BSP_PATH}/bsp_group_irq.c
    ${FSP_COMMON_BSP_PATH}/bsp_guard.c
    ${FSP_COMMON_BSP_PATH}/bsp_io.c
    ${FSP_COMMON_BSP_PATH}/bsp_macl.c
    ${FSP_COMMON_BSP_PATH}/bsp_security.c
    ${FSP_COMMON_BSP_PATH}/bsp_sbrk.c
    ${FSP_TARGET_BSP_PATH}/bsp_linker.c
    ${FSP_BOARD_BSP_PATH}/${FSP_BOARD_TARGET}/board_init.c
    ${FSP_BOARD_BSP_PATH}/${FSP_BOARD_TARGET}/board_leds.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/common_data.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/hal_warmstart.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/pin_data.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/vector_data.c
)

# FSP driver sources
target_sources(fsp_library
    INTERFACE
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_ioport/r_ioport.c
)
