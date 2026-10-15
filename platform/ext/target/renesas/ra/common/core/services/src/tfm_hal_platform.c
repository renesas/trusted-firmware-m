/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "common_target_cfg.h"
#include "tfm_hal_device_header.h"
#include "tfm_hal_platform.h"
#include "tfm_plat_defs.h"
#include "uart_stdout.h"

enum tfm_hal_status_t tfm_hal_platform_init(void)
{
    __enable_irq();
    stdio_init();

    return TFM_HAL_SUCCESS;
}

/*
 * PPC (Peripheral Protection Controller) functions.
 * These functions are provided as stubs since peripheral security is configured
 * by the FSP during early boot.
 *
 * TODO: Provide PPC support when enabling higher isolation levels.
 * Reference: tfm_hal_isolation_v8m.c implementation.
 */

void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_configure_to_non_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}
