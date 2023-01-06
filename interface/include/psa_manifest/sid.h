/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/***********  WARNING: This is an auto-generated file. Do not edit!  ***********/

#ifndef __PSA_MANIFEST_SID_H__
#define __PSA_MANIFEST_SID_H__

#ifdef __cplusplus
extern "C" {
#endif

/******** TFM_SP_PS ********/
#define TFM_PROTECTED_STORAGE_SERVICE_SID                          (0x00000060U)
#define TFM_PROTECTED_STORAGE_SERVICE_VERSION                      (1U)
#define TFM_PROTECTED_STORAGE_SERVICE_HANDLE                       (0x40000101U)

/******** TFM_SP_ITS ********/
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_SID                   (0x00000070U)
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_VERSION               (1U)
#define TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE                (0x40000102U)

/******** TFM_SP_CRYPTO ********/
#define TFM_CRYPTO_SID                                             (0x00000080U)
#define TFM_CRYPTO_VERSION                                         (1U)
#define TFM_CRYPTO_HANDLE                                          (0x40000100U)

/******** TFM_SP_PLATFORM ********/
#define TFM_PLATFORM_SERVICE_SID                                   (0x00000040U)
#define TFM_PLATFORM_SERVICE_VERSION                               (1U)
#define TFM_PLATFORM_SERVICE_HANDLE                                (0x40000105U)

/******** TFM_SP_INITIAL_ATTESTATION ********/
#define TFM_ATTESTATION_SERVICE_SID                                (0x00000020U)
#define TFM_ATTESTATION_SERVICE_VERSION                            (1U)
#define TFM_ATTESTATION_SERVICE_HANDLE                             (0x40000103U)

/******** TFM_SP_FWU ********/
#define TFM_FIRMWARE_UPDATE_SERVICE_SID                            (0x000000A0U)
#define TFM_FIRMWARE_UPDATE_SERVICE_VERSION                        (1U)
#define TFM_FIRMWARE_UPDATE_SERVICE_HANDLE                         (0x40000104U)

/******** TFM_SP_SECURE_TEST_PARTITION ********/
#define TFM_SECURE_CLIENT_SRV_DUMMY_SID                            (0x0000F000U)
#define TFM_SECURE_CLIENT_SRV_DUMMY_VERSION                        (1U)

/******** TFM_SP_PS_TEST ********/
#define TFM_PS_TEST_PREPARE_SID                                    (0x0000F0C0U)
#define TFM_PS_TEST_PREPARE_VERSION                                (1U)

/******** TFM_SP_SECURE_CLIENT_2 ********/
#define TFM_SECURE_CLIENT_2_SID                                    (0x0000F0E0U)
#define TFM_SECURE_CLIENT_2_VERSION                                (1U)

/******** TFM_SP_SLIH_TEST ********/
#define TFM_SLIH_TEST_CASE_SID                                     (0x0000F0A0U)
#define TFM_SLIH_TEST_CASE_VERSION                                 (1U)
#define TFM_SLIH_TEST_CASE_HANDLE                                  (0x40000106U)

/******** TFM_SFN_PARTITION1 ********/
#define SFN_TEST_STATELESS_SID                                     (0x0000F100U)
#define SFN_TEST_STATELESS_VERSION                                 (1U)
#define SFN_TEST_STATELESS_HANDLE                                  (0x40000107U)
#define SFN_TEST_CONNECTION_BASED_SID                              (0x0000F101U)
#define SFN_TEST_CONNECTION_BASED_VERSION                          (1U)

#ifdef __cplusplus
}
#endif

#endif /* __PSA_MANIFEST_SID_H__ */
