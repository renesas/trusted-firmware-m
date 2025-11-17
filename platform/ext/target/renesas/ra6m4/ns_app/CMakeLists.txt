#-------------------------------------------------------------------------------
# Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#
# TF-M Non-Secure Application with FreeRTOS for RA6M4
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

project(tfm_ns_app LANGUAGES C ASM)

# FreeRTOS kernel source directory (adjust path as needed)
set(FREERTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../lib/ext/freertos" CACHE PATH "FreeRTOS kernel directory")

if(NOT EXISTS ${FREERTOS_DIR})
    message(FATAL_ERROR "FreeRTOS directory not found: ${FREERTOS_DIR}")
endif()

# Non-Secure application target
add_executable(tfm_ns
    src/main_ns.c
    src/tfm_test_thread.c
)

# FreeRTOS kernel sources
target_sources(tfm_ns
    PRIVATE
        ${FREERTOS_DIR}/tasks.c
        ${FREERTOS_DIR}/queue.c
        ${FREERTOS_DIR}/list.c
        ${FREERTOS_DIR}/timers.c
        ${FREERTOS_DIR}/event_groups.c
        ${FREERTOS_DIR}/stream_buffer.c
        ${FREERTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
        ${FREERTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
        ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

# Include directories
target_include_directories(tfm_ns
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${FREERTOS_DIR}/include
        ${FREERTOS_DIR}/portable/GCC/ARM_CM33_NTZ/non_secure
        ${CMAKE_BINARY_DIR}/generated/interface/include
        ${CMAKE_SOURCE_DIR}/interface/include
        ${CMAKE_SOURCE_DIR}/interface/include/psa
        ${CMAKE_SOURCE_DIR}/interface/include/crypto
        ${CMAKE_SOURCE_DIR}/interface/include/tfm
        ${CMAKE_SOURCE_DIR}/secure_fw/spm/include
        ${CMAKE_SOURCE_DIR}/platform/include
        ${CMAKE_SOURCE_DIR}/platform/ext/common
        ${CMAKE_SOURCE_DIR}/platform/ext/target/renesas/ra6m4
        ${CMAKE_SOURCE_DIR}/platform/ext/cmsis/Include
        ${CMAKE_SOURCE_DIR}/platform/ext/cmsis/Device/ARM/ARMCM33/Include
)

# Link against TF-M non-secure interface
target_link_libraries(tfm_ns
    PRIVATE
        tfm_api_ns
        platform_ns
)

# Compiler definitions
target_compile_definitions(tfm_ns
    PRIVATE
        TFM_NS_CLIENT
        DOMAIN_NS=1
        __DOMAIN_NS=1
)

# Compiler options
target_compile_options(tfm_ns
    PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-std=c99>
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
        -fdata-sections
        -ffunction-sections
)

# Linker options
target_link_options(tfm_ns
    PRIVATE
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/tfm_ns.map
)

# Set output name
set_target_properties(tfm_ns
    PROPERTIES
        SUFFIX ".axf"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Generate hex and bin files
add_custom_command(TARGET tfm_ns
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:tfm_ns> ${CMAKE_BINARY_DIR}/bin/tfm_ns.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:tfm_ns> ${CMAKE_BINARY_DIR}/bin/tfm_ns.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:tfm_ns>
    COMMENT "Generating NS application hex and bin files"
)
