/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * TIMER0 interrupt fixture for the FLIH/SLIH test suites.
 *
 * This is an RA-specific replacement for platform/ext/common/tfm_interrupts.c, which
 * cannot be used on this family. That file is otherwise exactly what is needed - the
 * handler and the SPM registration hook - but its tfm_timer0_irq_init() calls
 *
 *     NVIC_ClearTargetState(TFM_TIMER0_IRQ);
 *
 * and on RA that single call leaves the interrupt in an INCONSISTENT state. Target state
 * is held in two places that must agree: NVIC->ITNS and R_CPSCU->ICUSARG. FSP writes them
 * as a pair in bsp_irq_cfg() (bsp_irq.c). NVIC_ClearTargetState() is plain CMSIS and knows
 * only about ITNS, so it would mark the interrupt non-secure to the NVIC while ICUSARG
 * still says secure. See target_cfg.c and DECISIONS D061.
 *
 * Nothing needs to replace it. On RA an interrupt is secure because the SECURE e2 project
 * configured it: bsp_irq_cfg() has already written IELSR, ITNS and ICUSARG from the
 * generated g_interrupt_event_link_select[] before main() runs. Attribution is a build-time
 * property of which project owns the peripheral, and there is no runtime API to change it.
 *
 * The file is named for what it is. The four RA ports used to carry a private
 * tfm_interrupts.c that was a byte-identical copy of the real HAL (ext/common/tfm_hal_nvic.c)
 * plus an invented tfm_hal_irq_set_priority(); those were deleted in D061 and this is not a
 * reintroduction of them.
 */

#include <stdint.h>

#include "tfm_hal_device_header.h"
#include "spm.h"
#include "tfm_hal_interrupt.h"
#include "tfm_peripherals_def.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"

static struct irq_t timer0_irq = {0};

/*
 * Reached from __VECTOR_TABLE[16 + VECTOR_NUMBER_AGT0_INT] in startup_ra6m5.c. The secure
 * image uses its OWN vector table, not FSP's generated g_vector_table - that array is
 * still linked, but only so bsp_irq_cfg() can read g_interrupt_event_link_select[] beside
 * it to program the ICU.
 */
void TFM_TIMER0_IRQ_Handler(void)
{
    spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
}

/*
 * Name is derived by the manifest tooling from the "source" field of the irqs entry in
 * tfm_{flih,slih}_test_service.yaml: TFM_TIMER0_IRQ -> tfm_timer0_irq_init. Do not rename.
 */
enum tfm_hal_status_t tfm_timer0_irq_init(void *p_pt,
                                          const struct irq_load_info_t *p_ildi)
{
    timer0_irq.p_ildi = p_ildi;
    timer0_irq.p_pt = p_pt;

    /* FF-M 1.1: the interrupt must arrive disabled. The owning partition enables it
     * through psa_irq_enable() when it is ready, and the SLIH tests depend on that
     * ordering. */
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);

    /* BL2 runs with interrupts masked but leaves the NVIC pending bits alone, and the
     * secure image does not clear them at startup. A pending bit surviving into partition
     * init fires the handler the instant the partition enables the IRQ. */
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);

    /* FSP set the priority from the project's ipl (priority 4 as configured). Re-set it to
     * TF-M's quantized default so the test's expectations about relative priority hold
     * regardless of what the configurator was left at. */
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);

    /* No NVIC_ClearTargetState() - see the file header. */

    return TFM_HAL_SUCCESS;
}
