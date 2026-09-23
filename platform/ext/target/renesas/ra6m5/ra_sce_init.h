/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * One-time SCE9 bring-up, shared by every user of the engine in the secure image.
 */

#ifndef RA_SCE_INIT_H
#define RA_SCE_INIT_H

#include "bsp_api.h"   /* fsp_err_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Power on, reset and self-check the SCE9, exactly once.
 *
 * HW_SCE_McuSpecificInit() software-resets the engine on EVERY call. Harmless at boot, but
 * the TRNG and the SCE9 cipher accelerator both need the engine up and neither controls
 * when the other first runs - so an unguarded second call would reset the SCE underneath
 * whatever operation the first user had in flight. Everything goes through here instead.
 *
 * Not reentrant: the secure image calls it from the crypto partition only, which is
 * single-threaded under SFN and IPC alike.
 */
fsp_err_t ra_sce_init(void);

#ifdef __cplusplus
}
#endif

#endif /* RA_SCE_INIT_H */
