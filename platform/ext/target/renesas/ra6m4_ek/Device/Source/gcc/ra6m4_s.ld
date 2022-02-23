/*
                  Linker File for Renesas FSP TFM
*/

INCLUDE memory_regions.ld

QSPI_FLASH_PRV_LENGTH = DEFINED(QSPI_FLASH_SIZE) ? ABSOLUTE(QSPI_FLASH_SIZE) : ABSOLUTE(QSPI_FLASH_LENGTH);
OSPI_DEVICE_0_PRV_LENGTH = DEFINED(OSPI_DEVICE_0_SIZE) ? ABSOLUTE(OSPI_DEVICE_0_SIZE) : ABSOLUTE(OSPI_DEVICE_0_LENGTH);
OSPI_DEVICE_1_PRV_LENGTH = DEFINED(OSPI_DEVICE_1_SIZE) ? ABSOLUTE(OSPI_DEVICE_1_SIZE) : ABSOLUTE(OSPI_DEVICE_1_LENGTH);

OPTION_SETTING_START_NS = 0x0100A180;

PROJECT_SECURE_OR_FLAT = !DEFINED(PROJECT_NONSECURE) && OPTION_SETTING_LENGTH && !DEFINED(FLASH_NS_IMAGE_START);
USE_OPTION_SETTING_NS = DEFINED(PROJECT_NONSECURE) && !DEFINED(FLASH_BOOTLOADER_LENGTH);


RAM_S_SIZE = (RAM_NS_START - RAM_START);
CMSE_VENEER_REGION_SIZE = (FLASH_NS_START - FLASH_NSC_START);

BOOT_TFM_SHARED_DATA_SIZE = 0x3E0;
BOOT_TFM_SHARED_SEED_SIZE = 0x20;

__S_PSP_STACK_SIZE = 0x0000800;

DATA_FLASH_LENGTH_S = DATA_FLASH_LENGTH / 2;
DATA_FLASH_NS_START = DATA_FLASH_START + DATA_FLASH_LENGTH_S;

MEMORY
{
  FLASH    (rx)  : ORIGIN = FLASH_IMAGE_START, LENGTH = FLASH_IMAGE_LENGTH
  RAM      (rwx) : ORIGIN = RAM_START, LENGTH = RAM_S_SIZE
  VENEERS  (rx)  : ORIGIN = FLASH_NSC_START, LENGTH = CMSE_VENEER_REGION_SIZE
  DATA_FLASH (rx) : ORIGIN = DATA_FLASH_START, LENGTH = DATA_FLASH_LENGTH
  QSPI_FLASH (rx) : ORIGIN = QSPI_FLASH_START, LENGTH = QSPI_FLASH_PRV_LENGTH
  OSPI_DEVICE_0 (rx) : ORIGIN = OSPI_DEVICE_0_START, LENGTH = OSPI_DEVICE_0_PRV_LENGTH
  OSPI_DEVICE_1 (rx) : ORIGIN = OSPI_DEVICE_1_START, LENGTH = OSPI_DEVICE_1_PRV_LENGTH
  SDRAM (rwx)     : ORIGIN = SDRAM_START, LENGTH = SDRAM_LENGTH
  OPTION_SETTING (r): ORIGIN = OPTION_SETTING_START, LENGTH = OPTION_SETTING_LENGTH
  OPTION_SETTING_S (r): ORIGIN = OPTION_SETTING_S_START, LENGTH = OPTION_SETTING_S_LENGTH
  ID_CODE (rx)    : ORIGIN = ID_CODE_START, LENGTH = ID_CODE_LENGTH
}

__psp_stack_size__ = __S_PSP_STACK_SIZE;

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a libc_nano.a)

ENTRY(Reset_Handler)

