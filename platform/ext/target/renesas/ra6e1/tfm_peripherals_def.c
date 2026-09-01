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
 * sentinel and tfm_hal_bind_boundary() skips the peripheral-programming branch entirely.
 * See target_cfg.h for why the type exists at all.
 *
 * SCI0 base is 0x40118000 on RA6E1 (R_SCI0), 0x100 of register space. Taken as the "std
 * UART" because TFM_DRIVER_STDIO names Driver_USART0 (device_cfg.h).
 */
struct platform_data_t tfm_peripheral_std_uart = {
    .periph_start     = 0x40118000,
    .periph_limit     = 0x401180FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

/*
 * Allow-list of named MMIO a partition may claim. A partition asking for anything not in
 * here fails tfm_hal_bind_boundary() with TFM_HAL_ERROR_GENERIC, which the SPM turns into
 * a panic during partition init - so an addition to a manifest needs an addition here.
 */
const uintptr_t partition_named_mmio_list[] = {
    (uintptr_t)TFM_PERIPHERAL_STD_UART,
};

const size_t partition_named_mmio_list_size =
    sizeof(partition_named_mmio_list) / sizeof(partition_named_mmio_list[0]);

void get_partition_named_mmio_list(const uintptr_t** list, size_t* length)
{
    *list = partition_named_mmio_list;
    *length = partition_named_mmio_list_size;
}
