/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/*
 * Entropy comes from the RSIP-E50D TRNG through PSA's external-RNG hook (sce_trng.c), not
 * from a stored NV seed. These two are checked against each other by
 * secure_fw/partitions/crypto/config_crypto_check.h - exactly one must be set.
 *
 * The NV-seed path would otherwise seed every device from PLAT_OTP_ID_ENTROPY_SEED, whose
 * default provisioning value is a hard-coded constant.
 */
#define CRYPTO_NV_SEED                          0
#define CRYPTO_EXT_RNG                          1

/*
 * Protected Storage sizing. The 2048-byte PS_MAX_ASSET_SIZE default does not fit.
 *
 * RA8M2 has NO data flash. ITS, PS and the NV counters share DF_EMULATION, a 64 KB MRAM
 * region (flash_layout.h), so the geometry is:
 *
 *   DF_EMULATION 65536 = NV counters 2048 + PS 31744 + ITS 31744
 *   PS block_size = 31744/64/2 sectors * 64 = 15872, num_blocks = 2
 *
 * With num_blocks == 2 the flash FS keeps its metadata and logical data block 0 in the
 * same physical block, so every live PS object must fit in one block alongside that
 * metadata. The binding constraint is therefore the SUM, not the per-object ceiling, and
 * it is dominated by fixed overhead that scales with PS_NUM_ASSETS:
 *
 *   FS metadata    8 + 12 + (N+3)*32
 *   object tables  2 * (48 + (N+1)*32)      both FS IDs are always live
 *   resident asset 36 + 72                  TFM_NS_PS_TEST_1004 writes it WRITE_ONCE
 *   => free for one asset object = 15488 - 96*N
 *
 * That must cover PS_MAX_ASSET_SIZE + 72, which caps N at 155. ra8m2_layout_checks.c
 * asserts the whole budget, so a change that breaks it fails the build rather than
 * returning PSA_ERROR_INSUFFICIENT_STORAGE from psa_ps_set at run time.
 *
 * PS_MAX_ASSET_SIZE also has a FLOOR: ps_object_table.c loads the object table into a
 * static buffer of that size and checks the fit with PS_UTILS_BOUND_CHECK, failing as
 * "size of array 'OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF' is negative". Measured on
 * RA6M5 by compiling: 256 and 384 fail, 448 passes. Both ends move with PS_NUM_ASSETS -
 * each extra asset adds 32 to the floor and takes 32 off the ceiling.
 *
 * Unchanged by the larger area: rewriting an existing max-size asset needs both copies
 * resident, 1168 bytes. That fits here, unlike on RA6M5.
 *
 * The investigation behind these formulae was done on RA6M5, whose PS block is 1536 B and
 * whose margin at N=5 is 88 bytes against ~14,400 here. See ra6m5/config_tfm_target.h for
 * that history, including why a failing ITS init reports pid 257 when it is the PS half
 * that fails.
 */
#define PS_MAX_ASSET_SIZE                       512

/* Kept at RA6M5's value, which this part's 15872-byte block clears by ~14,400 bytes. The
 * budget above allows up to 155; raise it here if an application needs more assets. */
#define PS_NUM_ASSETS                           5

#endif /* __CONFIG_TFM_TARGET_H__ */
