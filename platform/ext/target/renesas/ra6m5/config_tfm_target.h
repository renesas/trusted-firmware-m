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
 * its_flash_fs_validate_config(). With the RA6M5's 8 KB data flash the geometry is:
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
 * PS_MAX_OBJECT_SIZE is sizeof(struct ps_object_t): the asset plus 72 bytes of object header
 * (with PS_ENCRYPTION on). The 2048 default gives 2120, which fails both that check and
 * "max_file_size > block_size", so its_flash_fs_init_ctx() returns
 * PSA_ERROR_INVALID_ARGUMENT (-135) and the ITS partition init panics the SPM. The panic
 * reports pid 257 (ITS) although the PS half fails; the ITS half succeeds silently first.
 *
 * Per-object ceiling, if these are retuned:
 *
 *   PS_MAX_ASSET_SIZE <= 1348 - (32 * PS_NUM_ASSETS)
 *
 * At PS_NUM_ASSETS 10 that caps the asset at 1028. For larger objects, give PS more of the
 * data flash than ITS in flash_layout.h (currently split evenly): a 4096/2048 PS/ITS split
 * raises the PS cap to 1540 and leaves ITS a workable 652.
 *
 * Per-object floor: ps_object_table.c loads the object table into the static object-data
 * buffer, whose size is PS_MAX_ASSET_SIZE, and asserts the fit at compile time:
 *
 *   PS_UTILS_BOUND_CHECK(OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF,
 *                        PS_OBJ_TABLE_SIZE, PS_MAX_ASSET_SIZE);
 *
 * failing as "size of array 'OBJ_TABLE_NOT_FIT_IN_STATIC_OBJ_DATA_BUF' is negative".
 * sizeof(struct ps_obj_table_t) is PS_OBJ_TABLE_ENTRIES (PS_NUM_ASSETS + 1, not
 * PS_NUM_ASSETS) entries of 32 bytes, plus crypto metadata. Measured by compiling: 256 and
 * 384 fail, 448 passes, so the floor is in (384, 448]. Each extra asset adds 32 to the floor
 * and takes 32 off the ceiling.
 *
 * The binding constraint is neither per-object bound but the live set: the block holds every
 * live object at once, and the fixed overhead leaves little for the asset:
 *
 *   usable         = block_size - (8 + 12 + (PS_NUM_ASSETS + 3)*32)
 *   object tables  = 2 * (48 + (PS_NUM_ASSETS + 1)*32)   both FS IDs are always live
 *   1004's write-once asset, 36 + 72                     unremovable, survives every run
 *   => free for one asset object = 1152 - 96*PS_NUM_ASSETS
 *
 * At PS_NUM_ASSETS 10 that is 192 bytes, and test 1025's smallest cycle
 * (PS_MAX_ASSET_SIZE >> 2, so 128 bytes plus 72 of object header) needs 200. Each asset costs
 * 96 bytes of fixed overhead (32 of file metadata plus a 32-byte entry in each of the two
 * tables), so PS_NUM_ASSETS is the knob here, not PS_MAX_ASSET_SIZE.
 *
 * Struct sizes taken from the built objects: ps_obj_table_ctx is 408 (table 400 + 2 indices
 * + pad) and g_ps_object 584 at PS_MAX_ASSET_SIZE 512.
 *
 * Rewriting an existing max-size asset needs both copies resident, 1168 bytes, which does not
 * fit at any PS_NUM_ASSETS. Growing an asset already near PS_MAX_ASSET_SIZE returns
 * PSA_ERROR_INSUFFICIENT_STORAGE on this part.
 */
#define PS_MAX_ASSET_SIZE                       512

/* 1152 - 96*PS_NUM_ASSETS must cover PS_MAX_ASSET_SIZE + 72, which caps this at 5.
 * ra6m5_layout_checks.c asserts the budget. */
#define PS_NUM_ASSETS                           5


/*
 * Diagnostic only. Enables tfm_hal_post_partition_init_hook(), which spm_ipc.c calls once
 * every partition is bound. tfm_hal_platform.c uses it to report the MPU after per-partition
 * MMIO regions are programmed; the report in tfm_hal_platform_init() runs before the partition
 * loop and sees only the static set.
 *
 * Gated on CONFIG_TFM_ENABLE_MEMORY_PROTECT, necessarily. The hook body in tfm_hal_platform.c
 * is inside that #ifdef, while the call site in spm_ipc.c is inside
 * #if CONFIG_TFM_POST_PARTITION_INIT_HOOK == 1. Defined unconditionally, the two disagree in
 * exactly one configuration, isolation 1 with the IPC backend, where the caller exists and the
 * body does not:
 *
 *     spm_ipc.c:440: undefined reference to `tfm_hal_post_partition_init_hook'
 *
 * platform/CMakeLists.txt:125 sets CONFIG_TFM_ENABLE_MEMORY_PROTECT PUBLIC on platform_s for
 * TFM_ISOLATION_LEVEL > 1 or TEST_PSA_API=IPC, and tfm_spm links platform_s, so spm_ipc.c and
 * tfm_hal_platform.c see the same value (verified on the compile lines).
 *
 * Costs nothing below isolation 2: the hook compiles out, and SPMLOG_INFMSGVAL expands to
 * (void)(val) when SPM logging is off.
 */
#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
#define CONFIG_TFM_POST_PARTITION_INIT_HOOK     1
#endif

#endif /* __CONFIG_TFM_TARGET_H__ */
