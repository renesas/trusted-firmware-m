/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/*
 * Crypto partition stack size.
 * ML-DSA-87 is enabled in the runtime PSA crypto config. The deepest path
 * is sign (mldsa87_signature_internal ~123 KB + call chain, from .su), plus ~20% margin.
 */
#ifndef CRYPTO_STACK_SIZE
#define CRYPTO_STACK_SIZE               0x29000
#endif

/* IPC configuration */
#define CONFIG_TFM_HYBRID_PLAT_SCHED_TYPE    TFM_HYBRID_PLAT_SCHED_SPE

#endif /* __CONFIG_TFM_TARGET_H__ */
