#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# FSP library. The FSP sources are compiled once in this OBJECT library; its
# objects are linked directly into tfm_s (see common.cmake) so that weak
# overrides (hal_warmstart.c) and section-only data (vector_data.c) are always
# kept. Consumers such as platform_s only get the PUBLIC usage requirements
# (FSP headers + _RA_* build selectors).
add_library(fsp_library OBJECT)

# FSP-specific compile definitions (memory map + security attribute flags),
# populated by the SoC's fsp_post_config.cmake. Linked PRIVATE so they apply
# only to the FSP sources and do not leak to code outside fsp_library.
add_library(fsp_library_config INTERFACE)

target_link_libraries(fsp_library
    PUBLIC
    cmsis
    PRIVATE
    fsp_library_config
    cmsis_stack_override
)

target_compile_options(fsp_library
    PRIVATE
    ${COMPILER_CP_FLAG}
    ${COMPILER_CMSE_FLAG}
)

set(RENESAS_RA_FSP_LIB_PATH "DOWNLOAD" CACHE PATH "Path to FSP repo (or DOWNLOAD to fetch automatically)")
set(RENESAS_RA_FSP_LIB_VERSION "v6.4.0" CACHE STRING "The version of FSP to use")
set(RENESAS_RA_FSP_LIB_GIT_REMOTE "https://github.com/renesas/fsp.git" CACHE STRING "The git remote to fetch the FSP library from")

include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/fsp/fetch_lib.cmake)

# Renesas RA FSP library definitions
set(FSP_CMSIS_DEVICE_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/cmsis/Device)
set(FSP_COMMON_BSP_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/all)
set(FSP_TARGET_BSP_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/${FSP_SOC_TARGET})
set(FSP_BOARD_BSP_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/board)
set(FSP_LIBS_CORE_PATH ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/fsp)
set(FSP_GENERATED_PATH ${CMAKE_BINARY_DIR}/generated/platform/fsp)

# Resolve the BSP clock options (-DRA_BSP_CLK_*) and generate the bsp_clock_cfg.h
include(${FSP_LIBS_CORE_PATH}/bsp_clock_options.cmake)
configure_file(${FSP_LIBS_CORE_PATH}/ra_gen/bsp_clock_cfg.h.in
               ${FSP_GENERATED_PATH}/bsp_clock_cfg.h
               @ONLY)

# Include security configuration for RA platform code
include(${FSP_LIBS_CORE_PATH}/s_ns_memory.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_interrupt.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_peripheral.cmake)

# Derive partitions and export FSP security/memory macros on fsp_library_config
# (FSP sources only) from the settings above
include(${FSP_LIBS_CORE_PATH}/fsp_post_config.cmake)

# Mark as secure build for RA platform code
target_compile_definitions(fsp_library
    PUBLIC
    _RA_TZ_SECURE
    _RA_BOOT_IMAGE
    _RA_CORE=CPU0
)

target_include_directories(fsp_library
    PUBLIC
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
    ${FSP_GENERATED_PATH}
    PRIVATE
    # RTE_PinCfg.h / RTE_Device.h, needed by ra_gen/pin_data.c and common_data.h
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Native_Driver
    ${TARGET_PLATFORM_PATH}/CMSIS_Driver/Config
)

target_sources(fsp_library
    PRIVATE
    ${FSP_CMSIS_DEVICE_PATH}/RENESAS/Source/system.c
    ${FSP_COMMON_BSP_PATH}/bsp_clocks.c
    ${FSP_COMMON_BSP_PATH}/bsp_common.c
    ${FSP_COMMON_BSP_PATH}/bsp_delay.c
    ${FSP_COMMON_BSP_PATH}/bsp_group_irq.c
    ${FSP_COMMON_BSP_PATH}/bsp_guard.c
    ${FSP_COMMON_BSP_PATH}/bsp_io.c
    ${FSP_COMMON_BSP_PATH}/bsp_ipc.c
    ${FSP_COMMON_BSP_PATH}/bsp_irq.c
    ${FSP_COMMON_BSP_PATH}/bsp_macl.c
    ${FSP_COMMON_BSP_PATH}/bsp_register_protection.c
    ${FSP_COMMON_BSP_PATH}/bsp_sbrk.c
    ${FSP_COMMON_BSP_PATH}/bsp_sdram.c
    ${FSP_COMMON_BSP_PATH}/bsp_security.c
    ${FSP_TARGET_BSP_PATH}/bsp_linker.c
    ${FSP_BOARD_BSP_PATH}/${FSP_BOARD_TARGET}/board_init.c
    ${FSP_BOARD_BSP_PATH}/${FSP_BOARD_TARGET}/board_leds.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/common_data.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/hal_warmstart.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/pin_data.c
    ${FSP_LIBS_CORE_PATH}/ra_gen/vector_data.c
)

target_sources(fsp_library
    PRIVATE
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_ioport/r_ioport.c
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_mram/r_mram.c
    $<$<OR:$<BOOL:${TFM_S_REG_TEST}>,$<BOOL:${TFM_NS_REG_TEST}>>:${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_agt/r_agt.c>
)
