/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Build-time guard on the OTP / NV counter area size.
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

#include "flash_otp_nv_counters_backend.h"
#include "flash_layout.h"

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
