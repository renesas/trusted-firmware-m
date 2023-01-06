;/*
; * Copyright (c) 2009-2022 Arm Limited
; * Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
; * or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
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

/***********  WARNING: This is an auto-generated file. Do not edit!  ***********/

/* Linker script to configure memory regions. */
/* This file will be run trough the pre-processor. */

#include "region_defs.h"

/* Include file with definitions for section alignments.
 * Note: it should be included after region_defs.h to let platform define
 * default values if needed. */
#include "tfm_s_linker_alignments.h"

MEMORY
{
  FLASH    (rx)  : ORIGIN = S_CODE_START, LENGTH = S_CODE_SIZE
  RAM      (rwx) : ORIGIN = S_DATA_START, LENGTH = S_DATA_SIZE
#if defined(S_CODE_SRAM_ALIAS_BASE)
  CODE_RAM (rwx) : ORIGIN = S_CODE_SRAM_ALIAS_BASE, LENGTH = TOTAL_CODE_SRAM_SIZE
#endif
}

__msp_stack_size__ = S_MSP_STACK_SIZE;

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a libc_nano.a)

ENTRY(Reset_Handler)

SECTIONS
{
    /* Start address of the code. */
    Image$$PT_RO_START$$Base = ADDR(.TFM_VECTORS);

    .TFM_VECTORS : ALIGN(4)
    {
        __vectors_start__ = .;
        KEEP(*(.vectors))
        __vectors_end__ = .;
        . = ALIGN(4);
    } > FLASH

#ifdef CONFIG_TFM_USE_TRUSTZONE
    ASSERT(. <= ADDR(.TFM_VECTORS) + S_CODE_VECTOR_TABLE_SIZE, ".TFM_VECTORS section size overflow.")
    . = ADDR(.TFM_VECTORS) + S_CODE_VECTOR_TABLE_SIZE;
    /*
     * Place the CMSE Veneers (containing the SG instruction) after the code, in
     * a separate at least 32 bytes aligned region so that the SAU can
     * programmed to just set this region as Non-Secure Callable.
     */
    .gnu.sgstubs ALIGN(TFM_LINKER_VENEERS_ALIGNMENT) :
    {
        *(.gnu.sgstubs*)
        . = ALIGN(TFM_LINKER_VENEERS_ALIGNMENT);
    } > FLASH
    Image$$ER_VENEER$$Base = ADDR(.gnu.sgstubs);
    Image$$VENEER_ALIGN$$Limit = ADDR(.gnu.sgstubs) + SIZEOF(.gnu.sgstubs);
#endif

    .copy.table : ALIGN(4)
    {
        __copy_table_start__ = .;
#ifdef RAM_VECTORS_SUPPORT
        /* Copy interrupt vectors from flash to RAM */
        LONG (__vectors_start__)                            /* From */
        LONG (__ram_vectors_start__)                        /* To   */
        LONG ((__vectors_end__ - __vectors_start__) / 4)    /* Size */
#endif
        LONG (LOADADDR(.TFM_DATA))
        LONG (ADDR(.TFM_DATA))
        LONG (SIZEOF(.TFM_DATA) / 4)
        LONG (LOADADDR(.ER_TFM_SP_PS_RWZI))
        LONG (ADDR(.ER_TFM_SP_PS_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PS_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_ITS_RWZI))
        LONG (ADDR(.ER_TFM_SP_ITS_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_ITS_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_CRYPTO_RWZI))
        LONG (ADDR(.ER_TFM_SP_CRYPTO_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_CRYPTO_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_PLATFORM_RWZI))
        LONG (ADDR(.ER_TFM_SP_PLATFORM_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PLATFORM_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI))
        LONG (ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_FWU_RWZI))
        LONG (ADDR(.ER_TFM_SP_FWU_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_FWU_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI))
        LONG (ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_PS_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_PS_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_PS_TEST_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI))
        LONG (ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SECURE_CLIENT_2_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SP_SLIH_TEST_RWZI))
        LONG (ADDR(.ER_TFM_SP_SLIH_TEST_RWZI))
        LONG (SIZEOF(.ER_TFM_SP_SLIH_TEST_RWZI) / 4)
        LONG (LOADADDR(.ER_TFM_SFN_PARTITION1_RWZI))
        LONG (ADDR(.ER_TFM_SFN_PARTITION1_RWZI))
        LONG (SIZEOF(.ER_TFM_SFN_PARTITION1_RWZI) / 4)
