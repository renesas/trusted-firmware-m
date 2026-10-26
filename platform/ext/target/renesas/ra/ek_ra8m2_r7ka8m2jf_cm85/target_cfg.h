/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TARGET_CFG_H
#define TARGET_CFG_H

#include <stdint.h>
#include <tfm_peripherals_def.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFM_DRIVER_STDIO    Driver_USART8
#if (RA_CORE == RA_CM33)
#define NS_DRIVER_STDIO     Driver_USART9
#elif (RA_CORE == RA_CM85)
#define NS_DRIVER_STDIO     Driver_USART0
#endif

/* PPC bank type */
typedef uint32_t ppc_bank_t;

/* Value to indicate that a peripheral should not be configured via PPC */
#define PPC_SP_DO_NOT_CONFIGURE ((ppc_bank_t)-1)

#ifdef __cplusplus
}
#endif

#endif /* TARGET_CFG_H */
