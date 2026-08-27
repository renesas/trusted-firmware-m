/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Build-time guards on the flash layout derived from the e2 solution.
 *
 * Everything here is a static assertion; the file emits no code. It exists because each
 * of these has already been shipped wrong once and was invisible until hardware.
 */

#include "flash_otp_nv_counters_backend.h"
#include "flash_layout.h"
#include "region_defs.h"
/* bsp_api.h, not bsp_cfg.h directly: bsp_cfg.h pulls the board headers, which use
 * FSP_HEADER before it is defined. Entering through bsp_api.h gets the ordering right. */
#include "bsp_api.h"

/* ---------------------------------------------------------------------------------
 * Main stack
 *
 * bsp_init_stub.c aliases FSP's g_main_stack onto Image$$ARM_LIB_STACK$$ZI$$Base, so
 * FSP's SystemInit() sets MSPLIM to the base of the stack the CPU is really running on.
 * The other thing it does with that symbol is write its stack seal at
 * &g_main_stack[BSP_CFG_STACK_MAIN_BYTES]. The generated scatter file sizes .msp_stack
 * as S_MSP_STACK_SIZE - STACKSEAL_SIZE and reserves STACKSEAL_SIZE above it, so the two
 * agree only when S_MSP_STACK_SIZE is BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. If the
 * solution is regenerated with a different FSP main stack size, that write moves off the
 * reserved seal and into whatever the linker put above the stack.
 * --------------------------------------------------------------------------------- */
#ifndef STACKSEAL_SIZE
#define STACKSEAL_SIZE (8)
#endif

_Static_assert(S_MSP_STACK_SIZE == (BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE),
               "RA6E1: S_MSP_STACK_SIZE must be BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. "
               "FSP's SystemInit() writes its stack seal at "
               "&g_main_stack[BSP_CFG_STACK_MAIN_BYTES], which bsp_init_stub.c aliases to "
               "the TF-M stack base - so it must land on the reserved __StackSeal, not "
               "past the top of the stack. Update S_MSP_STACK_SIZE in region_defs.h to "
               "match the regenerated bsp_cfg.h.");

/* ---------------------------------------------------------------------------------
 * MCUboot slot geometry
 *
 * Each slot must be contiguous (span == sum of its components) and a whole number of
 * erase sectors. flash_area_get_sectors() walks a slot sector by sector and returns an
 * error on any remainder, so a misaligned slot appears only as boot_read_sectors()
 * returning BOOT_EFLASH from boot_prepare_image_for_update(). Meanwhile the FSP
 * generator sums the component sizes and imgtool pads to the span, so a gap makes the
 * signed image and the bootloader disagree about where the trailer magic lives.
 * See the note on TFM_SLOT_SPAN in flash_layout.h.
 * --------------------------------------------------------------------------------- */
#define RA6E1_SLOT_SUM_0 (BSP_PARTITION___BL_0_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                          BSP_PARTITION_FLASH_CPU0_C_SIZE + BSP_PARTITION___BL_0_P_T_SIZE)
#define RA6E1_SLOT_SUM_2 (BSP_PARTITION___BL_0_S_H_SIZE + BSP_PARTITION___BL_0_S_I_SIZE + \
                          BSP_PARTITION___BL_0_S_T_SIZE)
#define RA6E1_SLOT_SUM_1 (BSP_PARTITION___BL_1_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_N_SIZE + \
                          BSP_PARTITION___BL_1_P_T_SIZE)
#define RA6E1_SLOT_SUM_3 (BSP_PARTITION___BL_1_S_H_SIZE + BSP_PARTITION___BL_1_S_I_SIZE + \
                          BSP_PARTITION___BL_1_S_T_SIZE)

