/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * DDSC boundary symbols for FSP's bsp_security.c: the TCM pair only.
 *
 * ============================== Why this file exists ==============================
 *
 * FSP's R_BSP_SecurityInit() (bsp_security.c) programmes the SAU and the TrustZone
 * attribution on this port; that is why target_cfg.c's sau_and_idau_cfg() is empty. Most of
 * it takes its boundaries from the generated partition macros:
 *
 *     #if defined(BSP_PARTITION_FLASH_CPU0_S_START) && (0U == BSP_CFG_CPU_CORE)
 *         ... BSP_PARTITION_FLASH_CPU0_C_START, BSP_PARTITION_RAM_CPU0_N_START, ...
 *     #else
 *         ... gp_ddsc_FLASH_END, gp_ddsc_RAM_NSC, gp_ddsc_DATA_FLASH_END      <- NOT us
 *     #endif
 *
 * Both conditions hold here, so the partition branch is taken and no FLASH/RAM gp_ddsc
 * symbol is referenced.
 *
 * ITCM and DTCM attribution sits outside that choice, guarded only by
 *
 *     #if (BSP_CFG_CPU_CORE == 0) && (BSP_FEATURE_BSP_HAS_ITCM)   // and _HAS_DTCM
 *
 * which holds on RA8M2 CPU0. It computes the secure block count from
 * gp_ddsc_ITCM_START/END and gp_ddsc_DTCM_START/END unconditionally:
 *
 *     itcm_num_sec_blocks = (gp_ddsc_ITCM_END + blksz - 1 - gp_ddsc_ITCM_START) >> exponent;
 *     MEMSYSCTL->ITGU_LUT[0] = ~((1U << itcm_num_sec_blocks) - 1U);
 *
 * FSP normally defines those four in bsp_linker.c, with __section_begin("__ddsc_ITCM_START")
 * over marker sections its own generated linker script emits. This port uses TF-M's linker
 * script and excludes bsp_linker.c (cmake/modules/fsp_bsp.cmake), so without this file the
 * four symbols are undefined at link.
 *
 * RA6E1 and RA6M5 need nothing like it: their Cortex-M33 has no TCM,
 * BSP_FEATURE_BSP_HAS_ITCM and _HAS_DTCM are 0, and their built images reference no gp_ddsc
 * symbol.
 *
 * Secure role only. R_BSP_SecurityInit() is inside #if BSP_TZ_SECURE_BUILD, and BL2 is built
 * flat by design (CMakeLists.txt, FSP_TZ_DEFS_BL2 is empty), so BL2 compiles none of this and
 * must not link this file.
 *
 * ============================== What the values mean ==============================
 *
 * They come from the solution's TCM partitioning, the same source the FLASH/RAM branch uses,
 * so all of bsp_security.c describes one consistent map:
 *
 *     ITCM_CPU0_S   0x00000000  0x10000      DTCM_CPU0_S   0x20000000  0x10000
 *     ITCM_CPU0_N   0x10010000  0x10000      DTCM_CPU0_N   0x30010000  0x10000
 *
 * The lower half of each TCM is secure and the upper half non-secure, and the block
 * arithmetic above sees (END - START) = 0x10000 in both cases. Only the difference matters;
 * the real addresses are used so that a debugger shows meaningful values.
 *
 * TF-M places nothing in TCM (region_defs.h maps neither), so this attribution constrains
 * nothing the secure image relies on. It leaves the hardware in the state the solution
 * describes rather than whatever ITGU_LUT/DTGU_LUT hold after reset. If the secure image
 * starts using ITCM, this file must agree with region_defs.h, and making the whole TCM secure
 * means pointing END at the end of the whole TCM, not of the secure partition.
 *
 * Do not extend this to the FLASH/RAM/DATA_FLASH gp_ddsc symbols. If those become undefined,
 * the partition branch stopped being selected (BSP_PARTITION_FLASH_CPU0_S_START vanished or
 * BSP_CFG_CPU_CORE is no longer 0); find out why rather than defining them here. The generated
 * bsp_linker_info.h declares all twelve; only these four are defined here.
 */

#include <stdint.h>
#include "bsp_api.h"          /* BSP_DONT_REMOVE, and BSP_PARTITION_* via bsp_linker_info.h */

/* The generated bsp_linker_info.h declares
 *     extern uint32_t const * const gp_ddsc_ITCM_START;
 * so the definitions must match exactly: const pointer to const uint32_t, at file scope.
 * BSP_DONT_REMOVE keeps them through --gc-sections; the only references are from
 * bsp_security.c in another translation unit, and IAR otherwise drops them as unused data. */

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_START =
    (uint32_t const *) (BSP_PARTITION_ITCM_CPU0_S_START);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_END =
    (uint32_t const *) (BSP_PARTITION_ITCM_CPU0_S_START + BSP_PARTITION_ITCM_CPU0_S_SIZE);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_START =
    (uint32_t const *) (BSP_PARTITION_DTCM_CPU0_S_START);

BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_END =
    (uint32_t const *) (BSP_PARTITION_DTCM_CPU0_S_START + BSP_PARTITION_DTCM_CPU0_S_SIZE);

/* Fail the build if the solution stops describing the TCM split, rather than silently
 * attributing zero secure blocks and giving the whole TCM to the non-secure side. */
#if (BSP_PARTITION_ITCM_CPU0_S_SIZE) == 0
#error "RA8M2: ITCM_CPU0_S is empty - bsp_security.c would mark all of ITCM non-secure"
#endif
#if (BSP_PARTITION_DTCM_CPU0_S_SIZE) == 0
#error "RA8M2: DTCM_CPU0_S is empty - bsp_security.c would mark all of DTCM non-secure"
#endif
