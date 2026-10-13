/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common_target_cfg.h>
#include "target_cfg.h"

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* MPC is initialized in R_BSP_SecurityInit() in security.c */
	return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t ppc_init_cfg(void)
{
    /* PPC is initialized in R_BSP_SecurityInit() in security.c */
	return TFM_PLAT_ERR_SUCCESS;
}

void sau_and_idau_cfg(void)
{
    /* SAU and IDAU is initialized in R_BSP_SecurityInit() in security.c */
}
