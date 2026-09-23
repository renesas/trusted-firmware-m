/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * One-time SCE9 bring-up - see ra_sce_init.h.
 *
 * Built into the secure image (TRNG, and the cipher accelerator when CRYPTO_HW_ACCELERATOR
 * is on) and, with the accelerator on, into BL2 (SHA-256 for the image hash). Each image
 * has its own latch: BL2 resets the engine once for itself, finishes with it before it
 * hands over, and the secure image then resets it once more for its own use.
 */

#include <stdbool.h>

#include "ra_sce_init.h"

/* SCE9 primitive, from the fsp_sce module (r_sce_adapt.c). Declared here rather than by
 * including hw_sce_private.h so this file does not drag the SCE private headers into every
 * consumer of the platform library. */
extern fsp_err_t HW_SCE_McuSpecificInit(void);

/* Only a SUCCESSFUL init is latched, so a failure is retried on the next call. */
static bool ra_sce_ready = false;

fsp_err_t ra_sce_init(void)
{
    fsp_err_t err;

    if (ra_sce_ready) {
        return FSP_SUCCESS;
    }

    err = HW_SCE_McuSpecificInit();
    if (FSP_SUCCESS == err) {
        ra_sce_ready = true;
    }

    return err;
}
