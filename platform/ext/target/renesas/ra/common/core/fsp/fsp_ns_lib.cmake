#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------

# NS-side FSP library. Reuses the same FSP source tree as the S side

# FSP library for platform_ns code: an OBJECT library whose objects are linked
# directly into the NS executable (see ns/common.cmake). Consumers only get the
# PUBLIC usage requirements (FSP headers + _RA_* build selectors).
add_library(fsp_library OBJECT)

# FSP-specific compile definitions (memory map + security attribute flags),
# populated by the SoC's fsp_post_config.cmake. Linked PRIVATE so they apply
# only to the FSP sources.
add_library(fsp_library_config INTERFACE)

target_link_libraries(fsp_library
    PRIVATE
    fsp_library_config
)

# The FSP sources are compiled in their own target, so they don't inherit
# platform_ns's DOMAIN_NS; RTE_PinCfg.h (used by ra_gen/pin_data.c) needs it
# to select the NS-accessible pins.
target_compile_definitions(fsp_library
    PRIVATE
    DOMAIN_NS=1
)

# Renesas RA FSP library definitions
set(FSP_CMSIS_DEVICE_PATH ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/cmsis/Device)
set(FSP_COMMON_BSP_PATH   ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/all)
set(FSP_TARGET_BSP_PATH   ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/bsp/mcu/${FSP_SOC_TARGET})
set(FSP_BOARD_BSP_PATH    ${RENESAS_RA_FSP_LIB_PATH}/ra/board)
set(FSP_LIBS_CORE_PATH    ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/fsp)
set(FSP_GENERATED_PATH    ${CMAKE_CURRENT_SOURCE_DIR}/fsp)

# Include security configuration for RA platform code
include(${FSP_LIBS_CORE_PATH}/s_ns_memory.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_interrupt.cmake)
include(${FSP_LIBS_CORE_PATH}/s_ns_peripheral.cmake)

# Derive partitions and export FSP security/memory macros on fsp_library_config
# (FSP sources only) from the settings above
include(${FSP_LIBS_CORE_PATH}/fsp_post_config.cmake)

if(RA_CORE STREQUAL "cm33")
    # Mark as remote non-secure (CPU1) build for RA platform code.
    target_compile_definitions(fsp_library
        PUBLIC
        _RA_REMOTE_NONSECURE
        _RA_CORE=CPU1
    )
else()
    # Mark as local non-secure (CPU0, TrustZone) build for RA platform code.
    target_compile_definitions(fsp_library
        PUBLIC
        _RA_TZ_NONSECURE
        _RA_BOOT_IMAGE
        _RA_CORE=CPU0
    )
endif()

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
    ${TARGET_PLATFORM_PATH}/CMSIS_Driver/Config
    ${FSP_GENERATED_PATH}
    # CMSIS core headers from the exported NS platform tree (CMSIS_PATH is
    # not available in the NS build)
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/cmsis/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/cmsis/Include/m-profile
    PRIVATE
    # RTE_PinCfg.h, needed by ra_gen/pin_data.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Native_Driver
)

# FSP BSP sources
target_sources(fsp_library
    PRIVATE
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
    PRIVATE
    ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_ioport/r_ioport.c
    $<$<BOOL:${TFM_PARTITION_NS_AGENT_MAILBOX}>:${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_ipc/r_ipc.c>
)