#define RA6E1_CHECK_SLOT(n)                                                              \
    _Static_assert((FLASH_AREA_##n##_OFFSET % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,        \
                   "RA6E1: slot " #n " does not start on an erase-sector boundary. "     \
                   "boot_read_sectors() would erase from the wrong address on upgrade."); \
    _Static_assert((FLASH_AREA_##n##_SIZE % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,           \
                   "RA6E1: slot " #n " is not a whole number of erase sectors. "          \
                   "flash_area_get_sectors() rejects the remainder and BL2 fails with "   \
                   "BOOT_EFLASH. Fix the partitioning in the e2 solution.");               \
    _Static_assert(FLASH_AREA_##n##_SIZE == RA6E1_SLOT_SUM_##n,                            \
                   "RA6E1: slot " #n " has a gap between its components. The span and "    \
                   "the sum disagree, so imgtool and bootutil would place the trailer "    \
                   "magic differently. Make the solution partitions contiguous.")

RA6E1_CHECK_SLOT(0);
RA6E1_CHECK_SLOT(1);
RA6E1_CHECK_SLOT(2);
RA6E1_CHECK_SLOT(3);

/* A secondary slot holds a copy of the whole primary image. The primary expresses that
 * image as the solution's application regions - two of them for the secure image, since
 * the code and the NSC veneers carry different TrustZone attributes - while the
 * secondary is a single undifferentiated I region. Per the generator schema
 * (linker_macros.j2: "Secondary images must be same size as the area allocated for the
 * project"), the sizes have to match, or an upgrade overruns or wastes the slot. */
_Static_assert(BSP_PARTITION___BL_0_S_I_SIZE ==
                   (BSP_PARTITION_FLASH_CPU0_S_SIZE + BSP_PARTITION_FLASH_CPU0_C_SIZE),
               "RA6E1: image 0 secondary is not the size of the primary image "
               "(FLASH_CPU0_S + FLASH_CPU0_C).");
_Static_assert(BSP_PARTITION___BL_1_S_I_SIZE == BSP_PARTITION_FLASH_CPU0_N_SIZE,
               "RA6E1: image 1 secondary is not the size of the primary image "
               "(FLASH_CPU0_N).");

/* ---------------------------------------------------------------------------------
 * OTP / NV counter area size.
 *
 * flash_otp_nv_counters_backend.c opens init_otp_nv_counters_flash() with
 *
 *     if ((TFM_OTP_NV_COUNTERS_AREA_SIZE) < sizeof(struct flash_otp_nv_counters_region_t))
 *         return TFM_PLAT_ERR_SYSTEM_ERR;
 *
 * Both operands are compile-time constants, so when the area is too small the compiler
 * folds the test and discards the rest of the function - it becomes an unconditional
 * error return that never touches flash. Nothing warns, the image links, and the failure
 * only appears on hardware as OTP, ITS and PS all refusing to initialise.
 *
 * The struct grows with MCUBOOT_BUILTIN_KEY, PLATFORM_NS_NV_COUNTERS and
 * FLASH_NV_COUNTER_AM, none of which this port controls, so this is checked rather than
 * hard-coded. If it fires, give the secure DATA_FLASH partition more room in the e2
 * solution and raise TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h - do not shrink the
 * services to fit.
 */

#if !defined(TFM_OTP_NV_COUNTERS_AREA_SIZE)
#error "RA6E1: TFM_OTP_NV_COUNTERS_AREA_SIZE is not defined - check flash_layout.h"
#endif

/* The backend keeps a primary copy and a backup mirror, each of this size. */
_Static_assert(TFM_OTP_NV_COUNTERS_AREA_SIZE >=
                   sizeof(struct flash_otp_nv_counters_region_t),
               "RA6E1: TFM_OTP_NV_COUNTERS_AREA_SIZE is smaller than "
               "sizeof(struct flash_otp_nv_counters_region_t). "
               "init_otp_nv_counters_flash() would compile to an unconditional failure. "
               "Raise TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h - it is halved to give "
               "the area and its backup mirror.");

/* Both halves must be whole erase sectors: the backend erases with EraseSector() and
 * rejects a start address that is not sector aligned. */
_Static_assert((TFM_OTP_NV_COUNTERS_AREA_SIZE % TFM_OTP_NV_COUNTERS_SECTOR_SIZE) == 0,
               "RA6E1: the OTP area must be a whole number of erase sectors.");
