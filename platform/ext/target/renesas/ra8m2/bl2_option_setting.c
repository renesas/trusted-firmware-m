/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RA8M2 option-setting memory (OFS) emitter for the TF-M BL2 image.
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
 * ------------------------------------------------------------------------------------------
 * WHICH PROJECT'S OFS SETTINGS ACTUALLY LAND: THE BOOTLOADER'S. ONLY THE BOOTLOADER'S.
 *
 * This file is compiled into platform_bl2, so BSP_CFG_OPTION_SETTING_* resolves through the
 * include path of FSP_BL2_APP_DIR - ra8m2_iar_mcuboot. The ADDRESSES likewise come from that
 * project's generated memory_regions.icf, via option_settings.h.
 *
 * So: changing OFS in the SECURE project's e2 configuration has NO EFFECT on the image.
 * Nothing warns about it. Both projects currently define the same six groups, which makes the
 * mistake easy to make and impossible to notice - edit the secure project, rebuild, and the
 * old bootloader values are still what gets programmed.
 *
 * Change OFS in the BOOTLOADER project (ra8m2_iar_mcuboot), regenerate, and rebuild it in e2
 * so its Debug/memory_regions.icf is refreshed. Verify what landed rather than assuming:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * and the OFS brick guard prints the placed words on every build.
 * ------------------------------------------------------------------------------------------
 *
 * This mirrors the RASC bsp_linker.c OFS block for the groups it places, so it stays faithful
 * to the generated code. It is NOT a 1:1 copy any more and must not be made one: bsp_linker.c
 * carries every group the FSP family knows, and this part has a different set from the RA6
 * ports this file was seeded from. The guard at the bottom fails the build if the e2 project
 * enables a group this file does not emit, which is the mechanism that keeps the two in step
 * - not a comment asking the next reader to remember.
 */

#include "bsp_api.h"

/* BL2 runs TF-M's Reset_Handler too, so it has the same SystemInit()-before-.bss-zeroing
 * hazard as the secure image, and it is the image that fails first: MCUboot opens the
 * flash before anything else. See the matching check in ra8m2_layout_checks.c and
 * DESIGN.md 8.1. This file is BL2-only, which is why the check lives here. */
#if !defined(BSP_CFG_EARLY_INIT) || !(BSP_CFG_EARLY_INIT)
#error "RA8M2: BSP_CFG_EARLY_INIT is 0 in the bootloader e2 project. Set BSP > Early BSP Initialization to Enabled, regenerate and rebuild in e2 - otherwise SystemCoreClock is zeroed after SystemInit() and R_MRAM_Open() fails with FSP_ERR_FCLK. See DESIGN.md 8.1."
#endif

/* UNCRUSTIFY-OFF */
#ifndef BSP_BOOTLOADED_APPLICATION

/* RA8M2's option-setting groups - NOT RA6's.
 *
 * The solution defines exactly these six (ra_cfg/fsp_cfg/bsp/bsp_mcu_ofs_cfg.h):
 *     OFS0  OFS2  OFS1_SEC  OFS3_SEC  OFS1_SEL  OFS3_SEL
 *
 * OFS2, OFS3_SEC and OFS3_SEL DID NOT EXIST on RA6E1/RA6M5 and were missing here until
 * 2026-09-24: this file was seeded from RA6M5, so those three settings were configured in e2
 * and then silently dropped from the image - no error anywhere, because the RA6 groups they
 * were replaced by are simply #undef on this part and compiled out. Conversely DUALSEL,
 * BANKSEL and the non-OTP PBPS have been removed; RA8M2 has none of them.
 *
 * ADDING A GROUP TAKES THREE EDITS, and the linker ones are not optional: an emitted section
 * with no region is an ld/ILINK error (good), but a region with a WRONG address is silent and
 * permanent. Add it here, then to ra8m2_bl2.ld's MEMORY + placement, then to
 * ra8m2_bl2.icf's region + keep + place.
 *
 * Not yet placed by this port, with their addresses in the generated option_settings.h:
 * SAS, BPS, BPS_SEC (0x80 each), and the whole OTP block - FSBLCTRL0-2, SAMR, SACC00-03,
 * SACC10-13, PBPS, PBPS_SEC, ZHUK. Note OFS1, OFS3 and OTP_PBPS are non-secure alias
 * addresses; BL2 is built flat, so think before emitting those from this image.
 */
