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
#define TFM_SP_PLATFORM_SYSTEM_RESET_SID                           (0x00000040U)
#define TFM_SP_PLATFORM_SYSTEM_RESET_VERSION                       (1U)
#define TFM_SP_PLATFORM_IOCTL_SID                                  (0x00000041U)
#define TFM_SP_PLATFORM_IOCTL_VERSION                              (1U)
#define TFM_SP_PLATFORM_NV_COUNTER_SID                             (0x00000042U)
#define TFM_SP_PLATFORM_NV_COUNTER_VERSION                         (1U)

/******** TFM_SP_INITIAL_ATTESTATION ********/
#define TFM_ATTESTATION_SERVICE_SID                                (0x00000020U)
#define TFM_ATTESTATION_SERVICE_VERSION                            (1U)
#define TFM_ATTESTATION_SERVICE_HANDLE                             (0x40000103U)

/******** TFM_SP_FWU ********/
#define TFM_FWU_WRITE_SID                                          (0x000000A0U)
#define TFM_FWU_WRITE_VERSION                                      (1U)
#define TFM_FWU_INSTALL_SID                                        (0x000000A1U)
#define TFM_FWU_INSTALL_VERSION                                    (1U)
#define TFM_FWU_ABORT_SID                                          (0x000000A2U)
#define TFM_FWU_ABORT_VERSION                                      (1U)
#define TFM_FWU_QUERY_SID                                          (0x000000A3U)
#define TFM_FWU_QUERY_VERSION                                      (1U)
#define TFM_FWU_REQUEST_REBOOT_SID                                 (0x000000A4U)
#define TFM_FWU_REQUEST_REBOOT_VERSION                             (1U)
#define TFM_FWU_ACCEPT_SID                                         (0x000000A5U)
#define TFM_FWU_ACCEPT_VERSION                                     (1U)

/******** TFM_SP_SECURE_TEST_PARTITION ********/
#define TFM_SECURE_CLIENT_SRV_DUMMY_SID                            (0x0000F000U)
#define TFM_SECURE_CLIENT_SRV_DUMMY_VERSION                        (1U)

/******** TFM_SP_IPC_SERVICE_TEST ********/
#define IPC_SERVICE_TEST_BASIC_SID                                 (0x0000F080U)
#define IPC_SERVICE_TEST_BASIC_VERSION                             (1U)
#define IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_SID                    (0x0000F081U)
#define IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_VERSION                (1U)
#define IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SID          (0x0000F082U)
#define IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_VERSION      (1U)
#define IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_SID                    (0x0000F083U)
#define IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_VERSION                (1U)
#define IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_SID               (0x0000F084U)
#define IPC_SERVICE_TEST_CLIENT_PROGRAMMER_ERROR_VERSION           (1U)
#define IPC_SERVICE_TEST_STATELESS_ROT_SID                         (0x0000F085U)
#define IPC_SERVICE_TEST_STATELESS_ROT_VERSION                     (1U)
#define IPC_SERVICE_TEST_STATELESS_ROT_HANDLE                      (0x40000104U)
#define IPC_SERVICE_TEST_MMIOVEC_SID                               (0x0000F086U)
#define IPC_SERVICE_TEST_MMIOVEC_VERSION                           (1U)
#define IPC_SERVICE_TEST_MMIOVEC_HANDLE                            (0x40000105U)

/******** TFM_SP_IPC_CLIENT_TEST ********/
#define IPC_CLIENT_TEST_PSA_ACCESS_APP_MEM_SID                     (0x0000F061U)
#define IPC_CLIENT_TEST_PSA_ACCESS_APP_MEM_VERSION                 (1U)
#define IPC_CLIENT_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SID           (0x0000F062U)
#define IPC_CLIENT_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_VERSION       (1U)
#define IPC_CLIENT_TEST_APP_ACCESS_PSA_MEM_SID                     (0x0000F063U)
#define IPC_CLIENT_TEST_APP_ACCESS_PSA_MEM_VERSION                 (1U)
#define IPC_CLIENT_TEST_MEM_CHECK_SID                              (0x0000F064U)
#define IPC_CLIENT_TEST_MEM_CHECK_VERSION                          (1U)
#define IPC_CLIENT_TEST_RETRIEVE_APP_MEM_SID                       (0x0000F065U)
#define IPC_CLIENT_TEST_RETRIEVE_APP_MEM_VERSION                   (1U)

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

#ifdef __cplusplus
}
#endif

#endif /* __PSA_MANIFEST_SID_H__ */
