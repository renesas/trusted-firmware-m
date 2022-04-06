#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
# Copyright (c) 2021 STMicroelectronics. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(RA6M4_EK_DIR ${CMAKE_CURRENT_LIST_DIR})
set(RENESAS_COMMON_DIR ${CMAKE_CURRENT_LIST_DIR})

#def for ra6m4
#include(${RENESAS_COMMON_DIR}/CMakeLists.txt)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/board
        ${CMAKE_CURRENT_SOURCE_DIR}/Partition
)

target_compile_definitions(platform_region_defs
    INTERFACE
        $<$<OR:$<BOOL:${TEST_NS}>,$<BOOL:${TEST_S}>>:FLASH_LAYOUT_FOR_TEST>
)

#========================= Platform common defs ===============================#
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:GNU>:${RENESAS_COMMON_DIR}/Device/Source/gcc/ra6m4_s.ld>
    $<$<C_COMPILER_ID:ARMClang>:${RENESAS_COMMON_DIR}/Device/Source/armclang/ra6m4_ns.sct>
    $<$<C_COMPILER_ID:IAR>:${RENESAS_COMMON_DIR}/Device/Source/iar/ra6m4_ns.icf>
)

if(NS)
    target_add_scatter_file(tfm_ns
        $<$<C_COMPILER_ID:GNU>:${RENESAS_COMMON_DIR}/Device/Source/gcc/ra6m4_ns.ld>
        $<$<C_COMPILER_ID:ARMClang>:${RENESAS_COMMON_DIR}/Device/Source/armclang/ra6m4_ns.sct>
        $<$<C_COMPILER_ID:IAR>:${RENESAS_COMMON_DIR}/Device/Source/iar/ra6m4_ns.icf>
    )
    target_link_libraries(CMSIS_5_tfm_ns
        INTERFACE
            CMSIS_5_RTX_V8MMN
    )
endif()

if(BL2)
    target_add_scatter_file(bl2
        $<$<C_COMPILER_ID:GNU>:${RENESAS_COMMON_DIR}/Device/Source/gcc/ra6m4_bl2.ld>
        $<$<C_COMPILER_ID:ARMClang>:${RENESAS_COMMON_DIR}/Device/Source/armclang/ra6m4_bl2.sct>
        $<$<C_COMPILER_ID:IAR>:${RENESAS_COMMON_DIR}/Device/Source/iar/ra6m4_bl2.icf>
    )
endif()

#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
)

target_sources(platform_s
    PRIVATE
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)


#========================= Platform Non-Secure ================================#

target_include_directories(platform_ns
    PUBLIC
)

target_sources(platform_ns
    PRIVATE
)

#========================= Platform BL2 =======================================#

if(BL2)
    target_include_directories(platform_bl2
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/inc	
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/inc/api
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/inc/instances
            ${CMAKE_CURRENT_SOURCE_DIR}/ra_gen
            ${CMAKE_CURRENT_SOURCE_DIR}/ra_cfg/fsp_cfg
            ${CMAKE_CURRENT_SOURCE_DIR}/ra_cfg/fsp_cfg/bsp	
            ${CMAKE_CURRENT_SOURCE_DIR}/Partition
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all
            ${CMAKE_CURRENT_SOURCE_DIR}/Device/Include
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/ra6m4
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/cmsis/Device/RENESAS	
            ${CMAKE_CURRENT_SOURCE_DIR}/secure
			
            ${CMAKE_CURRENT_SOURCE_DIR}/Native_Driver
            ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_Driver
            ${MCUBOOT_PATH}/boot/bootutil/include
            $<TARGET_PROPERTY:bl2,BINARY_DIR>/ext/mcuboot
            ${CMAKE_SOURCE_DIR}/bl2/ext/mcuboot/include
    )

    target_sources(platform_bl2
        PRIVATE			
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c	
            ${CMAKE_CURRENT_SOURCE_DIR}/bl2/boot_hal.c	
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_clocks.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_common.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_delay.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_group_irq.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_guard.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_io.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_irq.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_register_protection.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_rom_registers.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_sbrk.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/bsp/mcu/all/bsp_security.c
            ${CMAKE_CURRENT_SOURCE_DIR}/fsp/src/r_flash_hp/r_flash_hp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_driver/Driver_Flash.c
            ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_driver/Driver_QSPI_Flash.c
            ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_driver/Driver_USART.c
            ${CMAKE_CURRENT_SOURCE_DIR}/ra_gen/common_data.c
    )

    target_compile_options(platform_bl2
        PUBLIC
            ${COMPILER_CMSE_FLAG}
    )

endif()

#end of include(${RENESAS_COMMON_DIR}/CMakeLists.txt)

#========================= Platform defs ===============================#

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
        # ${RENESAS_COMMON_DIR}/Device/Source/startup_ra6m4_s.c
)

if(NS)
    target_sources(tfm_ns
        PRIVATE
            # ${RENESAS_COMMON_DIR}/Device/Source/startup_ra6m4_ns.c
    )
endif()

if(BL2)
    target_sources(bl2
        PRIVATE
            ${RENESAS_COMMON_DIR}/fsp/src/bsp/cmsis/Device/RENESAS/source/startup.c
    )
endif()

# install(FILES ${RA6M4_EK_DIR}/Partition/flash_layout.h
#               ${RA6M4_EK_DIR}/Partition/region_defs.h
#         DESTINATION ${CMAKE_BINARY_DIR} )
        
# set (BL2_FILE_TO_PREPROCESS ${CMAKE_CURRENT_BINARY_DIR}/image_macros_to_preprocess_bl2.c)
# file(WRITE ${BL2_FILE_TO_PREPROCESS} ${BL2_PREPROCESSING})
# install(FILES ${BL2_FILE_TO_PREPROCESS}
#         DESTINATION ${CMAKE_BINARY_DIR} )
