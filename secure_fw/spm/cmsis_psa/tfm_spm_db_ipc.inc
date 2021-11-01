/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

#ifndef __TFM_SPM_DB_IPC_INC__
#define __TFM_SPM_DB_IPC_INC__

#include "tfm_common_config.h"
#include "psa_manifest/sid.h"

/**************************************************************************/
/** IRQ count per partition */
/**************************************************************************/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
#define TFM_PARTITION_TFM_SP_PS_IRQ_COUNT 0
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
#define TFM_PARTITION_TFM_SP_ITS_IRQ_COUNT 0
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
#define TFM_PARTITION_TFM_SP_AUDIT_LOG_IRQ_COUNT 0
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
#define TFM_PARTITION_TFM_SP_CRYPTO_IRQ_COUNT 0
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
#define TFM_PARTITION_TFM_SP_PLATFORM_IRQ_COUNT 0
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
#define TFM_PARTITION_TFM_SP_INITIAL_ATTESTATION_IRQ_COUNT 0
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
#define TFM_PARTITION_TFM_SP_CORE_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
#define TFM_PARTITION_TFM_SP_CORE_TEST_2_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
#define TFM_PARTITION_TFM_SP_SECURE_TEST_PARTITION_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
#define TFM_PARTITION_TFM_SP_IPC_SERVICE_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
#define TFM_PARTITION_TFM_SP_IPC_CLIENT_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
#define TFM_PARTITION_TFM_IRQ_TEST_1_IRQ_COUNT 1
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
#define TFM_PARTITION_TFM_SP_PS_TEST_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
#define TFM_PARTITION_TFM_SP_SECURE_CLIENT_2_IRQ_COUNT 0
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

/**************************************************************************/
/** Declarations of partition init functions */
/**************************************************************************/
extern void tfm_nspm_thread_entry(void);

#ifdef TFM_PARTITION_PROTECTED_STORAGE
extern void tfm_ps_req_mngr_init(void);
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
extern void tfm_its_req_mngr_init(void);
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
extern void audit_core_init(void);
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
extern void tfm_crypto_init(void);
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
extern void platform_sp_init(void);
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
extern void attest_partition_init(void);
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
extern void core_test_init(void);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
extern void core_test_2_init(void);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
extern void tfm_secure_client_service_init(void);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
extern void ipc_service_test_main(void);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
extern void ipc_client_test_main(void);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
extern void tfm_irq_test_1_init(void);
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
extern void tfm_ps_test_init(void);
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
extern void tfm_secure_client_2_init(void);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

/**************************************************************************/
/** Memory region declarations */
/**************************************************************************/
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Base);
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Limit);

