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
#include "ppar_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Stdio drivers, from the channels selected in RTE_Device.h (see RTE_Stdio.h) */
#define TFM_DRIVER_STDIO    RA_USART_DRIVER(RTE_S_STDIO_USART)
#define NS_DRIVER_STDIO     RA_USART_DRIVER(RTE_NS_STDIO_USART)

/**
 * \brief Defines the word offsets of Slave Peripheral Protection Controller
 *        Registers
 */
typedef enum ppc_bank_e {
    PPC_SP_DO_NOT_CONFIGURE = -1,
    PPC_SP_PERIPH_REG_B = 0,
    PPC_SP_PERIPH_REG_C,
    PPC_SP_PERIPH_REG_D,
    PPC_SP_PERIPH_REG_E,
} ppc_bank_t;

/**
 * \brief Secure interrupt setup in the ICU, for platform code only.
 *
 * \param[in] irq        NVIC IRQ number (ICU vector slot).
 * \param[in] elc_event  ELC event to route to the IRQ.
 */
void platform_icu_secure_irq_setup(IRQn_Type irq, uint32_t elc_event);

#ifdef __cplusplus
}
#endif

#endif /* TARGET_CFG_H */
