/*
 * Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-M Service Test Thread for RA6M4
 * Tests: Attestation, Storage, Crypto, and HUK Derivation services
 */

#include "tfm_test_thread.h"
#include "psa/error.h"
#include "psa/initial_attestation.h"
#include "psa/internal_trusted_storage.h"
#include "psa/crypto.h"
#include "tfm_ns_interface.h"
#include "tfm_platform_api.h"
#include "tfm_ioctl_api.h"
#include "tfm_crypto_defs.h"
#include "FreeRTOS.h"
#include "task.h"

/* External initialization function */
extern uint32_t tfm_ns_interface_init(void);

/* Test data and variables */
static uint8_t flash_test_data[128] =
    "Renesas RA6M4 TFM TF-M Flash service write test data 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21.";

/* Key derivation variables */
static const uint8_t derivedKey1Label[] = "key_1";
static psa_key_attributes_t derivedKey1Attributes = PSA_KEY_ATTRIBUTES_INIT;
static psa_key_handle_t derivedKey1Handle;
static psa_key_derivation_operation_t derivedKey1Operation = PSA_KEY_DERIVATION_OPERATION_INIT;
static volatile psa_status_t psa_err;
static uint8_t g_exportedDerivedKey1[PSA_BITS_TO_BYTES(256)];
static size_t g_exportedDerivedKey1Length = 0;

/**
 * Test TF-M Attestation Service
 */
static void test_attestation_service(void)
{
    uint8_t challenge[33] = {0x2a};
    size_t size = 0;
    size_t token_buf_size = 800;
    uint8_t token_buf[800] = {0x3b};
    size_t token_size = 0;

    /* Get token size */
    if (PSA_SUCCESS != psa_initial_attest_get_token_size(32, &size))
    {
        /* Attestation service get token size failed */
        while(1);
    }

    /* Get attestation token */
    if (PSA_SUCCESS != psa_initial_attest_get_token(challenge, 32, token_buf,
                                                     token_buf_size, &token_size))
    {
        /* Attestation service get token failed */
        while(1);
    }

    /* Attestation test passed */
}

/**
 * Test TF-M Internal Trusted Storage Service
 */
static void test_storage_service(void)
{
    uint8_t data1[10] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    uint8_t read_buf[10];
    psa_storage_uid_t uid = 1;
    size_t data_length = 10;
    psa_storage_create_flags_t flags = PSA_STORAGE_FLAG_NO_CONFIDENTIALITY |
                                       PSA_STORAGE_FLAG_NO_REPLAY_PROTECTION;

    /* Write data to storage */
    if (PSA_SUCCESS != psa_its_set(uid, data_length, (const void *)&data1[0], flags))
    {
        /* Storage service set failed */
        while(1);
    }

    /* Read data from storage */
    size_t read_length;
    if (PSA_SUCCESS != psa_its_get(uid, 0, data_length, (void *)read_buf, &read_length))
    {
        /* Storage service get failed */
        while(1);
    }

    /* Storage test passed */
}

/**
 * Test TF-M Crypto Service - Random Number Generation
 */
static void test_crypto_random(void)
{
    uint8_t random_data[10];
    size_t data_length = 10;

    if (PSA_SUCCESS != psa_generate_random(random_data, data_length))
    {
        /* Crypto random generation failed */
        while(1);
    }

    /* Random generation test passed */
}

/**
 * Test TF-M Crypto Service - SHA-256 Hashing
 */
