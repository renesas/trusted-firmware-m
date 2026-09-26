/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA crypto configuration adjustments for the Renesas RSIP-E50D accelerator.
 *
 * crypto_config_*.h includes this LAST under CRYPTO_HW_ACCELERATOR, so an #undef here
 * removes an algorithm or curve from the build. With MBEDTLS_PSA_CRYPTO_CONFIG the legacy
 * MBEDTLS_ECP_DP_* / MBEDTLS_ECDSA_DETERMINISTIC settings are derived from these, so this is
 * the one place to change them.
 *
 * The rule: do not advertise what the engine cannot do correctly. FSP's ECC ALT has no
 * software fallback (mbedtls_accelerator_config.h), so anything below would otherwise build
 * and then fail - or, worse, return a wrong kind of result - at run time.
 */

#ifndef CRYPTO_ACCELERATOR_CONFIG_H
#define CRYPTO_ACCELERATOR_CONFIG_H

/* P-521 and Curve25519: REMOVED FOR NOW, on flash grounds, not capability grounds.
 *
 * E50D genuinely has hardware procedures for both - ecp_can_do_sce() returns 1 for
 * MBEDTLS_ECP_DP_SECP521R1 and MBEDTLS_ECP_DP_CURVE25519 under
 * BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED, which bsp_feature.h defines as 1 for R7KA8M2 - and
 * an earlier revision of this file kept both on exactly that reasoning.
 *
 * What that missed is what the procedures COST. Each HW procedure is a large constant
 * instruction table, and enabling these two curves drags in their transitive closure of
 * hw_sce_p_func###.o members. Measured on tfm_s: the HW_SCE_* tables are 127,542 bytes on
 * E50D against 8,124 on SCE9, and the secure image sat at 293,564 of 294,400 bytes - 99.72%
 * of its slot, 836 bytes spare. There was no room to enable isolation 2, the IPC backend, or
 * to build Debug at all.
 *
 * So this matches the SCE9 configuration for now. The engine keeps secp256r1, secp256k1,
 * brainpoolP256r1, secp384r1 and brainpoolP384r1, which is the same curve set the validated
 * RA6M5/RA6E1 ports advertise, and nothing in TF-M itself needs more.
 *
 * TO BE RESTORED after the TF-M 2.3 migration, once the layout has room - it is a capability
 * this part has and the port should eventually expose. Deleting these two #undefs is the
 * whole change. DECISIONS D059. */
#undef PSA_WANT_ECC_SECP_R1_521
#undef PSA_WANT_ECC_MONTGOMERY_255

/* Deterministic ECDSA (RFC 6979). Not supported by FSP on the SCE - the FSP configurator does
 * not allow it - so this only brings TF-M's default configuration in line with FSP. The SCE
 * draws its own nonce (mbedtls_ecdsa_sign() does not use f_rng), and with
 * MBEDTLS_ECDSA_SIGN_ALT Mbed TLS's deterministic path is exactly that call with an HMAC-DRBG
 * as f_rng, so leaving it enabled would return a valid but randomised signature for
 * PSA_ALG_DETERMINISTIC_ECDSA. Nothing in TF-M needs it: the attestation key signs with
 * PSA_ALG_ECDSA. */
#undef PSA_WANT_ALG_DETERMINISTIC_ECDSA

#endif /* CRYPTO_ACCELERATOR_CONFIG_H */
