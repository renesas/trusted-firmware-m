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
 * PPC is an Arm CoreLink concept (SSE-200 and similar). RA6 has no PPC: peripheral
 * security attribution uses the Renesas PSAR registers, which FSP sets from the solution's
 * security attributes.
 *
 * TF-M's shared isolation HAL still requires the type and the sentinel whenever
 * CONFIG_TFM_MMIO_REGION_ENABLE is on, which it is as soon as any enabled partition declares
 * an mmio_region. tf-m-tests' common tfm_secure_client_service claims
 * TFM_PERIPHERAL_STD_UART, so this applies to every regression test build.
 *
 * As on other platforms without a PPC (rpi/rp2350), each platform_data_t points at the
 * sentinel, so tfm_hal_bind_boundary() takes the "do not configure" branch. The ppc_*() call
 * sites still compile, so target_cfg.c provides no-op bodies.
 */
typedef uint32_t ppc_bank_t;

/* Other platforms use an enum value; (ppc_bank_t)-1 has the same bit pattern, so the
 * comparison in tfm_hal_bind_boundary() behaves identically. */
#define PPC_SP_DO_NOT_CONFIGURE  ((ppc_bank_t)-1)

/**
 * \brief Configures SAU and IDAU.
 */
void sau_and_idau_cfg(void);

/**
 * \brief Configures the Memory Protection Controller (stub for RA6M5).
 */
enum tfm_plat_err_t mpc_init_cfg(void);

/**
 * \brief Configures the Peripheral Protection Controller (stub for RA6M5).
 */
enum tfm_plat_err_t ppc_init_cfg(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARGET_CFG_H__ */