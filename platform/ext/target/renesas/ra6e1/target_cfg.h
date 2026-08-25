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

/* Stub type for PPC bank (RA6M4 doesn't have PPC) */
typedef uint32_t ppc_bank_t;

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