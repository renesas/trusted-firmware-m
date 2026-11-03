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
 * RA-specific replacement for platform/ext/common/tfm_interrupts.c. That file provides the
 * same handler and SPM registration hook, but its tfm_timer0_irq_init() calls
 *
 *     NVIC_ClearTargetState(TFM_TIMER0_IRQ);
 *
 * which leaves an RA interrupt in an inconsistent state. RA holds the target state in two
 * places that must agree, NVIC->ITNS and R_CPSCU->ICUSARG, and FSP writes them as a pair
 * in bsp_irq_cfg() (bsp_irq.c). NVIC_ClearTargetState() is plain CMSIS and writes only
 * ITNS, so the NVIC would treat the interrupt as non-secure while ICUSARG still says
 * secure. See target_cfg.c and DECISIONS D061.
 *
 * No replacement call is needed. An RA interrupt is secure because the secure e2 project
 * configures it: bsp_irq_cfg() writes IELSR, ITNS and ICUSARG from the generated
 * g_interrupt_event_link_select[] before main() runs. Attribution is fixed at build time by
 * the project that owns the peripheral, and there is no runtime API to change it.
 *
 * This file is not the private tfm_interrupts.c that D061 deleted from the RA ports.
 */

#include <stdint.h>

#include "tfm_hal_device_header.h"
#include "spm.h"
#include "tfm_hal_interrupt.h"
#include "tfm_peripherals_def.h"
#include "interrupt.h"
#include "load/interrupt_defs.h"

/* For R_AGT_Open() and the generated g_timer0 instance - see tfm_timer0_irq_init(). */
#include "bsp_api.h"
#include "hal_data.h"

static struct irq_t timer0_irq = {0};

/*
 * Reached from __VECTOR_TABLE[16 + VECTOR_NUMBER_AGT0_INT] in startup_ra6m5.c. The secure
 * image uses its own vector table. FSP's generated g_vector_table is still linked, but only
 * so that bsp_irq_cfg() can read g_interrupt_event_link_select[] beside it to program the
 * ICU.
 */
void TFM_TIMER0_IRQ_Handler(void)
{
    /*
     * Clear the ICU latch and the NVIC pending bit here, before the SPM dispatches. This
     * vector entry runs in Handler mode, which is privileged.
     *
     * On RA a peripheral event does not reach the NVIC directly: it latches in
     * R_ICU->IELSR[n].IR and stays asserted until cleared. FSP clears it at the top of every
     * ISR with R_BSP_IrqStatusClear(), including agt_int_isr(), which this handler
     * replaces. Without the clear the interrupt fires once and the FLIH test hangs waiting
     * for the tenth.
     *
     * The partition's FLIH handler cannot always do it. interrupt.c:195-209 calls flih_func()
     * directly, in Handler mode, at isolation level 1 and for PSA RoT partitions above it
     * (tfm_hal_boundary_need_switch() false). For an Application RoT partition at level 2 or 3,
     * such as this fixture, it calls tfm_flih_deprivileged_handling(), which runs the handler
     * in Thread mode at the partition's privilege. There an ICU write faults (MMFSR 0x82, MMFAR
     * 0x40006300, ICU IELSR), and the NVIC is out of reach in the privileged-only PPB. Clearing
     * here is correct in every case. DECISIONS D117.
     *
     * The FLIH handler clears only AGT0's TUNDF flag, which its TFM_PERIPHERAL_TIMER0
     * mmio_region covers; see tfm_plat_test_secure_timer_clear_intr() in plat_test.c.
     *
     * Order is ICU first, then the peripheral flag in the handler, as in agt_int_isr(). The
     * read-back inside R_BSP_IrqStatusClear() is required: see "Operations During an
     * Interrupt" in the ICU section of the hardware manual. The barrier before the NVIC is
     * the one FSP's R_BSP_IrqClearPending() uses: the IR clear must land before the pending
     * clear, or the request re-asserts.
     */
    R_BSP_IrqStatusClear(TFM_TIMER0_IRQ);
    __DMB();
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);

    spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
}

/*
 * The manifest tooling derives this name from the "source" field of the irqs entry in
 * tfm_{flih,slih}_test_service.yaml: TFM_TIMER0_IRQ -> tfm_timer0_irq_init. Do not rename.
 */
enum tfm_hal_status_t tfm_timer0_irq_init(void *p_pt,
                                          const struct irq_load_info_t *p_ildi)
{
    timer0_irq.p_ildi = p_ildi;
    timer0_irq.p_pt = p_pt;

    /* FF-M 1.1: the interrupt must reach the partition disabled. The owning partition enables
     * it with psa_irq_enable() when ready, and the SLIH tests depend on that order. */
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);

    /* BL2 runs with interrupts masked but leaves the NVIC pending bits alone, and the secure
     * image does not clear them at startup. A pending bit that survives to partition init
     * fires the handler as soon as the partition enables the IRQ. */
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);

    /* FSP set the priority from the project's ipl (priority 4 as configured). Reset it to
     * TF-M's quantized default so the tests' assumptions about relative priority hold
     * whatever the configurator was left at. */
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);

    /* No NVIC_ClearTargetState() - see the file header. */

    /*
     * Open AGT0 here, not in plat_test.c. R_AGT_Open() calls R_BSP_IrqCfgEnable(), which
     * writes the NVIC, so it needs privilege. This function is the SPM's partition-init hook
     * and runs privileged. See "Privilege rules" in plat_test.c.
     *
     * g_timer0_ctrl and g_timer0_cfg are the generated instance from the secure e2 project
     * (ra_gen/hal_data.c): AGT0, 16-bit, periodic, PCLKB. Nothing else in the secure image
     * opens it; TF-M never calls g_hal_init().
     *
     * A failure is returned so that the SPM fails partition init, which is easier to
     * diagnose than an IRQ suite that never fires.
     */
    if (FSP_SUCCESS != R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg)) {
        return TFM_HAL_ERROR_GENERIC;
    }

    /*
     * R_AGT_Open() re-enables the interrupt in the NVIC through R_BSP_IrqCfgEnable(),
     * undoing the disable above. Repeat the disable, pending clear and priority so that
     * the interrupt reaches the partition disabled (FF-M 1.1, above).
     */
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);

    return TFM_HAL_SUCCESS;
}
