/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include "r_ipc.h"
#include "bsp_api.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"
#include "tfm_multi_core.h"
#include "tfm_peripherals_def.h"
#include "target_cfg.h"
#include "vector_data.h"
#if defined(TFM_PARTITION_SLIH_TEST) || defined(TFM_PARTITION_FLIH_TEST)
#include "plat_test.h"
#endif
#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
#include "platform_multicore.h"
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */

#if defined(TFM_PARTITION_SLIH_TEST) || defined(TFM_PARTITION_FLIH_TEST)
static struct irq_t timer0_irq = {0};
#endif /* defined(TFM_PARTITION_SLIH_TEST) || defined(TFM_PARTITION_FLIH_TEST) */

#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
static struct irq_t mailbox_irq = {0};
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */

#if defined(TFM_PARTITION_SLIH_TEST) || defined(TFM_PARTITION_FLIH_TEST)

enum tfm_hal_status_t tfm_timer0_irq_init(void *p_pt, const struct irq_load_info_t *p_ildi)
{
    timer0_irq.p_pt   = p_pt;
    timer0_irq.p_ildi = p_ildi;

    tfm_plat_test_secure_timer_init();

    platform_icu_secure_irq_setup(TFM_TIMER0_IRQ, ELC_EVENT_AGT0_INT);

    NVIC_ClearTargetState(TFM_TIMER0_IRQ);
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);

    return TFM_HAL_SUCCESS;
}

IRQ_HANDLER(TFM_TIMER0_IRQ)
{
    R_BSP_IrqStatusClear(TFM_TIMER0_IRQ);

    spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
}

#endif /* defined(TFM_PARTITION_SLIH_TEST) || defined(TFM_PARTITION_FLIH_TEST) */

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
    platform_icu_secure_irq_setup(MAILBOX_IRQ, ELC_EVENT_IPC_IRQ0);

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
