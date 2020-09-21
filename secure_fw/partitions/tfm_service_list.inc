/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

#ifndef __TFM_SERVICE_LIST_INC__
#define __TFM_SERVICE_LIST_INC__

#include "secure_fw/partitions/internal_trusted_storage/psa_manifest/tfm_internal_trusted_storage.h"
#include "secure_fw/partitions/crypto/psa_manifest/tfm_crypto.h"
#include "secure_fw/partitions/platform/psa_manifest/tfm_platform.h"
#include "secure_fw/partitions/initial_attestation/psa_manifest/tfm_initial_attestation.h"

const struct tfm_spm_service_db_t service_db[] =
{

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    /******** TFM_SP_ITS ********/
    {
        .name = "TFM_ITS_SET",
        .partition_id = TFM_SP_ITS,
        .signal = TFM_ITS_SET_SIGNAL,
        .sid = 0x00000070,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_ITS_GET",
        .partition_id = TFM_SP_ITS,
        .signal = TFM_ITS_GET_SIGNAL,
        .sid = 0x00000071,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_ITS_GET_INFO",
        .partition_id = TFM_SP_ITS,
        .signal = TFM_ITS_GET_INFO_SIGNAL,
        .sid = 0x00000072,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_ITS_REMOVE",
        .partition_id = TFM_SP_ITS,
        .signal = TFM_ITS_REMOVE_SIGNAL,
        .sid = 0x00000073,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_CRYPTO
    /******** TFM_SP_CRYPTO ********/
    {
        .name = "TFM_CRYPTO",
        .partition_id = TFM_SP_CRYPTO,
        .signal = TFM_CRYPTO_SIGNAL,
        .sid = 0x00000080,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    /******** TFM_SP_PLATFORM ********/
    {
        .name = "TFM_SP_PLATFORM_SYSTEM_RESET",
        .partition_id = TFM_SP_PLATFORM,
        .signal = TFM_SP_PLATFORM_SYSTEM_RESET_SIGNAL,
        .sid = 0x00000040,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_SP_PLATFORM_IOCTL",
        .partition_id = TFM_SP_PLATFORM,
        .signal = TFM_SP_PLATFORM_IOCTL_SIGNAL,
        .sid = 0x00000041,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_SP_PLATFORM_NV_COUNTER",
        .partition_id = TFM_SP_PLATFORM,
        .signal = TFM_SP_PLATFORM_NV_COUNTER_SIGNAL,
        .sid = 0x00000042,
        .non_secure_client = false,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    /******** TFM_SP_INITIAL_ATTESTATION ********/
    {
        .name = "TFM_ATTEST_GET_TOKEN",
        .partition_id = TFM_SP_INITIAL_ATTESTATION,
        .signal = TFM_ATTEST_GET_TOKEN_SIGNAL,
        .sid = 0x00000020,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_ATTEST_GET_TOKEN_SIZE",
        .partition_id = TFM_SP_INITIAL_ATTESTATION,
        .signal = TFM_ATTEST_GET_TOKEN_SIZE_SIGNAL,
        .sid = 0x00000021,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
    {
        .name = "TFM_ATTEST_GET_PUBLIC_KEY",
        .partition_id = TFM_SP_INITIAL_ATTESTATION,
        .signal = TFM_ATTEST_GET_PUBLIC_KEY_SIGNAL,
        .sid = 0x00000022,
        .non_secure_client = true,
        .version = 1,
        .version_policy = TFM_VERSION_POLICY_STRICT
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

};

/**************************************************************************/
/** The service list */
/**************************************************************************/
struct tfm_spm_service_t service[] =
{

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    /******** TFM_SP_ITS ********/
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_CRYPTO
    /******** TFM_SP_CRYPTO ********/
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    /******** TFM_SP_PLATFORM ********/
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    /******** TFM_SP_INITIAL_ATTESTATION ********/
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
    {
        .service_db = NULL,
        .partition = NULL,
        .handle_list = {0},
        .msg_queue = {0},
        .list = {0},
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

};

#endif /* __TFM_SERVICE_LIST_INC__ */
