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
 * RA parts have no MPC/PPC, so every platform_data_t here carries the PPC_SP_DO_NOT_CONFIGURE
 * sentinel and tfm_hal_bind_boundary() skips the peripheral-programming branch entirely.
 * See target_cfg.h for why the type exists at all.
 *
 * SCI0 base is 0x40358000 on RA8M2 - the device header defines R_SCI0_BASE as
 * (0x40358000UL + BASE_NS_OFFSET), and R_SCI1_BASE is 0x40358100, so the block is 0x100.
 * Taken as the "std UART" because TFM_DRIVER_STDIO names Driver_USART0 (device_cfg.h).
 *
 * This read 0x40118000 until 2026-10-01. That is RA6M5's SCI0, carried over wholesale when
 * this file was derived from ra6m5/tfm_peripherals_def.c, with only the comment's part name
 * edited. It appears nowhere in R7KA8M2JF_core0.h. It was latent because the port defaults
 * to RA8M2_STDOUT_RTT and isolation 1, where no MPU region is created from this entry - at
 * isolation 2 or 3 tfm_hal_bind_boundary() would have mapped 0x40118000, which decodes to
 * nothing, and left the real SCI0 unmapped. The sibling AGT0 entry below was checked against
 * the device header when it was added and is correct; this one never was.
 */
struct platform_data_t tfm_peripheral_std_uart = {
    .periph_start     = 0x40358000,
    .periph_limit     = 0x403580FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

/*
 * Allow-list of named MMIO a partition may claim. A partition asking for anything not in
 * here fails tfm_hal_bind_boundary() with TFM_HAL_ERROR_GENERIC, which the SPM turns into
 * a panic during partition init - so an addition to a manifest needs an addition here.
 */
/*
 * AGT0 (R_AGT0) is at 0x40221000 on RA8M2, 0x100 of register space - AGT1 starts at
 * 0x40221100. NOT RA6M5's 0x400E8000.
 *
 * The device header defines R_AGTX0_BASE as (0x40221000UL + BASE_NS_OFFSET), and
 * BASE_NS_OFFSET is 0 for a secure build - this part aliases peripherals the way it
 * aliases memory. The secure address is the one that belongs here.
 *
 * The block is a union of a 32-bit (AGTW) and 16-bit (AGT) register view over the same
 * addresses, so the region is the same either way; BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT is 0
 * on this part, so it is the 16-bit view.
 *
 * Claimed by tf-m-tests' FLIH and SLIH test services as TFM_PERIPHERAL_TIMER0.
 */
struct platform_data_t tfm_peripheral_timer0 = {
    .periph_start     = 0x40221000,
    .periph_limit     = 0x402210FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

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
