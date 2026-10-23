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
 * THIS USES THE FSP DRIVER, BY DESIGN.
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

#include <stdint.h>

#include "bsp_api.h"
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
  #error "RA8M2: AGT0 has no interrupt in the secure e2 project, so the FLIH/SLIH suites cannot raise one. In the SECURE project set the AGT (g_timer0) Interrupt Priority to a real priority instead of Disabled, regenerate, and rebuild. Configuring it in the secure project is also what makes the interrupt secure - see DECISIONS D061."
#endif

/*
 * _Static_assert, not #if: both macros expand to ((IRQn_Type) n), and a cast is not a
 * valid preprocessor constant expression - the #if form fails to parse rather than
 * comparing anything. Cost one IAR Pe018 to learn.
 */
_Static_assert((int)TFM_TIMER0_IRQ == (int)VECTOR_NUMBER_AGT0_INT,
               "RA8M2: TFM_TIMER0_IRQ in tfm_peripherals_def.h disagrees with the "
               "generated VECTOR_NUMBER_AGT0_INT. RASC has renumbered the ICU slots - "
               "update the constant in tfm_peripherals_def.h to match "
               "ra_gen/vector_data.h, and the vector slot in startup_ra8m2.c with it.");

/*
 * PRIVILEGE RULES FOR THIS FILE - read before changing anything here.
 *
 * This file is built into tfm_sprt, not platform_s, and start()/stop() below are called by
 * the FLIH and SLIH test services. Those are APPLICATION-ROT partitions, so from isolation
 * 2 they run UNPRIVILEGED. That rules out three things the obvious implementation uses:
 *
 *   1. ANY static in this file. tfm_sprt's code is placed in the unprivileged code region
 *      but its .bss is not - it lands in PT_PSA_ROT_DATA, which is ARM_MPU_PRIVILEGED.
 *      A "static bool timer_opened" here faulted with MMFSR 0x82 (DACCVIOL) at
 *      MMFAR 0x20011493. SPRT has to be stateless; an521's sprt timer driver has no
 *      statics either, for the same reason.
 *   2. The FSP driver API. R_AGT_Start(&g_timer0_ctrl) reads the control block, and
 *      g_timer0_ctrl is generated into ra_gen/hal_data.c, which lands in PT_PSA_ROT_DATA
 *      at 0x2200e784 on this part - inside PT_PSA_ROT_DATA (0x220018C0-0x2200FEC0, as the
 *      isolation-2 region dump reports it), so privileged. Worse, R_AGT_Open() calls
 *      R_BSP_IrqCfgEnable(), which
 *      writes the NVIC: the PPB is privileged-only by ARCHITECTURE, so no MPU region can
 *      grant it. Every interrupt-driven FSP driver does this in Open.
 *   3. The NVIC, directly. Same reason.
 *
 * So the split is: the privileged side (R_AGT_Open, NVIC setup) happens once in
 * tfm_timer0_irq_init(), which the SPM calls at partition init, and the two functions an
 * unprivileged partition calls touch nothing but the AGT0 register block - which the FLIH
 * service's TFM_PERIPHERAL_TIMER0 mmio_region already grants it.
 *
 * clear_intr() is the exception and may use the NVIC: it is called from
 * tfm_flih_test_timer_handler(), i.e. Handler mode, which is always privileged.
 *
 * None of this applies at isolation 1, where no MPU region is programmed. That is why the
 * FSP-driver version worked for five months of green regression runs.
 *
 * ADDING_FSP_MODULES.md has the general pattern for using an FSP driver from a partition.
 */

/*
 * AGTCR write values, from r_agt.c. Bits 7:4 are the status flags and clear by writing 0,
 * so writing 1 to them leaves them alone; bit 0 is TSTART. Whole-register writes, as FSP
 * does - a read-modify-write would clear whichever flag happened to read as 0.
 */
#define PLAT_TEST_AGTCR_START    (0xF1U)
#define PLAT_TEST_AGTCR_STOP     (0xF0U)

/*
 * The ICU latch and the NVIC pending bit are cleared in TFM_TIMER0_IRQ_Handler(), not here.
 * Both are out of reach of this file: the ICU is a peripheral outside the AGT0 mmio_region,
 * and the NVIC is the PPB, privileged-only by architecture. The vector entry runs in
 * Handler mode and does both before dispatching. See tfm_timer0_irq.c.
 */

/*
 * Called from the partition thread - UNPRIVILEGED from isolation 2. AGT0 registers only.
 *
 * The timer was opened and configured (16-bit, periodic, PCLKB) by tfm_timer0_irq_init().
 * If that did not happen the counter never starts and the test times out, which is
 * diagnosable; starting an unconfigured timer would not be.
 *
 * No ICU/NVIC clear here - it needs privilege, and TFM_TIMER0_IRQ_Handler() does it on
 * every interrupt instead.
 */
void tfm_plat_test_secure_timer_start(void)
{
    R_AGT0->AGT16.CTRL.AGTCR = PLAT_TEST_AGTCR_START;

    /* TCSTF is the count-status flag; it lags the write by up to 3 source cycles. FSP
     * waits on it in R_AGT_Start() and so must we, or a stop() immediately following a
     * start() can be lost. */
    while (0U == R_AGT0->AGT16.CTRL.AGTCR_b.TCSTF) {
        /* wait for the counter to be running */
    }
}

/*
 * Called from the partition's FLIH handler. For an ARoT partition at isolation 2 or 3 - the
 * configuration this fixture runs in - TF-M dispatches that handler in THREAD mode at the
 * partition's privilege, so this is UNPRIVILEGED, exactly like start() and stop(). Measured:
 * an earlier version cleared the ICU here and faulted with MMFSR 0x82, MMFAR 0x40006300.
 * AGT0 registers only.
 *
 * That is the ARoT-above-isolation-1 case, not FLIH in general: interrupt.c:195-209 calls
 * flih_func() directly - Handler mode, privileged - at isolation 1, and also above it
 * whenever tfm_hal_boundary_need_switch() is false, i.e. for PSA-RoT. Writing only AGT0
 * here is the safe intersection of all three. DECISIONS D117.
 *
 * The ICU latch is cleared ahead of us in TFM_TIMER0_IRQ_Handler(), preserving the
 * ICU-then-peripheral order that agt_int_isr() uses.
 */
void tfm_plat_test_secure_timer_clear_intr(void)
{
    /*
     * There is no FSP API for "clear the underflow flag" - agt_int_isr() does it inline,
     * and that ISR is not in the vector here. Read-modify-write is correct: the timer is
     * running, so TSTART must be preserved, and a flag that reads as 1 is written back as
     * 1, i.e. left alone. These flags clear by writing 0. TSTOP is write-only and reads as
     * 0, so the RMW does not stop the counter.
     */
    R_AGT0->AGT16.CTRL.AGTCR_b.TUNDF = 0;
}

/*
 * Called from the partition thread - UNPRIVILEGED from isolation 2. AGT0 registers only;
 * see the header of this file for why there is no ICU/NVIC clear here.
 */
void tfm_plat_test_secure_timer_stop(void)
{
    R_AGT0->AGT16.CTRL.AGTCR = PLAT_TEST_AGTCR_STOP;

    while (0U != R_AGT0->AGT16.CTRL.AGTCR_b.TCSTF) {
        /* wait for the counter to be stopped */
    }
}
