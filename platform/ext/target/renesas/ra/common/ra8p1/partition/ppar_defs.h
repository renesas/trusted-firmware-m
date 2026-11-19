/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PPAR_DEFS_H__
#define __PPAR_DEFS_H__

/*
 * RA8P1 peripheral privilege attribution (PPARB..PPARE) facts.
 *
 * Valid module bits of PPARB..PPARE: the non-reserved bits of the matching
 * PSARB..PSARE (HW manual Rev.1.30: PSARB p.3315, PSARC p.3317, PSARD p.3319,
 * PSARE p.3320; PPARB p.3323, PPARC p.3325, PPARD p.3326, PPARE p.3328).
 * Reserved PPAR bits read 1 and must be written 1; reserved PSAR bits read 0,
 * so they must be masked out when deriving "secure" from PSAR.
 *
 * Bank C differs from RA8M2: bit 15 is the graphics block (GLCDC, MIPI-DSI,
 * MIPI-CSI, DRW), which RA8M2 reserves.
 */
#define RA_PPC_VALID_B  0xFFCF1B90U
#define RA_PPC_VALID_C  0xDD01B983U
#define RA_PPC_VALID_D  0x1E787870U
#define RA_PPC_VALID_E  0xF83C830FU

#endif /* __PPAR_DEFS_H__ */
