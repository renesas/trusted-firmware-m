/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * RA6E1 non-secure smoke test - exercises each secure service in turn.
 *
 * Up to now the port has only ever been shown to BOOT: BL2 verifies and jumps, tfm_s
 * initialises its partitions, and the NS image runs. Not one PSA service had ever been
 * called across the boundary. This is what closes that gap.
 *
 * ONE SERVICE AT A TIME, and each step prints its banner BEFORE the call it describes.
 * That ordering is the whole point. A PSA call that fails usually returns a status, but
 * several failure modes on this port do not: the SPM calls tfm_core_panic() and spins, or
 * the boundary faults and the NS side never runs again. In those cases the last line on
 * the RTT channel is the only evidence, so it has to name the call that did not come back
 * rather than the last one that did.
 *
 * Output goes to RTT channel 0 via SEGGER_RTT_WriteString. Deliberately not printf: the NS
 * image is built with nano.specs and its heap is whatever the FSP project configured, and
 * a smoke test that needs a working heap to report a broken heap is no use. The formatting
 * here is a hand-rolled hex conversion for the same reason - no varargs, no reentrancy,
 * no allocation.
 *
 * Reading the output: every line starts "[NS] ". A step prints "N. <name> ... " and then
 * either "ok" or "FAIL <status>". If a line has no "ok" or "FAIL" after it, that call is
 * where the port died.
 */

#include <stdint.h>
#include <string.h>

#include "psa/client.h"
#include "psa/crypto.h"
#include "psa/internal_trusted_storage.h"
#include "psa/protected_storage.h"
#include "psa/initial_attestation.h"
#include "psa_manifest/sid.h"

#include "SEGGER_RTT.h"

#include "tfm_service_tests.h"

/* ------------------------------------------------------------------------- */
/* Minimal output helpers                                                     */
/* ------------------------------------------------------------------------- */

static void ns_puts(const char *s)
{
    (void)SEGGER_RTT_WriteString(0U, s);
}

/* Signed-aware: PSA statuses are negative, and reading -135 as 0xFFFFFF79 in every log was
 * a real source of confusion during secure-side bring-up. Print both forms. */
static void ns_put_status(int32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    char buf[11];
    uint32_t u = (uint32_t)v;
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[2 + i] = digits[(u >> ((7 - i) * 4)) & 0xFU];
    }
    buf[10] = '\0';
    ns_puts(buf);

    if (v < 0) {
        char dec[12];
        int32_t n = -v;
        int p = (int)sizeof(dec) - 1;

        dec[p] = '\0';
        while ((n > 0) && (p > 1)) {
            dec[--p] = (char)('0' + (n % 10));
            n /= 10;
        }
        dec[--p] = '-';
        ns_puts(" (");
        ns_puts(&dec[p]);
        ns_puts(")");
    }
}

static uint32_t step_no;
static uint32_t failures;

static void step(const char *name)
{
    static const char digits[] = "0123456789";
    char n[4];

    step_no++;
    n[0] = digits[(step_no / 10U) % 10U];
    n[1] = digits[step_no % 10U];
    n[2] = '.';
    n[3] = '\0';

    ns_puts("[NS] ");
    ns_puts((step_no < 10U) ? &n[1] : n);
    ns_puts(" ");
    ns_puts(name);
    ns_puts(" ... ");
}

static void ok(void)
{
    ns_puts("ok\r\n");
}

static void ok_val(const char *label, int32_t v)
{
    ns_puts("ok  ");
    ns_puts(label);
    ns_put_status(v);
    ns_puts("\r\n");
}

static void fail(int32_t status)
{
    failures++;
    ns_puts("FAIL status=");
    ns_put_status(status);
    ns_puts("\r\n");
}

static void fail_msg(const char *why)
{
    failures++;
    ns_puts("FAIL ");
    ns_puts(why);
    ns_puts("\r\n");
}

