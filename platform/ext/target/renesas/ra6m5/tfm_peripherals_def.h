/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY    (1UL << (__NVIC_PRIO_BITS - 2))

struct platform_data_t;

/*
 * Named MMIO regions.
 *
 * TFM_PERIPHERAL_STD_UART exists because tf-m-tests' COMMON tfm_secure_client_service
 * declares it as an mmio_region, so it is required by every regression test rather than by
 * any particular suite. Declaring it also flips CONFIG_TFM_MMIO_REGION_ENABLE on, which is
 * what drags in the PPC contract described in target_cfg.h.
 *
 * It names SCI0, which is what TFM_DRIVER_STDIO refers to (device_cfg.h) when the port is
 * built for UART output rather than RTT. The region is validated but not programmed: RA6
 * attributes peripherals through PSAR, so periph_ppc_bank is PPC_SP_DO_NOT_CONFIGURE and
 * at isolation level 1 no MPU region is created either. It is an allow-list entry, not a
 * protection mechanism - do not read it as one.
 */
extern struct platform_data_t tfm_peripheral_std_uart;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
