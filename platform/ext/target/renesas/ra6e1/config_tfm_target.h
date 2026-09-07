/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/*
 * Entropy comes from the SCE9 TRNG through PSA's external-RNG hook (sce_trng.c), not
 * from a stored NV seed. These two are checked against each other by
 * secure_fw/partitions/crypto/config_crypto_check.h - exactly one must be set.
 *
 * The NV-seed path would otherwise seed every device from PLAT_OTP_ID_ENTROPY_SEED, whose
 * default provisioning value is a hard-coded constant.
 */
#define CRYPTO_NV_SEED                          0
#define CRYPTO_EXT_RNG                          1

/*
 * Protected Storage object size. The 2048-byte default does not fit this part.
 *
 * tfm_its_init() initialises PS as well as ITS, and both go through
 * its_flash_fs_validate_config(). With the RA6E1's 8 KB data flash the geometry is:
 *
 *   secure data flash 8192 = NV counters 2048 + PS 3072 + ITS 3072
 *   PS  block_size = 3072/2 = 1536, num_blocks = 2   (flash_layout.h halves the area so
 *   ITS block_size = 3072/2 = 1536, num_blocks = 2    the FS has two blocks to rotate)
 *
 * With num_blocks == 2 the filesystem stores metadata and logical data block 0 in the
 * same physical block, so the largest file must fit in what is left of that block:
 *
 *   max_file_size <= block_size - all_metadata_size
 *   all_metadata_size = 8 + 1*12 + (PS_NUM_ASSETS + 3)*32 = 436 for PS
 *   => PS_MAX_OBJECT_SIZE <= 1536 - 436 = 1100
 *
 * PS_MAX_OBJECT_SIZE is sizeof(struct ps_object_t) - the asset plus 72 bytes of object
 * header (with PS_ENCRYPTION on). The default 2048 gives 2120, which fails BOTH that
 * check and the plainer "max_file_size > block_size", so its_flash_fs_init_ctx() returns
 * PSA_ERROR_INVALID_ARGUMENT (-135) and the whole ITS partition init panics the SPM. That
 * cost a long detour on 2026-08-29 because the failing pid reported is 257 (ITS) even
 * though it is the PS half that fails, and the ITS half succeeds silently before it.
 *
 * The general constraint if these are retuned is:
 *
 *   PS_MAX_ASSET_SIZE <= 1348 - (32 * PS_NUM_ASSETS)
 *
 * which at the default PS_NUM_ASSETS of 10 caps the asset at 1028 - i.e. 1024 would fit,
 * but with only 4 bytes to spare. If PS genuinely needs larger objects, give PS more of
 * the data flash than ITS in flash_layout.h (they are currently split evenly): a 4096/2048
 * PS/ITS split raises the PS cap to 1540 while still leaving ITS a workable 652.
 *
 * ---------------------------------------------------------------------------------------
 * A LOWER BOUND exists too, found 2026-08-30 while trying to reduce this value.
 * ps_object_table.c loads the object table into the static object-data buffer, whose size
 * is PS_MAX_ASSET_SIZE, and asserts the fit at compile time:
 *
 *   PS_UTILS_BOUND_CHECK(OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF,
 *                        PS_OBJ_TABLE_SIZE, PS_MAX_ASSET_SIZE);
 *
 * failing as "size of array 'OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF' is negative".
 * sizeof(struct ps_obj_table_t) is PS_OBJ_TABLE_ENTRIES - PS_NUM_ASSETS + 1, not
 * PS_NUM_ASSETS - entries of 32 bytes, plus crypto metadata. MEASURED by compiling: 256
 * and 384 fail, 448 passes, so the floor is in (384, 448].
 *
 * So the usable band is only about [448, 1028], and BOTH ends move with PS_NUM_ASSETS:
 * each extra asset adds 32 to the floor and takes 32 off the ceiling. This cannot be
 * reduced far without also cutting PS_NUM_ASSETS.
 *
 * TESTED 2026-08-30: tf-m-tests TFM_NS_PS_TEST_1025 fails its psa_ps_set at BOTH 512 and
 * 448, so the failure is NOT size-related and this is back at 512. Still open; see
 * RA6E1_SOLUTION.md. The remaining suspect is accumulated storage state rather than any
 * limit here - the test itself prints "Wipe the storage area to run the full test", and
 * WRITE_ONCE_UID is by definition unremovable and survives every run.
 * ---------------------------------------------------------------------------------------
 */
#define PS_MAX_ASSET_SIZE                       512

#endif /* __CONFIG_TFM_TARGET_H__ */
