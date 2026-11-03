/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * BL2 boot HAL override: enables the SAU, then brings the RSIP-E50D up before MCUboot hashes
 * any image.
 *
 * Built unconditionally, because of the SAU (below). Only the RSIP-E50D bring-up depends on
 * CRYPTO_HW_ACCELERATOR (CMakeLists:857). bl2_main.c calls boot_platform_post_init() after
 * boot_platform_init() and before boot_go_for_image_id() touches either slot.
 *
 * Not done in crypto_hw_accelerator_init(): the weak default of this function in
 * platform/ext/common/boot_hal_bl2.c calls that, but in BL2 it resolves to the stub bl2_main.c
 * defines for MCUBOOT_USE_PSA_CRYPTO, which does nothing and is not weak, so it cannot be
 * overridden.
 */

#include <stdint.h>

#include "boot_hal.h"
#include "fih.h"
#ifdef CRYPTO_HW_ACCELERATOR
#include "ra_sce_init.h"
#endif
#include "tfm_hal_device_header.h"

/*
 * ===================== BL2 must enable the SAU on this part =====================
 *
 * BL2 reads the non-secure image headers through MRAM's non-secure alias, 0x120B0000 and up
 * (Driver_Flash.c, MRAM_ADDR). On a part RDPM has partitioned, that is the only alias that
 * reaches those bytes; the secure alias stops at the Code Secure boundary.
 *
 * The access itself must also be non-secure. With the SAU disabled and SAU_CTRL.ALLNS clear,
 * Armv8-M attributes every address Secure, and the combined SAU/IDAU attribute is the more
 * secure of the two. BL2's load from 0x120B0000 then goes out as a secure transaction to the
 * non-secure alias and is refused. Both aliases fault: the secure one on the address, the
 * non-secure one on the attribute.
 *
 * FSP enables the SAU in R_BSP_SAUInit(), reached from SystemInit() through
 * R_BSP_SecurityInit(), but only inside #if BSP_TZ_SECURE_BUILD. BL2 is built as the flat FSP
 * role (fsp_bsp.cmake) by design, so it gets neither. Confirm with
 * `arm-none-eabi-nm bl2.elf | grep SAUInit`: absent in BL2, present in tfm_s.
 *
 * RA6M5 needs none of this. __SAUREGION_PRESENT is 0 there (an SAU with no regions, not no
 * SAU), FSP still sets SAU_CTRL.ALLNS so the IDAU decides, and BSP_FEATURE_TZ_NS_OFFSET is 0:
 * one alias.
 *
 * One region, not FSP's five. BL2 has no veneers, so neither NSC region applies, and MRAM's is
 * the only non-secure alias it touches. Everything outside an enabled SAU region stays Secure,
 * as before, so this changes exactly one thing. tfm_s installs the full set later.
 *
 * -------------------------------------------------------------------------------------
 * The bounds are imported from FSP's rm_mcuboot_port/flash_map.c, flash_area_open():
 *
 *     SAU->RNR  = RM_MCUBOOT_PORT_SAU_REGION_CODE_FLASH;                      // 0
 *     flash_ns_region_start = RM_MCUBOOT_PORT_FLASH_NS_START +
 *                             (RM_MCUBOOT_PORT_INTERNAL_FLASH_BLOCK_SIZE *
 *                              R_PSCU->CFSAMONA_b.CFS2);
 *     SAU->RBAR = flash_ns_region_start & SAU_RBAR_BADDR_Msk;
 *     flash_ns_region_end   = RM_MCUBOOT_PORT_FLASH_NS_START + BSP_ROM_SIZE_BYTES;
 *     SAU->RLAR = (flash_ns_region_end & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;
 *
 * where RM_MCUBOOT_PORT_FLASH_NS_START is BSP_FEATURE_FLASH_CODE_FLASH_START | (1 << 28)
 * and RM_MCUBOOT_PORT_INTERNAL_FLASH_BLOCK_SIZE is 0x8000 on MRAM parts.
 *
 * FSP's bounds are tighter than the architectural alias bounds: CFSAMONA.CFS2 is the Code
 * Flash Secure boundary RDPM programmed, counted in 32 KB blocks, so the region starts exactly
 * where the non-secure half does. The IDAU enforces the real split either way, but a region
 * that starts at the boundary cannot mark secure bytes non-secure even transiently.
 *
 * One deliberate deviation: RLAR.LADDR is the inclusive limit of the 32-byte block it names,
 * so FSP's ns_end runs the region 32 bytes past the end of MRAM. This file writes ns_end - 1
 * (D135, finding R148).
 *
 * Restated rather than included because rm_mcuboot_port is not built here: TF-M supplies the
 * MCUboot flash backend (DEFAULT_MCUBOOT_FLASH_MAP=ON). FSP_UPREV_CHECKLIST.md lists this copy.
 * ================================================================================
 */
#define BL2_SAU_REGION_CODE_FLASH   (0U)
#define BL2_SAU_FLASH_NS_START      (BSP_FEATURE_FLASH_CODE_FLASH_START | (1U << 28))
#define BL2_SAU_FLASH_BLOCK_SIZE    (0x8000U)

static void bl2_sau_init(void)
{
#if defined(__SAUREGION_PRESENT) && (__SAUREGION_PRESENT == 1U)
    uint32_t ns_start = BL2_SAU_FLASH_NS_START +
                        (BL2_SAU_FLASH_BLOCK_SIZE * R_PSCU->CFSAMONA_b.CFS2);
    uint32_t ns_end   = BL2_SAU_FLASH_NS_START + BSP_ROM_SIZE_BYTES;

    SAU->RNR  = BL2_SAU_REGION_CODE_FLASH;
    SAU->RBAR = ns_start & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = ((ns_end - 1U) & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    SAU->CTRL = SAU_CTRL_ENABLE_Msk;

    /* The attribution change must be in effect before the next load, not merely issued. */
    __DSB();
    __ISB();
#endif
}


int32_t boot_platform_post_init(void)
{
    /* First: neither the code below nor MCUboot can read the non-secure slots until this
     * has run. */
    bl2_sau_init();

#ifdef CRYPTO_HW_ACCELERATOR
    if (FSP_SUCCESS != ra_sce_init()) {
        return 1;
    }
#endif

    /* As the weak default does under CRYPTO_HW_ACCELERATOR. */
    fih_delay_init();

    return 0;
}
