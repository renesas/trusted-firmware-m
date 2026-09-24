/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDSC boundary symbols for FSP's bsp_security.c - the TCM pair only.
 *
 * ============================== WHY THIS FILE EXISTS ==============================
 *
 * FSP's bsp_security.c (R_BSP_SecurityInit) programs the SAU and the TrustZone attribution
 * on this port - that is why target_cfg.c's sau_and_idau_cfg() is an empty hook. Most of it
 * takes its boundaries from the generated partition macros:
 *
 *     #if defined(BSP_PARTITION_FLASH_CPU0_S_START) && (0U == BSP_CFG_CPU_CORE)
 *         ... BSP_PARTITION_FLASH_CPU0_C_START, BSP_PARTITION_RAM_CPU0_N_START, ...
 *     #else
 *         ... gp_ddsc_FLASH_END, gp_ddsc_RAM_NSC, gp_ddsc_DATA_FLASH_END      <- NOT us
 *     #endif
 *
 * Both conditions hold here, so the partition branch is taken and none of the FLASH/RAM
 * gp_ddsc symbols is referenced. Good.
 *
 * The ITCM and DTCM attribution is DIFFERENT. It sits outside that choice, guarded only by
 *
 *     #if (BSP_CFG_CPU_CORE == 0) && (BSP_FEATURE_BSP_HAS_ITCM)   // and _HAS_DTCM
 *
 * both of which are true on RA8M2 CPU0, and it computes the secure block count from
 * gp_ddsc_ITCM_START/END and gp_ddsc_DTCM_START/END unconditionally:
 *
 *     itcm_num_sec_blocks = (gp_ddsc_ITCM_END + blksz - 1 - gp_ddsc_ITCM_START) >> exponent;
 *     MEMSYSCTL->ITGU_LUT[0] = ~((1U << itcm_num_sec_blocks) - 1U);
 *
 * Normally FSP supplies those four from bsp_linker.c, which defines them with
 * __section_begin("__ddsc_ITCM_START") over marker sections its OWN generated linker script
 * emits. This port uses TF-M's linker script and EXCLUDES bsp_linker.c
 * (cmake/modules/fsp_bsp.cmake), so the markers do not exist and the four symbols are
 * undefined at link. Hence this file.
 *
 * WHY RA6E1 AND RA6M5 NEED NOTHING LIKE IT: Cortex-M33, no TCM. BSP_FEATURE_BSP_HAS_ITCM and
 * _HAS_DTCM are 0 there, so the blocks above compile out and bsp_security.c references no
 * gp_ddsc symbol at all - verified, their built images contain none. RA8M2's M85 is the first
 * part in this port with TCM, which is what makes these four load-bearing here.
 *
 * SECURE ROLE ONLY. R_BSP_SecurityInit lives inside #if BSP_TZ_SECURE_BUILD, and BL2 is
 * deliberately built FLAT (CMakeLists.txt, FSP_TZ_DEFS_BL2 is empty), so the bootloader
 * compiles none of this and must not link this file.
 *
 * ============================== WHAT THE VALUES MEAN ==============================
 *
 * Taken from the solution's own TCM partitioning, which is the same source the FLASH/RAM
 * branch uses - so all of bsp_security.c ends up describing one consistent map:
 *
 *     ITCM_CPU0_S   0x00000000  0x10000      DTCM_CPU0_S   0x20000000  0x10000
 *     ITCM_CPU0_N   0x10010000  0x10000      DTCM_CPU0_N   0x30010000  0x10000
 *
 * so the lower half of each TCM is secure and the upper half non-secure, and the block
 * arithmetic above sees (END - START) = 0x10000 in both cases. Only the DIFFERENCE matters -
 * the absolute base cancels - but the real addresses are used anyway so that a debugger
 * inspecting these reads something meaningful.
 *
 * TF-M PLACES NOTHING IN TCM. region_defs.h maps neither, so this attribution constrains
 * nothing the secure image itself relies on today; it is here so the hardware is left in the
 * state the solution describes rather than whatever ITGU_LUT/DTGU_LUT happen to hold after
 * reset. If a future revision starts using ITCM for the secure image, this is the file that
 * has to agree with region_defs.h - and making the whole TCM secure means pointing END at
 * the end of the WHOLE TCM, not just the secure partition.
 *
 * DO NOT extend this to the FLASH/RAM/DATA_FLASH gp_ddsc symbols. If those ever become
 * undefined it means the partition branch stopped being selected - because
 * BSP_PARTITION_FLASH_CPU0_S_START vanished or BSP_CFG_CPU_CORE stopped being 0 - and the
 * fix is to find out why, not to paper over it here. The generated bsp_linker_info.h
 * declares all twelve; only these four are ours to define.
 */

#include <stdint.h>
#include "bsp_api.h"          /* BSP_DONT_REMOVE, and BSP_PARTITION_* via bsp_linker_info.h */

/* The declarations in the generated bsp_linker_info.h are
 *     extern uint32_t const * const gp_ddsc_ITCM_START;
 * so the definitions must match exactly - const pointer to const uint32_t, at file scope.
 * BSP_DONT_REMOVE keeps them through --gc-sections: the only references are from
 * bsp_security.c in a different translation unit, and IAR in particular will otherwise drop
 * them as unused data. */

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_START =
    (uint32_t const *) (BSP_PARTITION_ITCM_CPU0_S_START);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_END =
    (uint32_t const *) (BSP_PARTITION_ITCM_CPU0_S_START + BSP_PARTITION_ITCM_CPU0_S_SIZE);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_START =
    (uint32_t const *) (BSP_PARTITION_DTCM_CPU0_S_START);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_END =
    (uint32_t const *) (BSP_PARTITION_DTCM_CPU0_S_START + BSP_PARTITION_DTCM_CPU0_S_SIZE);

/* Catch a solution that stops describing the TCM split, rather than silently attributing
 * zero secure blocks and handing the whole TCM to the non-secure side. */
#if (BSP_PARTITION_ITCM_CPU0_S_SIZE) == 0
#error "RA8M2: ITCM_CPU0_S is empty - bsp_security.c would mark all of ITCM non-secure"
#endif
#if (BSP_PARTITION_DTCM_CPU0_S_SIZE) == 0
#error "RA8M2: DTCM_CPU0_S is empty - bsp_security.c would mark all of DTCM non-secure"
#endif
