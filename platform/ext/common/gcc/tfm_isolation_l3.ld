;/*
; * Copyright (c) 2009-2020 Arm Limited
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
#ifdef TFM_ENABLE_IRQ_TEST
        LONG (LOADADDR(.ER_TFM_IRQ_TEST_1_RWZI))
        LONG (ADDR(.ER_TFM_IRQ_TEST_1_RWZI))
        LONG (SIZEOF(.ER_TFM_IRQ_TEST_1_RWZI))
#endif /* TFM_ENABLE_IRQ_TEST */
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
        LONG (ADDR(.TFM_SP_PS_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_PS_LINKER_STACK))
#endif /* TFM_PARTITION_PROTECTED_STORAGE */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
        LONG (ADDR(.TFM_SP_ITS_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_ITS_RWZI_BSS))
        LONG (ADDR(.TFM_SP_ITS_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_ITS_LINKER_STACK))
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */
#ifdef TFM_PARTITION_AUDIT_LOG
        LONG (ADDR(.TFM_SP_AUDIT_LOG_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_AUDIT_LOG_RWZI_BSS))
#endif /* TFM_PARTITION_AUDIT_LOG */
#ifdef TFM_PARTITION_CRYPTO
        LONG (ADDR(.TFM_SP_CRYPTO_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS))
        LONG (ADDR(.TFM_SP_CRYPTO_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_CRYPTO_LINKER_STACK))
#endif /* TFM_PARTITION_CRYPTO */
#ifdef TFM_PARTITION_PLATFORM
        LONG (ADDR(.TFM_SP_PLATFORM_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS))
        LONG (ADDR(.TFM_SP_PLATFORM_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_PLATFORM_LINKER_STACK))
