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

void ra6m5_flash_release_for_mcuboot(void);   /* cmsis_drivers/Driver_Flash.c, BL2 only */
#include "fih.h"
#ifdef RA6M5_BL2_SCE_INIT
#include "ra_sce_init.h"
#endif

/*
 * The base to add to a flash_area offset. Zero here - twice over.
 *
 * bl2_main.c computes the next image's vector table as
 *     vt = flash_base + rsp->br_image_off + rsp->br_hdr->ih_hdr_size
 * TF-M's own __WEAK version returns FLASH_DEVICE_BASE to compensate for its flat fa_off; it
 * lives in flash_map_extended.c, which is no longer linked (DEFAULT_MCUBOOT_FLASH_BACKEND
 * OFF), so the port must supply one.
 *
 * FSP's flash_map[] carries absolute addresses in fa_off. On RA6M5 that is the same number
 * as the offset, because code flash is based at 0x00000000 - so zero is correct whichever
 * convention you read it under. On RA8M2 the two differ by 32 MB and only the FSP reading
 * gives zero; keeping the implementations identical across the two ports is deliberate.
 */
int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    (void)fd_id;

    if (ret == NULL) {
        return -1;
    }

    *ret = 0u;
    return 0;
}

int32_t boot_platform_post_init(void)
{
    /* Hand the flash controller to FSP's flash_map_backend before MCUboot touches a
     * slot. boot_platform_init() has just opened it, and FSP's flash_area_open()
     * fails on FSP_ERR_ALREADY_OPEN - see the long note in cmsis_drivers/Driver_Flash.c.
     * DECISIONS D085. */
    ra6m5_flash_release_for_mcuboot();

#ifdef RA6M5_BL2_SCE_INIT
    if (FSP_SUCCESS != ra_sce_init()) {
        return 1;
    }

    /* As the weak default does under CRYPTO_HW_ACCELERATOR. */
    fih_delay_init();

#endif

    return 0;
}