static void test_crypto_hash(void)
{
    psa_algorithm_t alg = PSA_ALG_SHA_256;
    psa_hash_operation_t operation = {0};

    /* Test vector 1: "abc" */
    uint8_t input_1[] = {'a', 'b', 'c'};
    uint8_t expected_hash_1[] =
    {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde,
        0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xcb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };

    /* Test vector 2: "abcdddddddd" */
    uint8_t input_2[] = {'a', 'b', 'c', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'd'};
    uint8_t expected_hash_2[] =
    {
        0x7b, 0xd3, 0x40, 0xd0, 0x29, 0xf7, 0x61, 0xd7, 0xaf, 0x22, 0xaf, 0x96,
        0xeb, 0xaf, 0xba, 0x8f, 0xbe, 0x2b, 0xd6, 0xb3, 0x91, 0x58, 0xb6, 0xe2,
        0x13, 0x81, 0x64, 0x18, 0xad, 0xd4, 0x14, 0x3b
    };

    size_t expected_hash_len = PSA_HASH_LENGTH(alg);

    /* Test hash 1 */
    if (PSA_SUCCESS != psa_hash_setup(&operation, alg))
        while(1);
    if (PSA_SUCCESS != psa_hash_update(&operation, input_1, sizeof(input_1)))
        while(1);
    if (PSA_SUCCESS != psa_hash_verify(&operation, expected_hash_1, expected_hash_len))
        while(1);
    if (PSA_SUCCESS != psa_hash_abort(&operation))
        while(1);

    /* Test hash 2 */
    if (PSA_SUCCESS != psa_hash_setup(&operation, alg))
        while(1);
    if (PSA_SUCCESS != psa_hash_update(&operation, input_2, sizeof(input_2)))
        while(1);
    if (PSA_SUCCESS != psa_hash_verify(&operation, expected_hash_2, expected_hash_len))
        while(1);
    if (PSA_SUCCESS != psa_hash_abort(&operation))
        while(1);

    /* Hash test passed */
}

/**
 * Test TF-M HUK (Hardware Unique Key) Derivation
 */
static void test_huk_derivation(void)
{
    /* Set the key attributes for the derived key */
    psa_set_key_usage_flags(&derivedKey1Attributes, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&derivedKey1Attributes, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    psa_set_key_type(&derivedKey1Attributes, PSA_KEY_TYPE_DERIVE);
    psa_set_key_bits(&derivedKey1Attributes, 256);

    /* Set up a key derivation operation with HUK derivation as the alg */
    psa_err = psa_key_derivation_setup(&derivedKey1Operation, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    if (psa_err != PSA_SUCCESS)
        while(1);

    /* Setup a key derivation operation with HUK */
    psa_err = psa_key_derivation_input_key(&derivedKey1Operation, PSA_KEY_DERIVATION_INPUT_SECRET,
                                           TFM_BUILTIN_KEY_ID_HUK);
    if (psa_err != PSA_SUCCESS)
        while(1);

    /* Supply the key label as an input to the key derivation */
    psa_err = psa_key_derivation_input_bytes(&derivedKey1Operation, PSA_KEY_DERIVATION_INPUT_INFO,
                                             derivedKey1Label, sizeof(derivedKey1Label));
    if (psa_err != PSA_SUCCESS)
        while(1);

    /* Create the key from the key derivation operation */
    psa_err = psa_key_derivation_output_key(&derivedKey1Attributes, &derivedKey1Operation,
                                            &derivedKey1Handle);
    if (psa_err != PSA_SUCCESS)
        while(1);

    /* Free resources associated with the key derivation operation */
    psa_err = psa_key_derivation_abort(&derivedKey1Operation);
    if (psa_err != PSA_SUCCESS)
        while(1);

    /* Export the derived key for verification */
    if (PSA_SUCCESS != psa_export_key(derivedKey1Handle, g_exportedDerivedKey1,
                                     sizeof(g_exportedDerivedKey1), &g_exportedDerivedKey1Length))
        while(1);

    /* HUK derivation test passed */
}

/**
 * TF-M Test Thread Entry Point
 * Tests all TF-M services sequentially
 */
void tfm_test_thread_entry(void *pvParameters)
{
    (void)pvParameters;

    /* Initialize TF-M non-secure interface */
    tfm_ns_interface_init();

    /* Run all TF-M service tests */
    test_attestation_service();
    test_storage_service();
    test_crypto_random();
    test_crypto_hash();
    test_huk_derivation();

    /* All tests passed - enter idle loop */
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        /* All TF-M tests completed successfully */
    }
}
