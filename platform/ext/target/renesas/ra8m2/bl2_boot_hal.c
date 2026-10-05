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
#include "fih.h"
#ifdef CRYPTO_HW_ACCELERATOR
#include "ra_sce_init.h"
#endif
#include "tfm_hal_device_header.h"

/*
 * ===================== BL2 MUST ENABLE THE SAU ON THIS PART =====================
 *
 * BL2 reads the non-secure image headers out of MRAM's non-secure alias, 0x120B0000 and up
 * (Driver_Flash.c, MRAM_ADDR). On a part RDPM has partitioned, that is the ONLY alias that
 * reaches those bytes - the secure alias stops at the Code Secure boundary.
 *
 * Reaching them also needs the access itself to be non-secure. With the SAU disabled and
 * SAU_CTRL.ALLNS clear, Armv8-M attributes every address Secure, and the combined SAU/IDAU
 * attribute takes the more secure of the two - so BL2's load from 0x120B0000 goes out as a
 * SECURE transaction to the non-secure alias and is refused. Both aliases then fault: the
 * secure one on the address, the non-secure one on the attribute.
 *
 * FSP does this in R_BSP_SAUInit(), reached from SystemInit() via R_BSP_SecurityInit() - but
 * that call sits inside #if BSP_TZ_SECURE_BUILD, and BL2 is built as the flat FSP role
 * (fsp_bsp.cmake), deliberately, so BL2 gets neither. Confirm with
 * `arm-none-eabi-nm bl2.elf | grep SAUInit`: absent in BL2, present in tfm_s.
 *
 * RA6M5 needs none of this. __SAUREGION_PRESENT is 0 there - no SAU at all,
 * BSP_FEATURE_TZ_NS_OFFSET is 0, one alias.
 *
 * ONE REGION, NOT FSP'S FIVE. BL2 has no veneers, so neither NSC region applies, and the
 * only non-secure alias it touches is MRAM's. Everything outside an enabled SAU region stays
 * Secure, which is what it was before this ran, so enabling the SAU with this one region
 * changes exactly one thing. tfm_s installs the full set moments later.
 *
 * -------------------------------------------------------------------------------------
 * THE BOUNDS ARE IMPORTED FROM FSP's rm_mcuboot_port/flash_map.c, flash_area_open():
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
 * Taken verbatim because it is TIGHTER than the architectural alias bounds: CFSAMONA.CFS2 is
 * the Code Flash Secure boundary RDPM programmed, counted in 32 KB blocks, so the region
 * starts exactly where the non-secure half does rather than at the alias base. The IDAU
 * enforces the real split either way, but a region that starts at the boundary cannot mark
 * secure bytes non-secure even transiently.
 *
 * RESTATED rather than included, because rm_mcuboot_port is not built in this configuration:
 * TF-M supplies the MCUboot flash backend (DEFAULT_MCUBOOT_FLASH_MAP=ON) and this is the one
 * piece of that module BL2 still needs. If FSP changes the expression this copy must follow -
 * it is listed in fsp_cmake/BRIDGING_FILES.md for that reason.
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
    SAU->RLAR = (ns_end & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

    SAU->CTRL = SAU_CTRL_ENABLE_Msk;

    /* The attribution change must be in effect before the next load, not merely issued. */
    __DSB();
    __ISB();
#endif
}


int32_t boot_platform_post_init(void)
{
    /* First: nothing below here, and nothing MCUboot does after, can read the non-secure
     * slots until this has run. */
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
