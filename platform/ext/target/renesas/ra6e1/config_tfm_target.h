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
 * PROVISIONAL, 2026-08-30: reduced from 512 to 448 as a DIAGNOSTIC, not as a settled value.
 *
 * The tf-m-tests regression run passes 4 of 5 NS suites but fails one PS test:
 * TFM_NS_PS_TEST_1025, "Set, get and remove interface with different asset sizes", at
 * psa_ps_ns_interface_testsuite.c:1680 - "Set should not fail with valid UID". That test
 * scales itself off THIS constant, writing PS_MAX_ASSET_SIZE>>2, >>1 and then the full
 * value, removing each asset before the next, so at 512 its largest single set is 512 B.
 *
 * Ruled out by inspection, so this is not one of them:
 *   - config mismatch. The NS test compiles against this very header via
 *     TARGET_CONFIG_HEADER_FILE, so both sides agree on the value.
 *   - cumulative capacity. The test removes each asset inside the loop; only one is live.
 *   - the write-once asset. WRITE_ONCE_UID genuinely cannot be removed and persists for
 *     the whole run, but it is ~44 bytes.
 *   - PS_AES_KEY_USAGE_LIMIT. Its sanity check is exactly about the largest allowable
 *     object, but it is #if'd out at the configured 0.
 *   - this cap. 512 is well under the 1028 the formula above allows.
 *
 * What remains is free space or fragmentation in the single 1536-byte data block, of which
 * 436 is metadata and ~584 would be one encrypted 512-byte object, after tests 1001-1024
 * have churned it. The test prints no status code, so that is a hypothesis.
 *
 * There is also a LOWER bound, which 256 turned out to violate: ps_object_table.c loads
 * the object table into the static object-data buffer, whose size is PS_MAX_ASSET_SIZE,
 * and asserts the fit at compile time -
 *
 *   PS_UTILS_BOUND_CHECK(OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF,
 *                        PS_OBJ_TABLE_SIZE, PS_MAX_ASSET_SIZE);
 *
 * failing as "size of array 'OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF' is negative".
 * sizeof(struct ps_obj_table_t) is PS_OBJ_TABLE_ENTRIES - which is PS_NUM_ASSETS + 1, not
 * PS_NUM_ASSETS - entries of 32 bytes (16-byte AEAD tag, 8-byte uid, 4-byte client_id,
 * padded), plus the crypto metadata and version. MEASURED by compiling: 256 and 384 both
 * fail the assert, 448 passes, so the floor lies in (384, 448].
 *
 * So the usable band is roughly [448, 1028] and BOTH ends move with PS_NUM_ASSETS: each
 * extra asset adds 32 to the floor and takes 32 off the ceiling, closing it from both
 * sides. That is much narrower than it looks, and it means PS_MAX_ASSET_SIZE cannot be
 * reduced far without also cutting PS_NUM_ASSETS.
 *
 * 448 is therefore the bisect point - the smallest value that compiles - giving cycles
 * 112/224/448. It is only 12% below 512, so the two outcomes are correspondingly sharp:
 * if 1025 PASSES, PS really can hold a 448-byte object but not a 512-byte one in this
 * geometry, and the documented cap should become 448. If it still FAILS, size is not the
 * cause at all and this should go back to 512 while the real reason is found.
 *
 * Either way the underlying constraint is the part, not the code: 8 KB of data flash,
 * 2 KB of it NV counters, leaves PS a single 1536-byte block, and TF-M's PS suite is
 * written for platforms with considerably more. RESOLVE THIS COMMENT once the re-run
 * says which - do not leave 256 in place as though it were a considered choice.
 * ---------------------------------------------------------------------------------------
 */
#define PS_MAX_ASSET_SIZE                       448

#endif /* __CONFIG_TFM_TARGET_H__ */