#if defined(S_CODE_SRAM_ALIAS_BASE)
        LONG (LOADADDR(.ER_EFLASH_DRIVER_RO))
        LONG (ADDR(.ER_EFLASH_DRIVER_RO))
        LONG (SIZEOF(.ER_EFLASH_DRIVER_RO) / 4)
#endif
        __copy_table_end__ = .;
    } > FLASH

    .zero.table : ALIGN(4)
    {
        __zero_table_start__ = .;
#if defined(CONFIG_TFM_PARTITION_META)
        LONG (ADDR(.TFM_SP_META_PTR))
        LONG (SIZEOF(.TFM_SP_META_PTR) / 4)
#endif
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS) / 4)
        LONG (ADDR(.TFM_SP_PS_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PS_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_ITS_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_ITS_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_CRYPTO_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_PLATFORM_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_FWU_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_FWU_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_PS_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS))
        LONG (SIZEOF(.TFM_SP_SLIH_TEST_RWZI_BSS) / 4)
        LONG (ADDR(.TFM_SFN_PARTITION1_RWZI_BSS))
        LONG (SIZEOF(.TFM_SFN_PARTITION1_RWZI_BSS) / 4)
        __zero_table_end__ = .;
    } > FLASH

    /**** Section for holding partition RO load data */
    /*
     * Sort the partition info by priority to guarantee the initing order.
     * The first loaded partition will be inited at last in SFN model.
     */
    .TFM_SP_LOAD_LIST : ALIGN(4)
    {
       KEEP(*(.part_load_priority_lowest))
       KEEP(*(.part_load_priority_low))
       KEEP(*(.part_load_priority_normal))
       KEEP(*(.part_load_priority_high))
    } > FLASH
    Image$$TFM_SP_LOAD_LIST$$RO$$Base = ADDR(.TFM_SP_LOAD_LIST);
    Image$$TFM_SP_LOAD_LIST$$RO$$Limit = ADDR(.TFM_SP_LOAD_LIST) + SIZEOF(.TFM_SP_LOAD_LIST);

    /**** PSA RoT RO CODE + RO-data starts here */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    .TFM_SP_ITS_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_its.*:*(.text*)
        *tfm_*partition_its.*:*(.rodata*)
        *(TFM_SP_ITS_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_ITS_RO$$Base = ADDR(.TFM_SP_ITS_RO);
    Image$$TFM_SP_ITS_RO$$Limit = ADDR(.TFM_SP_ITS_RO) + SIZEOF(.TFM_SP_ITS_RO);

    .TFM_SP_CRYPTO_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_crypto.*:*(.text*)
        *tfm_*partition_crypto.*:*(.rodata*)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_CRYPTO_RO$$Base = ADDR(.TFM_SP_CRYPTO_RO);
    Image$$TFM_SP_CRYPTO_RO$$Limit = ADDR(.TFM_SP_CRYPTO_RO) + SIZEOF(.TFM_SP_CRYPTO_RO);

    .TFM_SP_PLATFORM_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_platform.*:*(.text*)
        *tfm_*partition_platform.*:*(.rodata*)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_PLATFORM_RO$$Base = ADDR(.TFM_SP_PLATFORM_RO);
    Image$$TFM_SP_PLATFORM_RO$$Limit = ADDR(.TFM_SP_PLATFORM_RO) + SIZEOF(.TFM_SP_PLATFORM_RO);

    .TFM_SP_INITIAL_ATTESTATION_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_attestation.*:*(.text*)
        *tfm_*partition_attestation.*:*(.rodata*)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO);
    Image$$TFM_SP_INITIAL_ATTESTATION_RO$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RO) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RO);

    .TFM_SP_FWU_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_fwu*:*(.text*)
        *tfm_*partition_fwu*:*(.rodata*)
        *(TFM_SP_FWU_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_FWU_RO$$Base = ADDR(.TFM_SP_FWU_RO);
    Image$$TFM_SP_FWU_RO$$Limit = ADDR(.TFM_SP_FWU_RO) + SIZEOF(.TFM_SP_FWU_RO);

    .TFM_SP_SECURE_TEST_PARTITION_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_secure_client_service.*:*(.text*)
        *tfm_*partition_secure_client_service.*:*(.rodata*)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO);
    Image$$TFM_SP_SECURE_TEST_PARTITION_RO$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RO) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RO);

    .TFM_SP_PS_TEST_RO : ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_ps_test.*:*(.text*)
        *tfm_*partition_ps_test.*:*(.rodata*)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_PS_TEST_RO$$Base = ADDR(.TFM_SP_PS_TEST_RO);
    Image$$TFM_SP_PS_TEST_RO$$Limit = ADDR(.TFM_SP_PS_TEST_RO) + SIZEOF(.TFM_SP_PS_TEST_RO);

    /**** PSA RoT RO CODE + data ends here */

    /**** APPLICATION RoT RO CODE + data starts here */
    .TFM_SP_PS_RO : ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_ps.*:*(.text*)
        *tfm_*partition_ps.*:*(.rodata*)
        *(TFM_SP_PS_APP-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_PS_CODE$$Base = ADDR(.TFM_SP_PS_RO);
    Image$$TFM_SP_PS_CODE$$Limit = ADDR(.TFM_SP_PS_RO) + SIZEOF(.TFM_SP_PS_RO);

    .TFM_SP_SECURE_CLIENT_2_RO : ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_secure_client_2.*:*(.text*)
        *tfm_*partition_secure_client_2.*:*(.rodata*)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RO);
    Image$$TFM_SP_SECURE_CLIENT_2_CODE$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RO) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RO);

    .TFM_SP_SLIH_TEST_RO : ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_slih_test.*:*(.text*)
        *tfm_*partition_slih_test.*:*(.rodata*)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SP_SLIH_TEST_CODE$$Base = ADDR(.TFM_SP_SLIH_TEST_RO);
    Image$$TFM_SP_SLIH_TEST_CODE$$Limit = ADDR(.TFM_SP_SLIH_TEST_RO) + SIZEOF(.TFM_SP_SLIH_TEST_RO);

    .TFM_SFN_PARTITION1_RO : ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT)
    {
        *tfm_*partition_sfn_backend_test.*:*(.text*)
        *tfm_*partition_sfn_backend_test.*:*(.rodata*)
        *(TFM_SFN_PARTITION1_APP-ROT_ATTR_FN)
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_SFN_PARTITION1_CODE$$Base = ADDR(.TFM_SFN_PARTITION1_RO);
    Image$$TFM_SFN_PARTITION1_CODE$$Limit = ADDR(.TFM_SFN_PARTITION1_RO) + SIZEOF(.TFM_SFN_PARTITION1_RO);

    /**** APPLICATION RoT RO CODE + data ends here */

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

    .ER_TFM_CODE : ALIGN(4)
    {
        *startup*(.text*)
        *libplatform_s*:*(.text*)
        *libtfm_spm*:*(.text*)

        *libplatform_s*:*(.rodata*)
        *libtfm_spm*:*(.rodata*)
    } > FLASH

    /**** Unprivileged Secure code starts here */
    .ER_UNPRIV_CODE : ALIGN(TFM_LINKER_UNPRIV_CODE_ALIGNMENT)
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

        . = ALIGN(TFM_LINKER_UNPRIV_CODE_ALIGNMENT);
    } > FLASH
    Image$$TFM_UNPRIV_CODE$$RO$$Base = ADDR(.ER_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE$$RO$$Limit = ADDR(.ER_UNPRIV_CODE) + SIZEOF(.ER_UNPRIV_CODE);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PT_RO_ALIGNMENT);
    Image$$PT_RO_END$$Base = .;

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
    . = ALIGN(TFM_LINKER_PT_PRIV_RWZI_ALIGNMENT);
    Image$$PT_PRIV_RWZI_START$$Base = .;

    /* shared_data and msp_stack are overlapping on purpose when
     * msp_stack is extended until the beginning of RAM, when shared_date
     * was read out by partitions
     */
    .tfm_bl2_shared_data : ALIGN(TFM_LINKER_BL2_SHARED_DATA_ALIGNMENT)
    {
        . += BOOT_TFM_SHARED_DATA_SIZE;
    } > RAM

    .msp_stack : ALIGN(TFM_LINKER_MSP_STACK_ALIGNMENT)
    {
        . += __msp_stack_size__ - 0x8;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);

    .msp_stack_seal_res :
    {
        . += 0x8;
    } > RAM
    __StackSeal = ADDR(.msp_stack_seal_res);

