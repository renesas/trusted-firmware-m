/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_peripherals_def.h"
#include "common_target_cfg.h"

/*
 * RA6 has no MPC/PPC, so every platform_data_t here carries the PPC_SP_DO_NOT_CONFIGURE
 * sentinel and tfm_hal_bind_boundary() skips the peripheral-programming branch. See
 * target_cfg.h for why the type exists.
 *
 * SCI0 (R_SCI0) is at 0x40118000 on RA6M5, with 0x100 bytes of register space. It is the
 * "std UART" because TFM_DRIVER_STDIO names Driver_USART0 (device_cfg.h).
 */
struct platform_data_t tfm_peripheral_std_uart = {
    .periph_start     = 0x40118000,
    .periph_limit     = 0x401180FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

/*
 * AGT0 (R_AGT0) is at 0x400E8000 on RA6M5, with 0x100 bytes of register space; AGT1 starts
 * at 0x400E8100. The 32-bit (AGTW) and 16-bit (AGT) register views share these addresses,
 * so the region is the same for either; the secure project selects the 16-bit view.
 *
 * Claimed by tf-m-tests' FLIH and SLIH test services as TFM_PERIPHERAL_TIMER0.
 */
struct platform_data_t tfm_peripheral_timer0 = {
    .periph_start     = 0x400E8000,
    .periph_limit     = 0x400E80FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

/*
 * Allow-list of named MMIO regions a partition may claim. A claim not listed here fails
 * tfm_hal_bind_boundary() with TFM_HAL_ERROR_GENERIC, which the SPM turns into a panic at
 * partition init, so a new mmio_region in a manifest needs an entry here.
 */
const uintptr_t partition_named_mmio_list[] = {
    (uintptr_t)TFM_PERIPHERAL_STD_UART,
    (uintptr_t)TFM_PERIPHERAL_TIMER0,
};

const size_t partition_named_mmio_list_size =
    sizeof(partition_named_mmio_list) / sizeof(partition_named_mmio_list[0]);

void get_partition_named_mmio_list(const uintptr_t** list, size_t* length)
{
    *list = partition_named_mmio_list;
    *length = partition_named_mmio_list_size;
}
