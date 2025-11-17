/*
 * Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-M Service Test Thread Header
 */

#ifndef TFM_TEST_THREAD_H
#define TFM_TEST_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TF-M Test Thread Entry Point
 *
 * This thread tests all TF-M services:
 * - Initial Attestation
 * - Internal Trusted Storage
 * - Crypto (Random, Hash)
 * - HUK Derivation
 *
 * @param pvParameters FreeRTOS task parameters (unused)
 */
void tfm_test_thread_entry(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* TFM_TEST_THREAD_H */