#if defined(ENABLE_HEAP)
    __heap_size__ = S_HEAP_SIZE;
    .heap : ALIGN(8)
    {
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += __heap_size__;
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */
    } > RAM
#endif

    /**** PSA RoT DATA start here */

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_ITS_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_ITS_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_its.*:*(.data*)
        *(TFM_SP_ITS_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_ITS_RWZI);
    Image$$ER_TFM_SP_ITS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_ITS_RWZI) + SIZEOF(.ER_TFM_SP_ITS_RWZI);

    .TFM_SP_ITS_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_ITS_RWZI = .;
        *tfm_*partition_its.*:*(.bss*)
        *tfm_*partition_its.*:*(COMMON)
        *(TFM_SP_ITS_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_ITS_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Base = ADDR(.TFM_SP_ITS_RWZI_BSS);
    Image$$ER_TFM_SP_ITS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_ITS_RWZI_BSS) + SIZEOF(.TFM_SP_ITS_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_ITS_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_CRYPTO_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_CRYPTO_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_crypto.*:*(.data*)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_CRYPTO_RWZI);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_CRYPTO_RWZI) + SIZEOF(.ER_TFM_SP_CRYPTO_RWZI);

    .TFM_SP_CRYPTO_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_CRYPTO_RWZI = .;
        *tfm_*partition_crypto.*:*(.bss*)
        *tfm_*partition_crypto.*:*(COMMON)
        *(TFM_SP_CRYPTO_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_CRYPTO_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_RWZI_BSS);
    Image$$ER_TFM_SP_CRYPTO_RWZI$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_RWZI_BSS) + SIZEOF(.TFM_SP_CRYPTO_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_CRYPTO_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PLATFORM_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PLATFORM_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_platform.*:*(.data*)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PLATFORM_RWZI);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PLATFORM_RWZI) + SIZEOF(.ER_TFM_SP_PLATFORM_RWZI);

    .TFM_SP_PLATFORM_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_PLATFORM_RWZI = .;
        *tfm_*partition_platform.*:*(.bss*)
        *tfm_*partition_platform.*:*(COMMON)
        *(TFM_SP_PLATFORM_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PLATFORM_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_RWZI_BSS);
    Image$$ER_TFM_SP_PLATFORM_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_RWZI_BSS) + SIZEOF(.TFM_SP_PLATFORM_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PLATFORM_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_INITIAL_ATTESTATION_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_attestation.*:*(.data*)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI) + SIZEOF(.ER_TFM_SP_INITIAL_ATTESTATION_RWZI);

    .TFM_SP_INITIAL_ATTESTATION_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_INITIAL_ATTESTATION_RWZI = .;
        *tfm_*partition_attestation.*:*(.bss*)
        *tfm_*partition_attestation.*:*(COMMON)
        *(TFM_SP_INITIAL_ATTESTATION_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_INITIAL_ATTESTATION_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);
    Image$$ER_TFM_SP_INITIAL_ATTESTATION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_INITIAL_ATTESTATION_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_FWU_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_FWU_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_fwu*:*(.data*)
        *(TFM_SP_FWU_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_FWU_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_FWU_RWZI);
    Image$$ER_TFM_SP_FWU_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_FWU_RWZI) + SIZEOF(.ER_TFM_SP_FWU_RWZI);

    .TFM_SP_FWU_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_FWU_RWZI = .;
        *tfm_*partition_fwu*:*(.bss*)
        *tfm_*partition_fwu*:*(COMMON)
        *(TFM_SP_FWU_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_FWU_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_FWU_RWZI$$ZI$$Base = ADDR(.TFM_SP_FWU_RWZI_BSS);
    Image$$ER_TFM_SP_FWU_RWZI$$ZI$$Limit = ADDR(.TFM_SP_FWU_RWZI_BSS) + SIZEOF(.TFM_SP_FWU_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_FWU_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SECURE_TEST_PARTITION_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_secure_client_service.*:*(.data*)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_TEST_PARTITION_RWZI);

    .TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI = .;
        *tfm_*partition_secure_client_service.*:*(.bss*)
        *tfm_*partition_secure_client_service.*:*(COMMON)
        *(TFM_SP_SECURE_TEST_PARTITION_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_TEST_PARTITION_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_TEST_PARTITION_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_TEST_PARTITION_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SECURE_TEST_PARTITION_PRIVATE_DATA_END$$Base = .;


    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PS_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PS_TEST_RWZI : ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_ps_test.*:*(.data*)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_TEST_RWZI);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_TEST_RWZI) + SIZEOF(.ER_TFM_SP_PS_TEST_RWZI);

    .TFM_SP_PS_TEST_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_PS_TEST_RWZI = .;
        *tfm_*partition_ps_test.*:*(.bss*)
        *tfm_*partition_ps_test.*:*(COMMON)
        *(TFM_SP_PS_TEST_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_TEST_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_PS_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_PS_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_PSA_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PS_TEST_PRIVATE_DATA_END$$Base = .;

    /**** PSA RoT DATA end here */
    . = ALIGN(TFM_LINKER_PT_PRIV_RWZI_ALIGNMENT);

    /* Position tag */
    Image$$PT_PRIV_RWZI_END$$Base = .;

    /**** APPLICATION RoT DATA start here */
    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PS_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_PS_RWZI : ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_ps.*:*(.data*)
        *(TFM_SP_PS_APP-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_PS_RWZI);
    Image$$.ER_TFM_SP_PS_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_PS_RWZI) + SIZEOF(.ER_TFM_SP_PS_RWZI);

    .TFM_SP_PS_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_PS_RWZI = .;
        *tfm_*partition_ps.*:*(.bss*)
        *tfm_*partition_ps.*:*(COMMON)
        *(TFM_SP_PS_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_PS_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Base = ADDR(.TFM_SP_PS_RWZI_BSS);
    Image$$ER_TFM_SP_PS_RWZI$$ZI$$Limit = ADDR(.TFM_SP_PS_RWZI_BSS) + SIZEOF(.TFM_SP_PS_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_PS_PRIVATE_DATA_END$$Base = .;

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SECURE_CLIENT_2_RWZI : ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_secure_client_2.*:*(.data*)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);
    Image$$.ER_TFM_SP_SECURE_CLIENT_2_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SECURE_CLIENT_2_RWZI) + SIZEOF(.ER_TFM_SP_SECURE_CLIENT_2_RWZI);

    .TFM_SP_SECURE_CLIENT_2_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_SECURE_CLIENT_2_RWZI = .;
        *tfm_*partition_secure_client_2.*:*(.bss*)
        *tfm_*partition_secure_client_2.*:*(COMMON)
        *(TFM_SP_SECURE_CLIENT_2_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SECURE_CLIENT_2_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Base = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);
    Image$$ER_TFM_SP_SECURE_CLIENT_2_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS) + SIZEOF(.TFM_SP_SECURE_CLIENT_2_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SECURE_CLIENT_2_PRIVATE_DATA_END$$Base = .;

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SLIH_TEST_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SP_SLIH_TEST_RWZI : ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_slih_test.*:*(.data*)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SP_SLIH_TEST_RWZI$$RW$$Base = ADDR(.ER_TFM_SP_SLIH_TEST_RWZI);
    Image$$.ER_TFM_SP_SLIH_TEST_RWZI$$RW$$Limit = ADDR(.ER_TFM_SP_SLIH_TEST_RWZI) + SIZEOF(.ER_TFM_SP_SLIH_TEST_RWZI);

    .TFM_SP_SLIH_TEST_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SP_SLIH_TEST_RWZI = .;
        *tfm_*partition_slih_test.*:*(.bss*)
        *tfm_*partition_slih_test.*:*(COMMON)
        *(TFM_SP_SLIH_TEST_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SP_SLIH_TEST_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SP_SLIH_TEST_RWZI$$ZI$$Base = ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS);
    Image$$ER_TFM_SP_SLIH_TEST_RWZI$$ZI$$Limit = ADDR(.TFM_SP_SLIH_TEST_RWZI_BSS) + SIZEOF(.TFM_SP_SLIH_TEST_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SP_SLIH_TEST_PRIVATE_DATA_END$$Base = .;

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SFN_PARTITION1_PRIVATE_DATA_START$$Base = .;

    .ER_TFM_SFN_PARTITION1_RWZI : ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT)
    {
        *tfm_*partition_sfn_backend_test.*:*(.data*)
        *(TFM_SFN_PARTITION1_APP-ROT_ATTR_RW)
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$.ER_TFM_SFN_PARTITION1_RWZI$$RW$$Base = ADDR(.ER_TFM_SFN_PARTITION1_RWZI);
    Image$$.ER_TFM_SFN_PARTITION1_RWZI$$RW$$Limit = ADDR(.ER_TFM_SFN_PARTITION1_RWZI) + SIZEOF(.ER_TFM_SFN_PARTITION1_RWZI);

    .TFM_SFN_PARTITION1_RWZI_BSS : ALIGN(4)
    {
        start_of_TFM_SFN_PARTITION1_RWZI = .;
        *tfm_*partition_sfn_backend_test.*:*(.bss*)
        *tfm_*partition_sfn_backend_test.*:*(COMMON)
        *(TFM_SFN_PARTITION1_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_SFN_PARTITION1_RWZI) ? 0 : 4;
        . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    } > RAM AT> RAM
    Image$$ER_TFM_SFN_PARTITION1_RWZI$$ZI$$Base = ADDR(.TFM_SFN_PARTITION1_RWZI_BSS);
    Image$$ER_TFM_SFN_PARTITION1_RWZI$$ZI$$Limit = ADDR(.TFM_SFN_PARTITION1_RWZI_BSS) + SIZEOF(.TFM_SFN_PARTITION1_RWZI_BSS);

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_TFM_SFN_PARTITION1_PRIVATE_DATA_END$$Base = .;

    /**** APPLICATION RoT DATA end here */

    /* Position tag */
    . = ALIGN(TFM_LINKER_APP_ROT_LINKER_DATA_ALIGNMENT);
    Image$$PT_APP_RWZI_END$$Base = .;

#ifdef RAM_VECTORS_SUPPORT
    .ramVectors (NOLOAD) : ALIGN(TFM_LINKER_RAM_VECTORS_ALIGNMENT)
    {
        __ram_vectors_start__ = .;
        KEEP(*(.ram_vectors))
        __ram_vectors_end__   = .;
    } > RAM
    .TFM_DATA __ram_vectors_end__ : ALIGN(4)
#else

    .TFM_DATA : ALIGN(4)
#endif
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

#if defined(CONFIG_TFM_PARTITION_META)
    .TFM_SP_META_PTR : ALIGN(TFM_LINKER_SP_META_PTR_ALIGNMENT)
    {
        *(.bss.SP_META_PTR_SPRTL_INST)
        . = ALIGN(TFM_LINKER_SP_META_PTR_ALIGNMENT);
    } > RAM AT> RAM
    Image$$TFM_SP_META_PTR$$ZI$$Base = ADDR(.TFM_SP_META_PTR);
    Image$$TFM_SP_META_PTR$$ZI$$Limit = ADDR(.TFM_SP_META_PTR) + SIZEOF(.TFM_SP_META_PTR);
#endif

    .TFM_BSS : ALIGN(4)
    {
        __bss_start__ = .;

        /* The runtime partition placed order is same as load partition */
        __partition_runtime_start__ = .;
        KEEP(*(.bss.part_runtime_priority_lowest))
        KEEP(*(.bss.part_runtime_priority_low))
        KEEP(*(.bss.part_runtime_priority_normal))
        KEEP(*(.bss.part_runtime_priority_high))
        __partition_runtime_end__ = .;
        . = ALIGN(4);

        /* The runtime service placed order is same as load partition */
        __service_runtime_start__ = .;
        KEEP(*(.bss.serv_runtime_priority_lowest))
        KEEP(*(.bss.serv_runtime_priority_low))
        KEEP(*(.bss.serv_runtime_priority_normal))
        KEEP(*(.bss.serv_runtime_priority_high))
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

    Load$$LR$$LR_NS_PARTITION$$Base = NS_PARTITION_START;

#ifdef BL2
    Load$$LR$$LR_SECONDARY_PARTITION$$Base = SECONDARY_PARTITION_START;
#endif /* BL2 */

    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);
}
