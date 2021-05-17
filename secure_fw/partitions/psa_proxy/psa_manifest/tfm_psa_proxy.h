/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

#ifndef __PSA_MANIFEST_TFM_PSA_PROXY_H__
#define __PSA_MANIFEST_TFM_PSA_PROXY_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TFM_CRYPTO_SIGNAL                                       (1U << (0 + 4))
#define TFM_ATTEST_GET_TOKEN_SIGNAL                             (1U << (1 + 4))
#define TFM_ATTEST_GET_TOKEN_SIZE_SIGNAL                        (1U << (2 + 4))
#define TFM_ATTEST_GET_PUBLIC_KEY_SIGNAL                        (1U << (3 + 4))
#define TFM_ITS_SET_SIGNAL                                      (1U << (4 + 4))
#define TFM_ITS_GET_SIGNAL                                      (1U << (5 + 4))
#define TFM_ITS_GET_INFO_SIGNAL                                 (1U << (6 + 4))
#define TFM_ITS_REMOVE_SIGNAL                                   (1U << (7 + 4))
#define TFM_SP_PLATFORM_SYSTEM_RESET_SIGNAL                     (1U << (8 + 4))
#define TFM_SP_PLATFORM_IOCTL_SIGNAL                            (1U << (9 + 4))
#define TFM_SP_PLATFORM_NV_COUNTER_SIGNAL                       (1U << (10 + 4))
#define TFM_PS_SET_SIGNAL                                       (1U << (11 + 4))
#define TFM_PS_GET_SIGNAL                                       (1U << (12 + 4))
#define TFM_PS_GET_INFO_SIGNAL                                  (1U << (13 + 4))
#define TFM_PS_REMOVE_SIGNAL                                    (1U << (14 + 4))
#define TFM_PS_GET_SUPPORT_SIGNAL                               (1U << (15 + 4))

#ifdef __cplusplus
}
#endif

#endif /* __PSA_MANIFEST_TFM_PSA_PROXY_H__ */
