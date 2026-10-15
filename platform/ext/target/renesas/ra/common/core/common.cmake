#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Shared RA platform build/install logic, included from each board's
# CMakeLists.txt. The board file must set FSP_SOC_TARGET (e.g. ra8p1) and
# FSP_BOARD_TARGET (e.g. ra8p1_ek) before including this file.

cmake_policy(SET CMP0076 NEW)
set(CMAKE_FSP_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../common/core/fsp)

#========================= FSP library configuration options ==================#

# Include FSP library configuration for platform_s code
include(${CMAKE_FSP_LIBS_PATH}/fsp_s_lib.cmake)

# FSP library needs the TF-M partition headers to resolve FLASH_CPU0_C_START /
# FLASH_CPU0_C_SIZE (now defined in region_defs.h via S_CODE_LIMIT).
target_include_directories(fsp_library
    INTERFACE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/partition
)

#========================= platform_region_defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Native_Driver
    ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/partition
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/inc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_Driver/Config
)

target_compile_definitions(platform_region_defs
    INTERFACE
    S_RAM_CODE_SIZE=${S_RAM_CODE_SIZE}
    FLASH_BASE_ADDRESS=${FLASH_BASE_ADDRESS}
    FLASH_TOTAL_SIZE=${FLASH_TOTAL_SIZE}
    FLASH_AREA_IMAGE_SECTOR_SIZE=${FLASH_AREA_IMAGE_SECTOR_SIZE}
    FLASH_PAGE_SIZE=${FLASH_PAGE_SIZE}
    FLASH_PROGRAM_UNIT=${FLASH_PROGRAM_UNIT}
    FLASH_AREA_BL2_OFFSET=${FLASH_AREA_BL2_OFFSET}
    FLASH_AREA_BL2_SIZE=${FLASH_AREA_BL2_SIZE}
    FLASH_S_PARTITION_SIZE=${FLASH_S_PARTITION_SIZE}
    FLASH_CPU0_NS_PARTITION_SIZE=${FLASH_CPU0_NS_PARTITION_SIZE}
    FLASH_CPU1_NS_PARTITION_SIZE=${FLASH_CPU1_NS_PARTITION_SIZE}
    CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET=${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET}
    CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET=${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET}
    FLASH_PS_AREA_SIZE=${FLASH_PS_AREA_SIZE}
    FLASH_ITS_AREA_SIZE=${FLASH_ITS_AREA_SIZE}
    FLASH_OTP_NV_COUNTERS_AREA_SIZE=${FLASH_OTP_NV_COUNTERS_AREA_SIZE}
    TOTAL_RAM_SIZE=${TOTAL_RAM_SIZE}
    S_DATA_SIZE=${S_DATA_SIZE}
    CPU0_NS_RAM_DATA_SIZE=${CPU0_NS_RAM_DATA_SIZE}
    CPU1_NS_RAM_DATA_SIZE=${CPU1_NS_RAM_DATA_SIZE}
    S_ROM_ALIAS_BASE=${S_ROM_ALIAS_BASE}
    CPU0_NS_ROM_ALIAS_BASE=${CPU0_NS_ROM_ALIAS_BASE}
    CPU1_NS_ROM_ALIAS_BASE=${CPU1_NS_ROM_ALIAS_BASE}
    S_RAM_ALIAS_BASE=${S_RAM_ALIAS_BASE}
    CPU0_NS_RAM_ALIAS_BASE=${CPU0_NS_RAM_ALIAS_BASE}
    CPU1_NS_RAM_ALIAS_BASE=${CPU1_NS_RAM_ALIAS_BASE}
)