/* ------------------------------------------------------------------------- */
/* Test data                                                                  */
/* ------------------------------------------------------------------------- */

#define TEST_UID_ITS    ((psa_storage_uid_t)0x5241364531U)
#define TEST_UID_PS     ((psa_storage_uid_t)0x5241364532U)

static const uint8_t test_payload[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
};

/* SHA-256 of the empty string - the standard vector, so a wrong answer is recognisable
 * rather than just "not what we stored". */
static const uint8_t sha256_empty[] = {
    0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
    0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
    0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
    0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
};

/* ------------------------------------------------------------------------- */
/* Steps                                                                      */
/* ------------------------------------------------------------------------- */

/*
 * 1-2: the boundary itself, before any partition is involved.
 *
 * psa_framework_version() is the cheapest possible NS->S round trip: it reaches
 * tfm_psa_framework_version_veneer and returns a constant. If this hangs, nothing is wrong
 * with any service - the SAU/IDAU attribution, the veneer table or the NSC window is.
 * psa_version() then proves SID routing reaches a partition without asking it to do work.
 */
static void test_boundary(void)
{
    uint32_t ver;

    step("psa_framework_version");
    ver = psa_framework_version();
    if (ver == PSA_FRAMEWORK_VERSION) {
        ok_val("version=", (int32_t)ver);
    } else {
        fail((int32_t)ver);
    }

    step("psa_version(crypto)");
    ver = psa_version(TFM_CRYPTO_SID);
    if (ver != PSA_VERSION_NONE) {
        ok_val("version=", (int32_t)ver);
    } else {
        fail_msg("service not present");
    }
}

/*
 * 3-5: crypto. init first, because everything else in the partition depends on it and a
 * failure here would otherwise surface as a confusing error inside a later call. Random
 * before hash: it is the shorter path, and it exercises the SCE9 TRNG that this port wires
 * up itself (sce_trng.c) instead of using a stored NV seed.
 */
static void test_crypto(void)
{
    psa_status_t st;
    uint8_t rnd[16];
    uint8_t digest[32];
    size_t digest_len = 0U;

    step("psa_crypto_init");
    st = psa_crypto_init();
    if (st != PSA_SUCCESS) {
        fail(st);
        return;
    }
    ok();

    step("psa_generate_random (SCE9 TRNG)");
    memset(rnd, 0, sizeof(rnd));
    st = psa_generate_random(rnd, sizeof(rnd));
    if (st != PSA_SUCCESS) {
        fail(st);
    } else {
        /* All-zero is a valid random result once in 2^128, so rejecting it is safe and it
         * catches the much likelier case of the buffer never being written at all. */
        size_t i;
        uint8_t acc = 0U;

        for (i = 0; i < sizeof(rnd); i++) {
            acc |= rnd[i];
        }
        if (acc == 0U) {
            fail_msg("all zero - buffer not written");
        } else {
            ok();
        }
    }

    step("psa_hash_compute SHA-256");
    st = psa_hash_compute(PSA_ALG_SHA_256, NULL, 0U,
                          digest, sizeof(digest), &digest_len);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else if ((digest_len != sizeof(sha256_empty)) ||
               (memcmp(digest, sha256_empty, sizeof(sha256_empty)) != 0)) {
        fail_msg("wrong digest");
    } else {
        ok();
    }
}

/*
 * 6-8: ITS. set/get/remove as one sequence, because a get that never had a matching set
 * proves nothing. This is the partition whose init returned -135 during secure bring-up
 * (block geometry), so it is worth reaching from the NS side rather than trusting that a
 * clean boot means a working filesystem.
 */
