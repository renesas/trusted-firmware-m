/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * RA8M2 option-setting memory (OFS) emitter for the TF-M BL2 image.
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
 * ------------------------------------------------------------------------------------------
 * Only the bootloader project's OFS settings land.
 *
 * This file is compiled into platform_bl2, so BSP_CFG_OPTION_SETTING_* resolve through the
 * include path of FSP_BL2_APP_DIR (ra8m2_iar_mcuboot). The addresses likewise come from that
 * project's generated memory_regions.icf, through option_settings.h.
 *
 * Changing OFS in the secure project's e2 configuration has no effect on the image, and nothing
 * warns. Both projects define the same six groups, so the mistake is easy to make and hard to
 * notice: the old bootloader values are still what gets programmed.
 *
 * Change OFS in the bootloader project (ra8m2_iar_mcuboot), regenerate, and rebuild it in e2
 * so its Debug/memory_regions.icf is refreshed. Verify what landed:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * The OFS brick guard also prints the placed words on every build.
 * ------------------------------------------------------------------------------------------
 *
 * This mirrors the RASC bsp_linker.c OFS block for the groups it places. It is not a 1:1 copy
 * and must not become one: bsp_linker.c carries every group the FSP family knows, and this part
 * has a different set from RA6. The guard at the end of this file fails the build if the e2
 * project enables a group this file does not emit.
 */

#include "bsp_api.h"

/* BL2 runs TF-M's Reset_Handler too, so it has the secure image's SystemInit()-before-.bss
 * hazard, and fails first because MCUboot opens the flash before anything else. The secure
 * image's check is in ra8m2_layout_checks.c; this file is BL2-only, so BL2's is here.
 * See DESIGN.md 8.1. */
#if !defined(BSP_CFG_EARLY_INIT) || !(BSP_CFG_EARLY_INIT)
#error "RA8M2: BSP_CFG_EARLY_INIT is 0 in the bootloader e2 project. Set BSP > Early BSP Initialization to Enabled, regenerate and rebuild in e2 - otherwise SystemCoreClock is zeroed after SystemInit(), every MRAM timeout is computed as 0, and the first command that has to wait returns FSP_ERR_TIMEOUT. See DESIGN.md 8.1."
#endif

/* UNCRUSTIFY-OFF */
#ifndef BSP_BOOTLOADED_APPLICATION

/* RA8M2's option-setting groups, not RA6's.
 *
 * The solution defines exactly these six (ra_cfg/fsp_cfg/bsp/bsp_mcu_ofs_cfg.h):
 *     OFS0  OFS2  OFS1_SEC  OFS3_SEC  OFS1_SEL  OFS3_SEL
 * RA8M2 has no DUALSEL, BANKSEL or non-OTP PBPS.
 *
 * Adding a group takes three edits, and the linker ones are not optional: an emitted section
 * with no region is an ld/ILINK error, but a region with a wrong address is silent and
 * permanent. Add it here, then to ra8m2_bl2.ld's MEMORY and placement, then to ra8m2_bl2.icf's
 * region, keep and place.
 *
 * Not yet placed by this port, with their addresses in the generated option_settings.h:
 * SAS, BPS, BPS_SEC (0x80 each), and the whole OTP block: FSBLCTRL0-2, SAMR, SACC00-03,
 * SACC10-13, PBPS, PBPS_SEC, ZHUK. OFS1, OFS3 and OTP_PBPS are non-secure alias addresses and
 * BL2 is built flat, so check the alias before emitting those from this image.
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

/* ============================ Enabled-but-unplaced guard ============================
 *
 * Without this guard, a setting turned on in e2 is silently dropped: nothing above emits a
 * section for it, so there is no linker error and the image boots without the configured
 * option word. For a block-protect or FSBL setting that is a security expectation not met.
 *
 * Every BSP_CFG_OPTION_SETTING_* name FSP recognises for this part that this port does not
 * place is listed below and fails the build if defined. Supporting one takes the three edits
 * described above the emissions, plus deleting its line here.
 *
 * OFS1_SEC_NO_HOCOFRQ is deliberately absent. It is a variant of
 * BSP_CFG_OPTION_SETTING_OFS1_SEC (the same word with the HOCO frequency field left alone),
 * not a separate option word; the solution defines both, and the OFS1_SEC emission above
 * consumes it.
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
