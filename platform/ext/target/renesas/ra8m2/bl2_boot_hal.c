/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * BL2 boot HAL override: bring the RSIP-E50D up before MCUboot hashes any image.
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

void ra8m2_flash_release_for_mcuboot(void);   /* cmsis_drivers/Driver_Flash.c, BL2 only */
#include "fih.h"
#include "ra_sce_init.h"

/*
 * MCUboot's flash_area_* offsets are ABSOLUTE ADDRESSES on this port, so the base to add to
 * them is zero.
 *
 * bl2_main.c computes the next image's vector table as
 *
 *     vt = flash_base + rsp->br_image_off + rsp->br_hdr->ih_hdr_size
 *
 * where flash_base comes from here and br_image_off is the slot's fa_off. TF-M's own
 * flash_map carries fa_off as a flat offset into the device and its __WEAK implementation of
 * this returns FLASH_DEVICE_BASE to make up the difference. FSP's flash_map[], generated into
 * bsp_linker_info.h, carries the full address instead - 0x02068000 for the secure primary
 * slot, 0x120B0000 for the non-secure one, alias included - which is exactly what makes
 * MRAM's two aliases a non-problem for the backend. Adding a base on top of that would land
 * the jump 32 MB past the image.
 *
 * Not __WEAK: this REPLACES TF-M's version, which is in flash_map_extended.c and is no longer
 * linked (DEFAULT_MCUBOOT_FLASH_BACKEND=OFF). It is here rather than in the mcuboot_port
 * module because it is TF-M's contract, not FSP's - FSP's standalone bootloader never calls
 * it.
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
    ra8m2_flash_release_for_mcuboot();

    if (FSP_SUCCESS != ra_sce_init()) {
        return 1;
    }

    /* As the weak default does under CRYPTO_HW_ACCELERATOR. */
    fih_delay_init();

    return 0;
}
