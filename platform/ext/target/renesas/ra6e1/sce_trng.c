/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA external RNG for RA6E1, backed by the SCE9 TRNG.
 *
 * Why this exists: with CRYPTO_HW_ACCELERATOR off, TF-M registers exactly one entropy
 * source - a 64-byte NV seed in ITS, seeded once from PLAT_OTP_ID_ENTROPY_SEED. The
 * default provisioning bundle sets that OTP value to a hard-coded constant
 * (platform/ext/common/provisioning_bundle/provisioning_config.cmake), so every device
 * would come up with identical entropy. Fine for bring-up, not for anything that signs.
 *
 * RA6E1 has no standalone TRNG peripheral (BSP_FEATURE_RSIP_TRNG_SUPPORTED == 0); the
 * random source is inside SCE9 (BSP_FEATURE_RSIP_SCE9_SUPPORTED == 1). So we take it
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG)

/* SCE9 primitives, from the fsp_sce module (r_sce_adapt.c). Declared here rather than by
 * including hw_sce_private.h so this file does not drag the SCE private headers into
 * every consumer of platform_s. */
extern fsp_err_t HW_SCE_McuSpecificInit(void);
extern fsp_err_t HW_SCE_RNG_Read(uint32_t *OutData_Text);

/* HW_SCE_RNG_Read fills a fixed 128-bit block per call. */
#define SCE_TRNG_BLOCK_WORDS  (4U)
#define SCE_TRNG_BLOCK_BYTES  (SCE_TRNG_BLOCK_WORDS * sizeof(uint32_t))

static bool sce_trng_ready = false;

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

    /* Powers on SCE, resets it and runs its self-check. Idempotent in effect, but it is
     * not free, so only do it once. */
    if (!sce_trng_ready) {
        if (FSP_SUCCESS != HW_SCE_McuSpecificInit()) {
            *output_length = 0U;
            return PSA_ERROR_HARDWARE_FAILURE;
        }
        sce_trng_ready = true;
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
