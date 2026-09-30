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
 * built for UART output rather than RTT. The region is validated but not programmed: RA parts
 * attribute peripherals through PSAR/PSCU, so periph_ppc_bank is PPC_SP_DO_NOT_CONFIGURE and
 * at isolation level 1 no MPU region is created either. It is an allow-list entry, not a
 * protection mechanism - do not read it as one.
 */
extern struct platform_data_t tfm_peripheral_std_uart;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)

/*
 * AGT0, the secure test timer for the FLIH/SLIH interrupt suites.
 *
 * tf-m-tests' tfm_{flih,slih}_test_service.yaml name TFM_PERIPHERAL_TIMER0 as an
 * mmio_region and TFM_TIMER0_IRQ as an irq source, both verbatim, so neither may be
 * renamed. Defined unconditionally: the struct is 16 bytes and an unclaimed allow-list
 * entry costs nothing, whereas making them conditional would need the
 * PLATFORM_*_IRQ_TEST_SUPPORT flags plumbed through as compile definitions.
 *
 * TFM_TIMER0_IRQ IS A LITERAL HERE, AND THAT IS A HAZARD. The real number lives in the
 * generated ra_gen/vector_data.h as VECTOR_NUMBER_AGT0_INT, but this header is included
 * from translation units that do not have that header on their include path. plat_test.c
 * includes both and #errors if they disagree, so a RASC renumbering breaks the build
 * loudly instead of routing the interrupt to Default_Handler.
 *
 * RASC assigns ICU slots densely in the order interrupts were added to the project, so
 * adding any interrupt ahead of the AGT renumbers this one.
 */
extern struct platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_TIMER0    (&tfm_peripheral_timer0)
#define TFM_TIMER0_IRQ           ((IRQn_Type) 0)

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
