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
 * sentinel and tfm_hal_bind_boundary() skips the peripheral-programming branch. See
 * target_cfg.h for why the type exists.
 *
 * SCI0 (R_SCI0) is at 0x40358000 on RA8M2, with 0x100 bytes of register space: the device
 * header defines R_SCI0_BASE as (0x40358000UL + BASE_NS_OFFSET), and R_SCI1_BASE is
 * 0x40358100. It is the "std UART" because TFM_DRIVER_STDIO names Driver_USART0
 * (device_cfg.h). RA6M5's SCI0 address does not apply here (D077).
 */
struct platform_data_t tfm_peripheral_std_uart = {
    .periph_start     = 0x40358000,
    .periph_limit     = 0x403580FF,
    .periph_ppc_bank  = PPC_SP_DO_NOT_CONFIGURE,
    .periph_ppc_mask  = 0,
};

/*
 * AGT0 (R_AGT0) is at 0x40221000 on RA8M2, with 0x100 bytes of register space; AGT1 starts
 * at 0x40221100. RA6M5's 0x400E8000 does not apply here.
 *
 * The device header defines R_AGTX0_BASE as (0x40221000UL + BASE_NS_OFFSET), and
 * BASE_NS_OFFSET is 0 in a secure build: this part aliases peripherals as it aliases memory.
 * The secure address belongs here.
 *
 * The 32-bit (AGTW) and 16-bit (AGT) register views share these addresses, so the region is
 * the same for either; BSP_FEATURE_AGT_AGTW_CHANNEL_COUNT is 0 on this part, so it is the
 * 16-bit view.
 *
 * Claimed by tf-m-tests' FLIH and SLIH test services as TFM_PERIPHERAL_TIMER0.
 */
struct platform_data_t tfm_peripheral_timer0 = {
    .periph_start     = 0x40221000,
    .periph_limit     = 0x402210FF,
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
