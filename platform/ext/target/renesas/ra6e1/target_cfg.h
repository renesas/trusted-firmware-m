/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TARGET_CFG_H__
#define __TARGET_CFG_H__

#include <stdint.h>
#include "tfm_plat_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PPC is an Arm CoreLink concept (SSE-200 and friends). RA6 has no PPC - peripheral
 * security attribution uses the Renesas PSAR registers instead, set up by FSP from the
 * solution's security attributes rather than by TF-M.
 *
 * TF-M's shared isolation HAL nevertheless requires the type and the sentinel whenever
 * CONFIG_TFM_MMIO_REGION_ENABLE is on, which happens as soon as ANY enabled partition
 * declares an mmio_region. The regression tests do: tf-m-tests' COMMON
 * tfm_secure_client_service claims TFM_PERIPHERAL_STD_UART, so this is on the path for
 * every regression test, not only the peripheral ones.
 *
 * Platforms with no real PPC all do the same thing - see rpi/rp2350 - declaring the
 * sentinel and pointing each platform_data_t at it, so tfm_hal_bind_boundary() takes the
 * "do not configure" branch. The ppc_*() functions still have to LINK, because those call
 * sites are compiled even though the branch is never taken; target_cfg.c has no-op bodies.
 */
typedef uint32_t ppc_bank_t;

/* Signed -1 where other platforms use an enum; the same bit pattern here, so the
 * comparison in tfm_hal_bind_boundary() behaves identically. */
#define PPC_SP_DO_NOT_CONFIGURE  ((ppc_bank_t)-1)

/**
 * \brief Configures SAU and IDAU.
 */
void sau_and_idau_cfg(void);

/**
 * \brief Configures the Memory Protection Controller (stub for RA6M4).
 */
enum tfm_plat_err_t mpc_init_cfg(void);

/**
 * \brief Configures the Peripheral Protection Controller (stub for RA6M4).
 */
enum tfm_plat_err_t ppc_init_cfg(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARGET_CFG_H__ */