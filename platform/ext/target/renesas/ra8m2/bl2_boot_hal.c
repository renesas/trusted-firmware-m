/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * BL2 boot HAL override. Two jobs, both of which must happen before MCUboot touches a slot:
 * enable the SAU so BL2 can reach the non-secure half of MRAM at all, and - when BL2's
 * SHA-256 runs on the engine - bring up the RSIP-E50D.
 *
 * bl2_main.c calls boot_platform_post_init() after boot_platform_init() and before
 * boot_go_for_image_id() reads the first image header, which is the earliest point where
 * either matters and the latest point where it is still in time.
 *
 * Why the SCE bring-up is here and not in crypto_hw_accelerator_init(): the weak default of
 * this function in platform/ext/common/boot_hal_bl2.c does call that, but in BL2 it resolves
 * to the stub bl2_main.c defines for MCUBOOT_USE_PSA_CRYPTO, which does nothing and cannot
 * be overridden (it is not weak).
 *
 * The weak default does nothing at all without CRYPTO_HW_ACCELERATOR, so overriding it
 * unconditionally loses nothing.
 */

#include <stdint.h>

#include "boot_hal.h"
#include "fih.h"
#include "tfm_hal_device_header.h"

#ifdef RA8M2_BL2_SCE_INIT
#include "ra_sce_init.h"
#endif

/*
 * ===================== BL2 MUST ENABLE THE SAU ON THIS PART =====================
 *
 * BL2 reads the non-secure image headers out of MRAM's non-secure alias, 0x120B0000 and up
 * (Driver_Flash.c, MRAM_ADDR). On a part RDPM has partitioned, that is the ONLY alias that
 * reaches those bytes - the secure alias stops at the Code Secure boundary.
 *
 * Reaching them also needs the access itself to be non-secure, and that is what this
 * function is for. With the SAU disabled and SAU_CTRL.ALLNS clear, Armv8-M attributes every
 * address as Secure, and the combined SAU/IDAU attribute takes the more secure of the two -
 * so BL2's load from 0x120B0000 goes out as a SECURE transaction to the non-secure alias
 * and is refused. Both aliases then fault, which is exactly the symptom: the secure one
 * because of the address, the non-secure one because of the attribute.
 *
 * FSP does this in R_BSP_SAUInit(), reached from SystemInit() via R_BSP_SecurityInit() -
 * but that call sits inside #if BSP_TZ_SECURE_BUILD and BL2 is built as the flat FSP role
 * (fsp_bsp.cmake), deliberately, so BL2 gets neither. Confirm with
 * `arm-none-eabi-nm bl2.elf | grep SAUInit`: absent in BL2, present in tfm_s.
 *
 * RA6M5 needed none of this. __SAUREGION_PRESENT is 0 there - no SAU at all,
 * BSP_FEATURE_TZ_NS_OFFSET is 0, one alias - and this port's BL2 was derived from it.
 *
 * ONE REGION, NOT FSP'S FIVE. BL2 has no veneers, so neither NSC region applies, and the
 * only non-secure alias it touches is MRAM's. Everything outside an enabled SAU region
 * stays Secure, which is also what it was before this ran, so enabling the SAU with this
 * one region changes exactly one thing. tfm_s installs the full set moments later.
 *
 * The bounds are the IDAU's, restated from BSP_PRV_SAU_NS_REGION_1_* in FSP's
 * bsp_security.c because those are private to that file. They are architectural for the
 * part, not layout-derived: the NS alias of code space is 0x10000000-0x1FFFFFFF whatever
 * the partitioning is, and the partitioning decides only which bytes answer inside it.
 * ================================================================================
 */
#define BL2_SAU_REGION_MRAM_NS        (0U)
#define BL2_SAU_MRAM_NS_BASE_ADDRESS  (0x10000000U)
#define BL2_SAU_MRAM_NS_LIMIT_ADDRESS (0x1FFFFFFFU)

static void bl2_sau_init(void)
{
#if defined(__SAUREGION_PRESENT) && (__SAUREGION_PRESENT == 1U)
    SAU->RNR  = BL2_SAU_REGION_MRAM_NS;
    SAU->RBAR = BL2_SAU_MRAM_NS_BASE_ADDRESS & SAU_RBAR_BADDR_Msk;
    SAU->RLAR = (BL2_SAU_MRAM_NS_LIMIT_ADDRESS & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

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

#ifdef RA8M2_BL2_SCE_INIT
    if (FSP_SUCCESS != ra_sce_init()) {
        return 1;
    }

    /* As the weak default does under CRYPTO_HW_ACCELERATOR. */
    fih_delay_init();
#endif

    return 0;
}
