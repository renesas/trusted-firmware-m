/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TFM_SERVICE_TESTS_H__
#define __TFM_SERVICE_TESTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Runs every secure service once, in order, reporting to RTT channel 0.
 * Called from __wrap_hal_entry() before the FSP application entry point. */
void tfm_service_tests_run(void);

#ifdef __cplusplus
}
#endif

#endif /* __TFM_SERVICE_TESTS_H__ */
