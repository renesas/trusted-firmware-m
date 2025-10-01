/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tfm_hal_isolation.h"
#include "tfm_hal_defs.h"
#include "region_defs.h"
#include "cmsis.h"

enum tfm_hal_status_t tfm_hal_set_up_static_boundaries(
                                        uintptr_t *p_spm_boundary)
{
    /* Disable MPU */
    MPU->CTRL = 0;

    /* MPU Region 0: Secure code (Flash) - RO, Executable */
    MPU->RNR  = 0;
    MPU->RBAR = S_CODE_START;
    MPU->RASR = ((31 - __CLZ(S_CODE_SIZE) - 1) << MPU_RASR_SIZE_Pos) |
                (0x08U << MPU_RASR_TEX_Pos) |
                (0x01U << MPU_RASR_C_Pos) |
                (0x01U << MPU_RASR_B_Pos) |
                (0x06U << MPU_RASR_AP_Pos) |
                MPU_RASR_ENABLE_Msk;

    /* MPU Region 1: Secure RAM - RW, Non-Executable */
    MPU->RNR  = 1;
    MPU->RBAR = S_DATA_START;
    MPU->RASR = ((31 - __CLZ(S_DATA_SIZE) - 1) << MPU_RASR_SIZE_Pos) |
                (0x08U << MPU_RASR_TEX_Pos) |
                (0x01U << MPU_RASR_C_Pos) |
                (0x01U << MPU_RASR_B_Pos) |
                (0x03U << MPU_RASR_AP_Pos) |
                (0x01U << MPU_RASR_XN_Pos) |
                MPU_RASR_ENABLE_Msk;

    /* Enable MPU */
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;

    __DSB();
    __ISB();

    if (p_spm_boundary) {
        *p_spm_boundary = (uintptr_t)(S_CODE_START + S_CODE_SIZE - CMSE_VENEER_REGION_SIZE);
    }

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_memory_check(uintptr_t boundary, uintptr_t base,
                                            size_t size, uint32_t access_type)
{
    (void)boundary;
    (void)access_type;

    uintptr_t end = base + size - 1;

    if (end < base) {
        return TFM_HAL_ERROR_MEM_FAULT;
    }

    /* Check Secure memory */
    if ((base >= S_CODE_START && base <= S_CODE_LIMIT) ||
        (base >= S_DATA_START && base <= S_DATA_LIMIT)) {
        if ((end > S_CODE_LIMIT && end < S_DATA_START) ||
            (end > S_DATA_LIMIT)) {
            return TFM_HAL_ERROR_MEM_FAULT;
        }
        return TFM_HAL_SUCCESS;
    }

    /* Check Non-Secure memory */
    if ((base >= NS_CODE_START && base <= NS_CODE_LIMIT) ||
        (base >= NS_DATA_START && base <= NS_DATA_LIMIT)) {
        if ((end > NS_CODE_LIMIT && end < NS_DATA_START) ||
            (end > NS_DATA_LIMIT)) {
            return TFM_HAL_ERROR_MEM_FAULT;
        }
        return TFM_HAL_SUCCESS;
    }

    return TFM_HAL_ERROR_MEM_FAULT;
}