#ifdef TFM_PARTITION_PROTECTED_STORAGE
REGION_DECLARE(Image$$, TFM_SP_PS_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_PS_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_PS_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_PS_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
REGION_DECLARE(Image$$, TFM_SP_ITS_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_ITS_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_ITS_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_ITS_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
REGION_DECLARE(Image$$, TFM_SP_AUDIT_LOG_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_AUDIT_LOG_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_AUDIT_LOG_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_AUDIT_LOG_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
REGION_DECLARE(Image$$, TFM_SP_CRYPTO_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_CRYPTO_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_CRYPTO_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_CRYPTO_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
REGION_DECLARE(Image$$, TFM_SP_PLATFORM_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_PLATFORM_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_PLATFORM_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_PLATFORM_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
REGION_DECLARE(Image$$, TFM_SP_INITIAL_ATTESTATION_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_INITIAL_ATTESTATION_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
REGION_DECLARE(Image$$, TFM_SP_CORE_TEST_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_CORE_TEST_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_CORE_TEST_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_CORE_TEST_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
REGION_DECLARE(Image$$, TFM_SP_CORE_TEST_2_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_CORE_TEST_2_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_CORE_TEST_2_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_CORE_TEST_2_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
REGION_DECLARE(Image$$, TFM_SP_SECURE_TEST_PARTITION_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_SECURE_TEST_PARTITION_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
REGION_DECLARE(Image$$, TFM_SP_IPC_SERVICE_TEST_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_IPC_SERVICE_TEST_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
REGION_DECLARE(Image$$, TFM_SP_IPC_CLIENT_TEST_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_IPC_CLIENT_TEST_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
REGION_DECLARE(Image$$, TFM_IRQ_TEST_1_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_IRQ_TEST_1_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_IRQ_TEST_1_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_IRQ_TEST_1_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
REGION_DECLARE(Image$$, TFM_SP_PS_TEST_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_PS_TEST_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_PS_TEST_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_PS_TEST_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
REGION_DECLARE(Image$$, TFM_SP_SECURE_CLIENT_2_LINKER, _STACK$$ZI$$Base);
REGION_DECLARE(Image$$, TFM_SP_SECURE_CLIENT_2_LINKER, _STACK$$ZI$$Limit);
#if TFM_LVL == 3
REGION_DECLARE(Image$$, PT_TFM_SP_SECURE_CLIENT_2_PRIVATE, _DATA_START$$Base);
REGION_DECLARE(Image$$, PT_TFM_SP_SECURE_CLIENT_2_PRIVATE, _DATA_END$$Base);
#endif
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

/**************************************************************************/
/** Dependencies array for Secure Partition */
/**************************************************************************/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
static uint32_t dependencies_TFM_SP_PS[] =
{
    TFM_CRYPTO_SID,
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
    TFM_SP_PLATFORM_NV_COUNTER_SID,
};
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_CRYPTO
static uint32_t dependencies_TFM_SP_CRYPTO[] =
{
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
};
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
static uint32_t dependencies_TFM_SP_INITIAL_ATTESTATION[] =
{
    TFM_CRYPTO_SID,
};
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
static uint32_t dependencies_TFM_SP_CORE_TEST[] =
{
    SPM_CORE_TEST_2_INVERT_SID,
    SPM_CORE_TEST_2_GET_EVERY_SECOND_BYTE_SID,
    SPM_CORE_TEST_2_SLAVE_SERVICE_SID,
};
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static uint32_t dependencies_TFM_SP_SECURE_TEST_PARTITION[] =
{
    TFM_SECURE_CLIENT_2_SID,
    TFM_CRYPTO_SID,
    TFM_PS_SET_SID,
    TFM_PS_GET_SID,
    TFM_PS_GET_INFO_SID,
    TFM_PS_REMOVE_SID,
    TFM_PS_GET_SUPPORT_SID,
    TFM_ITS_SET_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_GET_INFO_SID,
    TFM_ITS_REMOVE_SID,
    TFM_ATTEST_GET_TOKEN_SID,
    TFM_ATTEST_GET_TOKEN_SIZE_SID,
    TFM_ATTEST_GET_PUBLIC_KEY_SID,
    TFM_PS_TEST_PREPARE_SID,
    TFM_SP_PLATFORM_SYSTEM_RESET_SID,
    TFM_SP_PLATFORM_IOCTL_SID,
    TFM_FWU_WRITE_SID,
    TFM_FWU_INSTALL_SID,
    TFM_FWU_ABORT_SID,
    TFM_FWU_QUERY_SID,
    TFM_FWU_REQUEST_REBOOT_SID,
    TFM_FWU_ACCEPT_SID,
    TFM_FFM11_SERVICE1_SID,
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
static uint32_t dependencies_TFM_SP_IPC_CLIENT_TEST[] =
{
    IPC_SERVICE_TEST_PSA_ACCESS_APP_READ_ONLY_MEM_SID,
    IPC_SERVICE_TEST_PSA_ACCESS_APP_MEM_SID,
    IPC_SERVICE_TEST_BASIC_SID,
    IPC_SERVICE_TEST_APP_ACCESS_PSA_MEM_SID,
    TFM_SECURE_CLIENT_2_SID,
};
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_PS
static uint32_t dependencies_TFM_SP_PS_TEST[] =
{
    TFM_CRYPTO_SID,
    TFM_ITS_GET_SID,
    TFM_ITS_REMOVE_SID,
};
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
static uint32_t dependencies_TFM_SP_SECURE_CLIENT_2[] =
{
    TFM_ITS_GET_SID,
    TFM_CRYPTO_SID,
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

/**************************************************************************/
/** The static data of the partition list */
/**************************************************************************/
const struct partition_static_t static_data_list[] =
{
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_NON_SECURE_ID,
#if TFM_MULTI_CORE_TOPOLOGY
        .partition_flags      = SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_IPC,
#else
        .partition_flags      = SPM_PART_FLAG_APP_ROT | SPM_PART_FLAG_IPC,
#endif
        .partition_priority   = TFM_PRIORITY_LOW,
        .partition_init       = tfm_nspm_thread_entry,
    },

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_PS,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_ps_req_mngr_init,
        .dependencies_num     = 6,
        .p_dependencies       = dependencies_TFM_SP_PS,
    },
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_ITS,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_its_req_mngr_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_AUDIT_LOG,
        .partition_flags      = 0
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = audit_core_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_CRYPTO,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_crypto_init,
        .dependencies_num     = 4,
        .p_dependencies       = dependencies_TFM_SP_CRYPTO,
    },
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_PLATFORM,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = platform_sp_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_INITIAL_ATTESTATION,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = attest_partition_init,
        .dependencies_num     = 1,
        .p_dependencies       = dependencies_TFM_SP_INITIAL_ATTESTATION,
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_CORE_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = core_test_init,
        .dependencies_num     = 3,
        .p_dependencies       = dependencies_TFM_SP_CORE_TEST,
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_CORE_TEST_2,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = core_test_2_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_SECURE_TEST_PARTITION,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_secure_client_service_init,
        .dependencies_num     = 24,
        .p_dependencies       = dependencies_TFM_SP_SECURE_TEST_PARTITION,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_IPC_SERVICE_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(HIGH),
        .partition_init       = ipc_service_test_main,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_IPC_CLIENT_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = ipc_client_test_main,
        .dependencies_num     = 5,
        .p_dependencies       = dependencies_TFM_SP_IPC_CLIENT_TEST,
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_IRQ_TEST_1,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_irq_test_1_init,
        .dependencies_num     = 0,
        .p_dependencies       = NULL,
    },
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_PS_TEST,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_PSA_ROT | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_ps_test_init,
        .dependencies_num     = 3,
        .p_dependencies       = dependencies_TFM_SP_PS_TEST,
    },
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
        .psa_framework_version = 0x0100,
        .partition_id         = TFM_SP_SECURE_CLIENT_2,
        .partition_flags      = SPM_PART_FLAG_IPC
                              | SPM_PART_FLAG_APP_ROT
                              ,
        .partition_priority   = TFM_PRIORITY(NORMAL),
        .partition_init       = tfm_secure_client_2_init,
        .dependencies_num     = 2,
        .p_dependencies       = dependencies_TFM_SP_SECURE_CLIENT_2,
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

};

/**************************************************************************/
/** The platform data of the partition list */
/**************************************************************************/
#ifdef TFM_PARTITION_AUDIT_LOG
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_AUDIT_LOG[] =
{
#ifdef AUDIT_UART_REDIRECTION
    TFM_PERIPHERAL_UART1,
#endif /* AUDIT_UART_REDIRECTION */
    NULL
};
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_TEST_CORE
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_CORE_TEST[] =
{
    TFM_PERIPHERAL_FPGA_IO,
    NULL
};
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_SP_SECURE_TEST_PARTITION[] =
{
    TFM_PERIPHERAL_STD_UART,
    NULL
};
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_ENABLE_IRQ_TEST
const struct tfm_spm_partition_platform_data_t *
    platform_data_list_TFM_IRQ_TEST_1[] =
{
    TFM_PERIPHERAL_TIMER0,
    NULL
};
#endif /* TFM_ENABLE_IRQ_TEST */

const struct tfm_spm_partition_platform_data_t **platform_data_list_list[] =
{
    NULL,

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    NULL,
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    NULL,
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    platform_data_list_TFM_SP_AUDIT_LOG,
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    NULL,
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    NULL,
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    NULL,
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    platform_data_list_TFM_SP_CORE_TEST,
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
    NULL,
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    platform_data_list_TFM_SP_SECURE_TEST_PARTITION,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    NULL,
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    NULL,
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    platform_data_list_TFM_IRQ_TEST_1,
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
    NULL,
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    NULL,
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

};

/**************************************************************************/
/** The memory data of the partition list */
/**************************************************************************/
const struct tfm_spm_partition_memory_data_t memory_data_list[] =
{
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(ARM_LIB_STACK, $$ZI$$Base),
        .data_limit           = PART_REGION_ADDR(ARM_LIB_STACK, $$ZI$$Limit),
#endif
        .stack_bottom         = PART_REGION_ADDR(ARM_LIB_STACK, $$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(ARM_LIB_STACK, $$ZI$$Limit),
    },
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_PS_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_PS_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_PS_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_PS_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_ITS_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_ITS_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_ITS_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_ITS_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_AUDIT_LOG_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_AUDIT_LOG_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_AUDIT_LOG_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_AUDIT_LOG_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_CRYPTO_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_CRYPTO_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_CRYPTO_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_CRYPTO_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_PLATFORM_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_PLATFORM_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_PLATFORM_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_PLATFORM_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_INITIAL_ATTESTATION_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_INITIAL_ATTESTATION_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_CORE_TEST_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_CORE_TEST_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_CORE_TEST_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_CORE_TEST_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_CORE_TEST_2_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_CORE_TEST_2_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_CORE_TEST_2_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_CORE_TEST_2_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_SECURE_TEST_PARTITION_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_SECURE_TEST_PARTITION_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_IPC_SERVICE_TEST_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_IPC_SERVICE_TEST_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_IPC_CLIENT_TEST_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_IPC_CLIENT_TEST_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_IRQ_TEST_1_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_IRQ_TEST_1_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_IRQ_TEST_1_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_IRQ_TEST_1_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_PS
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_PS_TEST_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_PS_TEST_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_PS_TEST_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_PS_TEST_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {
#if TFM_LVL == 3
        .data_start           = PART_REGION_ADDR(PT_TFM_SP_SECURE_CLIENT_2_PRIVATE, _DATA_START$$Base),
        .data_limit           = PART_REGION_ADDR(PT_TFM_SP_SECURE_CLIENT_2_PRIVATE, _DATA_END$$Base),
#endif
        .stack_bottom         = PART_REGION_ADDR(TFM_SP_SECURE_CLIENT_2_LINKER, _STACK$$ZI$$Base),
        .stack_top            = PART_REGION_ADDR(TFM_SP_SECURE_CLIENT_2_LINKER, _STACK$$ZI$$Limit),
    },
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

};

/**************************************************************************/
/** The partition list for the DB */
/**************************************************************************/
static struct partition_t partition_list [] =
{
    {0}, /* placeholder for Non-secure internal partition */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PS */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    {0},
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_ITS */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    {0},
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_AUDIT_LOG */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_AUDIT_LOG
    {0},
#endif /* TFM_PARTITION_AUDIT_LOG */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CRYPTO */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_CRYPTO
    {0},
#endif /* TFM_PARTITION_CRYPTO */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PLATFORM */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_PLATFORM
    {0},
#endif /* TFM_PARTITION_PLATFORM */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_INITIAL_ATTESTATION */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {0},
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CORE_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE
    {0},
#endif /* TFM_PARTITION_TEST_CORE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_CORE_TEST_2 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE
    {0},
#endif /* TFM_PARTITION_TEST_CORE */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_SECURE_TEST_PARTITION */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {0},
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_IPC_SERVICE_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE_IPC
    {0},
#endif /* TFM_PARTITION_TEST_CORE_IPC */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_IPC_CLIENT_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_CORE_IPC
    {0},
#endif /* TFM_PARTITION_TEST_CORE_IPC */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_IRQ_TEST_1 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_ENABLE_IRQ_TEST
    {0},
#endif /* TFM_ENABLE_IRQ_TEST */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_PS_TEST */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_PS
    {0},
#endif /* TFM_PARTITION_TEST_PS */

    /* -----------------------------------------------------------------------*/
    /* - Partition DB record for TFM_SP_SECURE_CLIENT_2 */
    /* -----------------------------------------------------------------------*/
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    {0},
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

};

struct spm_partition_db_t g_spm_partition_db = {
    .is_init = 0,
    .partition_count = sizeof(partition_list) / sizeof(partition_list[0]),
    .partitions = partition_list,
};

#endif /* __TFM_SPM_DB_IPC_INC__ */