target_compile_definitions(tfm_config
    INTERFACE
    FLASH_BASE_ADDRESS=${FLASH_BASE_ADDRESS}
    FLASH_TOTAL_SIZE=${FLASH_TOTAL_SIZE}
    FLASH_AREA_IMAGE_SECTOR_SIZE=${FLASH_AREA_IMAGE_SECTOR_SIZE}
    FLASH_PAGE_SIZE=${FLASH_PAGE_SIZE}
    FLASH_PROGRAM_UNIT=${FLASH_PROGRAM_UNIT}
    FLASH_AREA_BL2_OFFSET=${FLASH_AREA_BL2_OFFSET}
    FLASH_AREA_BL2_SIZE=${FLASH_AREA_BL2_SIZE}
    FLASH_S_PARTITION_SIZE=${FLASH_S_PARTITION_SIZE}
    FLASH_CPU0_NS_PARTITION_SIZE=${FLASH_CPU0_NS_PARTITION_SIZE}
    FLASH_CPU1_NS_PARTITION_SIZE=${FLASH_CPU1_NS_PARTITION_SIZE}
    CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET=${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET}
    CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET=${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET}
    FLASH_PS_AREA_SIZE=${FLASH_PS_AREA_SIZE}
    FLASH_ITS_AREA_SIZE=${FLASH_ITS_AREA_SIZE}
    FLASH_OTP_NV_COUNTERS_AREA_SIZE=${FLASH_OTP_NV_COUNTERS_AREA_SIZE}
    TOTAL_RAM_SIZE=${TOTAL_RAM_SIZE}
    S_DATA_SIZE=${S_DATA_SIZE}
    CPU0_NS_RAM_DATA_SIZE=${CPU0_NS_RAM_DATA_SIZE}
    CPU1_NS_RAM_DATA_SIZE=${CPU1_NS_RAM_DATA_SIZE}
    S_ROM_ALIAS_BASE=${S_ROM_ALIAS_BASE}
    CPU0_NS_ROM_ALIAS_BASE=${CPU0_NS_ROM_ALIAS_BASE}
    CPU1_NS_ROM_ALIAS_BASE=${CPU1_NS_ROM_ALIAS_BASE}
    S_RAM_ALIAS_BASE=${S_RAM_ALIAS_BASE}
    CPU0_NS_RAM_ALIAS_BASE=${CPU0_NS_RAM_ALIAS_BASE}
    CPU1_NS_RAM_ALIAS_BASE=${CPU1_NS_RAM_ALIAS_BASE}
)

#========================= Platform common defs ===============================#

set(RENESAS_RA_SCRIPT_DIR ${CMAKE_BINARY_DIR}/generated/platform/ext/target/renesas/ra/common/core/scripts)

target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:ARMClang>:${RENESAS_RA_SCRIPT_DIR}/armclang/tfm_isolation_s.sct>
    $<$<C_COMPILER_ID:GNU>:${RENESAS_RA_SCRIPT_DIR}/gcc/tfm_isolation_s.ld>
    $<$<C_COMPILER_ID:Clang>:${RENESAS_RA_SCRIPT_DIR}/atfe/tfm_isolation_s.ld>
)

target_sources(tfm_s
    PRIVATE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Device/Source/startup.c
)

#========================= Platform Secure ====================================#

target_compile_options(platform_s
    PUBLIC
    ${COMPILER_CP_FLAG}
    ${COMPILER_CMSE_FLAG}
)

target_link_options(platform_s
    PUBLIC
    ${LINKER_CP_OPTION}
)

target_link_libraries(platform_s
    PUBLIC
    fsp_library
    cmsis
)

target_sources(platform_s
    INTERFACE
        $<$<STREQUAL:"${TFM_FIH_PROFILE}","HIGH">:${PLATFORM_DIR}/ext/common/template/tfm_fih_platform.c>
    PRIVATE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/CMSIS_Driver/Driver_USART.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/CMSIS_Driver/Driver_Flash.c
    ${PLATFORM_DIR}/ext/common/uart_stdout.c
)

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
    target_cfg.c
    ${PLATFORM_DIR}/ext/common/tfm_hal_platform_v8m.c
    ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_platform.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_platform_system.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_peripherals_def.c
    $<$<BOOL:${TFM_MULTI_CORE_TOPOLOGY}>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_multi_core.c>
    $<$<BOOL:${TFM_MULTI_CORE_TOPOLOGY}>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_mailbox.c>
)

#========================= Files for building NS side platform ================#

configure_file(${RENESAS_RA_COMMON_SOURCE_DIR}/core/ns/cpuarch_ns.cmake.in
               ${CMAKE_BINARY_DIR}/generated/platform/cmake/cpuarch.cmake
               @ONLY)
configure_file(${RENESAS_RA_COMMON_SOURCE_DIR}/core/ns/config.cmake.in
               ${CMAKE_BINARY_DIR}/generated/platform/cmake/config.cmake
               @ONLY)

install(FILES       ${CMAKE_BINARY_DIR}/generated/platform/cmake/cpuarch.cmake
                    ${CMAKE_BINARY_DIR}/generated/platform/cmake/config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(DIRECTORY   ${TARGET_PLATFORM_PATH}/ns/
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(FILES       ${TARGET_PLATFORM_PATH}/check_config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

# Install NS linker scripts
install(FILES       ${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/gcc/tfm_common_ns.ld
                    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/armclang/tfm_common_ns.sct
                    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/scripts/atfe/tfm_common_ns.ldc
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/linker_scripts
)

# NS-side mailbox platform source
if(RA_CM33_NS_PRESENT)
    install(FILES       ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/platform_ns_mailbox.c
            DESTINATION ${INSTALL_PLATFORM_NS_DIR}/../interface/src/multi_core
    )
endif()

# Install test configs if present
if(EXISTS ${TARGET_PLATFORM_PATH}/tests)
    install(DIRECTORY   ${TARGET_PLATFORM_PATH}/tests
            DESTINATION ${INSTALL_PLATFORM_NS_DIR})
endif()
