;/*
; * Copyright (c) 2009-2021 Arm Limited
; *
; * Licensed under the Apache License, Version 2.0 (the "License");
; * you may not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; *     http://www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an "AS IS" BASIS,
; * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; *
; *
; * This file is derivative of CMSIS V5.00 gcc_arm.ld
; */

/*********** WARNING: This is an auto-generated file. Do not edit! ***********/

/* Linker script to configure memory regions. */
/* This file will be run trough the pre-processor. */

#include "region_defs.h"

MEMORY
{
  FLASH    (rx)  : ORIGIN = S_CODE_START, LENGTH = S_CODE_SIZE
  RAM      (rwx) : ORIGIN = S_DATA_START, LENGTH = S_DATA_SIZE
#if defined(S_CODE_SRAM_ALIAS_BASE)
  CODE_RAM (rwx) : ORIGIN = S_CODE_SRAM_ALIAS_BASE, LENGTH = TOTAL_CODE_SRAM_SIZE
#endif
  VENEERS  (rx)  : ORIGIN = CMSE_VENEER_REGION_START, LENGTH = CMSE_VENEER_REGION_SIZE
}

__heap_size__  = S_HEAP_SIZE;
__psp_stack_size__ = S_PSP_STACK_SIZE;
__msp_init_stack_size__ = S_MSP_STACK_SIZE_INIT;

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a libc_nano.a)

ENTRY(Reset_Handler)

SECTIONS
{
    .TFM_VECTORS : ALIGN(4)
    {
        __vectors_start__ = .;
        KEEP(*(.vectors))
        __vectors_end__ = .;
        *startup*(.text*)
        . = ALIGN(4);
    } > FLASH

    .copy.table : ALIGN(4)
    {
        __copy_table_start__ = .;
        LONG (LOADADDR(.TFM_DATA))
        LONG (ADDR(.TFM_DATA))
        LONG (SIZEOF(.TFM_DATA))
#ifdef TFM_PARTITION_PROTECTED_STORAGE
        LONG (LOADADDR(.ER_TFM_SP_PS_RWZI))
        LONG (ADDR(.ER_TFM_SP_PS_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PS_RWZI))
#endif /* TFM_PARTITION_PROTECTED_STORAGE */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
        LONG (LOADADDR(.ER_TFM_SP_ITS_RWZI))
        LONG (ADDR(.ER_TFM_SP_ITS_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_ITS_RWZI))
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */
#ifdef TFM_PARTITION_AUDIT_LOG
        LONG (LOADADDR(.ER_TFM_SP_AUDIT_LOG_RWZI))
        LONG (ADDR(.ER_TFM_SP_AUDIT_LOG_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_AUDIT_LOG_RWZI))
#endif /* TFM_PARTITION_AUDIT_LOG */
#ifdef TFM_PARTITION_CRYPTO
        LONG (LOADADDR(.ER_TFM_SP_CRYPTO_RWZI))
        LONG (ADDR(.ER_TFM_SP_CRYPTO_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_CRYPTO_RWZI))
#endif /* TFM_PARTITION_CRYPTO */
#ifdef TFM_PARTITION_PLATFORM
        LONG (LOADADDR(.ER_TFM_SP_PLATFORM_RWZI))
        LONG (ADDR(.ER_TFM_SP_PLATFORM_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PLATFORM_RWZI))
#endif /* TFM_PARTITION_PLATFORM */
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
        LONG (LOADADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI))
        LONG (ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI))
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (LOADADDR(.ER_TFM_SP_CORE_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_CORE_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_CORE_TEST_RWZI))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (LOADADDR(.ER_TFM_SP_CORE_TEST_2_RWZI))
        LONG (ADDR(.ER_TFM_SP_CORE_TEST_2_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_CORE_TEST_2_RWZI))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (LOADADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI))
        LONG (ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (LOADADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (LOADADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_PS
        LONG (LOADADDR(.ER_TFM_SP_PS_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_PS_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PS_TEST_RWZI))
#endif /* TFM_PARTITION_TEST_PS */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (LOADADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI))
        LONG (ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SECURE_CLIENT_2_RWZI))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_FIRMWARE_UPDATE
        LONG (LOADADDR(.ER_TFM_SP_FWU_RWZI))
        LONG (ADDR(.ER_TFM_SP_FWU_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_FWU_RWZI))
#endif /* TFM_PARTITION_FIRMWARE_UPDATE */
#ifdef TFM_PARTITION_FFM11
        LONG (LOADADDR(.ER_TFM_SP_FFM11_RWZI))
        LONG (ADDR(.ER_TFM_SP_FFM11_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_FFM11_RWZI))
#endif /* TFM_PARTITION_FFM11 */
#ifdef ATTEST_TEST_GET_PUBLIC_KEY
        LONG (LOADADDR(.ER_TFM_SP_ATTEST_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_ATTEST_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_ATTEST_TEST_RWZI))
#endif /* ATTEST_TEST_GET_PUBLIC_KEY */
#ifdef TFM_ENABLE_SLIH_TEST
        LONG (LOADADDR(.ER_TFM_SP_SLIH_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_SLIH_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SLIH_TEST_RWZI))
#endif /* TFM_ENABLE_SLIH_TEST */
#ifdef TFM_ENABLE_FLIH_TEST
        LONG (LOADADDR(.ER_TFM_SP_FLIH_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_FLIH_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_FLIH_TEST_RWZI))
#endif /* TFM_ENABLE_FLIH_TEST */
#if defined(S_CODE_SRAM_ALIAS_BASE)
        LONG (LOADADDR(.ER_EFLASH_DRIVER_RO))
        LONG (ADDR(.ER_EFLASH_DRIVER_RO))
        LONG (SIZEOF(.ER_EFLASH_DRIVER_RO))
#endif
        __copy_table_end__ = .;
    } > FLASH

    .zero.table : ALIGN(4)
    {
        __zero_table_start__ = .;
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS))
#ifdef TFM_PARTITION_PROTECTED_STORAGE
        LONG (ADDR(.TFM_SP_PS_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PS_RWZI_BSS))
