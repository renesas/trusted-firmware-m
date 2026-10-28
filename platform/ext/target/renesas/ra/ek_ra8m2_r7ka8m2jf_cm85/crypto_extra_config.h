/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CRYPTO_EXTRA_CONFIG_H__
#define __CRYPTO_EXTRA_CONFIG_H__

/*
 * Enable ML-DSA-87 in the runtime Crypto partition via the PQCP
 * (mldsa-native) driver.
 *
 * Requires a large Crypto partition stack, see CRYPTO_STACK_SIZE in
 * config_tfm_target.h.
 *
 * \warning Experimental, may change or be removed without notice.
 */
#define TF_PSA_CRYPTO_PQCP_MLDSA_ENABLED
#define TF_PSA_CRYPTO_PQCP_MLDSA_87_ENABLED

#endif /* __CRYPTO_EXTRA_CONFIG_H__ */
