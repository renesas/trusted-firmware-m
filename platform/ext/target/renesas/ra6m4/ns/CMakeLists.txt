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

# Add NS-side CMSIS drivers and stubs
target_sources(platform_ns
    PRIVATE
        ${PLATFORM_DIR}/cmsis_drivers/Driver_USART.c
        ${PLATFORM_DIR}/bsp_init_stub.c  # Provides weak symbols for g_hal_init and FreeRTOS TZ functions
        ${PLATFORM_DIR}/ns/syscalls_ns.c  # Newlib syscalls for NS app
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
        ${CMAKE_BINARY_DIR}/lib/ext/cmsis-src/CMSIS/Driver/Include
)

# Compile definitions for non-secure side
target_compile_definitions(platform_ns
    PUBLIC
        DOMAIN_NS=1
        __DOMAIN_NS=1
)

############################ TF-M NS API Library ################################

# TF-M Non-Secure API library
# This library provides the PSA API implementation for the NS side
add_library(tfm_api_ns STATIC)

# TF-M interface source directory
set(INTERFACE_SRC_DIR ${CMAKE_SOURCE_DIR}/interface/src)
set(INTERFACE_INC_DIR ${CMAKE_SOURCE_DIR}/interface/include)

# Add TF-M API sources based on enabled partitions
target_sources(tfm_api_ns
    PUBLIC
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${INTERFACE_SRC_DIR}/tfm_platform_api.c>
        $<$<BOOL:${TFM_PARTITION_PROTECTED_STORAGE}>:${INTERFACE_SRC_DIR}/tfm_ps_api.c>
        $<$<BOOL:${TFM_PARTITION_INTERNAL_TRUSTED_STORAGE}>:${INTERFACE_SRC_DIR}/tfm_its_api.c>
        $<$<BOOL:${TFM_PARTITION_CRYPTO}>:${INTERFACE_SRC_DIR}/tfm_crypto_api.c>
        $<$<BOOL:${TFM_PARTITION_INITIAL_ATTESTATION}>:${INTERFACE_SRC_DIR}/tfm_attest_api.c>
        $<$<BOOL:${TFM_PARTITION_FIRMWARE_UPDATE}>:${INTERFACE_SRC_DIR}/tfm_fwu_api.c>
)

# TrustZone NS API (for secure gateway veneers)
if(CONFIG_TFM_USE_TRUSTZONE)
    target_sources(tfm_api_ns
        PRIVATE
            ${INTERFACE_SRC_DIR}/tfm_tz_psa_ns_api.c
    )

    # Link against secure veneers
    target_link_libraries(tfm_api_ns
        PUBLIC
            ${CMAKE_BINARY_DIR}/secure_fw/s_veneers.o
    )
endif()

# Include TF-M interface headers
target_include_directories(tfm_api_ns
    PUBLIC
        ${INTERFACE_INC_DIR}
        ${INTERFACE_INC_DIR}/crypto_keys
        ${CMAKE_BINARY_DIR}/generated/interface/include
)

# Link against PSA interface and config
target_link_libraries(tfm_api_ns
    PUBLIC
        psa_interface
)

if(TFM_PARTITION_CRYPTO)
    target_link_libraries(tfm_api_ns
        PUBLIC
            psa_crypto_config
    )
endif()
