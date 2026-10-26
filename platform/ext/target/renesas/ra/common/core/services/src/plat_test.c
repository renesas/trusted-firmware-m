/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdbool.h>

#include "tfm_plat_test.h"
#include "tfm_peripherals_def.h"
#include "bsp_api.h"
#include "bsp_clock_cfg.h"
#include "r_agt.h"

extern void agt_int_isr(void);

#define PCLKB_HZ                        (BSP_CFG_CLOCK_CPUCLK_SRC_FREQ / (1U << BSP_CFG_PCLKB_DIV))
#define TIMER_FREQ_HZ                   (1000U)   /* 1 ms tick for this plat test */
#define TIMER_PERIOD_COUNTS             (PCLKB_HZ / TIMER_FREQ_HZ)

static agt_instance_ctrl_t agt0_ctrl;

static const agt_extended_cfg_t agt0_extend = {
    .count_source            = AGT_CLOCK_PCLKB,
    .agto                    = AGT_PIN_CFG_DISABLED,
    .agtoab_settings_b.agtoa = AGT_PIN_CFG_DISABLED,
    .agtoab_settings_b.agtob = AGT_PIN_CFG_DISABLED,
    .measurement_mode        = AGT_MEASURE_DISABLED,
    .agtio_filter            = AGT_AGTIO_FILTER_NONE,
    .enable_pin              = AGT_ENABLE_PIN_NOT_USED,
    .trigger_edge            = AGT_TRIGGER_EDGE_RISING,
    .counter_bit_width       = AGT_COUNTER_BIT_WIDTH_16,
};

static timer_cfg_t agt0_cfg = {
    .mode          = TIMER_MODE_PERIODIC,
    .source_div    = TIMER_SOURCE_DIV_1,
    .period_counts = TIMER_PERIOD_COUNTS,
    .channel       = 0U,
    .cycle_end_ipl = DEFAULT_IRQ_PRIORITY,
    .cycle_end_irq = FSP_INVALID_VECTOR,   /* Let the TF-M enable this IRQ */
    .p_extend      = &agt0_extend,
};

void tfm_plat_test_secure_timer_start(void)
{
    if (!agt0_ctrl.open) {
        R_AGT_Open(&agt0_ctrl, &agt0_cfg);
        /* Set the interrupt context for FSP handlers */
        R_FSP_IsrContextSet(TFM_TIMER0_IRQ, &agt0_ctrl);
    }

    R_AGT_Start(&agt0_ctrl);
}

void tfm_plat_test_secure_timer_clear_intr(void)
{
    /* This function is a FLIH (first-level interrupt handler).
     * Clear the ICU IELSR.IR bit and all AGT status flags.
     */
    agt_int_isr();
}

void tfm_plat_test_secure_timer_stop(void)
{
    R_AGT_Stop(&agt0_ctrl);
}