#endif /* TFM_PARTITION_PLATFORM */
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS))
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK))
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (ADDR(.TFM_SP_CORE_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_RWZI_BSS))
        LONG (ADDR(.TFM_SP_CORE_TEST_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_CORE
        LONG (ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_2_RWZI_BSS))
        LONG (ADDR(.TFM_SP_CORE_TEST_2_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_CORE_TEST_2_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_CORE */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS))
        LONG (ADDR(.TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS))
        LONG (ADDR(.TFM_SP_IPC_SERVICE_TEST_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_IPC_SERVICE_TEST_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_PARTITION_TEST_CORE_IPC
        LONG (ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS))
        LONG (ADDR(.TFM_SP_IPC_CLIENT_TEST_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_IPC_CLIENT_TEST_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_CORE_IPC */
#ifdef TFM_ENABLE_IRQ_TEST
        LONG (ADDR(.TFM_IRQ_TEST_1_RWZI_BSS))
        LONG (SIZEOF(.TFM_IRQ_TEST_1_RWZI_BSS))
        LONG (ADDR(.TFM_IRQ_TEST_1_LINKER_STACK))
        LONG (SIZEOF(.TFM_IRQ_TEST_1_LINKER_STACK))
#endif /* TFM_ENABLE_IRQ_TEST */
#ifdef TFM_PARTITION_TEST_PS
        LONG (ADDR(.TFM_SP_PS_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS))
        LONG (ADDR(.TFM_SP_PS_TEST_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_PS_TEST_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_PS */
#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
        LONG (ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS))
        LONG (ADDR(.TFM_SP_SECURE_CLIENT_2_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_SECURE_CLIENT_2_LINKER_STACK))
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */
        __zero_table_end__ = .;
    } > FLASH

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_RO_START$$Base = .;

    /**** PSA RoT RO CODE + RO-data starts here */
#ifdef TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    .TFM_SP_ITS_RO : ALIGN(32)
    {
        *tfm_partition_its.*:*(.text*)
        *tfm_partition_its.*:*(.rodata*)
        *(TFM_SP_ITS_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_ITS_RO$$Base = ADDR(.TFM_SP_ITS_RO);
    Image$$TFM_SP_ITS_RO$$Limit = ADDR(.TFM_SP_ITS_RO) + SIZEOF(.TFM_SP_ITS_RO);
#endif /* TFM_PARTITION_INTERNAL_TRUSTED_STORAGE */

#ifdef TFM_PARTITION_AUDIT_LOG
    .TFM_SP_AUDIT_LOG_RO : ALIGN(32)
    {
        *tfm_partition_audit.*:*(.text*)
        *tfm_partition_audit.*:*(.rodata*)
        *(TFM_SP_AUDIT_LOG_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_AUDIT_LOG_RO$$Base = ADDR(.TFM_SP_AUDIT_LOG_RO);
    Image$$TFM_SP_AUDIT_LOG_RO$$Limit = ADDR(.TFM_SP_AUDIT_LOG_RO) + SIZEOF(.TFM_SP_AUDIT_LOG_RO);
#endif /* TFM_PARTITION_AUDIT_LOG */

#ifdef TFM_PARTITION_CRYPTO
    .TFM_SP_CRYPTO_RO : ALIGN(32)
    {
        *tfm_partition_crypto.*:*(.text*)
        *tfm_partition_crypto.*:*(.rodata*)
        *(TFM_SP_CRYPTO_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CRYPTO_RO$$Base = ADDR(.TFM_SP_CRYPTO_RO);
    Image$$TFM_SP_CRYPTO_RO$$Limit = ADDR(.TFM_SP_CRYPTO_RO) + SIZEOF(.TFM_SP_CRYPTO_RO);
#endif /* TFM_PARTITION_CRYPTO */

#ifdef TFM_PARTITION_PLATFORM
    .TFM_SP_PLATFORM_RO : ALIGN(32)
    {
        *tfm_partition_platform.*:*(.text*)
        *tfm_partition_platform.*:*(.rodata*)
        *(TFM_SP_PLATFORM_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PLATFORM_RO$$Base = ADDR(.TFM_SP_PLATFORM_RO);
    Image$$TFM_SP_PLATFORM_RO$$Limit = ADDR(.TFM_SP_PLATFORM_RO) + SIZEOF(.TFM_SP_PLATFORM_RO);
#endif /* TFM_PARTITION_PLATFORM */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    .TFM_SP_INITIAL_ATTESTATION_RO : ALIGN(32)
    {
        *tfm_partition_attestation.*:*(.text*)
        *tfm_partition_attestation.*:*(.rodata*)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO);
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RO);
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_TEST_CORE
    .TFM_SP_CORE_TEST_RO : ALIGN(32)
    {
        *tfm_ss_core_test.*(.text*)
        *tfm_ss_core_test.*:*(.rodata*)
        *(TFM_SP_CORE_TEST_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CORE_TEST_RO$$Base = ADDR(.TFM_SP_CORE_TEST_RO);
    Image$$TFM_SP_CORE_TEST_RO$$Limit = ADDR(.TFM_SP_CORE_TEST_RO) + SIZEOF(.TFM_SP_CORE_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    .TFM_SP_SECURE_TEST_PARTITION_RO : ALIGN(32)
    {
        *tfm_secure_client_service.*(.text*)
        *tfm_secure_client_service.*:*(.rodata*)
        *test_framework*(.text*)
        *test_framework*:*(.rodata*)
        *uart_stdout.*(.text*)
        *uart_stdout.*:*(.rodata*)
        *Driver_USART.*(.text*)
        *Driver_USART.*:*(.rodata*)
        *arm_uart_drv.*(.text*)
        *arm_uart_drv.*:*(.rodata*)
        *uart_pl011_drv.*(.text*)
        *uart_pl011_drv.*:*(.rodata*)
        *uart_cmsdk_drv*(.text*)
        *uart_cmsdk_drv*:*(.rodata*)
        *secure_suites.*(.text*)
        *secure_suites.*:*(.rodata*)
        *attestation_s_interface_testsuite.*(.text*)
        *attestation_s_interface_testsuite.*:*(.rodata*)
        *(TFM_SP_SECURE_TEST_PARTITION_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO);
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RO);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    .TFM_SP_IPC_SERVICE_TEST_RO : ALIGN(32)
    {
        *tfm_ipc_service_test.*(.text*)
        *tfm_ipc_service_test.*:*(.rodata*)
        *(TFM_SP_IPC_SERVICE_TEST_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_IPC_SERVICE_TEST_RO$$Base = ADDR(.TFM_SP_IPC_SERVICE_TEST_RO);
    Image$$TFM_SP_IPC_SERVICE_TEST_RO$$Limit = ADDR(.TFM_SP_IPC_SERVICE_TEST_RO) + SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_PARTITION_TEST_PS
    .TFM_SP_PS_TEST_RO : ALIGN(32)
    {
        *tfm_ps_test_service.*(.text*)
        *tfm_ps_test_service.*:*(.rodata*)
        *(TFM_SP_PS_TEST_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PS_TEST_RO$$Base = ADDR(.TFM_SP_PS_TEST_RO);
    Image$$TFM_SP_PS_TEST_RO$$Limit = ADDR(.TFM_SP_PS_TEST_RO) + SIZEOF(.TFM_SP_PS_TEST_RO);
#endif /* TFM_PARTITION_TEST_PS */

    /**** PSA RoT RO CODE + data ends here */

    /**** APPLICATION RoT RO CODE + data starts here */
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    .TFM_SP_PS_RO : ALIGN(32)
    {
        *tfm_partition_ps.*:*(.text*)
        *tfm_partition_ps.*:*(.rodata*)
        *test_ps_nv_counters.*(.text*)
        *test_ps_nv_counters.*:*(.rodata*)
        *(TFM_SP_PS_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PS_CODE$$Base = ADDR(.TFM_SP_PS_RO);
    Image$$TFM_SP_PS_CODE$$Limit = ADDR(.TFM_SP_PS_RO) + SIZEOF(.TFM_SP_PS_RO);
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

#ifdef TFM_PARTITION_TEST_CORE
    .TFM_SP_CORE_TEST_2_RO : ALIGN(32)
    {
        *tfm_ss_core_test_2.*(.text*)
        *tfm_ss_core_test_2.*:*(.rodata*)
        *(TFM_SP_CORE_TEST_2_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CORE_TEST_2_CODE$$Base = ADDR(.TFM_SP_CORE_TEST_2_RO);
    Image$$TFM_SP_CORE_TEST_2_CODE$$Limit = ADDR(.TFM_SP_CORE_TEST_2_RO) + SIZEOF(.TFM_SP_CORE_TEST_2_RO);
#endif /* TFM_PARTITION_TEST_CORE */

#ifdef TFM_PARTITION_TEST_CORE_IPC
    .TFM_SP_IPC_CLIENT_TEST_RO : ALIGN(32)
    {
        *tfm_ipc_client_test.*(.text*)
        *tfm_ipc_client_test.*:*(.rodata*)
        *(TFM_SP_IPC_CLIENT_TEST_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_IPC_CLIENT_TEST_CODE$$Base = ADDR(.TFM_SP_IPC_CLIENT_TEST_RO);
    Image$$TFM_SP_IPC_CLIENT_TEST_CODE$$Limit = ADDR(.TFM_SP_IPC_CLIENT_TEST_RO) + SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RO);
#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST
    .TFM_IRQ_TEST_1_RO : ALIGN(32)
    {
        *tfm_irq_test_service_1.*(.text*)
        *tfm_irq_test_service_1.*:*(.rodata*)
        *timer_cmsdk*(.text*)
        *timer_cmsdk*:*(.rodata*)
        *(TFM_IRQ_TEST_1_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_IRQ_TEST_1_CODE$$Base = ADDR(.TFM_IRQ_TEST_1_RO);
    Image$$TFM_IRQ_TEST_1_CODE$$Limit = ADDR(.TFM_IRQ_TEST_1_RO) + SIZEOF(.TFM_IRQ_TEST_1_RO);
#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES
    .TFM_SP_SECURE_CLIENT_2_RO : ALIGN(32)
    {
        *tfm_secure_client_2.*(.text*)
        *tfm_secure_client_2.*:*(.rodata*)
        *(TFM_SP_SECURE_CLIENT_2_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RO);
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RO) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RO);
#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

    /**** APPLICATION RoT RO CODE + data ends here */

    /**** Unprivileged Secure code starts here */
    .ER_UNPRIV_CODE : ALIGN(32)
    {
        *platform_retarget_dev.*(.text*)
        *platform_retarget_dev.*(.rodata*)
        *device_definition.*(.text*)
        *device_definition.*(.rodata*)
        *(SFN)
        *armlib*:*(.text*)
        *armlib*:*(.rodata*)
        *libtfm_sprt*:*(.text*)
        *libtfm_sprt*:*(.rodata*)
        *psa_client.*(.text*)              /* NXP */
        *psa_client.*(.rodata*)
        *psa_service.*(.text*)             /* NXP */
        *psa_service.*(.rodata*)
        *psa_lifecycle.*(.text*)             /* NXP */
        *psa_lifecycle.*(.rodata*)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_UNPRIV_CODE$$RO$$Base = ADDR(.ER_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE$$RO$$Limit = ADDR(.ER_UNPRIV_CODE) + SIZEOF(.ER_UNPRIV_CODE);

    .ER_TFM_CODE : ALIGN(4)
    {
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
    } > FLASH

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
        *tfm_partition_its.*:*(.data*)
        *(TFM_SP_ITS_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_ITS_RWZI);
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_ITS_RWZI) + SIZEOF(.ER_TFM_SP_ITS_RWZI);

    .TFM_SP_ITS_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_ITS_RWZI = .;
        *tfm_partition_its.*:*(.bss*)
        *tfm_partition_its.*:*(COMMON)
        *(TFM_SP_ITS_ATTR_ZI)
        . += (. - start_of_TFM_SP_ITS_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Base = ADDR(.TFM_SP_ITS_RWZI_BSS);
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_ITS_RWZI_BSS) + SIZEOF(.TFM_SP_ITS_RWZI_BSS);


    .TFM_SP_ITS_LINKER_STACK : ALIGN(128)
    {
        . += 0x680;
    } > RAM
    Image$$TFM_SP_ITS_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_ITS_LINKER_STACK);
    Image$$TFM_SP_ITS_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_ITS_LINKER_STACK) + SIZEOF(.TFM_SP_ITS_LINKER_STACK);


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
        *tfm_partition_audit.*:*(.data*)
        *(TFM_SP_AUDIT_LOG_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_AUDIT_LOG_RWZI);
    Image$$ER_TFM_SP_AUDIT_LOG_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_AUDIT_LOG_RWZI) + SIZEOF(.ER_TFM_SP_AUDIT_LOG_RWZI);

    .TFM_SP_AUDIT_LOG_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_AUDIT_LOG_RWZI = .;
        *tfm_partition_audit.*:*(.bss*)
        *tfm_partition_audit.*:*(COMMON)
        *(TFM_SP_AUDIT_LOG_ATTR_ZI)
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
        *tfm_partition_crypto.*:*(.data*)
        *(TFM_SP_CRYPTO_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CRYPTO_RWZI);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CRYPTO_RWZI) + SIZEOF(.ER_TFM_SP_CRYPTO_RWZI);

    .TFM_SP_CRYPTO_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CRYPTO_RWZI = .;
        *tfm_partition_crypto.*:*(.bss*)
        *tfm_partition_crypto.*:*(COMMON)
        *(TFM_SP_CRYPTO_ATTR_ZI)
        . += (. - start_of_TFM_SP_CRYPTO_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_RWZI_BSS);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_RWZI_BSS) + SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS);


    .TFM_SP_CRYPTO_LINKER_STACK : ALIGN(128)
    {
        . += 0x2000;
    } > RAM
    Image$$TFM_SP_CRYPTO_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_LINKER_STACK);
    Image$$TFM_SP_CRYPTO_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER_STACK) + SIZEOF(.TFM_SP_CRYPTO_LINKER_STACK);


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
        *tfm_partition_platform.*:*(.data*)
        *(TFM_SP_PLATFORM_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PLATFORM_RWZI);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PLATFORM_RWZI) + SIZEOF(.ER_TFM_SP_PLATFORM_RWZI);

    .TFM_SP_PLATFORM_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PLATFORM_RWZI = .;
        *tfm_partition_platform.*:*(.bss*)
        *tfm_partition_platform.*:*(COMMON)
        *(TFM_SP_PLATFORM_ATTR_ZI)
        . += (. - start_of_TFM_SP_PLATFORM_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_RWZI_BSS);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_RWZI_BSS) + SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS);


    .TFM_SP_PLATFORM_LINKER_STACK : ALIGN(128)
    {
        . += 0x0500;
    } > RAM
    Image$$TFM_SP_PLATFORM_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_LINKER_STACK);
    Image$$TFM_SP_PLATFORM_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER_STACK) + SIZEOF(.TFM_SP_PLATFORM_LINKER_STACK);


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
        *tfm_partition_attestation.*:*(.data*)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI) + SIZEOF(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);

    .TFM_SP_INITIAL_ATTESTATION_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_INITIAL_ATTESTATION_RWZI = .;
        *tfm_partition_attestation.*:*(.bss*)
        *tfm_partition_attestation.*:*(COMMON)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_ZI)
        . += (. - start_of_TFM_SP_INITIAL_ATTESTATION_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);


    .TFM_SP_INITIAL_ATTESTATION_LINKER_STACK : ALIGN(128)
    {
        . += 0x0A80;
    } > RAM
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK);


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
        *tfm_ss_core_test.*(.data*)
        *(TFM_SP_CORE_TEST_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CORE_TEST_RWZI);
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CORE_TEST_RWZI) + SIZEOF(.ER_TFM_SP_CORE_TEST_RWZI);

    .TFM_SP_CORE_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CORE_TEST_RWZI = .;
        *tfm_ss_core_test.*(.bss*)
        *tfm_ss_core_test.*(COMMON)
        *(TFM_SP_CORE_TEST_ATTR_ZI)
        . += (. - start_of_TFM_SP_CORE_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_CORE_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_CORE_TEST_RWZI_BSS);


    .TFM_SP_CORE_TEST_LINKER_STACK : ALIGN(128)
    {
        . += 0x0380;
    } > RAM
    Image$$TFM_SP_CORE_TEST_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_LINKER_STACK);
    Image$$TFM_SP_CORE_TEST_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_LINKER_STACK) + SIZEOF(.TFM_SP_CORE_TEST_LINKER_STACK);


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
        *tfm_secure_client_service.*(.data*)
        *test_framework*(.data*)
        *uart_stdout.*(.data*)
        *Driver_USART.*(.data*)
        *arm_uart_drv.*(.data*)
        *uart_pl011_drv.*(.data*)
        *uart_cmsdk_drv*(.data*)
        *secure_suites.*(.data*)
        *attestation_s_interface_testsuite.*(.data*)
        *(TFM_SP_SECURE_TEST_PARTITION_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);

    .TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI = .;
        *tfm_secure_client_service.*(.bss*)
        *tfm_secure_client_service.*(COMMON)
        *test_framework*(.bss*)
        *test_framework*(COMMON)
        *uart_stdout.*(.bss*)
        *uart_stdout.*(COMMON)
        *Driver_USART.*(.bss*)
        *Driver_USART.*(COMMON)
        *arm_uart_drv.*(.bss*)
        *arm_uart_drv.*(COMMON)
        *uart_pl011_drv.*(.bss*)
        *uart_pl011_drv.*(COMMON)
        *uart_cmsdk_drv*(.bss*)
        *uart_cmsdk_drv*(COMMON)
        *secure_suites.*(.bss*)
        *secure_suites.*(COMMON)
        *attestation_s_interface_testsuite.*(.bss*)
        *attestation_s_interface_testsuite.*(COMMON)
        *(TFM_SP_SECURE_TEST_PARTITION_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);


    .TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK : ALIGN(128)
    {
        . += 0x0D00;
    } > RAM
    Image$$TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK);
    Image$$TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_LINKER_STACK);


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
        *tfm_ipc_service_test.*(.data*)
        *(TFM_SP_IPC_SERVICE_TEST_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI);
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI) + SIZEOF(.ER_TFM_SP_IPC_SERVICE_TEST_RWZI);

    .TFM_SP_IPC_SERVICE_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_IPC_SERVICE_TEST_RWZI = .;
        *tfm_ipc_service_test.*(.bss*)
        *tfm_ipc_service_test.*(COMMON)
        *(TFM_SP_IPC_SERVICE_TEST_ATTR_ZI)
        . += (. - start_of_TFM_SP_IPC_SERVICE_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_IPC_SERVICE_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_IPC_SERVICE_TEST_RWZI_BSS);


    .TFM_SP_IPC_SERVICE_TEST_LINKER_STACK : ALIGN(128)
    {
        . += 0x0220;
    } > RAM
    Image$$TFM_SP_IPC_SERVICE_TEST_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_IPC_SERVICE_TEST_LINKER_STACK);
    Image$$TFM_SP_IPC_SERVICE_TEST_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_IPC_SERVICE_TEST_LINKER_STACK) + SIZEOF(.TFM_SP_IPC_SERVICE_TEST_LINKER_STACK);


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
        *tfm_ps_test_service.*(.data*)
        *(TFM_SP_PS_TEST_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_TEST_RWZI);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_TEST_RWZI) + SIZEOF(.ER_TFM_SP_PS_TEST_RWZI);

    .TFM_SP_PS_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PS_TEST_RWZI = .;
        *tfm_ps_test_service.*(.bss*)
        *tfm_ps_test_service.*(COMMON)
        *(TFM_SP_PS_TEST_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS);


    .TFM_SP_PS_TEST_LINKER_STACK : ALIGN(128)
    {
        . += 0x500;
    } > RAM
    Image$$TFM_SP_PS_TEST_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_PS_TEST_LINKER_STACK);
    Image$$TFM_SP_PS_TEST_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_PS_TEST_LINKER_STACK) + SIZEOF(.TFM_SP_PS_TEST_LINKER_STACK);


    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_PS_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_PS */

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
        *tfm_partition_ps.*:*(.data*)
        *test_ps_nv_counters.*(.data*)
        *(TFM_SP_PS_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_RWZI);
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_RWZI) + SIZEOF(.ER_TFM_SP_PS_RWZI);

    .TFM_SP_PS_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PS_RWZI = .;
        *tfm_partition_ps.*:*(.bss*)
        *tfm_partition_ps.*:*(COMMON)
        *test_ps_nv_counters.*(.bss*)
        *test_ps_nv_counters.*(COMMON)
        *(TFM_SP_PS_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_RWZI_BSS);
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_RWZI_BSS) + SIZEOF(.TFM_SP_PS_RWZI_BSS);


    .TFM_SP_PS_LINKER_STACK : ALIGN(128)
    {
        . += 0x800;
    } > RAM
    Image$$TFM_SP_PS_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_PS_LINKER_STACK);
    Image$$TFM_SP_PS_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_PS_LINKER_STACK) + SIZEOF(.TFM_SP_PS_LINKER_STACK);


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
        *tfm_ss_core_test_2.*(.data*)
        *(TFM_SP_CORE_TEST_2_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_CORE_TEST_2_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CORE_TEST_2_RWZI);
    Image$$.ER_TFM_SP_CORE_TEST_2_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CORE_TEST_2_RWZI) + SIZEOF(.ER_TFM_SP_CORE_TEST_2_RWZI);

    .TFM_SP_CORE_TEST_2_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CORE_TEST_2_RWZI = .;
        *tfm_ss_core_test_2.*(.bss*)
        *tfm_ss_core_test_2.*(COMMON)
        *(TFM_SP_CORE_TEST_2_ATTR_ZI)
        . += (. - start_of_TFM_SP_CORE_TEST_2_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CORE_TEST_2_RWZI$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS);
    Image$$ER_TFM_SP_CORE_TEST_2_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_2_RWZI_BSS) + SIZEOF(.TFM_SP_CORE_TEST_2_RWZI_BSS);


    .TFM_SP_CORE_TEST_2_LINKER_STACK : ALIGN(128)
    {
        . += 0x0280;
    } > RAM
    Image$$TFM_SP_CORE_TEST_2_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_CORE_TEST_2_LINKER_STACK);
    Image$$TFM_SP_CORE_TEST_2_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_CORE_TEST_2_LINKER_STACK) + SIZEOF(.TFM_SP_CORE_TEST_2_LINKER_STACK);


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
        *tfm_ipc_client_test.*(.data*)
        *(TFM_SP_IPC_CLIENT_TEST_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI);
    Image$$.ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI) + SIZEOF(.ER_TFM_SP_IPC_CLIENT_TEST_RWZI);

    .TFM_SP_IPC_CLIENT_TEST_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_IPC_CLIENT_TEST_RWZI = .;
        *tfm_ipc_client_test.*(.bss*)
        *tfm_ipc_client_test.*(COMMON)
        *(TFM_SP_IPC_CLIENT_TEST_ATTR_ZI)
        . += (. - start_of_TFM_SP_IPC_CLIENT_TEST_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_IPC_CLIENT_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_IPC_CLIENT_TEST_RWZI_BSS);


    .TFM_SP_IPC_CLIENT_TEST_LINKER_STACK : ALIGN(128)
    {
        . += 0x0300;
    } > RAM
    Image$$TFM_SP_IPC_CLIENT_TEST_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_IPC_CLIENT_TEST_LINKER_STACK);
    Image$$TFM_SP_IPC_CLIENT_TEST_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_IPC_CLIENT_TEST_LINKER_STACK) + SIZEOF(.TFM_SP_IPC_CLIENT_TEST_LINKER_STACK);


    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_IPC_CLIENT_TEST_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_CORE_IPC */

