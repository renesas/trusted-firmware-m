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

#========================= platform_region_defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/Native_Driver
    ${RENESAS_RA_COMMON_SOURCE_DIR}/${FSP_SOC_TARGET}/partition
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/inc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_Driver/Config
)

# Memory map values are defined once on tfm_config below, which
# platform_region_defs links and which is exported to the NS build. Only the
# S-only values are added here.
target_compile_definitions(platform_region_defs
    INTERFACE
    S_RAM_CODE_SIZE=${S_RAM_CODE_SIZE}
)

target_compile_definitions(tfm_config
    INTERFACE
    FLASH_BASE_ADDRESS=${FLASH_BASE_ADDRESS}
    FLASH_TOTAL_SIZE=${FLASH_TOTAL_SIZE}
    FLASH_AREA_IMAGE_SECTOR_SIZE=${FLASH_AREA_IMAGE_SECTOR_SIZE}
    FLASH_PAGE_SIZE=${FLASH_PAGE_SIZE}
    FLASH0_SECTOR_SIZE=${FLASH0_SECTOR_SIZE}
    FLASH0_PROGRAM_UNIT=${FLASH0_PROGRAM_UNIT}
    FLASH1_SECTOR_SIZE=${FLASH1_SECTOR_SIZE}
    FLASH1_PROGRAM_UNIT=${FLASH1_PROGRAM_UNIT}
    $<$<BOOL:${FLASH1_DEV_NAME}>:FLASH1_DEV_NAME=${FLASH1_DEV_NAME}>
    $<$<BOOL:${RA_CPU0_NS_EXTERNAL}>:RA_CPU0_NS_EXTERNAL=1>
    $<$<BOOL:${RA_CPU1_NS_EXTERNAL}>:RA_CPU1_NS_EXTERNAL=1>
    $<$<BOOL:${RA_CM33_NS_PRESENT}>:RA_CM33_NS_PRESENT=1>
    FLASH_S_REGION_SIZE=${FLASH_S_REGION_SIZE}
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
    CPU0_NS_RAM_OFFSET=${CPU0_NS_RAM_OFFSET}
    CPU1_NS_RAM_OFFSET=${CPU1_NS_RAM_OFFSET}
    CMSE_VENEER_REGION_SIZE=${FLASH_CPU0_C_SIZE}
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

# FSP objects are linked directly into the image (not through an archive) so
# weak overrides and section-only data are kept.
target_link_libraries(tfm_s
    PRIVATE
    fsp_library
)

#========================= Renesas Partition Data =============================#

# Generates the partition data file next to the first image programmed
# (tfm_s.rpd next to tfm_s.bin, or bl2.rpd next to bl2.bin with BL2), for the
# Renesas tools (e2 studio / RFP) to set the security boundaries. Each
# @NAME@ in the template is a partition variable from the SoC's
# s_ns_memory.cmake / fsp_post_config.cmake, written out in hex.
function(ra_generate_rpd _template _output)
    file(READ ${_template} _content)
    string(REGEX MATCHALL "@[A-Z0-9_]+@" _refs "${_content}")
    foreach(_ref IN LISTS _refs)
        string(REGEX REPLACE "^@(.*)@$" "\\1" _var "${_ref}")
        if(NOT DEFINED ${_var})
            message(FATAL_ERROR "${_template}: ${_var} is not defined")
        endif()
        # math() results are decimal; normalize to hex. Function scope, so the
        # caller's value is left untouched.
        math(EXPR ${_var} "${${_var}}" OUTPUT_FORMAT HEXADECIMAL)
    endforeach()
    configure_file(${_template} ${_output} @ONLY)
endfunction()

if(BL2)
    set(_ra_rpd_file bl2.rpd)
else()
    set(_ra_rpd_file tfm_s.rpd)
endif()

ra_generate_rpd(${RENESAS_RA_SOC_SOURCE_DIR}/partition/tfm_s.rpd.in
                ${CMAKE_BINARY_DIR}/bin/${_ra_rpd_file})

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
    $<$<OR:$<BOOL:${TFM_S_REG_TEST}>,$<BOOL:${TFM_NS_REG_TEST}>>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/plat_test.c>
)

#========================= Platform BL2 =======================================#

if(BL2)
    include(${CMAKE_CURRENT_LIST_DIR}/bl2.cmake)
endif()

#========================= Firmware Update partition ==========================#

# DEFAULT_MCUBOOT_FLASH_MAP is OFF, so the FWU partition (bl2/src/flash_map.c)
# uses the platform flash map, the same one as BL2.
if(BL2 AND TFM_PARTITION_FIRMWARE_UPDATE)
    target_sources(tfm_psa_rot_partition_fwu
        PRIVATE
        ${RENESAS_RA_COMMON_SOURCE_DIR}/core/bl2/flash_map_bl2.c
    )
endif()

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
    target_cfg.c
    ${PLATFORM_DIR}/ext/common/tfm_hal_platform_v8m.c
    ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_platform.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_platform_system.c
    ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_peripherals_def.c
    $<$<BOOL:${TFM_MULTI_CORE_TOPOLOGY}>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/platform_multicore.c>
    $<$<BOOL:${TFM_MULTI_CORE_TOPOLOGY}>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_multi_core.c>
    $<$<BOOL:${TFM_MULTI_CORE_TOPOLOGY}>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_hal_mailbox.c>
    $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_interrupts.c>
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

install(FILES       ${CMAKE_BINARY_DIR}/generated/platform/fsp/bsp_clock_cfg.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/fsp
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
    install(
        FILES
            ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/platform_ns_mailbox.c
            ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/platform_multicore.c
        DESTINATION
            ${INSTALL_PLATFORM_NS_DIR}/../interface/src/multi_core
    )

    install(
        FILES
            ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/inc/platform_multicore.h
        DESTINATION
            ${INSTALL_PLATFORM_NS_DIR}/../interface/include/multi_core
    )
endif()

# NS-side RA ioctl interface, installed so it can be reused outside the TF-M NS build (e.g. Zephyr driver init)
install(FILES       ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/inc/tfm_ioctl_api.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/../interface/include
)

install(FILES       ${RENESAS_RA_COMMON_SOURCE_DIR}/core/services/src/tfm_ioctl_ns_api.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/../interface/src
)

# Install test configs if present
if(EXISTS ${TARGET_PLATFORM_PATH}/tests)
    install(DIRECTORY   ${TARGET_PLATFORM_PATH}/tests
            DESTINATION ${INSTALL_PLATFORM_NS_DIR})
endif()
