/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA external RNG for RA8M2, backed by the RSIP-E50D TRNG.
 *
 * Why this exists: with CRYPTO_HW_ACCELERATOR off, TF-M registers exactly one entropy
 * source - a 64-byte NV seed in ITS, seeded once from PLAT_OTP_ID_ENTROPY_SEED. The
 * default provisioning bundle sets that OTP value to a hard-coded constant
 * (platform/ext/common/provisioning_bundle/provisioning_config.cmake), so every device
 * would come up with identical entropy. Fine for bring-up, not for anything that signs.
 *
 * RA8M2 has no standalone TRNG peripheral (BSP_FEATURE_RSIP_TRNG_SUPPORTED == 0); the
 * random source is inside the RSIP (BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED == 1; note
 * BSP_FEATURE_RSIP_SCE9_SUPPORTED is 0 on this part). So we take it
 * straight from the SCE primitives rather than through FSP's mbedTLS/rm_psa_crypto stack,
 * whose generated config wires PSA ITS to littlefs and fights TF-M's own ITS
 * (DESIGN.md 6). Nothing here depends on that config.
 *
 * config_tfm_target.h sets CRYPTO_EXT_RNG=1 / CRYPTO_NV_SEED=0, and
 * mbedtls_extra_config.h defines MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG, which is what makes
 * PSA call this instead of the NV-seed path.
 */

#include "psa/crypto.h"
/* bsp_api.h, not fsp_common_api.h: the latter includes fsp_version.h -> bsp_api.h ->
 * bsp_cfg.h -> the board headers, which use FSP_HEADER before fsp_common_api.h has got
 * as far as defining it. Entering through bsp_api.h defines it first. */
#include "bsp_api.h"           /* fsp_err_t, FSP_SUCCESS - no SCE private headers */
#include "ra_sce_init.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Engine primitive, from the fsp_sce module (r_sce_adapt.c). Declared here rather than by
 * including hw_sce_private.h so this file does not drag the SCE private headers into
 * every consumer of platform_s. The engine itself is brought up by ra_sce_init()
 * (ra_sce_init.c), shared with the cipher accelerator. */
extern fsp_err_t HW_SCE_RNG_Read(uint32_t *OutData_Text);

#if defined(MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG)

/* HW_SCE_RNG_Read fills a fixed 128-bit block per call. */
#define SCE_TRNG_BLOCK_WORDS  (4U)
#define SCE_TRNG_BLOCK_BYTES  (SCE_TRNG_BLOCK_WORDS * sizeof(uint32_t))

psa_status_t mbedtls_psa_external_get_random(
    mbedtls_psa_external_random_context_t *context,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    uint32_t block[SCE_TRNG_BLOCK_WORDS];
    size_t produced = 0U;

    (void)context;

    if ((output == NULL) || (output_length == NULL)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Powers on SCE, resets it and runs its self-check - once, shared with the cipher
     * accelerator. */
    if (FSP_SUCCESS != ra_sce_init()) {
        *output_length = 0U;
        return PSA_ERROR_HARDWARE_FAILURE;
    }

    while (produced < output_size) {
        size_t remaining = output_size - produced;
        size_t chunk = (remaining < SCE_TRNG_BLOCK_BYTES) ? remaining : SCE_TRNG_BLOCK_BYTES;

        if (FSP_SUCCESS != HW_SCE_RNG_Read(block)) {
            /* Report what we actually produced; do not hand back a partly-filled buffer
             * as if it were good. */
            (void)memset(output, 0, output_size);
            (void)memset(block, 0, sizeof(block));
            *output_length = 0U;
            return PSA_ERROR_HARDWARE_FAILURE;
        }

        (void)memcpy(output + produced, (const uint8_t *)block, chunk);
        produced += chunk;
    }

    /* Do not leave the last block on the stack. */
    (void)memset(block, 0, sizeof(block));

    *output_length = produced;

    return PSA_SUCCESS;
}

#endif /* MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG */
