/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 * Copyright 2019-2020 NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_plat_defs.h"
#include "tfm_plat_test.h"
#include "device_definition.h"
#include "tfm_peripherals_def.h"

#define TIMER_RELOAD_VALUE          (CTIMER_CLK_FREQ * 1) /* 1 sec */

#ifdef TEST_NS_SLIH_IRQ

#if (__ARM_FEATURE_CMSE & 0x2) /* Secure */

extern void TFM_TIMER0_IRQ_Handler(void); /* Implemented in secure_fw\core\tfm_secure_irq_handlers.inc */

void CTIMER_IRQ_HANDLER(void)
{
    uint32_t int_stat = CTIMER_GetStatusFlags(CTIMER); /* Get Interrupt status flags */

    /* Clear the status flags that were set */
    CTIMER_ClearStatusFlags(CTIMER, int_stat);

    TFM_TIMER0_IRQ_Handler(); /* Call the TFM handler. */
}

void tfm_plat_test_secure_timer_start(void)
{
    /* Match Configuration */
    ctimer_match_config_t matchConfig;
    ctimer_config_t config;

    /* Use 12 MHz clock for some of the Ctimer */
    CLOCK_AttachClk(CTIMER_CLK_ATTACH);

    CTIMER_GetDefaultConfig(&config);
    CTIMER_Init(CTIMER, &config);

    /* Initializes the configure structure. */
    memset(&matchConfig, 0, sizeof(matchConfig));
    matchConfig.enableCounterReset = true;
    matchConfig.enableCounterStop  = true;
    matchConfig.matchValue        = TIMER_RELOAD_VALUE;
    matchConfig.enableInterrupt    = true;
    CTIMER_SetupMatch(CTIMER, kCTIMER_Match_0, &matchConfig);

    CTIMER_StartTimer(CTIMER);
}

void tfm_plat_test_secure_timer_stop(void)
{
    CTIMER_Deinit(CTIMER);
}

#else

extern void TIMER1_Handler (void); /* Implemented in \test\suites\core\non_secure\core_ns_positive_testsuite.c */

void CTIMER_NS_IRQ_HANDLER(void)
{
    uint32_t int_stat = CTIMER_GetStatusFlags(CTIMER_NS); /* Get Interrupt status flags */

    /* Clear the status flags that were set */
    CTIMER_ClearStatusFlags(CTIMER_NS, int_stat);

    TIMER1_Handler(); /* TFM handler */
}

void tfm_plat_test_non_secure_timer_start(void)
{
    /* Match Configuration */
    ctimer_match_config_t matchConfig;
    ctimer_config_t config;

    /* Use 12 MHz clock for some of the Ctimer */
    CLOCK_AttachClk(CTIMER_NS_CLK_ATTACH);

    CTIMER_GetDefaultConfig(&config);
    CTIMER_Init(CTIMER_NS, &config);

    /* Initializes the configure structure. */
    memset(&matchConfig, 0, sizeof(matchConfig));
    matchConfig.enableCounterReset = true;
    matchConfig.enableCounterStop  = true;
    matchConfig.matchValue         = TIMER_RELOAD_VALUE;
    matchConfig.enableInterrupt    = true;
    CTIMER_SetupMatch(CTIMER_NS, kCTIMER_Match_0, &matchConfig);

    CTIMER_StartTimer(CTIMER_NS);
}

void tfm_plat_test_non_secure_timer_stop(void)
{
    CTIMER_Deinit(CTIMER_NS);
}
#endif /* (__ARM_FEATURE_CMSE & 0x2) */

#endif /* TEST_NS_SLIH_IRQ */
