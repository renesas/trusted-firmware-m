/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Force-included into FSP's rm_mcuboot_port/flash_map.c ONLY, by
 * cmake/modules/fsp_mcuboot_port.cmake. Nothing #includes this by name.
 *
 * ================= TF-M's MCUboot IS NOT THE ONE RASC SHIPS =================
 *
 * DESIGN.md 5 records the two as byte-identical. They are not. Renesas's fork, at
 * <bootloader project>/ra/mcu-tools/MCUboot, has a larger boot_hooks.h - 287 lines against
 * TF-M's 181 - carrying three hook macros TF-M's copy does not define at all:
 *
 *     BOOT_HOOK_FLASH_AREA_CALL
 *     BOOT_HOOK_FIND_SLOT_CALL
 *     BOOT_HOOK_GO_CALL_FIH
 *
 * flash_map.c uses exactly ONE of them, once, in flash_area_id_from_multi_image_slot():
 *
 *     rc = BOOT_HOOK_FLASH_AREA_CALL(flash_area_id_from_multi_image_slot_hook,
 *                                    BOOT_HOOK_REGULAR, image_index, slot, &id);
 *
 * Without it the preprocessor leaves the line alone and the compiler reports
 * flash_area_id_from_multi_image_slot_hook undeclared - which is the only thing standing
 * between TF-M's BL2 and FSP's flash backend.
 *
 * MCUBOOT_FLASH_AREA_ID_HOOKS is off on both sides, and in that branch Renesas's macro is
 * HOOK_CALL_NOP(f, ret_default, ...), i.e. ret_default with f never evaluated. The
 * definition below is that branch, and nothing else. The alternative was repointing
 * MCUBOOT_PATH at Renesas's fork and re-supplying the build glue RASC strips (DESIGN.md 5) -
 * a far larger change for one no-op macro.
 *
 * WHEN TO DELETE THIS: if TF-M ever uprevs to an MCUboot that defines the macro itself. The
 * #ifndef below means that would NOT fail loudly, so the guard is here to keep the build
 * working rather than to detect the uprev - check for it when bumping MCUBOOT_PATH.
 * ============================================================================
 */

#ifndef RA6M5_MCUBOOT_HOOK_SHIM_H
#define RA6M5_MCUBOOT_HOOK_SHIM_H

/* boot_hooks.h declares nothing it needs - no stddef, no stdbool, no fih. The force-include
 * list supplies stddef.h and fault_injection_hardening.h; stdbool is here because this file
 * is what pulls boot_hooks.h in first. */
#include <stdbool.h>
#include <stddef.h>

#include "bootutil/boot_hooks.h"

#ifndef BOOT_HOOK_FLASH_AREA_CALL
  #ifdef MCUBOOT_FLASH_AREA_ID_HOOKS
    #error "RA6M5: MCUBOOT_FLASH_AREA_ID_HOOKS is set, but TF-M's bootutil has no BOOT_HOOK_FLASH_AREA_CALL to implement it. Either turn the hooks off, or move BL2 to Renesas's MCUboot fork - see the note above."
  #endif
  #define BOOT_HOOK_FLASH_AREA_CALL(f, ret_default, ...)   ret_default
#endif

#endif /* RA6M5_MCUBOOT_HOOK_SHIM_H */
