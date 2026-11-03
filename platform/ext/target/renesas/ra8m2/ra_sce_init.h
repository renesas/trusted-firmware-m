/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * One-time RSIP-E50D bring-up, shared by every user of the engine in the secure image.
 */

#ifndef RA_SCE_INIT_H
#define RA_SCE_INIT_H

#include "bsp_api.h"   /* fsp_err_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Power on, reset and self-check the RSIP-E50D, exactly once.
 *
 * HW_SCE_McuSpecificInit() software-resets the engine on every call. The TRNG and the
 * cipher accelerator both need the engine, and neither controls which runs first, so a
 * second unguarded call would reset the engine under an operation already in flight. All
 * users call this function instead.
 *
 * Not reentrant: the secure image calls it only from the crypto partition, which is
 * single-threaded under both SFN and IPC.
 */
fsp_err_t ra_sce_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RA_SCE_INIT_H */
