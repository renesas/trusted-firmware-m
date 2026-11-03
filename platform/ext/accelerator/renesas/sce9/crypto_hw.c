/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * crypto_hw.h implementation for the Renesas SCE9, secure crypto partition only. BL2 uses
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
 * context onto a wrapped key. The only writes to that flag in the whole compiled set are two
 * in FSP's psa_crypto_aead.c, both inside #if defined(MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C), and
 * nothing here defines that macro: FSP generates it into ra_cfg/arm/mbedtls/config.h, which
 * TF-M does not compile - sce9_fsp_cfg.h restates the parts the ALT sources need, and that is
 * not one of them. So the flag stays 0 and this function never runs.
 *
 * An application cannot reach it through the API either. psa_crypto.c gates its vendor-key
 * branches on the same macro, so a vendor-defined key type falls through psa_generate_key()
 * and psa_import_key() to the transparent path and returns PSA_ERROR_NOT_SUPPORTED. Wrapped
 * keys are unavailable in this port, not merely untested.
 *
 * The call site in gcm_alt.c is not under the macro, so the symbol is still needed to link.
 * It panics rather than returning: defining MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C without also
 * building the vendor driver should fail at once, not run a wrapped-key procedure with
 * nothing behind it.
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
    /* The SCE stays powered: the TRNG keeps using it for the life of the secure image. */
    return 0;
}

int crypto_hw_accelerator_huk_derive_key(const uint8_t *label,
                                         size_t label_size,
                                         const uint8_t *context,
                                         size_t context_size,
                                         uint8_t *key,
                                         size_t key_size)
{
    /* Not provided. SCE9 has a hardware unique key, but it never leaves the engine and is
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
