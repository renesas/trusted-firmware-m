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
 * tf-m-tests' suites/spm/irq/service/tfm_{flih,slih}_test_service.c call the three secure
 * functions below. Nothing in tests_reg calls tfm_plat_test_non_secure_timer_start() or
 * _stop(), so they are absent rather than stubbed.
 *
 * The work is split by privilege:
 *   - tfm_timer0_irq_init() in tfm_timer0_irq.c opens AGT0 once with R_AGT_Open(), at
 *     partition init.
 *   - TFM_TIMER0_IRQ_Handler() in tfm_timer0_irq.c clears the ICU latch and the NVIC
 *     pending bit on every interrupt.
 *   - This file starts, stops and acknowledges the counter by writing AGT0 registers only.
 *     "Privilege rules" below gives the reasons.
 *
 * Do not replace the AGTCR writes with R_AGT_Start()/R_AGT_Stop(), and do not re-derive
 * R_AGT_Open()'s register sequence by hand: it writes AGTMR2 before AGTMR1 and performs
 * the compare-match parking.
 *
 * bsp_irq_cfg() programs the ICU event link (IELSR) once from the generated event-link
 * table; see target_cfg.c, ra8m2_icu_vector_data_anchor.
 */

#include <stdint.h>

#include "bsp_api.h"
#include "tfm_hal_device_header.h"
#include "tfm_peripherals_def.h"
#include "tfm_plat_test.h"
#include "vector_data.h"

/*
 * The IRQ number must come from the generated vector table, never from a literal. RASC
 * assigns ICU slots densely in the order interrupts were added to the project, so adding
 * an earlier interrupt renumbers this one. tfm_peripherals_def.h must carry a literal
 * because it is included where vector_data.h is not; the assertion below ties the two.
 */
#if !defined(VECTOR_NUMBER_AGT0_INT)
  #error "RA8M2: AGT0 has no interrupt in the secure e2 project, so the FLIH/SLIH suites cannot raise one. In the SECURE project set the AGT (g_timer0) Interrupt Priority to a real priority instead of Disabled, regenerate, and rebuild. Configuring it in the secure project is also what makes the interrupt secure - see DECISIONS D061."
#endif

/*
 * _Static_assert, not #if: both macros expand to ((IRQn_Type) n), and a cast is not a
 * valid preprocessor constant expression, so the #if form fails to parse (IAR Pe018).
 */
_Static_assert((int)TFM_TIMER0_IRQ == (int)VECTOR_NUMBER_AGT0_INT,
               "RA8M2: TFM_TIMER0_IRQ in tfm_peripherals_def.h disagrees with the "
               "generated VECTOR_NUMBER_AGT0_INT. RASC has renumbered the ICU slots - "
               "update the constant in tfm_peripherals_def.h to match "
               "ra_gen/vector_data.h, and the vector slot in startup_ra8m2.c with it.");

/*
 * Privilege rules. Read before changing this file.
 *
 * This file is built into tfm_sprt, not platform_s. start() and stop() are called by the
 * FLIH and SLIH test services, which are Application RoT partitions and run unprivileged
 * from isolation level 2. That rules out three things:
 *
 *   1. Any static in this file. tfm_sprt's code is in the unprivileged code region, but its
 *      .bss lands in PT_PSA_ROT_DATA, which is ARM_MPU_PRIVILEGED. A static faults with
 *      MMFSR 0x82 (DACCVIOL). an521's SPRT timer driver has no statics for the same reason.
 *   2. The FSP driver API. R_AGT_Start(&g_timer0_ctrl) reads the control block, which is
 *      generated into ra_gen/hal_data.c and lands in privileged PT_PSA_ROT_DATA.
 *      R_AGT_Open() also calls R_BSP_IrqCfgEnable(), which writes the NVIC. Every
 *      interrupt-driven FSP driver does this in Open.
 *   3. The NVIC. It is in the PPB, which is privileged-only by architecture, so no MPU
 *      region can grant it.
 *
 * The functions below therefore touch only the AGT0 register block, which the FLIH
 * service's TFM_PERIPHERAL_TIMER0 mmio_region grants.
 *
 * At isolation level 1 no MPU region is programmed, so a breach of these rules passes there
 * and faults only from level 2.
 *
 * ADDING_FSP_MODULES.md §7 gives the general pattern for using an FSP driver from a
 * partition.
 */

/*
 * AGTCR write values, from r_agt.c. Bits 7:4 are status flags that clear on a write of 0,
 * so writing 1 leaves them unchanged; bit 0 is TSTART. Write the whole register, as FSP
 * does: a read-modify-write would clear whichever flag happened to read as 0.
 */
#define PLAT_TEST_AGTCR_START    (0xF1U)
#define PLAT_TEST_AGTCR_STOP     (0xF0U)

/*
 * Called from the partition thread, unprivileged from isolation level 2. AGT0 registers
 * only.
 *
 * tfm_timer0_irq_init() has opened and configured the timer (16-bit, periodic, PCLKB). If
 * it has not, the counter never starts and the test times out, which is diagnosable;
 * starting an unconfigured timer would not be.
 */
void tfm_plat_test_secure_timer_start(void)
{
    R_AGT0->AGT16.CTRL.AGTCR = PLAT_TEST_AGTCR_START;

    /* TCSTF, the count-status flag, lags the write by up to 3 source cycles. Wait for it,
     * as R_AGT_Start() does, or a stop() straight after a start() can be lost. */
    while (0U == R_AGT0->AGT16.CTRL.AGTCR_b.TCSTF) {
        /* wait for the counter to be running */
    }
}

/*
 * Called from the partition's FLIH handler. For an Application RoT partition at isolation
 * level 2 or 3, which is how this fixture runs, TF-M dispatches that handler in Thread mode
 * at the partition's privilege, so this code is unprivileged like start() and stop(). In
 * the other dispatch cases it runs in Handler mode; writing only AGT0 is safe in all of
 * them. AGT0 registers only.
 *
 * TFM_TIMER0_IRQ_Handler() in tfm_timer0_irq.c clears the ICU latch first, keeping the
 * ICU-then-peripheral order of agt_int_isr(). It also describes the dispatch cases and the
 * fault an ICU write here causes (D117).
 */
void tfm_plat_test_secure_timer_clear_intr(void)
{
    /*
     * FSP has no API to clear the underflow flag; agt_int_isr() clears it inline, and that
     * ISR is not in this vector table. A read-modify-write is correct here: the running
     * timer's TSTART is preserved, a flag that reads as 1 is written back as 1 and so left
     * alone (these flags clear on a write of 0), and TSTOP is write-only and reads as 0, so
     * the write does not stop the counter.
     */
    R_AGT0->AGT16.CTRL.AGTCR_b.TUNDF = 0;
}

/*
 * Called from the partition thread, unprivileged from isolation level 2. AGT0 registers
 * only.
 */
void tfm_plat_test_secure_timer_stop(void)
{
    R_AGT0->AGT16.CTRL.AGTCR = PLAT_TEST_AGTCR_STOP;

    while (0U != R_AGT0->AGT16.CTRL.AGTCR_b.TCSTF) {
        /* wait for the counter to be stopped */
    }
}
