/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub implementation of FSP BSP initialization structures
 * TF-M handles its own startup, so we provide minimal stubs here
 */

/* stdint.h first: bsp_linker_info.h declares uint32_t-typed tables and does not include
 * it itself. This file used to rely on platform_s pulling stdint in through some other
 * path; it is compiled into fsp_bsp now, so make it self-sufficient. */
#include <stdint.h>
#include "bsp_linker_info.h"

/* Empty initialization tables - TF-M handles startup */
static const bsp_init_zero_info_t zero_list[] = {
    /* Empty - no zero initialization needed */
};

static const bsp_init_copy_info_t copy_list[] = {
    /* Empty - no copy initialization needed */
};

static const bsp_mpu_nocache_info_t nocache_list[] = {
    /* Empty - no nocache regions */
};

/* Weak stub initialization data structure - can be overridden by real g_init_info */
__attribute__((weak)) const bsp_init_info_t g_init_info =
{
    .zero_count  = 0,
    .p_zero_list = zero_list,
    .copy_count  = 0,
    .p_copy_list = copy_list,
    .nocache_count  = 0,
    .p_nocache_list = nocache_list
};

/* Stub main stack - only for secure side (NS uses FSP startup.c) */
#include <stdint.h>
#ifndef BSP_CFG_STACK_MAIN_BYTES
#define BSP_CFG_STACK_MAIN_BYTES 0x400  /* 1KB dummy stack */
#endif

/* Weak g_main_stack - allows NS startup.c to override */
#if !defined(DOMAIN_NS) && !defined(__DOMAIN_NS)
__attribute__((weak)) uint8_t g_main_stack[BSP_CFG_STACK_MAIN_BYTES] __attribute__((section(".noinit")));
#endif

/* Forward declarations for weak stubs */
void g_hal_init(void);
uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize);
void vPortFreeSecureContext(uint32_t *pulSecureContext);

/* Weak stub for HAL init - FSP applications provide their own implementation */
__attribute__((weak)) void g_hal_init(void) {
    /* Empty - FSP applications provide their own implementation */
}

/* Weak stubs for FreeRTOS TrustZone functions - needed for NS with FreeRTOS */
__attribute__((weak)) uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize) {
    (void)ulSecureStackSize;
    return 0;  /* Stub - real implementation in FreeRTOS port */
}

__attribute__((weak)) void vPortFreeSecureContext(uint32_t *pulSecureContext) {
    (void)pulSecureContext;
    /* Stub - real implementation in FreeRTOS port */
}
