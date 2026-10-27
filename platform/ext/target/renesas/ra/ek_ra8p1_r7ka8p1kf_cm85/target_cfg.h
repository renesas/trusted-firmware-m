/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TARGET_CFG_H
#define TARGET_CFG_H

#include <stdint.h>
#include <tfm_peripherals_def.h>
#include "RTE_Stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Stdio drivers, from the channels selected in RTE_Device.h (see RTE_Stdio.h) */
#define TFM_DRIVER_STDIO     RA_USART_DRIVER(RTE_S_STDIO_USART)
#define NS_DRIVER_STDIO      RA_USART_DRIVER(RTE_NS_STDIO_USART)

/* PPC bank type */
typedef uint32_t ppc_bank_t;

/* Value to indicate that a peripheral should not be configured via PPC */
#define PPC_SP_DO_NOT_CONFIGURE ((ppc_bank_t)-1)

#ifdef __cplusplus
}
#endif

#endif /* TARGET_CFG_H */
