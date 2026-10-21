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

/* For R_AGT_Open() and the generated g_timer0 instance - see tfm_timer0_irq_init(). */
#include "bsp_api.h"
#include "hal_data.h"

static struct irq_t timer0_irq = {0};

/*
 * Reached from __VECTOR_TABLE[16 + VECTOR_NUMBER_AGT0_INT] in startup_ra6m5.c. The secure
 * image uses its OWN vector table, not FSP's generated g_vector_table - that array is
 * still linked, but only so bsp_irq_cfg() can read g_interrupt_event_link_select[] beside
 * it to program the ICU.
 */
void TFM_TIMER0_IRQ_Handler(void)
{
    /*
     * Clear the ICU's latched request HERE, before the SPM dispatches, because this is the
     * last privileged code on the path.
     *
     * TF-M runs FLIH handlers in THREAD mode at the owning partition's privilege - measured,
     * not assumed: at isolation 2 the ARoT FLIH handler faulted on this very write with
     * MMFSR 0x82, MMFAR 0x40006300 (ICU IELSR), "came from secure FW in thread mode". So the
     * handler cannot touch the ICU, and it cannot touch the NVIC either - that is the PPB,
     * privileged-only by architecture. This vector entry is Handler mode and can do both.
     *
     * What is left for the partition's FLIH handler is clearing AGT0's own TUNDF flag, which
     * its TFM_PERIPHERAL_TIMER0 mmio_region does cover. See plat_test.c.
     *
     * Order is ICU first, then the peripheral flag in the handler - the order agt_int_isr()
     * uses. The read-back inside R_BSP_IrqStatusClear() is required, not defensive: see
     * "Operations During an Interrupt" in the ICU section of the hardware manual. The barrier
     * before the NVIC is FSP's own, for the reason its R_BSP_IrqClearPending() has one - the
     * IR clear must land before the pending clear or the request re-asserts.
     */
    R_BSP_IrqStatusClear(TFM_TIMER0_IRQ);
    __DMB();
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);

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

    /*
     * Open AGT0 HERE, not in plat_test.c, and the reason only appears above isolation 1.
     *
     * R_AGT_Open() calls R_BSP_IrqCfgEnable(), which writes the NVIC. The PPB is
     * privileged-only by architecture - no MPU region can grant it - so Open can never be
     * called from an unprivileged partition. This function is the SPM's partition-init
     * hook and runs privileged, which makes it the correct home.
     *
     * g_timer0_ctrl / g_timer0_cfg are the generated instance from the secure e2 project
     * (ra_gen/hal_data.c): AGT0, 16-bit, periodic, PCLKB. Nothing else in the secure image
     * opens it - TF-M never calls g_hal_init().
     *
     * Once open, plat_test.c starts and stops the counter with bare AGTCR writes, which is
     * all an unprivileged ARoT partition can reach. See the header of that file.
     *
     * A failure here is reported rather than ignored: the SPM then fails partition init,
     * which is far easier to diagnose than an IRQ suite that silently never fires.
     */
    if (FSP_SUCCESS != R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg)) {
        return TFM_HAL_ERROR_GENERIC;
    }

    /*
     * R_AGT_Open() re-enables the interrupt in the NVIC via R_BSP_IrqCfgEnable(), undoing
     * the disable above. FF-M 1.1 requires the interrupt to reach the partition disabled -
     * the owning partition enables it through psa_irq_enable() when it is ready, and the
     * SLIH tests depend on that ordering - so disable it again after the open.
     */
    NVIC_DisableIRQ(TFM_TIMER0_IRQ);
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);
    NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);

    return TFM_HAL_SUCCESS;
}