static void test_its(void)
{
    psa_status_t st;
    uint8_t readback[sizeof(test_payload)];
    size_t got = 0U;

    step("psa_its_set");
    st = psa_its_set(TEST_UID_ITS, sizeof(test_payload), test_payload,
                     PSA_STORAGE_FLAG_NONE);
    if (st != PSA_SUCCESS) {
        fail(st);
        return;
    }
    ok();

    step("psa_its_get + verify");
    memset(readback, 0, sizeof(readback));
    st = psa_its_get(TEST_UID_ITS, 0U, sizeof(readback), readback, &got);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else if ((got != sizeof(test_payload)) ||
               (memcmp(readback, test_payload, sizeof(test_payload)) != 0)) {
        fail_msg("data mismatch");
    } else {
        ok();
    }

    /* Removed so a re-run starts from the same state - the data flash is only 8 KB and
     * these tests are expected to be run repeatedly during bring-up. */
    step("psa_its_remove");
    st = psa_its_remove(TEST_UID_ITS);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else {
        ok();
    }
}

/*
 * 9-11: PS. Same shape as ITS, but a different partition and - because PS_ENCRYPTION is
 * ON - a different code path: every write goes through AES-GCM, so this also exercises
 * crypto from inside the secure side rather than from NS.
 */
static void test_ps(void)
{
    psa_status_t st;
    uint8_t readback[sizeof(test_payload)];
    size_t got = 0U;

    step("psa_ps_set (AES-GCM)");
    st = psa_ps_set(TEST_UID_PS, sizeof(test_payload), test_payload,
                    PSA_STORAGE_FLAG_NONE);
    if (st != PSA_SUCCESS) {
        fail(st);
        return;
    }
    ok();

    step("psa_ps_get + verify");
    memset(readback, 0, sizeof(readback));
    st = psa_ps_get(TEST_UID_PS, 0U, sizeof(readback), readback, &got);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else if ((got != sizeof(test_payload)) ||
               (memcmp(readback, test_payload, sizeof(test_payload)) != 0)) {
        fail_msg("data mismatch");
    } else {
        ok();
    }

    step("psa_ps_remove");
    st = psa_ps_remove(TEST_UID_PS);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else {
        ok();
    }
}

/*
 * 12: attestation, asked only for the token SIZE.
 *
 * Deliberately not a full token: fetching one needs a
 * PSA_INITIAL_ATTEST_MAX_TOKEN_SIZE buffer on the NS stack and pulls in COSE signing over
 * the attestation key. The size query still proves the partition is alive, that its HAL
 * answers (this port supplies attest_hal.c), and that the boundary carries an
 * out-parameter correctly. Widen this once the split build lands and there is somewhere
 * sensible to put the buffer.
 */
static void test_attestation(void)
{
    psa_status_t st;
    size_t token_size = 0U;

    step("psa_initial_attest_get_token_size");
    st = psa_initial_attest_get_token_size(32U, &token_size);
    if (st != PSA_SUCCESS) {
        fail(st);
    } else if (token_size == 0U) {
        fail_msg("zero size");
    } else {
        ok_val("bytes=", (int32_t)token_size);
    }
}

/*
 * 13: platform. psa_version() rather than a real call: the platform service's operations
 * are things like system reset and IOCTL, and a smoke test should not reset the board it
 * is reporting from.
 */
static void test_platform(void)
{
    uint32_t ver;

    step("psa_version(platform)");
    ver = psa_version(TFM_PLATFORM_SERVICE_SID);
    if (ver != PSA_VERSION_NONE) {
        ok_val("version=", (int32_t)ver);
    } else {
        fail_msg("service not present");
    }
}

/* ------------------------------------------------------------------------- */

void tfm_service_tests_run(void)
{
    step_no = 0U;
    failures = 0U;

    ns_puts("\r\n[NS] ==== RA6E1 secure service smoke test ====\r\n");

    test_boundary();
    test_crypto();
    test_its();
    test_ps();
    test_attestation();
    test_platform();

    ns_puts("[NS] ==== ");
    if (failures == 0U) {
        ns_puts("ALL PASSED");
    } else {
        ns_put_status((int32_t)failures);
        ns_puts(" FAILED");
    }
    ns_puts(" ====\r\n");
}