#ifdef TFM_ENABLE_IRQ_TEST

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_IRQ_TEST_1_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_IRQ_TEST_1_RWZI : ALIGN(32)
    {
        *tfm_irq_test_service_1.*(.data*)
        *timer_cmsdk*(.data*)
        *(TFM_IRQ_TEST_1_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_IRQ_TEST_1_RWZI$$RW$$Base = ADDR(.ER_TFM_IRQ_TEST_1_RWZI);
    Image$$.ER_TFM_IRQ_TEST_1_RWZI$$RW$$Limit = ADDR(.ER_TFM_IRQ_TEST_1_RWZI) + SIZEOF(.ER_TFM_IRQ_TEST_1_RWZI);

    .TFM_IRQ_TEST_1_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_IRQ_TEST_1_RWZI = .;
        *tfm_irq_test_service_1.*(.bss*)
        *tfm_irq_test_service_1.*(COMMON)
        *timer_cmsdk*(.bss*)
        *timer_cmsdk*(COMMON)
        *(TFM_IRQ_TEST_1_ATTR_ZI)
        . += (. - start_of_TFM_IRQ_TEST_1_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_IRQ_TEST_1_RWZI$$ZI$$Base = ADDR(.TFM_IRQ_TEST_1_RWZI_BSS);
    Image$$ER_TFM_IRQ_TEST_1_RWZI$$ZI$$Limit = ADDR(.TFM_IRQ_TEST_1_RWZI_BSS) + SIZEOF(.TFM_IRQ_TEST_1_RWZI_BSS);


    .TFM_IRQ_TEST_1_LINKER_STACK : ALIGN(128)
    {
        . += 0x0400;
    } > RAM
    Image$$TFM_IRQ_TEST_1_LINKER_STACK$$ZI$$Base = ADDR(.TFM_IRQ_TEST_1_LINKER_STACK);
    Image$$TFM_IRQ_TEST_1_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_IRQ_TEST_1_LINKER_STACK) + SIZEOF(.TFM_IRQ_TEST_1_LINKER_STACK);


    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_IRQ_TEST_1_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_ENABLE_IRQ_TEST */

#ifdef TFM_PARTITION_TEST_SECURE_SERVICES

    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SECURE_CLIENT_2_RWZI : ALIGN(32)
    {
        *tfm_secure_client_2.*(.data*)
        *(TFM_SP_SECURE_CLIENT_2_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);

    .TFM_SP_SECURE_CLIENT_2_RWZI_BSS : ALIGN(32)
    {
        start_of_TFM_SP_SECURE_CLIENT_2_RWZI = .;
        *tfm_secure_client_2.*(.bss*)
        *tfm_secure_client_2.*(COMMON)
        *(TFM_SP_SECURE_CLIENT_2_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_CLIENT_2_RWZI) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);


    .TFM_SP_SECURE_CLIENT_2_LINKER_STACK : ALIGN(128)
    {
        . += 0x300;
    } > RAM
    Image$$TFM_SP_SECURE_CLIENT_2_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_LINKER_STACK);
    Image$$TFM_SP_SECURE_CLIENT_2_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_LINKER_STACK) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_LINKER_STACK);


    /* Position tag */
    . = ALIGN(32);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_END$$Base = .;

#endif /* TFM_PARTITION_TEST_SECURE_SERVICES */

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
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM AT> RAM
    Image$$ER_TFM_DATA$$ZI$$Base = ADDR(.TFM_BSS);
    Image$$ER_TFM_DATA$$ZI$$Limit = ADDR(.TFM_BSS) + SIZEOF(.TFM_BSS);

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
