/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PPAR_DEFS_H__
#define __PPAR_DEFS_H__

/*
 * RA8M2 peripheral privilege attribution (PPARB..PPARE) facts.
 *
 * Valid module bits of PPARB..PPARE: the non-reserved bits of the matching
 * PSARB..PSARE (HW manual Rev.1.30, PSARB..PSARE 51.8.1..51.8.4, PPARB..PPARE
 * 51.8.6..51.8.9). Reserved PPAR bits read 1 and must be written 1; reserved
 * PSAR bits read 0, so they must be masked out when deriving "secure" from PSAR.
 */
#define RA_PPC_VALID_B  0xFFCF1B90U
#define RA_PPC_VALID_C  0xDD013983U
#define RA_PPC_VALID_D  0x1E787870U
#define RA_PPC_VALID_E  0xF83C830FU

#endif /* __PPAR_DEFS_H__ */
