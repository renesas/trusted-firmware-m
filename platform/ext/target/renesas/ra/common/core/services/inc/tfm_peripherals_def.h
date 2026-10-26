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

extern struct platform_data_t tfm_peripheral_std_uart;
extern struct platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_STD_UART (&tfm_peripheral_std_uart)
#define TFM_PERIPHERAL_TIMER0   (&tfm_peripheral_timer0)

#define TFM_TIMER0_IRQ 0

#ifdef TFM_MULTI_CORE_TOPOLOGY
/*
 * ns_agent_mailbox.yaml declares an IRQ with "source": "MAILBOX_IRQ", so the
 * manifest tool needs this symbol to resolve.
 */
#define MAILBOX_IRQ FSP_INVALID_VECTOR
#endif /* TFM_MULTI_CORE_TOPOLOGY */

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
