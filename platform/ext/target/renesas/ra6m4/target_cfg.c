/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "target_cfg.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "cmsis.h"
#include "bsp_api.h"
#include "hal_data.h"

enum tfm_plat_err_t target_cfg_init(void)
{
    /* Initialize FSP common/board support */
    g_common_init();

    /* Configure SAU (Security Attribution Unit) */
    /* SAU Region 0: Non-Secure Flash */
    SAU->RNR  = 0;
    SAU->RBAR = NS_CODE_START & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (NS_CODE_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* SAU Region 1: Non-Secure RAM */
    SAU->RNR  = 1;
    SAU->RBAR = NS_DATA_START & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (NS_DATA_LIMIT & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    /* SAU Region 2: Non-Secure Peripherals */
    SAU->RNR  = 2;
    SAU->RBAR = NS_PERIPHERAL_ALIAS_BASE & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = ((NS_PERIPHERAL_ALIAS_BASE + PERIPHERAL_SIZE - 1) & SAU_RLAR_LADDR_Msk)
                | SAU_RLAR_ENABLE_Msk;

    /* Enable SAU */
    SAU->CTRL = SAU_CTRL_ENABLE_Msk | SAU_CTRL_ALLNS_Msk;

    /* Configure NVIC */
    NVIC_SetPriorityGrouping(0x3);

    return TFM_PLAT_ERR_SUCCESS;
}