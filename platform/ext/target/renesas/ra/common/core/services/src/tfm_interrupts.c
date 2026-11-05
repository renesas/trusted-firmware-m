/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Renesas RA8 SPE mailbox interrupt integration.
 */

#include <stddef.h>
#include <stdint.h>

#include "r_ipc.h"
#include "bsp_api.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"
#include "tfm_multi_core.h"
#include "tfm_peripherals_def.h"
#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
#include "platform_multicore.h"
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */

static struct irq_t timer0_irq = {0};

#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
static struct irq_t mailbox_irq = {0};
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */

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

#ifdef TFM_PARTITION_NS_AGENT_MAILBOX

/* Initialize the Secure runtime mailbox interrupt. */
enum tfm_hal_status_t mailbox_irq_init(void *p_pt,
                                       const struct irq_load_info_t *p_ildi)
{
    if ((p_pt == NULL) ||
        (p_ildi == NULL) ||
        (p_ildi->source != (uint32_t) MAILBOX_IRQ)) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    /*
     * Associate the remote NSPE client ID range with the mailbox interrupt
     * source so that requests are routed to the correct NS Agent.
     */
    if (tfm_multi_core_register_client_id_range(
            CLIENT_ID_OWNER_MAGIC,
            p_ildi->source) != 0) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    mailbox_irq.p_pt = p_pt;
    mailbox_irq.p_ildi = p_ildi;
    icu_secure_irq_setup(MAILBOX_IRQ, ELC_EVENT_IPC_IRQ0);

    NVIC_ClearTargetState(MAILBOX_IRQ);
    NVIC_SetPriority(MAILBOX_IRQ, DEFAULT_IRQ_PRIORITY);
    NVIC_DisableIRQ(MAILBOX_IRQ);

    return TFM_HAL_SUCCESS;
}

/* Handle a runtime mailbox notification through the FSP IPC ISR. */
IRQ_HANDLER(MAILBOX_IRQ)
{
    /*
     * The patched FSP ipc_isr() must tolerate a NULL callback. It clears
     * the ICU request and acknowledges the received IPC event.
     */
    ipc_isr();

    /* The FIFO is only used during init; keep a stale word from holding RDY. */
    platform_mailbox_fifo_flush();

    if ((mailbox_irq.p_pt == NULL) ||
        (mailbox_irq.p_ildi == NULL)) {
        tfm_core_panic();
    }

    spm_handle_interrupt(mailbox_irq.p_pt,
                         mailbox_irq.p_ildi);
}

#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */
