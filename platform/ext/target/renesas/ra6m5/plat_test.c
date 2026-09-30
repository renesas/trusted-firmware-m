/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Secure test timer for the FLIH/SLIH interrupt suites.
 *
 * tf-m-tests' suites/spm/irq/service/tfm_{flih,slih}_test_service.c call these three
 * functions to raise and acknowledge a real secure interrupt. Only the SECURE three are
 * needed: nothing in tests_reg references tfm_plat_test_non_secure_timer_start()/_stop(),
 * so those are absent here rather than stubbed.
 *
 * --------------------------------------------------------------------------------------
 * THIS USES THE FSP DRIVER, DELIBERATELY.
 *
 * An earlier revision drove the AGT registers directly, on the reasoning that R_AGT_Open()
 * installs a callback that can never fire (TF-M owns the vector) and that tracking the
 * open state was not worth it. That was wrong twice over: the hand-rolled sequence missed
 * the AGTMR2-before-AGTMR1 ordering and the compare-match parking R_AGT_Open() does, and
 * the driver's register work is proven on this silicon in a way a re-derivation is not.
 * The unfireable callback costs nothing.
 *
 * What the driver does NOT do, and this file therefore must:
 *
 *   - Clear the ICU's latched request. On RA the peripheral event does not reach the NVIC
 *     directly; it latches in R_ICU->IELSR[n].IR and stays asserted until cleared. FSP
 *     does this at the top of every ISR via R_BSP_IrqStatusClear(), including
 *     agt_int_isr() - which TF-M replaces with TFM_TIMER0_IRQ_Handler, so the obligation
 *     moves here. Without it the interrupt fires exactly once and the FLIH test hangs
 *     waiting for the tenth.
 *
 * The ICU event link itself (IELSR) is programmed once by FSP's bsp_irq_cfg() from the
 * generated event-link table - see the anchor note in target_cfg.c for why getting that
 * table linked at all took some doing.
 * --------------------------------------------------------------------------------------
 */

#include <stdbool.h>
#include <stdint.h>

#include "bsp_api.h"
#include "hal_data.h"
#include "tfm_hal_device_header.h"
#include "tfm_peripherals_def.h"
#include "tfm_plat_test.h"
#include "vector_data.h"

/*
 * The IRQ number MUST come from the generated vector table, never from a literal. RASC
 * assigns ICU slots densely in the order interrupts were added to the project, so adding
 * any earlier interrupt renumbers this one. tfm_peripherals_def.h has to carry a constant
 * (it is included where vector_data.h is not), so the two are tied together here.
 */
#if !defined(VECTOR_NUMBER_AGT0_INT)
  #error "RA6M5: AGT0 has no interrupt in the secure e2 project, so the FLIH/SLIH suites cannot raise one. In the SECURE project set the AGT (g_timer0) Interrupt Priority to a real priority instead of Disabled, regenerate, and rebuild. Configuring it in the secure project is also what makes the interrupt secure - see DECISIONS D061."
#endif

/*
 * _Static_assert, not #if: both macros expand to ((IRQn_Type) n), and a cast is not a
 * valid preprocessor constant expression - the #if form fails to parse rather than
 * comparing anything. Cost one IAR Pe018 to learn.
 */
_Static_assert((int)TFM_TIMER0_IRQ == (int)VECTOR_NUMBER_AGT0_INT,
               "RA6M5: TFM_TIMER0_IRQ in tfm_peripherals_def.h disagrees with the "
               "generated VECTOR_NUMBER_AGT0_INT. RASC has renumbered the ICU slots - "
               "update the constant in tfm_peripherals_def.h to match "
               "ra_gen/vector_data.h, and the vector slot in startup_ra6m5.c with it.");

/*
 * R_AGT_Open() returns FSP_ERR_ALREADY_OPEN on a second call, and the test HAL starts and
 * stops the timer once per test case, so the open is done once and latched here.
 *
 * Not a re-entrancy hazard: the SPM calls these from the owning partition's context, one
 * at a time.
 */
static bool plat_test_timer_opened = false;

/*
 * Clear the ICU's latched request for this vector.
 *
 * The register read-back inside R_BSP_IrqStatusClear() is required, not defensive - see
 * "Operations During an Interrupt" in the ICU section of the hardware manual. The barrier
 * before touching the NVIC is FSP's, for the same reason its R_BSP_IrqClearPending() has
 * one: the IR clear must land before the pending clear, or the request re-asserts.
 */
static void plat_test_icu_status_clear(void)
{
    R_BSP_IrqStatusClear(TFM_TIMER0_IRQ);
    __DMB();
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);
}

void tfm_plat_test_secure_timer_start(void)
{
    if (!plat_test_timer_opened) {
        /* g_timer0_ctrl / g_timer0_cfg are the generated instance from the secure e2
         * project (ra_gen/hal_data.c): AGT0, 16-bit, periodic, PCLKB. Nothing else in the
         * secure image opens it - TF-M never calls g_hal_init(). */
        if (FSP_SUCCESS != R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg)) {
            /* Leave the timer shut. The test then fails rather than running against a
             * timer in an unknown state, and a failure is diagnosable where a hang is not. */
            return;
        }
        plat_test_timer_opened = true;
    }

    /* A request may already be latched in the ICU from a previous test case. Clear it
     * before the count starts, or the first underflow appears to arrive instantly. */
    plat_test_icu_status_clear();

    (void)R_AGT_Start(&g_timer0_ctrl);
}

void tfm_plat_test_secure_timer_clear_intr(void)
{
    /*
     * ICU latch first, then the peripheral flag - the order agt_int_isr() uses
     * (R_BSP_IrqStatusClear() at entry, AGTCR flags at exit). Clearing only the peripheral
     * flag is why this interrupt fired exactly once.
     */
    plat_test_icu_status_clear();

    /*
     * There is no FSP API for "clear the underflow flag" - agt_int_isr() does it inline,
     * and that ISR is not in the vector here. Read-modify-write is correct: the timer is
     * running, so TSTART must be preserved, and a flag that reads as 1 is written back as
     * 1, i.e. left alone. These flags clear by writing 0. TSTOP is write-only and reads as
     * 0, so the RMW does not stop the counter.
     */
    R_AGT0->AGT16.CTRL.AGTCR_b.TUNDF = 0;
}

void tfm_plat_test_secure_timer_stop(void)
{
    if (plat_test_timer_opened) {
        (void)R_AGT_Stop(&g_timer0_ctrl);
    }
    plat_test_icu_status_clear();
}
