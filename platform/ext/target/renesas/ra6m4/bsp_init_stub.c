/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub implementation of FSP BSP initialization structures
 * TF-M handles its own startup, so we provide minimal stubs here
 */

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

/* Stub initialization data structure */
const bsp_init_info_t g_init_info =
{
    .zero_count  = 0,
    .p_zero_list = zero_list,
    .copy_count  = 0,
    .p_copy_list = copy_list,
    .nocache_count  = 0,
    .p_nocache_list = nocache_list
};

/* Stub main stack - TF-M manages its own stack from linker script */
#include <stdint.h>
#ifndef BSP_CFG_STACK_MAIN_BYTES
#define BSP_CFG_STACK_MAIN_BYTES 0x400  /* 1KB dummy stack */
#endif

uint8_t g_main_stack[BSP_CFG_STACK_MAIN_BYTES] __attribute__((section(".noinit")));
