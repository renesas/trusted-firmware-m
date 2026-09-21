/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RA6M4 option-setting memory (OFS) emitter for the TF-M BL2 image.
 *
 * WHY THIS FILE EXISTS (and is not just the FSP bsp_linker.c):
 * The RASC-generated FSP bsp_linker.c emits exactly these .option_setting_*
 * sections, but it then #includes bsp_linker_info.h, which drags in FSP's own
 * C-runtime init tables and MCUboot parameters (MCUBOOT_IMAGE_NUMBER, OSPI/
 * nocache copy-table symbols) that conflict with TF-M's BL2 startup/linker/
 * MCUboot config. So we cannot compile bsp_linker.c wholesale here. Instead we
 * emit ONLY the OFS sections, and - crucially - the VALUES come straight from
 * the RASC config (BSP_CFG_OPTION_SETTING_* in ra_cfg .../bsp_mcu_ofs_cfg.h,
 * reached via bsp_api.h). A user who reconfigures clocks/OFS/watchdog in RASC
 * and regenerates flows through here unchanged - no hand-authored OFS values.
 *
 * BL2-ONLY: OFS must be in the bootloader image only. The secure/NS images are
 * MCUboot-signed (imgtool needs a contiguous payload) and two images programming
 * OFS would collide - so this file is compiled into BL2 only, and its guards
 * mirror FSP's (#ifndef BSP_BOOTLOADED_APPLICATION for the secure/TZ variants).
 *
 * This mirrors the RASC bsp_linker.c OFS block 1:1 so it stays faithful to the
 * generated code; keep it in sync if a future FSP adds an option-setting region.
 */

#include "bsp_api.h"

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
