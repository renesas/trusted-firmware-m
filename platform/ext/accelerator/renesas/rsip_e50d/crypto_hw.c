/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * crypto_hw.h implementation for the Renesas RSIP-E50D, secure crypto partition only. BL2 uses
 * the crypto_hw stubs in bl2/ext/mcuboot/bl2_main.c; its SHA-256 is still accelerated (see
 * CMakeLists.txt, BL2 section).
 *
 * secure_fw/partitions/crypto/crypto_init.c calls crypto_hw_accelerator_init() before
 * psa_crypto_init() when CRYPTO_HW_ACCELERATOR and LEGACY_DRIVER_API_ENABLED are both defined.
 * LEGACY_DRIVER_API_ENABLED is TF-M's name for the *_ALT model this accelerator uses. After
 * init, the ALT sources call HW_SCE_* directly.
 */

#include <stddef.h>
#include <stdint.h>

#include "crypto_hw.h"
#include "ra_sce_init.h"
#include "psa/service.h"   /* psa_panic() */

/*
 * FSP's gcm_alt.c (and ccm_alt.c) call this when ctx->vendor_flag is set, to switch the
 * context onto a wrapped key. The flag is a RUNTIME value set by FSP's patched Mbed TLS core
 * (psa_crypto_cipher.c, psa_crypto_mac.c under ra/arm/mbedtls), which is the core this port
 * compiles, so the call is reachable: no TF-M test asks for a wrapped key, but nothing stops
 * an application from asking. The implementation lives in aes_vendor.c, the wrapped-key
 * driver this accelerator does not build, so the symbol is needed here to link.
 *
 * Reaching it means a wrapped-key procedure with no driver behind it, so fail loudly rather
 * than carry on.
 */
void psa_aead_setup_vendor(void *ctx);

void psa_aead_setup_vendor(void *ctx)
{
    (void)ctx;
    psa_panic();
}

int crypto_hw_accelerator_init(void)
{
    /* Shared with the platform's TRNG: whichever runs first brings the engine up, and the
     * other does not reset it. */
    return (FSP_SUCCESS == ra_sce_init()) ? 0 : -1;
}

int crypto_hw_accelerator_finish(void)
{
    /* The engine stays powered: the TRNG keeps using it for the life of the secure image. */
    return 0;
}

int crypto_hw_accelerator_huk_derive_key(const uint8_t *label,
                                         size_t label_size,
                                         const uint8_t *context,
                                         size_t context_size,
                                         uint8_t *key,
                                         size_t key_size)
{
    /* Not provided. E50D has a hardware unique key, but it never leaves the engine and is
     * used for key wrapping, not exposed as a KDF. The port keeps TF-M's default HUK path
     * (PLATFORM_DEFAULT_CRYPTO_KEYS). */
    (void)label;
    (void)label_size;
    (void)context;
    (void)context_size;
    (void)key;
    (void)key_size;
    return -1;
}

int crypto_hw_apply_debug_permissions(uint8_t *permissions_mask, uint32_t len)
{
    /* No PSA-ADAC on this port. */
    (void)permissions_mask;
    (void)len;
    return -1;
}
