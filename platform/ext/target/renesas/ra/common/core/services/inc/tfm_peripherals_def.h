/*
 * Copyright (c) 2018-2021, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#include "bsp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY (1UL << (__NVIC_PRIO_BITS - 2))

/* Forward declaration - the full struct is in common_target_cfg.h after ppc_bank_t is defined */
struct platform_data_t;

/* Concatenate two tokens after expanding them */
#define RA_PERIPHERAL_CAT(a, b)  RA_PERIPHERAL_CAT_(a, b)
#define RA_PERIPHERAL_CAT_(a, b) a##b

/*
 * 1 when the FSP switch RENESAS_RA_PSAR_NS_<x> (0 or 1, from the build) makes the
 * peripheral secure, else 0. The result is a plain token, so the preprocessor can
 * select on it (RA_PERIPHERAL_ENTRY in tfm_peripherals_def.c).
 */
#define RA_PSAR_SECURE(x)       RA_PSAR_SECURE_(RENESAS_RA_PSAR_NS_##x)
#define RA_PSAR_SECURE_(ns)     RA_PERIPHERAL_CAT(RA_SECURE_OF_NS_, ns)
#define RA_SECURE_OF_NS_0       1
#define RA_SECURE_OF_NS_1       0

/* 1 when both peripherals are secure, else 0 */
#define RA_PSAR_SECURE_AND(x, y) \
    RA_PERIPHERAL_CAT(RA_AND_, RA_PERIPHERAL_CAT(RA_PSAR_SECURE(x), RA_PSAR_SECURE(y)))
#define RA_AND_11               1
#define RA_AND_10               0
#define RA_AND_01               0
#define RA_AND_00               0

/*
 * Platform data of the secure peripherals and their manifest names
 * (TFM_PERIPHERAL_<NAME>). The list RA_PERIPHERALS(X, SEP) is per SoC: <soc>/partition/
 * tfm_peripherals_list.h. A peripheral that is not secure is not in the list of
 * allowed MMIO (partition_named_mmio_list is built at compile time from the
 * secure ones only), so a Secure Partition that names it fails to load.
 */
#include "tfm_peripherals_list.h"

/* Names used by the regression tests: the STDIO UART is SCI8, the timer is AGT0 */
#define TFM_PERIPHERAL_STD_UART TFM_PERIPHERAL_SCI8
#define TFM_PERIPHERAL_TIMER0   TFM_PERIPHERAL_AGT0

/* Platform data of each entry of RA_PERIPHERALS(X, SEP) */
#define RA_PERIPHERAL_DECLARE(name, ...) \
    extern struct platform_data_t tfm_peripheral_##name
RA_PERIPHERALS(RA_PERIPHERAL_DECLARE, ;)

#define TFM_TIMER0_IRQ 0
#define MAILBOX_IRQ 1

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