SECTIONS
{
    .TFM_VECTORS : ALIGN(4)
    {
    	__vectors_start__ = .;
        KEEP(*(.fixed_vectors*))
        KEEP(*(.application_vectors*))
        . = ALIGN(4);
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

        LONG (LOADADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_DATA))

        LONG (LOADADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_DATA))

        __copy_table_end__ = .;
    } > FLASH

    .zero.table : ALIGN(4)
    {
        __zero_table_start__ = .;
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS))

        LONG (ADDR(.TFM_PSA_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_BSS))

        LONG (ADDR(.TFM_APP_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_BSS))


        __zero_table_end__ = .;
    } > FLASH

    .TFM_UNPRIV_CODE : ALIGN(32)
    {
        *libc_nano*:*(.text*)
        *libc_nano*:*(.rodata*)
        *(SFN)
        *libgcc*:*(.text*)
        *libgcc*:*(.rodata*)
        *sprt/*.o(.text .text* .rodata .rodata*)
        *psa_client.*(.text*)              /* NXP */
        *psa_client.*(.rodata*)
        *psa_service.*(.text*)             /* NXP */
        *psa_service.*(.rodata*)
        *psa_lifecycle.*(.text*)             /* NXP */
        *psa_lifecycle.*(.rodata*)
        *tfm_log_raw.*(.text*)             /* NXP */
        *tfm_log_raw.*(.rodata*)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_UNPRIV_CODE$$RO$$Base = ADDR(.TFM_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE$$RO$$Limit = ADDR(.TFM_UNPRIV_CODE) + SIZEOF(.TFM_UNPRIV_CODE);
    
    
    /**** Section for holding partition RO load data */
    .TFM_SP_LOAD_LIST : ALIGN(4)
    {
       KEEP(*(.part_load))
    } > FLASH
    Image$$TFM_SP_LOAD_LIST$$RO$$Base = ADDR(.TFM_SP_LOAD_LIST);
    Image$$TFM_SP_LOAD_LIST$$RO$$Limit = ADDR(.TFM_SP_LOAD_LIST) + SIZEOF(.TFM_SP_LOAD_LIST);
    . = ALIGN(32);
    
    /**** PSA RoT RO part (CODE + RODATA) start here */
    Image$$TFM_PSA_CODE_START$$Base = .;

    .TFM_PSA_ROT_LINKER : ALIGN(32)
    {
        *crypto_init.*(.text*)
        *crypto_init.*(.rodata*)
        *crypto_alloc.*(.text*)
        *crypto_alloc.*(.rodata*)
        *crypto_cipher.*(.text*)
        *crypto_cipher.*(.rodata*)
        *crypto_hash.*(.text*)
        *crypto_hash.*(.rodata*)
        *crypto_mac.*(.text*)
        *crypto_mac.*(.rodata*)
        *crypto_key.*(.text*)
        *crypto_key.*(.rodata*)
        *crypto_aead.*(.text*)
        *crypto_aead.*(.rodata*)
        *crypto_asymmetric.*(.text*)
        *crypto_asymmetric.*(.rodata*)
        *crypto_key_derivation.*(.text*)
        *crypto_key_derivation.*(.rodata*)
        *intermedia_tfm_crypto.*(.text*)
        *intermedia_tfm_crypto.*(.rodata*)

        *tfm_attest.*(.text*)
        *tfm_attest.*(.rodata*)
        *tfm_attest_req_mngr.*(.text*)
        *tfm_attest_req_mngr.*(.rodata*)
        *attest_core.*(.text*)
        *attest_core.*(.rodata*)
        *attest_asymmetric_key.*(.text*)
        *attest_asymmetric_key.*(.rodata*)
        *attest_token_encode.*(.text*)
        *attest_token_encode.*(.rodata*)
        *intermedia_tfm_initial_attestation.*(.text*)
        *intermedia_tfm_initial_attestation.*(.rodata*)

        *tfm_its_req_mngr.*(.text*)
        *tfm_its_req_mngr.*(.rodata*)
        tfm_internal_trusted_storage.*(.text*)
        tfm_internal_trusted_storage.*(.rodata*)
        *its_utils.*(.text*)
        *its_utils.*(.rodata*)
        *flash/its_flash.*(.text*)
        *flash/its_flash.*(.rodata*)
        *flash/its_flash_nand.*(.text*)
        *flash/its_flash_nand.*(.rodata*)
        *flash/its_flash_nor.*(.text*)
        *flash/its_flash_nor.*(.rodata*)
        *flash/its_flash_ram.*(.text*)
        *flash/its_flash_ram.*(.rodata*)
        *flash_fs/its_flash_fs.*(.text*)
        *flash_fs/its_flash_fs.*(.rodata*)
        *flash_fs/its_flash_fs_dblock.*(.text*)
        *flash_fs/its_flash_fs_dblock.*(.rodata*)
        *flash_fs/its_flash_fs_mblock.*(.text*)
        *flash_fs/its_flash_fs_mblock.*(.rodata*)
        *intermedia_tfm_internal_trusted_storage.*(.text*)
        *intermedia_tfm_internal_trusted_storage.*(.rodata*)

        *platform_sp.*(.text*)
        *platform_sp.*(.rodata*)
        *intermedia_tfm_platform.*(.text*)
        *intermedia_tfm_platform.*(.rodata*)

/*        *tfm_psa_rot_partition*:*(.text*)
        *tfm_psa_rot_partition*:*(.rodata*) */
        *(TFM_*_PSA-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH

    Image$$TFM_PSA_ROT_LINKER$$RO$$Base = ADDR(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$RO$$Limit = ADDR(.TFM_PSA_ROT_LINKER) + SIZEOF(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$Base = ADDR(.TFM_PSA_ROT_LINKER);
    Image$$TFM_PSA_ROT_LINKER$$Limit = ADDR(.TFM_PSA_ROT_LINKER) + SIZEOF(.TFM_PSA_ROT_LINKER);

    /**** PSA RoT RO part (CODE + RODATA) end here */
    Image$$TFM_PSA_CODE_END$$Base = .;

    /**** APPLICATION RoT RO part (CODE + RODATA) start here */
    Image$$TFM_APP_CODE_START$$Base = .;

    .TFM_APP_ROT_LINKER : ALIGN(32)
    {
        *tfm_app_rot_partition*:*(.text*)
        *tfm_app_rot_partition*:*(.rodata*)
        *(TFM_*_APP-ROT_ATTR_FN)
        . = ALIGN(32);
    } > FLASH

    Image$$TFM_APP_ROT_LINKER$$RO$$Base = ADDR(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$RO$$Limit = ADDR(.TFM_APP_ROT_LINKER) + SIZEOF(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$Base = ADDR(.TFM_APP_ROT_LINKER);
    Image$$TFM_APP_ROT_LINKER$$Limit = ADDR(.TFM_APP_ROT_LINKER) + SIZEOF(.TFM_APP_ROT_LINKER);

    /**** APPLICATION RoT RO part (CODE + RODATA) end here */
    Image$$TFM_APP_CODE_END$$Base = .;


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

    /**** Base address of secure data area */
    .tfm_secure_data_start :
    {
        . = ABSOLUTE(RAM_START) ;
    } > RAM

    /*
     * MPU on Armv6-M/v7-M core in multi-core topology may require more strict
     * alignment that MPU region base address must align with the MPU region
     * size.
     * As a result, in multi-core topology, to save memory resource and MPU
     * regions, unprivileged data sections and privileged data sections are
     * separated and gathered in unprivileged/privileged data area respectively.
     * Keep BL2 shared data and MSP stack at the beginning of the secure data
     * area in single Armv8-M topology, while move the two areas to the
     * beginning of privileged data region in multi-core topology.
     */


    /* shared_data and msp_stack are overlapping on purpose when
     * msp_stack is extended until the beginning of RAM, when shared_date
     * was read out by partitions
     */
    .tfm_bl2_shared_data : ALIGN(32)
    {
        . += BOOT_TFM_SHARED_DATA_SIZE;
    } > RAM
    .tfm_bl2_shared_boot_seed : ALIGN(32)
    {
        . += BOOT_TFM_SHARED_SEED_SIZE;
    } > RAM

    .noinit (NOLOAD):
    {
        . = ALIGN(4);
        __noinit_start = .;
        KEEP(*(.noinit*))
        . = ALIGN(8);
        __noinit_end = .;
    } > RAM

    .msp_stack : ALIGN(32)
    {
        KEEP(*(.stack));
    } > RAM
    Image$$ARM_LIB_STACK_MSP$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK_MSP$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);

    /* PSP is unprivileged in single-core topology */
    .psp_stack : ALIGN(32)
    {
        . += __psp_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.psp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.psp_stack) + SIZEOF(.psp_stack);



    /**** APPLICATION RoT DATA start here */
    Image$$TFM_APP_RW_STACK_START$$Base = .;

    .TFM_APP_ROT_LINKER_DATA : ALIGN(32)
    {
        *tfm_app_rot_partition*:*(.data*)
        *(TFM_*_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_APP_ROT_LINKER_DATA);
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_APP_ROT_LINKER_DATA) + SIZEOF(.TFM_APP_ROT_LINKER_DATA);

    .TFM_APP_ROT_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_APP_ROT_LINKER = .;
        *tfm_app_rot_partition*:*(.bss*)
        *tfm_app_rot_partition*:*(COMMON)
        *(TFM_*_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_APP_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_APP_ROT_LINKER_BSS);
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_APP_ROT_LINKER_BSS) + SIZEOF(.TFM_APP_ROT_LINKER_BSS);

    /**** APPLICATION RoT DATA end here */
    Image$$TFM_APP_RW_STACK_END$$Base = .;


    .heap : ALIGN(8)
    {
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        KEEP(*(.heap))
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */
    } > RAM AT> RAM

    /**** PSA RoT DATA start here */

    Image$$TFM_PSA_RW_STACK_START$$Base = .;

    .TFM_PSA_ROT_LINKER_DATA : ALIGN(32)
    {
        *crypto_init.*(.data*)
        *crypto_alloc.*(.data*)
        *crypto_cipher.*(.data*)
        *crypto_hash.*(.data*)
        *crypto_mac.*(.data*)
        *crypto_key.*(.data*)
        *crypto_aead.*(.data*)
        *crypto_asymmetric.*(.data*)
        *crypto_key_derivation.*(.data*)
        *intermedia_tfm_crypto.*(.data*)

        *tfm_attest.*(.data*)
        *tfm_attest_req_mngr.*(.data*)
        *attest_core.*(.data*)
        *attest_asymmetric_key.*(.data*)
        *attest_token_encode.*(.data*)
        *intermedia_tfm_initial_attestation.*(.data*)

        *tfm_its_req_mngr.*(.data*)
        tfm_internal_trusted_storage.*(.data*)
        *its_utils.*(.data*)
        *flash/its_flash.*(.data*)
        *flash/its_flash_nand.*(.data*)
        *flash/its_flash_nor.*(.data*)
        *flash/its_flash_ram.*(.data*)
        *flash_fs/its_flash_fs.*(.data*)
        *flash_fs/its_flash_fs_dblock.*(.data*)
        *flash_fs/its_flash_fs_mblock.*(.data*)
        *intermedia_tfm_internal_trusted_storage.*(.data*)

        *platform_sp.*(.data*)
        *intermedia_tfm_platform.*(.data*)
        *tfm_psa_rot_partition*:*(.data*)
        *(TFM_*_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_PSA_ROT_LINKER_DATA);
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_PSA_ROT_LINKER_DATA) + SIZEOF(.TFM_PSA_ROT_LINKER_DATA);

    .TFM_PSA_ROT_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_PSA_ROT_LINKER = .;
        *crypto_init.*(.bss*)
        *crypto_init.*(COMMON)
        *crypto_alloc.*(.bss*)
        *crypto_alloc.*(COMMON)
        *crypto_cipher.*(.bss*)
        *crypto_cipher.*(COMMON)
        *crypto_hash.*(.bss*)
        *crypto_hash.*(COMMON)
        *crypto_mac.*(.bss*)
        *crypto_mac.*(COMMON)
        *crypto_key.*(.bss*)
        *crypto_key.*(COMMON)
        *crypto_aead.*(.bss*)
        *crypto_aead.*(COMMON)
        *crypto_asymmetric.*(.bss*)
        *crypto_asymmetric.*(COMMON)
        *crypto_key_derivation.*(.bss*)
        *crypto_key_derivation.*(COMMON)
        *intermedia_tfm_crypto.*(.bss*)
        *intermedia_tfm_crypto.*(COMMON)

        *tfm_attest.*(.bss*)
        *tfm_attest.*(COMMON)
        *tfm_attest_req_mngr.*(.bss*)
        *tfm_attest_req_mngr.*(COMMON)
        *attest_core.*(.bss*)
        *attest_core.*(COMMON)
        *attest_asymmetric_key.*(.bss*)
        *attest_asymmetric_key.*(COMMON)
        *attest_token_encode.*(.bss*)
        *attest_token_encode.*(COMMON)
        *intermedia_tfm_initial_attestation.*(.bss*)
        *intermedia_tfm_initial_attestation.*(COMMON)

        *tfm_its_req_mngr.*(.bss*)
        *tfm_its_req_mngr.*(COMMON)
        tfm_internal_trusted_storage.*(.bss*)
        tfm_internal_trusted_storage.*(COMMON)
        *its_utils.*(.bss*)
        *its_utils.*(COMMON)
        *flash/its_flash.*(.bss*)
        *flash/its_flash.*(COMMON)
        *flash/its_flash_nand.*(.bss*)
        *flash/its_flash_nand.*(COMMON)
        *flash/its_flash_nor.*(.bss*)
        *flash/its_flash_nor.*(COMMON)
        *flash/its_flash_ram.*(.bss*)
        *flash/its_flash_ram.*(COMMON)
        *flash_fs/its_flash_fs.*(.bss*)
        *flash_fs/its_flash_fs.*(COMMON)
        *flash_fs/its_flash_fs_dblock.*(.bss*)
        *flash_fs/its_flash_fs_dblock.*(COMMON)
        *flash_fs/its_flash_fs_mblock.*(.bss*)
        *flash_fs/its_flash_fs_mblock.*(COMMON)
        *intermedia_tfm_internal_trusted_storage.*(.bss*)
        *intermedia_tfm_internal_trusted_storage.*(COMMON)

        *platform_sp.*(.bss*)
        *platform_sp.*(COMMON)
        *intermedia_tfm_platform.*(.bss*)
        *intermedia_tfm_platform.*(COMMON)

        *tfm_psa_rot_partition*:*(.bss*)
        *tfm_psa_rot_partition*:*(COMMON)
        *(TFM_*_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_PSA_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_PSA_ROT_LINKER_BSS);
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_PSA_ROT_LINKER_BSS) + SIZEOF(.TFM_PSA_ROT_LINKER_BSS);

    /**** PSA RoT DATA end here */
    Image$$TFM_PSA_RW_STACK_END$$Base = .;
    .TFM_DATA : ALIGN(4)
    {
        . = ALIGN(4);
        __Code_In_RAM_Start = .;
        KEEP(*(.code_in_ram*))
        __Code_In_RAM_End = .;
        
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

    Load$$LR$$LR_NS_PARTITION$$Base = FLASH_NS_START;
    Load$$LR$$LR_NS_IMAGE$$Base = FLASH_NS_IMAGE_START;

    /* Data flash. */
    .data_flash :
    {
        . = ORIGIN(DATA_FLASH);
        __tz_DATA_FLASH_S = .;
        __Data_Flash_Start = .;
        . += DATA_FLASH_LENGTH_S;
        __Data_Flash_End = .;

        __tz_DATA_FLASH_N = ABSOLUTE(DATA_FLASH_NS_START);
    } > DATA_FLASH


    /* Note: There are no secure/non-secure boundaries for QSPI.  These symbols are provided for the RA configuration tool. */
    __tz_QSPI_FLASH_S = ORIGIN(QSPI_FLASH);

    /* QSPI_FLASH section to be downloaded via debugger */
    .qspi_flash :
    {
        __qspi_flash_start__ = .;
        KEEP(*(.qspi_flash*))
        KEEP(*(.code_in_qspi*))
        __qspi_flash_end__ = .;
    } > QSPI_FLASH
    __qspi_flash_code_size__ = __qspi_flash_end__ - __qspi_flash_start__;

    /* QSPI_FLASH non-retentive section, creates a copy in internal flash that can be copied to QSPI */
    __qspi_flash_code_addr__ = __etext + (__data_end__ - __data_start__);
    .qspi_non_retentive : AT (__qspi_flash_code_addr__)
    {
        __qspi_non_retentive_start__ = .;
        KEEP(*(.qspi_non_retentive*))
        __qspi_non_retentive_end__ = .;
    } > QSPI_FLASH
    __qspi_non_retentive_size__ = __qspi_non_retentive_end__ - __qspi_non_retentive_start__;

    __qspi_region_max_size__ = 0x4000000;   /* Must be the same as defined in MEMORY above */
    __qspi_region_start_address__ = __qspi_flash_start__;
    __qspi_region_end_address__ = __qspi_flash_start__ + __qspi_region_max_size__;

    /* Note: There are no secure/non-secure boundaries for QSPI.  These symbols are provided for the RA configuration tool. */
    __tz_QSPI_FLASH_N = __qspi_non_retentive_end__;

    /* Note: There are no secure/non-secure boundaries for QSPI.  These symbols are provided for the RA configuration tool. */
    __tz_OSPI_DEVICE_0_S = ORIGIN(OSPI_DEVICE_0);

    /* OSPI_DEVICE_0 section to be downloaded via debugger */
    .OSPI_DEVICE_0 :
    {
        __ospi_device_0_start__ = .;
        KEEP(*(.ospi_device_0*))
        KEEP(*(.code_in_ospi_device_0*))
        __ospi_device_0_end__ = .;
    } > OSPI_DEVICE_0
    __ospi_device_0_code_size__ = __ospi_device_0_end__ - __ospi_device_0_start__;

    /* OSPI_DEVICE_0 non-retentive section, creates a copy in internal flash that can be copied to OSPI */
    __ospi_device_0_code_addr__ = __etext + (__data_end__ - __data_start__);
    .ospi_device_0_non_retentive : AT (__ospi_device_0_code_addr__)
    {
        __ospi_device_0_non_retentive_start__ = .;
        KEEP(*(.ospi_device_0_non_retentive*))
        __ospi_device_0_non_retentive_end__ = .;
    } > OSPI_DEVICE_0
    __ospi_device_0_non_retentive_size__ = __ospi_device_0_non_retentive_end__ - __ospi_device_0_non_retentive_start__;

    __ospi_device_0_region_max_size__ = 0x8000000;   /* Must be the same as defined in MEMORY above */
    __ospi_device_0_region_start_address__ = __ospi_device_0_start__;
    __ospi_device_0_region_end_address__ = __ospi_device_0_start__ + __ospi_device_0_region_max_size__;

    /* Note: There are no secure/non-secure boundaries for OSPI.  These symbols are provided for the RA configuration tool. */
    __tz_OSPI_DEVICE_0_N = __ospi_device_0_non_retentive_end__;

    /* Note: There are no secure/non-secure boundaries for OSPI.  These symbols are provided for the RA configuration tool. */
    __tz_OSPI_DEVICE_1_S = ORIGIN(OSPI_DEVICE_1);

    /* OSPI_DEVICE_1 section to be downloaded via debugger */
    .OSPI_DEVICE_1 :
    {
        __ospi_device_1_start__ = .;
        KEEP(*(.ospi_device_1*))
        KEEP(*(.code_in_ospi_device_1*))
        __ospi_device_1_end__ = .;
    } > OSPI_DEVICE_1
    __ospi_device_1_code_size__ = __ospi_device_1_end__ - __ospi_device_1_start__;

    /* OSPI_DEVICE_1 non-retentive section, creates a copy in internal flash that can be copied to OSPI */
    __ospi_device_1_code_addr__ = __etext + (__data_end__ - __data_start__);
    .ospi_device_1_non_retentive : AT (__ospi_device_1_code_addr__)
    {
        __ospi_device_1_non_retentive_start__ = .;
        KEEP(*(.ospi_device_1_non_retentive*))
        __ospi_device_1_non_retentive_end__ = .;
    } > OSPI_DEVICE_1
    __ospi_device_1_non_retentive_size__ = __ospi_device_1_non_retentive_end__ - __ospi_device_1_non_retentive_start__;

    __ospi_device_1_region_max_size__ = 0x10000000;   /* Must be the same as defined in MEMORY above */
    __ospi_device_1_region_start_address__ = __ospi_device_1_start__;
    __ospi_device_1_region_end_address__ = __ospi_device_1_start__ + __ospi_device_1_region_max_size__;

    /* Note: There are no secure/non-secure boundaries for OSPI.  These symbols are provided for the RA configuration tool. */
    __tz_OSPI_DEVICE_1_N = __ospi_device_1_non_retentive_end__;
    
        /* Note: There are no secure/non-secure boundaries for SDRAM.  These symbols are provided for the RA configuration tool. */
    __tz_SDRAM_S = ORIGIN(SDRAM);

    /* SDRAM */
    .sdram (NOLOAD):
    {
        __SDRAM_Start = .;
        KEEP(*(.sdram*))
        KEEP(*(.frame*))
        __SDRAM_End = .;
    } > SDRAM

    /* Note: There are no secure/non-secure boundaries for SDRAM.  These symbols are provided for the RA configuration tool. */
    __tz_SDRAM_N = __SDRAM_End;

    /* Note: There are no secure/non-secure boundaries for ID_CODE.  These symbols are provided for the RA configuration tool. */
    __tz_ID_CODE_S = ORIGIN(ID_CODE);

    .id_code :
    {
        __ID_Code_Start = .;
        KEEP(*(.id_code*))
        __ID_Code_End = .;
    } > ID_CODE

    /* Note: There are no secure/non-secure boundaries for ID_CODE.  These symbols are provided for the RA configuration tool. */
    __tz_ID_CODE_N = __ID_Code_End;

    /* Symbol required for RA Configuration tool. */
    __tz_OPTION_SETTING_S = ORIGIN(OPTION_SETTING);

    .option_setting :
    {
        __OPTION_SETTING_Start = .;
        KEEP(*(.option_setting_ofs0))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_Start + 0x10 : __OPTION_SETTING_Start;
        KEEP(*(.option_setting_dualsel))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_Start + 0x34 : __OPTION_SETTING_Start;
        KEEP(*(.option_setting_sas))
        __OPTION_SETTING_End = .;
    } > OPTION_SETTING = 0xFF

    /* Symbol required for RA Configuration tool. */
    __tz_OPTION_SETTING_N = OPTION_SETTING_START_NS;

    .option_setting_ns :
    {
        __OPTION_SETTING_NS_Start = .;
        KEEP(*(.option_setting_ofs1))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x10 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_banksel))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x40 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_bps0))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x44 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_bps1))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x48 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_bps2))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x60 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_pbps0))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x64 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_pbps1))
        . = USE_OPTION_SETTING_NS ? __OPTION_SETTING_NS_Start + 0x68 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_pbps2))
        __OPTION_SETTING_NS_End = .;
    } > OPTION_SETTING = 0xFF

    /* Symbol required for RA Configuration tool. */
    __tz_OPTION_SETTING_S_S = ORIGIN(OPTION_SETTING_S);

    .option_setting_s :
    {
        __OPTION_SETTING_S_Start = .;
        KEEP(*(.option_setting_ofs1_sec))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x10 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_banksel_sec))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x40 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sec0))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x44 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sec1))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x48 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sec2))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x60 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_pbps_sec0))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x64 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_pbps_sec1))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x68 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_pbps_sec2))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x80 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_ofs1_sel))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0x90 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_banksel_sel))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0xC0 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sel0))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0xC4 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sel1))
        . = PROJECT_SECURE_OR_FLAT ? __OPTION_SETTING_S_Start + 0xC8 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sel2))
        __OPTION_SETTING_S_End = .;
    } > OPTION_SETTING_S = 0xFF

    /* Symbol required for RA Configuration tool. */
    __tz_OPTION_SETTING_S_N = __OPTION_SETTING_S_End;

    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);
    PROVIDE(__StackLimit = Image$$ARM_LIB_STACK_MSP$$ZI$$Base);
    PROVIDE(__data_start__ = 0);
    PROVIDE(__data_end__   = 0);
    PROVIDE(__etext        = 0);
    PROVIDE(__Vectors = __vectors_start__);
    __tz_FLASH_C = ABSOLUTE(FLASH_NSC_START);
    __tz_FLASH_N = ABSOLUTE(FLASH_NS_START);
    __tz_FLASH_S = ABSOLUTE(FLASH_START);
    
    __tz_RAM_S = ORIGIN(RAM);
    __tz_RAM_C = ABSOLUTE(RAM_NSC_START);
    __tz_RAM_N = ABSOLUTE(RAM_NS_START);
    
    /* Note: There are no secure/non-secure boundaries for QSPI.  These symbols are provided for the RA configuration tool. */
    __tz_QSPI_FLASH_S = ORIGIN(QSPI_FLASH);
   
}

