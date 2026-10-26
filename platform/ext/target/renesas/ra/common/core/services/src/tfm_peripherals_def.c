/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <stddef.h>
#include <stdint.h>

#include "common_target_cfg.h"
#include "target_cfg.h"
#include "tfm_hal_device_header.h"
#include "tfm_peripherals_def.h"
#include "tfm_utils.h"

/* Platform data for the standard UART (SCI8) */
struct platform_data_t tfm_peripheral_std_uart = {
    .periph_start = R_SCI_B8_BASE,
    .periph_limit = R_SCI_B8_BASE + 0xFF,
    .periph_ppc_bank = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask = 0,
};

/* Platform data for the AGT0 timer used by the SLIH/FLIH tests (one 0x100-byte AGT channel block) */
struct platform_data_t tfm_peripheral_timer0 = {
    .periph_start = R_AGTX0_BASE,
    .periph_limit = R_AGTX0_BASE + 0xFF,
    .periph_ppc_bank = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask = 0,
};

/* Allowed named MMIO of this platform */
static const uintptr_t partition_named_mmio_list[] = {
    (uintptr_t)TFM_PERIPHERAL_STD_UART,
    (uintptr_t)TFM_PERIPHERAL_TIMER0,
};

void get_partition_named_mmio_list(const uintptr_t **list, size_t *length)
{
    *list = partition_named_mmio_list;
    *length = ARRAY_SIZE(partition_named_mmio_list);
}
