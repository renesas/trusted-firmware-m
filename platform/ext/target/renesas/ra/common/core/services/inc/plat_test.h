/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_TEST_H__
#define __PLAT_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Opens AGT0 for the IRQ tests, with the timer left stopped.
 *
 * Called by the SPM from tfm_timer0_irq_init() while it runs privileged, so
 * the FSP driver state is set up before the test partition runs. Defined in
 * plat_test.c, which is only built in regression test builds.
 */
void tfm_plat_test_secure_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __PLAT_TEST_H__ */
