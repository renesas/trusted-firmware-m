/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RA6M5 option-setting memory (OFS) emitter for the TF-M BL2 image.
 *
 * Not FSP's bsp_linker.c: that file emits the same .option_setting_* sections but also
 * includes bsp_linker_info.h, which brings FSP's C-runtime init tables and MCUboot parameters
 * (MCUBOOT_IMAGE_NUMBER, OSPI/nocache copy-table symbols) that conflict with TF-M's BL2
 * startup, linker script and MCUboot config. This file emits only the OFS sections. The values
 * come from the RASC config (BSP_CFG_OPTION_SETTING_* in ra_cfg .../bsp_mcu_ofs_cfg.h, reached
 * through bsp_api.h), so an OFS, clock or watchdog change made in RASC flows through unchanged.
 *
 * BL2 only (DESIGN.md 8.4): the secure and NS images are MCUboot-signed and imgtool needs a
 * contiguous payload, and two images programming OFS would collide. The guards mirror FSP's
 * (#ifndef BSP_BOOTLOADED_APPLICATION for the secure/TZ variants).
 *
 * This mirrors the RASC bsp_linker.c OFS block 1:1; keep it in sync if a future FSP adds an
 * option-setting region.
 */

#include "bsp_api.h"

/* BL2 runs TF-M's Reset_Handler too, so it has the secure image's SystemInit()-before-.bss
 * hazard, and fails first because MCUboot opens the flash before anything else. The secure
 * image's check is in ra6m5_layout_checks.c; this file is BL2-only, so BL2's is here.
 * See DESIGN.md 8.1. */
#if !defined(BSP_CFG_EARLY_INIT) || !(BSP_CFG_EARLY_INIT)
#error "RA6M5: BSP_CFG_EARLY_INIT is 0 in the bootloader e2 project. Set BSP > Early BSP Initialization to Enabled, regenerate and rebuild in e2 - otherwise SystemCoreClock is zeroed after SystemInit() and R_FLASH_HP_Open() fails with FSP_ERR_FCLK. See DESIGN.md 8.1."
#endif

/* UNCRUSTIFY-OFF */
#ifndef BSP_BOOTLOADED_APPLICATION

#if defined BSP_CFG_OPTION_SETTING_OFS0 && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs0") g_bsp_cfg_option_setting_ofs0[] = {BSP_CFG_OPTION_SETTING_OFS0};
#endif
#if defined BSP_CFG_OPTION_SETTING_DUALSEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_dualsel") g_bsp_cfg_option_setting_dualsel[] = {BSP_CFG_OPTION_SETTING_DUALSEL};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs1") g_bsp_cfg_option_setting_ofs1[] = {BSP_CFG_OPTION_SETTING_OFS1};
#endif
#if defined BSP_CFG_OPTION_SETTING_BANKSEL
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_banksel") g_bsp_cfg_option_setting_banksel[] = {BSP_CFG_OPTION_SETTING_BANKSEL};
#endif
#if defined BSP_CFG_OPTION_SETTING_BPS
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_bps") g_bsp_cfg_option_setting_bps[] = {BSP_CFG_OPTION_SETTING_BPS};
#endif
#if defined BSP_CFG_OPTION_SETTING_PBPS
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_pbps") g_bsp_cfg_option_setting_pbps[] = {BSP_CFG_OPTION_SETTING_PBPS};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs1_sec") g_bsp_cfg_option_setting_ofs1_sec[] = {BSP_CFG_OPTION_SETTING_OFS1_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_BANKSEL_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_banksel_sec") g_bsp_cfg_option_setting_banksel_sec[] = {BSP_CFG_OPTION_SETTING_BANKSEL_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_BPS_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_bps_sec") g_bsp_cfg_option_setting_bps_sec[] = {BSP_CFG_OPTION_SETTING_BPS_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_PBPS_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_pbps_sec") g_bsp_cfg_option_setting_pbps_sec[] = {BSP_CFG_OPTION_SETTING_PBPS_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1_SEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs1_sel") g_bsp_cfg_option_setting_ofs1_sel[] = {BSP_CFG_OPTION_SETTING_OFS1_SEL};
#endif
#if defined BSP_CFG_OPTION_SETTING_BANKSEL_SEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_banksel_sel") g_bsp_cfg_option_setting_banksel_sel[] = {BSP_CFG_OPTION_SETTING_BANKSEL_SEL};
#endif
#if defined BSP_CFG_OPTION_SETTING_BPS_SEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_bps_sel") g_bsp_cfg_option_setting_bps_sel[] = {BSP_CFG_OPTION_SETTING_BPS_SEL};
#endif

#endif /* BSP_BOOTLOADED_APPLICATION */
/* UNCRUSTIFY-ON */
