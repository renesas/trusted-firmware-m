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
 * WHY THIS DRIVES AGT REGISTERS DIRECTLY INSTEAD OF CALLING R_AGT_*
 *
 * The secure e2 project configures AGT0 as g_timer0, and that configuration is what makes
 * the interrupt secure on RA (DECISIONS D061) - but the FSP driver is deliberately not
 * used for the register work:
 *
 *   - R_AGT_Open() installs an FSP callback and expects agt_int_isr() to be the handler.
 *     TF-M owns this vector (__VECTOR_TABLE in startup_ra6m5.c), so the FSP callback can
 *     never fire and R_AGT_Open()'s IRQ plumbing would be dead weight.
 *   - The test HAL starts and stops the timer repeatedly across test cases. Tracking
 *     whether g_timer0_ctrl is currently open, from code the SPM calls in interrupt
 *     context, is more state than three register writes deserve.
 *
 * The FSP driver is still linked (flash_on_flash_hp and timer_on_agt are both configured)
 * and g_timer0 still exists; it is simply not this file's path to the hardware.
 * --------------------------------------------------------------------------------------
 */

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
  #error "RA6M5: AGT0 has no interrupt in the secure e2 project, so the FLIH/SLIH suites \
cannot raise one. In the SECURE project set the AGT (g_timer0) 'Interrupt Priority' to a \
real priority instead of Disabled, regenerate, and rebuild. Configuring it in the secure \
project is also what makes the interrupt secure - see DECISIONS D061."
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
 * 16-bit AGT at PCLKB with no prescaler. At 100 MHz PCLKB a full 0xFFFF reload underflows
 * in ~655 us, which is long enough that the SLIH path reaches its test assertions before
 * the interrupt lands and short enough that no test waits on it noticeably.
 *
 * The project configures the same period (0x10000 raw counts, PCLKB); this file does not
 * read that configuration because it does not use the driver, so the value is repeated.
 * They do not have to agree - only this one is programmed.
 */
#define PLAT_TEST_AGT_RELOAD    (0xFFFFU)

/* R_AGT0 is an R_AGTX0_Type on this part: a union of a 32-bit (AGTW) and a 16-bit (AGT)
 * register view over the same block at 0x400E8000. The project selects 16-bit. */
#define PLAT_TEST_AGT           (R_AGT0->AGT16)

/*
 * Stop the counter and wait for it to actually stop.
 *
 * AGT is on PCLKB and its control writes are synchronised into that domain, so TSTART
 * clearing is not observable until TCSTF follows. AGT and AGTMR1 must not be written while
 * TCSTF is set, which is why every reconfiguration goes through here first.
 */
static void plat_test_agt_stop(void)
{
    PLAT_TEST_AGT.CTRL.AGTCR_b.TSTOP = 1;
    while (0U != PLAT_TEST_AGT.CTRL.AGTCR_b.TCSTF) {
        /* wait for the count to stop */
    }
}

void tfm_plat_test_secure_timer_start(void)
{
    /* Idempotent: releasing an already-running module is a no-op. Needed because nothing
     * else in the secure image opens AGT0 - R_AGT_Open() would normally do this. */
    R_BSP_MODULE_START(FSP_IP_AGT, 0);

    plat_test_agt_stop();

    /* TMOD = 000 (timer mode), TEDGPL = 0, TCK = 000 (PCLKB). */
    PLAT_TEST_AGT.CTRL.AGTMR1 = 0U;
    /* CKS ignored for PCLKB, LPM off. */
    PLAT_TEST_AGT.CTRL.AGTMR2 = 0U;

    PLAT_TEST_AGT.AGT = PLAT_TEST_AGT_RELOAD;

    /* Whole-byte write while stopped: clears TEDGF/TUNDF/TCMAF/TCMBF together and leaves
     * TSTART clear. Flags are cleared by writing 0, so this must not be a read-modify-
     * write - reading a set flag and writing it back would leave it set. */
    PLAT_TEST_AGT.CTRL.AGTCR = 0U;

    PLAT_TEST_AGT.CTRL.AGTCR_b.TSTART = 1;
    while (0U == PLAT_TEST_AGT.CTRL.AGTCR_b.TCSTF) {
        /* wait for the count to start */
    }
}

void tfm_plat_test_secure_timer_clear_intr(void)
{
    /*
     * Read-modify-write of AGTCR, which is correct HERE and wrong in _start(): the timer
     * is running, so TSTART must be preserved, and any other flag that happens to be set
     * is read back and rewritten as 1, i.e. left alone. Only TUNDF is cleared. TSTOP is
     * write-only and reads as 0, so the RMW does not stop the counter.
     */
    PLAT_TEST_AGT.CTRL.AGTCR_b.TUNDF = 0;

    /* The NVIC pending bit latches independently of the peripheral flag. Leaving it set
     * re-enters the handler the moment the partition re-enables the IRQ, which presents
     * as a spurious second interrupt in the FLIH tests. */
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);
}

void tfm_plat_test_secure_timer_stop(void)
{
    plat_test_agt_stop();
    PLAT_TEST_AGT.CTRL.AGTCR = 0U;
    NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);
}
