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
 * tf-m-tests' common tfm_secure_client_service declares TFM_PERIPHERAL_STD_UART as an
 * mmio_region, so every regression build needs it, whatever the suite. Declaring it also
 * turns on CONFIG_TFM_MMIO_REGION_ENABLE, which brings in the PPC contract described in
 * target_cfg.h.
 *
 * It names SCI0, which TFM_DRIVER_STDIO refers to (device_cfg.h) when the port is built for
 * UART output rather than RTT. The region is validated but not programmed: RA parts
 * attribute peripherals through PSAR/PSCU, so periph_ppc_bank is PPC_SP_DO_NOT_CONFIGURE, and
 * at isolation level 1 no MPU region is created either. It is an allow-list entry, not a
 * protection mechanism.
 */
extern struct platform_data_t tfm_peripheral_std_uart;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)

/*
 * AGT0, the secure test timer for the FLIH/SLIH interrupt suites.
 *
 * tf-m-tests' tfm_{flih,slih}_test_service.yaml name TFM_PERIPHERAL_TIMER0 as an
 * mmio_region and TFM_TIMER0_IRQ as an irq source, verbatim, so neither may be renamed.
 * Both are defined unconditionally: the struct is 16 bytes and an unclaimed allow-list
 * entry costs nothing, whereas a condition would need the PLATFORM_*_IRQ_TEST_SUPPORT flags
 * passed through as compile definitions.
 *
 * WARNING: TFM_TIMER0_IRQ is a literal. It must equal VECTOR_NUMBER_AGT0_INT from the
 * generated ra_gen/vector_data.h, which is not on the include path of every file that
 * includes this header. A _Static_assert in plat_test.c fails the build if the two
 * disagree; without it, a RASC renumbering would route the interrupt to Default_Handler.
 * See plat_test.c for why RASC renumbers.
 */
extern struct platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_TIMER0    (&tfm_peripheral_timer0)
#define TFM_TIMER0_IRQ           ((IRQn_Type) 0)

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