#endif /* TFM_PARTITION_PROTECTED_STORAGE */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
        LONG (ADDR(.TFM_SP_ITS_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_ITS_RWZI_BSS))
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */
#ifdef TFM_PARTITION_AUDIT_LOG
        LONG (ADDR(.TFM_SP_AUDIT_LOG_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_AUDIT_LOG_RWZI_BSS))
#endif /* TFM_PARTITION_AUDIT_LOG */
#ifdef TFM_PARTITION_CRYPTO
        LONG (ADDR(.TFM_SP_CRYPTO_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS))
#endif /* TFM_PARTITION_CRYPTO */
#ifdef TFM_PARTITION_PLATFORM
        LONG (ADDR(.TFM_SP_PLATFORM_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS))
#endif /* TFM_PARTITION_PLATFORM */
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS))
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (ADDR(.TFM_SP_CORE_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_2_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_PS
        LONG (ADDR(.TFM_SP_PS_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_PS */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_FIRMWARE_UPDATE
        LONG (ADDR(.TFM_SP_FWU_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_FWU_RWZI_BSS))
#endif /* TFM_PARTITION_FIRMWARE_UPDATE */
#ifdef TFM_PARTITION_FFM11
        LONG (ADDR(.TFM_SP_FFM11_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_FFM11_RWZI_BSS))
#endif /* TFM_PARTITION_FFM11 */
#ifdef ATTEST_TEST_GET_PUBLIC_KEY
        LONG (ADDR(.TFM_SP_ATTEST_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_ATTEST_TEST_RWZI_BSS))
#endif /* ATTEST_TEST_GET_PUBLIC_KEY */
#ifdef TFM_ENABLE_SLIH_TEST
        LONG (ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SLIH_TEST_RWZI_BSS))
#endif /* TFM_ENABLE_SLIH_TEST */
#ifdef TFM_ENABLE_FLIH_TEST
        LONG (ADDR(.TFM_SP_FLIH_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_FLIH_TEST_RWZI_BSS))
#endif /* TFM_ENABLE_FLIH_TEST */
        __zero_table_end__ = .;
    } > FLASH

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_RO_START$$Base = .;

    /**** Section for holding partition RO load data */
    .TFM_SP_LOAD_LIST : ALIGN(4)
    {
       KEEP(*(.part_load))
    } > FLASH
    Image$$TFM_SP_LOAD_LIST$$RO$$Base = ADDR(.TFM_SP_LOAD_LIST);
    Image$$TFM_SP_LOAD_LIST$$RO$$Limit = ADDR(.TFM_SP_LOAD_LIST) + SIZEOF(.TFM_SP_LOAD_LIST);
    . = ALIGN(32);

    /**** PSA RoT RO CODE + RO-data starts here */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    .TFM_SP_ITS_RO : ALIGN(32)
    {
        *tfm_*partition_its.*:*(.text*)
        *tfm_*partition_its.*:*(.rodata*)
        *(TFM_SP_ITS_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_ITS_RO$$Base = ADDR(.TFM_SP_ITS_RO);
    Image$$TFM_SP_ITS_RO$$Limit = ADDR(.TFM_SP_ITS_RO) + SIZEOF(.TFM_SP_ITS_RO);
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    .TFM_SP_AUDIT_LOG_RO : ALIGN(32)
    {
        *tfm_*partition_audit.*:*(.text*)
        *tfm_*partition_audit.*:*(.rodata*)
        *(TFM_SP_AUDIT_LOG_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_AUDIT_LOG_RO$$Base = ADDR(.TFM_SP_AUDIT_LOG_RO);
    Image$$TFM_SP_AUDIT_LOG_RO$$Limit = ADDR(.TFM_SP_AUDIT_LOG_RO) + SIZEOF(.TFM_SP_AUDIT_LOG_RO);
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    .TFM_SP_CRYPTO_RO : ALIGN(32)
    {
        *tfm_*partition_crypto.*:*(.text*)
        *tfm_*partition_crypto.*:*(.rodata*)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CRYPTO_RO$$Base = ADDR(.TFM_SP_CRYPTO_RO);
    Image$$TFM_SP_CRYPTO_RO$$Limit = ADDR(.TFM_SP_CRYPTO_RO) + SIZEOF(.TFM_SP_CRYPTO_RO);
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    .TFM_SP_PLATFORM_RO : ALIGN(32)
    {
        *tfm_*partition_platform.*:*(.text*)
        *tfm_*partition_platform.*:*(.rodata*)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PLATFORM_RO$$Base = ADDR(.TFM_SP_PLATFORM_RO);
    Image$$TFM_SP_PLATFORM_RO$$Limit = ADDR(.TFM_SP_PLATFORM_RO) + SIZEOF(.TFM_SP_PLATFORM_RO);
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    .TFM_SP_INITIAL_ATTESTATION_RO : ALIGN(32)
    {
        *tfm_*partition_attestation.*:*(.text*)
        *tfm_*partition_attestation.*:*(.rodata*)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO);
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RO);
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    .TFM_SP_CORE_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_core_test.*:*(.text*)
        *tfm_*partition_core_test.*:*(.rodata*)
        *(TFM_SP_CORE_TEST_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CORE_TEST_RO$$Base = ADDR(.TFM_SP_CORE_TEST_RO);
    Image$$TFM_SP_CORE_TEST_RO$$Limit = ADDR(.TFM_SP_CORE_TEST_RO) + SIZEOF(.TFM_SP_CORE_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    .TFM_SP_SECURE_TEST_PARTITION_RO : ALIGN(32)
    {
        *tfm_*partition_secure_client_service.*:*(.text*)
        *tfm_*partition_secure_client_service.*:*(.rodata*)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO);
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RO);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    .TFM_SP_IPC_SERVICE_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_ipc_service.*:*(.text*)
        *tfm_*partition_ipc_service.*:*(.rodata*)
        *(TFM_SP_IPC_SERVICE_TEST_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_IPC_SERVICE_TEST_RO$$Base = ADDR(.TFM_SP_IPC_SERVICE_TEST_RO);
    Image$$TFM_SP_IPC_SERVICE_TEST_RO$$Limit = ADDR(.TFM_SP_IPC_SERVICE_TEST_RO) + SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_PS
    .TFM_SP_PS_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_ps_test.*:*(.text*)
        *tfm_*partition_ps_test.*:*(.rodata*)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PS_TEST_RO$$Base = ADDR(.TFM_SP_PS_TEST_RO);
    Image$$TFM_SP_PS_TEST_RO$$Limit = ADDR(.TFM_SP_PS_TEST_RO) + SIZEOF(.TFM_SP_PS_TEST_RO);
#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_FIRMWARE_UPDATE
    .TFM_SP_FWU_RO : ALIGN(32)
    {
        *tfm_*partition_fwu*:*(.text*)
        *tfm_*partition_fwu*:*(.rodata*)
        *(TFM_SP_FWU_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_FWU_RO$$Base = ADDR(.TFM_SP_FWU_RO);
    Image$$TFM_SP_FWU_RO$$Limit = ADDR(.TFM_SP_FWU_RO) + SIZEOF(.TFM_SP_FWU_RO);
#endif /* TFM_PARTITION_FIRMWARE_UPDATE */

#ifdef ATTEST_TEST_GET_PUBLIC_KEY
    .TFM_SP_ATTEST_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_attest_test_service.*:*(.text*)
        *tfm_*partition_attest_test_service.*:*(.rodata*)
        *(TFM_SP_ATTEST_TEST_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_ATTEST_TEST_RO$$Base = ADDR(.TFM_SP_ATTEST_TEST_RO);
    Image$$TFM_SP_ATTEST_TEST_RO$$Limit = ADDR(.TFM_SP_ATTEST_TEST_RO) + SIZEOF(.TFM_SP_ATTEST_TEST_RO);
#endif /* ATTEST_TEST_GET_PUBLIC_KEY */

    /**** PSA RoT RO CODE + data ends here */

    /**** APPLICATION RoT RO CODE + data starts here */
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    .TFM_SP_PS_RO : ALIGN(32)
    {
        *tfm_*partition_ps.*:*(.text*)
        *tfm_*partition_ps.*:*(.rodata*)
        *(TFM_SP_PS_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PS_CODE$$Base = ADDR(.TFM_SP_PS_RO);
    Image$$TFM_SP_PS_CODE$$Limit = ADDR(.TFM_SP_PS_RO) + SIZEOF(.TFM_SP_PS_RO);
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_TEST_CORE
    .TFM_SP_CORE_TEST_2_RO : ALIGN(32)
    {
        *tfm_*partition_core_test_2.*:*(.text*)
        *tfm_*partition_core_test_2.*:*(.rodata*)
        *(TFM_SP_CORE_TEST_2_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CORE_TEST_2_CODE$$Base = ADDR(.TFM_SP_CORE_TEST_2_RO);
    Image$$TFM_SP_CORE_TEST_2_CODE$$Limit = ADDR(.TFM_SP_CORE_TEST_2_RO) + SIZEOF(.TFM_SP_CORE_TEST_2_RO);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    .TFM_SP_IPC_CLIENT_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_ipc_client.*:*(.text*)
        *tfm_*partition_ipc_client.*:*(.rodata*)
        *(TFM_SP_IPC_CLIENT_TEST_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_IPC_CLIENT_TEST_CODE$$Base = ADDR(.TFM_SP_IPC_CLIENT_TEST_RO);
    Image$$TFM_SP_IPC_CLIENT_TEST_CODE$$Limit = ADDR(.TFM_SP_IPC_CLIENT_TEST_RO) + SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    .TFM_SP_SECURE_CLIENT_2_RO : ALIGN(32)
    {
        *tfm_*partition_secure_client_2.*:*(.text*)
        *tfm_*partition_secure_client_2.*:*(.rodata*)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RO);
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RO) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RO);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_FFM11
    .TFM_SP_FFM11_RO : ALIGN(32)
    {
        *tfm_*partition_ffm11.*:*(.text*)
        *tfm_*partition_ffm11.*:*(.rodata*)
        *(TFM_SP_FFM11_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_FFM11_CODE$$Base = ADDR(.TFM_SP_FFM11_RO);
    Image$$TFM_SP_FFM11_CODE$$Limit = ADDR(.TFM_SP_FFM11_RO) + SIZEOF(.TFM_SP_FFM11_RO);
#endif /* TFM_PARTITION_FFM11 */

#ifdef TFM_ENABLE_SLIH_TEST
    .TFM_SP_SLIH_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_slih_test.*:*(.text*)
        *tfm_*partition_slih_test.*:*(.rodata*)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_SLIH_TEST_CODE$$Base = ADDR(.TFM_SP_SLIH_TEST_RO);
    Image$$TFM_SP_SLIH_TEST_CODE$$Limit = ADDR(.TFM_SP_SLIH_TEST_RO) + SIZEOF(.TFM_SP_SLIH_TEST_RO);
#endif /* TFM_ENABLE_SLIH_TEST */

#ifdef TFM_ENABLE_FLIH_TEST
    .TFM_SP_FLIH_TEST_RO : ALIGN(32)
    {
        *tfm_*partition_flih_test.*:*(.text*)
        *tfm_*partition_flih_test.*:*(.rodata*)
        *(TFM_SP_FLIH_TEST_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_FLIH_TEST_CODE$$Base = ADDR(.TFM_SP_FLIH_TEST_RO);
    Image$$TFM_SP_FLIH_TEST_CODE$$Limit = ADDR(.TFM_SP_FLIH_TEST_RO) + SIZEOF(.TFM_SP_FLIH_TEST_RO);
#endif /* TFM_ENABLE_FLIH_TEST */

    /**** APPLICATION RoT RO CODE + data ends here */

    .ER_TFM_CODE : ALIGN(4)
    {
        *libplatform_s*:*(.text*)
        *libtfm_spm*:*(.text*)

        *libplatform_s*:*(.rodata*)
        *libtfm_spm*:*(.rodata*)
    } > FLASH

    /**** Unprivileged Secure code starts here */
    .ER_UNPRIV_CODE : ALIGN(32)
    {
        *(SFN)
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
         *crtbegin.o(.dtors)
         *crtbegin?.o(.dtors)
         *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
         *(SORT(.dtors.*))
         *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))

        . = ALIGN(32);
    } > FLASH
    Image$$TFM_UNPRIV_CODE$$RO$$Base = ADDR(.ER_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE$$RO$$Limit = ADDR(.ER_UNPRIV_CODE) + SIZEOF(.ER_UNPRIV_CODE);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_RO_END$$Base = .;

#if defined(S_CODE_SRAM_ALIAS_BASE)
    .ER_EFLASH_DRIVER_RO : ALIGN(4)
    {
        *Driver_GFC100_EFlash.o(.text*)
        *Driver_GFC100_EFlash.o(.rodata*)
        *gfc100_eflash_drv.o(.text*)
        *gfc100_eflash_drv.o(.rodata*)
        *musca_b1_eflash_drv.o(.text*)
        *musca_b1_eflash_drv.o(.rodata*)
        . = ALIGN(4); /* This alignment is needed to make the section size 4 bytes aligned */
    } > CODE_RAM AT > FLASH
    Image$$ER_EFLASH_DRIVER_RO$$Base = ADDR(.ER_EFLASH_DRIVER_RO);
    Image$$ER_EFLASH_DRIVER_RO$$Limit = ADDR(.ER_EFLASH_DRIVER_RO) + SIZEOF(.ER_EFLASH_DRIVER_RO);
#endif

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    /**** Base address of secure data area */
    .tfm_secure_data_start :
    {
        . = ABSOLUTE(S_DATA_START) ;
    } > RAM

   /* Position tag */
    . = ALIGN(32);
    Image$$PT_PRIV_RWZI_START$$Base = .;

    /* shared_data and msp_stack are overlapping on purpose when
     * msp_stack is extended until the beginning of RAM, when shared_date
     * was read out by partitions
     */
    .tfm_bl2_shared_data : ALIGN(32)
    {
        . += BOOT_TFM_SHARED_DATA_SIZE;
    } > RAM

    .msp_stack : ALIGN(32)
    {
        . += __msp_init_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK_MSP$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK_MSP$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);

    /**** PSA RoT DATA start here */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_ITS_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_ITS_RWZI : ALIGN(32)
    {
        *tfm_*partition_its.*:*(.data*)
        *(TFM_SP_ITS_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_ITS_RWZI);
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_ITS_RWZI) + SIZEOF(.ER_TFM_SP_ITS_RWZI);

    .TFM_SP_ITS_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_ITS_RWZI = .;
        *tfm_*partition_its.*:*(.bss*)
        *tfm_*partition_its.*:*(COMMON)
        *(TFM_SP_ITS_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_ITS_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Base = ADDR(.TFM_SP_ITS_RWZI_BSS);
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_ITS_RWZI_BSS) + SIZEOF(.TFM_SP_ITS_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_ITS_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_AUDIT_LOG_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_AUDIT_LOG_RWZI : ALIGN(32)
    {
        *tfm_*partition_audit.*:*(.data*)
        *(TFM_SP_AUDIT_LOG_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_AUDIT_LOG_RWZI);
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_AUDIT_LOG_RWZI) + SIZEOF(.ER_TFM_SP_AUDIT_LOG_RWZI);

    .TFM_SP_AUDIT_LOG_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_AUDIT_LOG_RWZI = .;
        *tfm_*partition_audit.*:*(.bss*)
        *tfm_*partition_audit.*:*(COMMON)
        *(TFM_SP_AUDIT_LOG_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_AUDIT_LOG_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$ZI$$Base = ADDR(.TFM_SP_AUDIT_LOG_RWZI_BSS);
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$ZI$$Limit = ADDR(.TFM_SP_AUDIT_LOG_RWZI_BSS) + SIZEOF(.TFM_SP_AUDIT_LOG_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_AUDIT_LOG_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CRYPTO_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_CRYPTO_RWZI : ALIGN(32)
    {
        *tfm_*partition_crypto.*:*(.data*)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CRYPTO_RWZI);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CRYPTO_RWZI) + SIZEOF(.ER_TFM_SP_CRYPTO_RWZI);

    .TFM_SP_CRYPTO_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CRYPTO_RWZI = .;
        *tfm_*partition_crypto.*:*(.bss*)
        *tfm_*partition_crypto.*:*(COMMON)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_CRYPTO_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_RWZI_BSS);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_RWZI_BSS) + SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CRYPTO_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PLATFORM_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PLATFORM_RWZI : ALIGN(32)
    {
        *tfm_*partition_platform.*:*(.data*)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PLATFORM_RWZI);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PLATFORM_RWZI) + SIZEOF(.ER_TFM_SP_PLATFORM_RWZI);

    .TFM_SP_PLATFORM_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PLATFORM_RWZI = .;
        *tfm_*partition_platform.*:*(.bss*)
        *tfm_*partition_platform.*:*(COMMON)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PLATFORM_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_RWZI_BSS);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_RWZI_BSS) + SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PLATFORM_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_INITIAL_ATTESTATION_RWZI : ALIGN(32)
    {
        *tfm_*partition_attestation.*:*(.data*)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI) + SIZEOF(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);

    .TFM_SP_INITIAL_ATTESTATION_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_INITIAL_ATTESTATION_RWZI = .;
        *tfm_*partition_attestation.*:*(.bss*)
        *tfm_*partition_attestation.*:*(COMMON)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_INITIAL_ATTESTATION_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CORE_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_CORE_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_core_test.*:*(.data*)
        *(TFM_SP_CORE_TEST_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CORE_TEST_RWZI);
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CORE_TEST_RWZI) + SIZEOF(.ER_TFM_SP_CORE_TEST_RWZI);

    .TFM_SP_CORE_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CORE_TEST_RWZI = .;
        *tfm_*partition_core_test.*:*(.bss*)
        *tfm_*partition_core_test.*:*(COMMON)
        *(TFM_SP_CORE_TEST_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_CORE_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_CORE_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CORE_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SECURE_TEST_PARTITION_RWZI : ALIGN(32)
    {
        *tfm_*partition_secure_client_service.*:*(.data*)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);

    .TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI = .;
        *tfm_*partition_secure_client_service.*:*(.bss*)
        *tfm_*partition_secure_client_service.*:*(COMMON)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_IPC_SERVICE_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_ipc_service.*:*(.data*)
        *(TFM_SP_IPC_SERVICE_TEST_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI);
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI) + SIZEOF(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI);

    .TFM_SP_IPC_SERVICE_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_IPC_SERVICE_TEST_RWZI = .;
        *tfm_*partition_ipc_service.*:*(.bss*)
        *tfm_*partition_ipc_service.*:*(COMMON)
        *(TFM_SP_IPC_SERVICE_TEST_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_IPC_SERVICE_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_IPC_SERVICE_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_PS

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PS_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PS_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_ps_test.*:*(.data*)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_TEST_RWZI);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_TEST_RWZI) + SIZEOF(.ER_TFM_SP_PS_TEST_RWZI);

    .TFM_SP_PS_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PS_TEST_RWZI = .;
        *tfm_*partition_ps_test.*:*(.bss*)
        *tfm_*partition_ps_test.*:*(COMMON)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PS_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_PS */

#ifdef TFM_PARTITION_FIRMWARE_UPDATE

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FWU_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_FWU_RWZI : ALIGN(32)
    {
        *tfm_*partition_fwu*:*(.data*)
        *(TFM_SP_FWU_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_FWU_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_FWU_RWZI);
    Image$$ER_TFM_SP_FWU_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_FWU_RWZI) + SIZEOF(.ER_TFM_SP_FWU_RWZI);

    .TFM_SP_FWU_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_FWU_RWZI = .;
        *tfm_*partition_fwu*:*(.bss*)
        *tfm_*partition_fwu*:*(COMMON)
        *(TFM_SP_FWU_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_FWU_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_FWU_RWZI$$ZI$$Base = ADDR(.TFM_SP_FWU_RWZI_BSS);
    Image$$ER_TFM_SP_FWU_RWZI$$ZI$$Limit = ADDR(.TFM_SP_FWU_RWZI_BSS) + SIZEOF(.TFM_SP_FWU_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FWU_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_FIRMWARE_UPDATE */

#ifdef ATTEST_TEST_GET_PUBLIC_KEY

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_ATTEST_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_ATTEST_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_attest_test_service.*:*(.data*)
        *(TFM_SP_ATTEST_TEST_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_ATTEST_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_ATTEST_TEST_RWZI);
    Image$$ER_TFM_SP_ATTEST_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_ATTEST_TEST_RWZI) + SIZEOF(.ER_TFM_SP_ATTEST_TEST_RWZI);

    .TFM_SP_ATTEST_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_ATTEST_TEST_RWZI = .;
        *tfm_*partition_attest_test_service.*:*(.bss*)
        *tfm_*partition_attest_test_service.*:*(COMMON)
        *(TFM_SP_ATTEST_TEST_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_ATTEST_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_ATTEST_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_ATTEST_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_ATTEST_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_ATTEST_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_ATTEST_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_ATTEST_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* ATTEST_TEST_GET_PUBLIC_KEY */

    /**** PSA RoT DATA end here */
    . = ALIGN(32);

    /* Position tag */
    Image$$PT_PRIV_RWZI_END$$Base = .;

    /* PSP is unprivileged in single-core topology */
    .psp_stack : ALIGN(32)
    {
        . += (__psp_stack_size__);
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.psp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.psp_stack) + SIZEOF(.psp_stack);

    /**** APPLICATION RoT DATA start here */
#ifdef TFM_PARTITION_PROTECTED_STORAGE

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PS_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PS_RWZI : ALIGN(32)
    {
        *tfm_*partition_ps.*:*(.data*)
        *(TFM_SP_PS_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_RWZI);
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_RWZI) + SIZEOF(.ER_TFM_SP_PS_RWZI);

    .TFM_SP_PS_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PS_RWZI = .;
        *tfm_*partition_ps.*:*(.bss*)
        *tfm_*partition_ps.*:*(COMMON)
        *(TFM_SP_PS_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_RWZI_BSS);
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_RWZI_BSS) + SIZEOF(.TFM_SP_PS_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PS_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_TEST_CORE

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CORE_TEST_2_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_CORE_TEST_2_RWZI : ALIGN(32)
    {
        *tfm_*partition_core_test_2.*:*(.data*)
        *(TFM_SP_CORE_TEST_2_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_CORE_TEST_2_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CORE_TEST_2_RWZI);
    Image$$.ER_TFM_SP_CORE_TEST_2_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CORE_TEST_2_RWZI) + SIZEOF(.ER_TFM_SP_CORE_TEST_2_RWZI);

    .TFM_SP_CORE_TEST_2_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CORE_TEST_2_RWZI = .;
        *tfm_*partition_core_test_2.*:*(.bss*)
        *tfm_*partition_core_test_2.*:*(COMMON)
        *(TFM_SP_CORE_TEST_2_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_CORE_TEST_2_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CORE_TEST_2_RWZI$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS);
    Image$$ER_TFM_SP_CORE_TEST_2_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS) + SIZEOF(.TFM_SP_CORE_TEST_2_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_CORE_TEST_2_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE_IPC

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_IPC_CLIENT_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_ipc_client.*:*(.data*)
        *(TFM_SP_IPC_CLIENT_TEST_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI);
    Image$$.ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI) + SIZEOF(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI);

    .TFM_SP_IPC_CLIENT_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_IPC_CLIENT_TEST_RWZI = .;
        *tfm_*partition_ipc_client.*:*(.bss*)
        *tfm_*partition_ipc_client.*:*(COMMON)
        *(TFM_SP_IPC_CLIENT_TEST_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_IPC_CLIENT_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SECURE_CLIENT_2_RWZI : ALIGN(32)
    {
        *tfm_*partition_secure_client_2.*:*(.data*)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);

    .TFM_SP_SECURE_CLIENT_2_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_SECURE_CLIENT_2_RWZI = .;
        *tfm_*partition_secure_client_2.*:*(.bss*)
        *tfm_*partition_secure_client_2.*:*(COMMON)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_CLIENT_2_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_FFM11

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FFM11_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_FFM11_RWZI : ALIGN(32)
    {
        *tfm_*partition_ffm11.*:*(.data*)
        *(TFM_SP_FFM11_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_FFM11_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_FFM11_RWZI);
    Image$$.ER_TFM_SP_FFM11_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_FFM11_RWZI) + SIZEOF(.ER_TFM_SP_FFM11_RWZI);

    .TFM_SP_FFM11_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_FFM11_RWZI = .;
        *tfm_*partition_ffm11.*:*(.bss*)
        *tfm_*partition_ffm11.*:*(COMMON)
        *(TFM_SP_FFM11_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_FFM11_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_FFM11_RWZI$$ZI$$Base = ADDR(.TFM_SP_FFM11_RWZI_BSS);
    Image$$ER_TFM_SP_FFM11_RWZI$$ZI$$Limit = ADDR(.TFM_SP_FFM11_RWZI_BSS) + SIZEOF(.TFM_SP_FFM11_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FFM11_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_FFM11 */

#ifdef TFM_ENABLE_SLIH_TEST

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SLIH_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SLIH_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_slih_test.*:*(.data*)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_SLIH_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SLIH_TEST_RWZI);
    Image$$.ER_TFM_SP_SLIH_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SLIH_TEST_RWZI) + SIZEOF(.ER_TFM_SP_SLIH_TEST_RWZI);

    .TFM_SP_SLIH_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_SLIH_TEST_RWZI = .;
        *tfm_*partition_slih_test.*:*(.bss*)
        *tfm_*partition_slih_test.*:*(COMMON)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SLIH_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SLIH_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_SLIH_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_SLIH_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SLIH_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_ENABLE_SLIH_TEST */

#ifdef TFM_ENABLE_FLIH_TEST

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FLIH_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_FLIH_TEST_RWZI : ALIGN(32)
    {
        *tfm_*partition_flih_test.*:*(.data*)
        *(TFM_SP_FLIH_TEST_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_FLIH_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_FLIH_TEST_RWZI);
    Image$$.ER_TFM_SP_FLIH_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_FLIH_TEST_RWZI) + SIZEOF(.ER_TFM_SP_FLIH_TEST_RWZI);

    .TFM_SP_FLIH_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_FLIH_TEST_RWZI = .;
        *tfm_*partition_flih_test.*:*(.bss*)
        *tfm_*partition_flih_test.*:*(COMMON)
        *(TFM_SP_FLIH_TEST_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_FLIH_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_FLIH_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_FLIH_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_FLIH_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_FLIH_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_FLIH_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_FLIH_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_ENABLE_FLIH_TEST */

    /**** APPLICATION RoT DATA end here */

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_APP_RWZI_END$$Base = .;

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    .TFM_SP_SECURE_TEST_PARTITION_LINKER_DATA : ALIGN(32)
    {
        *libc_nano*:*(.data*)
        . = ALIGN(32);
    } > RAM AT> FLASH

    .TFM_SP_SECURE_TEST_PARTITION_LINKER_BSS : ALIGN(32)
    {
        /* FixMe:
         * Test framework use printf to print out test result. Implementation of
         * printf in GCC libc use static data and heap as well. To be able to
         * execute test suites with TFM_LVL=3 this workaround is needed to
         * allocate libc static data and heap within the data section of secure
         * test partition. This can be removed if test service will be executed
         * in privileged mode.
         */
        *libc_nano*:*(.bss*)
        *libc_nano*:*(COMMON)

        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += __heap_size__;
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */

        . = ALIGN(32);
    } > RAM AT> RAM
#else /* TFM_PARTITION_TEST_SECURE_SERVICES */
    .heap : ALIGN(8)
    {
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += __heap_size__;
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */
    } > RAM AT> RAM
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

    .TFM_DATA : ALIGN(4)
    {
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);

    } > RAM AT> FLASH
    Image$$ER_TFM_DATA$$RW$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$RW$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA);

    .TFM_BSS : ALIGN(4)
    {
        __bss_start__ = .;
        __partition_runtime_start__ = .;
        KEEP(*(.bss.part_runtime))
        __partition_runtime_end__ = .;
        . = ALIGN(4);
        __service_runtime_start__ = .;
        KEEP(*(.bss.serv_runtime))
        __service_runtime_end__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM AT> RAM
    Image$$ER_TFM_DATA$$ZI$$Base = ADDR(.TFM_BSS);
    Image$$ER_TFM_DATA$$ZI$$Limit = ADDR(.TFM_BSS) + SIZEOF(.TFM_BSS);
    Image$$ER_PART_RT_POOL$$ZI$$Base = __partition_runtime_start__;
    Image$$ER_PART_RT_POOL$$ZI$$Limit = __partition_runtime_end__;
    Image$$ER_SERV_RT_POOL$$ZI$$Base = __service_runtime_start__;
    Image$$ER_SERV_RT_POOL$$ZI$$Limit = __service_runtime_end__;

    Image$$ER_TFM_DATA$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA) + SIZEOF(.TFM_BSS);

    /*
     * Place the CMSE Veneers (containing the SG instruction) after the code, in a
     * separate 32 bytes aligned region so that the SAU can programmed to just set
     * this region as Non-Secure Callable.
     */
    .gnu.sgstubs : ALIGN(32)
    {
        *(.gnu.sgstubs*)
        . = ALIGN(32);
    } > VENEERS AT> VENEERS
    Load$$LR$$LR_VENEER$$Base = ADDR(.gnu.sgstubs);
    Load$$LR$$LR_VENEER$$Limit = ADDR(.gnu.sgstubs) + SIZEOF(.gnu.sgstubs);

    Load$$LR$$LR_NS_PARTITION$$Base = NS_PARTITION_START;

#ifdef BL2
    Load$$LR$$LR_SECONDARY_PARTITION$$Base = SECONDARY_PARTITION_START;
#endif /* BL2 */

    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);
}
