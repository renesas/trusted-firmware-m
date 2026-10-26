/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "tfm_hal_device_header.h"
#include "spm.h"
#include "tfm_hal_interrupt.h"
#include "tfm_peripherals_def.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"
#include "bsp_api.h"

static struct irq_t timer0_irq = {0};

/*
 * The ICU secure interrupt initialization for Renesas RA MCUs:
 *  - Set the IRQ as secure in the ICU security attribution register (ICUSARG).
 *  - Route the ELC event to the IRQ vector (IELSR) and select it (INTSELR).
 */
static void icu_secure_irq_setup(IRQn_Type irq, uint32_t elc_event)
{
    const uint32_t index = (uint32_t) irq / 32U;
    const uint32_t bit   = (uint32_t) irq % 32U;
    uint32_t volatile *p_icusarg = &R_CPSCU->ICUSARG;

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);
    p_icusarg[index] &= ~(1U << bit);
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);
    R_ICU->IELSR[irq] = elc_event;
    R_ICU->INTSELR[elc_event >> 5U] &= ~(1U << (elc_event % 32U));
}

enum tfm_hal_status_t tfm_timer0_irq_init(void *p_pt, const struct irq_load_info_t *p_ildi)
{
    timer0_irq.p_pt   = p_pt;
    timer0_irq.p_ildi = p_ildi;

    icu_secure_irq_setup(TFM_TIMER0_IRQ, ELC_EVENT_AGT0_INT);

    NVIC_ClearTargetState(TFM_TIMER0_IRQ);
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);

    return TFM_HAL_SUCCESS;
}

IRQ_HANDLER(TFM_TIMER0_IRQ)
{
    spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
}
