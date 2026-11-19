/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_plat_defs.h"
#include "plat_test.h"
#include "tfm_plat_test.h"
#include "tfm_peripherals_def.h"
#include "bsp_api.h"
#include "bsp_clock_cfg.h"
#include "r_agt.h"

#define PCLKB_HZ                        (BSP_CFG_CLOCK_CPUCLK_SRC_FREQ / (1U << BSP_CFG_PCLKB_DIV))
#define TIMER_FREQ_HZ                   (1000U)   /* 1 ms tick for this plat test */
#define TIMER_PERIOD_COUNTS             (PCLKB_HZ / TIMER_FREQ_HZ)

/*
 * The SLIH/FLIH test partitions call tfm_plat_test_secure_timer_start/stop
 * unprivileged at isolation level 2+. The functions are tagged with the
 * partition's APP-ROT section in tfm_plat_test.h. The AGT instance control
 * block is placed in the same partition's private ZI section, so the FSP
 * R_AGT_Start/R_AGT_Stop can access it from the partition. Empty at isolation
 * level 1.
 */
#if defined(TFM_PARTITION_SLIH_TEST)
#define PLAT_TEST_TIMER_PARTITION_NAME  "TFM_SP_SLIH_TEST"
#elif defined(TFM_PARTITION_FLIH_TEST)
#define PLAT_TEST_TIMER_PARTITION_NAME  "TFM_SP_FLIH_TEST"
#endif

#ifdef PLAT_TEST_TIMER_PARTITION_NAME
#define PLAT_TEST_TIMER_DATA_ATTR \
    TFM_LINK_SET_ZI_IN_PARTITION_SECTION(PLAT_TEST_TIMER_PARTITION_NAME, "APP-ROT")
#else
#define PLAT_TEST_TIMER_DATA_ATTR
#endif

static PLAT_TEST_TIMER_DATA_ATTR agt_instance_ctrl_t agt0_ctrl;

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

static const timer_cfg_t agt0_cfg = {
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
    (void)R_AGT_Start(&agt0_ctrl);
}

void tfm_plat_test_secure_timer_clear_intr(void)
{
    volatile R_AGTX0_AGT16_CTRL_Type *p_ctrl =
        agt0_ctrl.is_agtw ? &agt0_ctrl.p_reg->AGT32.CTRL : &agt0_ctrl.p_reg->AGT16.CTRL;

    p_ctrl->AGTCR = (uint8_t)(p_ctrl->AGTCR & R_AGTX0_AGT16_CTRL_AGTCR_TSTART_Msk);
}

void tfm_plat_test_secure_timer_stop(void)
{
    (void)R_AGT_Stop(&agt0_ctrl);
}

void tfm_plat_test_secure_timer_init(void)
{
    if (!agt0_ctrl.open) {
        R_AGT_Open(&agt0_ctrl, &agt0_cfg);
        R_FSP_IsrContextSet(TFM_TIMER0_IRQ, &agt0_ctrl);
    }
}
