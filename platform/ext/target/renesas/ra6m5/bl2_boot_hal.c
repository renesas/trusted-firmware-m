/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * BL2 boot HAL override: bring the SCE9 up before MCUboot hashes any image.
 *
 * Built only with CRYPTO_HW_ACCELERATOR, when BL2's SHA-256 runs on the SCE
 * (platform/ext/accelerator/renesas/sce9). bl2_main.c calls boot_platform_post_init() after
 * boot_platform_init() and before boot_go_for_image_id() touches either slot.
 *
 * Why here and not in crypto_hw_accelerator_init(): the weak default of this function in
 * platform/ext/common/boot_hal_bl2.c does call that, but in BL2 it resolves to the stub
 * bl2_main.c defines for MCUBOOT_USE_PSA_CRYPTO, which does nothing and cannot be overridden
 * (it is not weak).
 */

#include <stdint.h>

#include "boot_hal.h"
#include "fih.h"
#include "ra_sce_init.h"

int32_t boot_platform_post_init(void)
{
    if (FSP_SUCCESS != ra_sce_init()) {
        return 1;
    }

    /* As the weak default does under CRYPTO_HW_ACCELERATOR. */
    fih_delay_init();

    return 0;
}
