/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_hal_device_header.h"
#include "tfm_hal_multi_core.h"

#if !defined(BSP_PARTITION_FLASH_CPU1_S_START)
#error "BSP_PARTITION_FLASH_CPU1_S_START is not defined."
#endif

void tfm_hal_boot_ns_cpu(uintptr_t start_addr)
{
    (void)start_addr;

    /* R_BSP_SecondaryCoreStart boots CPU1 from BSP_PARTITION_FLASH_CPU1_S_START. */
    R_BSP_SecondaryCoreStart();
}

void tfm_hal_wait_for_ns_cpu_ready(void)
{
    volatile uint16_t cnt = UINT32_MAX;

    /* Stub: will be supported in other MRs*/
    /*
     * Add a delay to wait CPU1 completely done it initialization
     * to avoid RACE condition while both core configure the MSTPB in STDIO initialization
     */
    while (cnt--) {
    }
}