#if defined BSP_CFG_OPTION_SETTING_OFS0 && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs0") g_bsp_cfg_option_setting_ofs0[] = {BSP_CFG_OPTION_SETTING_OFS0};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS2 && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs2") g_bsp_cfg_option_setting_ofs2[] = {BSP_CFG_OPTION_SETTING_OFS2};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs1_sec") g_bsp_cfg_option_setting_ofs1_sec[] = {BSP_CFG_OPTION_SETTING_OFS1_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS3_SEC && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs3_sec") g_bsp_cfg_option_setting_ofs3_sec[] = {BSP_CFG_OPTION_SETTING_OFS3_SEC};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1_SEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs1_sel") g_bsp_cfg_option_setting_ofs1_sel[] = {BSP_CFG_OPTION_SETTING_OFS1_SEL};
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS3_SEL && !BSP_TZ_NONSECURE_BUILD
BSP_DONT_REMOVE static const uint32_t BSP_PLACE_IN_SECTION(".option_setting_ofs3_sel") g_bsp_cfg_option_setting_ofs3_sel[] = {BSP_CFG_OPTION_SETTING_OFS3_SEL};
#endif

/* ============================ ENABLED-BUT-UNPLACED GUARD ============================
 *
 * The hazard this closes: a user turns a setting ON in e2, rebuilds the TF-M port, and the
 * setting is SILENTLY DROPPED. Nothing above emits a section for it, so there is no section,
 * so no linker error, so no diagnostic anywhere - the image just boots without the option
 * word the user configured. For a block-protect or FSBL setting that is a security
 * expectation quietly not met.
 *
 * It is not hypothetical: OFS2, OFS3_SEC and OFS3_SEL were configured in this very solution
 * and dropped exactly this way, because this file was seeded from RA6M5 and RA6M5 has no such
 * groups. It went unnoticed until the OFS addresses were audited on 2026-09-24.
 *
 * So every BSP_CFG_OPTION_SETTING_* name FSP recognises for this part and that this port does
 * NOT place is listed below and fails the build if it appears. Adding support is three edits -
 * see the note at the top of this file - and deleting the line here is the fourth, which is
 * the point: you cannot enable one without being told what else to do.
 *
 * NOTE OFS1_SEC_NO_HOCOFRQ is deliberately absent from this list. It is a VARIANT of
 * BSP_CFG_OPTION_SETTING_OFS1_SEC (the same word with the HOCO frequency field left alone),
 * not a separate option word, and the solution defines both. It is consumed through the
 * OFS1_SEC emission above.
 * =================================================================================== */
#if defined BSP_CFG_OPTION_SETTING_BPS
#error "RA8M2: BSP_CFG_OPTION_SETTING_BPS is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_BPS_SEC
#error "RA8M2: BSP_CFG_OPTION_SETTING_BPS_SEC is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_DUALSEL
#error "RA8M2: BSP_CFG_OPTION_SETTING_DUALSEL is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1
#error "RA8M2: BSP_CFG_OPTION_SETTING_OFS1 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS1_ICSATS
#error "RA8M2: BSP_CFG_OPTION_SETTING_OFS1_ICSATS is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OFS3
#error "RA8M2: BSP_CFG_OPTION_SETTING_OFS3 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_SAS
#error "RA8M2: BSP_CFG_OPTION_SETTING_SAS is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL0
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL0 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL1
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL1 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL2
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_FSBLCTRL2 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_PBPS
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_PBPS is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_PBPS_SEC
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_PBPS_SEC is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC00
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC00 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC01
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC01 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC02
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC02 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC03
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC03 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC10
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC10 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC11
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC11 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC12
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC12 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SACC13
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SACC13 is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_SAMR
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_SAMR is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif
#if defined BSP_CFG_OPTION_SETTING_OTP_ZHUK
#error "RA8M2: BSP_CFG_OPTION_SETTING_OTP_ZHUK is set in the e2 project but this port does not place it. It would be silently dropped. Add the emission here plus a region and placement in BOTH ra8m2_bl2.ld and ra8m2_bl2.icf, then remove this guard - addresses are in the generated option_settings.h."
#endif

#endif /* BSP_BOOTLOADED_APPLICATION */
/* UNCRUSTIFY-ON */
