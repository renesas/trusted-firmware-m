;
DATA_FLASH_START = 0x08000000;
DATA_FLASH_LENGTH = 0x0001000;
QSPI_FLASH_START = 0x60000000;
QSPI_FLASH_LENGTH = 0x4000000;
SDRAM_START = 0x90000000;
SDRAM_LENGTH = 0x1000000;
OPTION_SETTING_START = 0x0100A100;
OPTION_SETTING_LENGTH = 0x100;
OPTION_SETTING_S_START = 0x0100A200;
OPTION_SETTING_S_LENGTH = 0x100;
FLASH_NSC_START = 0x7F800;
FLASH_NS_START = 0x80000;
RAM_NSC_START = 0x2001FC00;
RAM_NS_START = 0x20020000;
DATA_FLASH_NS_START = 0x08001000;
OPTION_SETTING_START_NS = 0x0100A180;
OPTION_SETTING_START_S = 0x0100A100;
MEMORY
{
  FLASH (rx) : ORIGIN = (((0x00000000) + (((0x0) + (0x0))))), LENGTH = (((0x30800) - (0x0) - (0x800)) - (0x400))
  RAM (rwx) : ORIGIN = (((0x20000000) + (0x0))), LENGTH = ((0x40000) / 2)
  CODE_RAM (rwx) : ORIGIN = (0x00058000), LENGTH = (0x00040000)
  VENEERS (rx) : ORIGIN = (((((0x00000000) + (((0x0) + (0x0))))) + (((0x30800) - (0x0) - (0x800)) - (0x400)) - 1) + 1), LENGTH = (0x400)
  DATA_FLASH (rx) : ORIGIN = DATA_FLASH_START, LENGTH = DATA_FLASH_LENGTH
  OPTION_SETTING (r): ORIGIN = OPTION_SETTING_START, LENGTH = OPTION_SETTING_LENGTH
  OPTION_SETTING_S (r): ORIGIN = OPTION_SETTING_S_START, LENGTH = OPTION_SETTING_S_LENGTH
}
__heap_size__ = (0x0001000);
__psp_stack_size__ = (0x0000800);
__msp_init_stack_size__ = (0x0000400);
GROUP(libgcc.a libc.a libm.a libnosys.a libc_nano.a)
ENTRY(Reset_Handler)
SECTIONS
{
    __TZ_NS_PROJECT = LENGTH(OPTION_SETTING) && (ABSOLUTE(OPTION_SETTING_START_S) != ORIGIN(OPTION_SETTING));
    __TZ_S_PROJECT = LENGTH(OPTION_SETTING) && (ABSOLUTE(OPTION_SETTING_START_S) == ORIGIN(OPTION_SETTING));
    __tz_OPTION_SETTING_S = ORIGIN(OPTION_SETTING);
    .option_setting :
    {
        __OPTION_SETTING_Start = .;
        KEEP(*(.option_setting_ofs0))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_Start + 0x10 : __OPTION_SETTING_Start;
        KEEP(*(.option_setting_dualsel))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_Start + 0x34 : __OPTION_SETTING_Start;
        KEEP(*(.option_setting_sas))
        __OPTION_SETTING_End = .;
    } > OPTION_SETTING = 0xFF
    __tz_OPTION_SETTING_N = OPTION_SETTING_START_NS;
    .option_setting_ns :
    {
        __OPTION_SETTING_NS_Start = .;
        KEEP(*(.option_setting_ofs1))
        . = __TZ_NS_PROJECT ? __OPTION_SETTING_NS_Start + 0x10 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_banksel))
        . = __TZ_NS_PROJECT ? __OPTION_SETTING_NS_Start + 0x40 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_bps))
        . = __TZ_NS_PROJECT ? __OPTION_SETTING_NS_Start + 0x60 : __OPTION_SETTING_NS_Start;
        KEEP(*(.option_setting_pbps))
        __OPTION_SETTING_NS_End = .;
    } > OPTION_SETTING = 0xFF
    __tz_OPTION_SETTING_S_S = ORIGIN(OPTION_SETTING_S);
    .option_setting_s :
    {
        __OPTION_SETTING_S_Start = .;
        KEEP(*(.option_setting_ofs1_sec))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0x10 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_banksel_sec))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0x40 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sec))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0x60 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_pbps_sec))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0x80 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_ofs1_sel))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0x90 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_banksel_sel))
        . = __TZ_S_PROJECT ? __OPTION_SETTING_S_Start + 0xC0 : __OPTION_SETTING_S_Start;
        KEEP(*(.option_setting_bps_sel))
        __OPTION_SETTING_S_End = .;
    } > OPTION_SETTING_S = 0xFF
    __tz_OPTION_SETTING_S_N = __OPTION_SETTING_S_End;
    .TFM_VECTORS : ALIGN(4)
    {
        __vectors_start__ = .;
        KEEP(*(.vectors))
        *startup*(.text*)
        . = ALIGN(4);
        __vectors_end__ = .;
    } > FLASH
    .copy.table : ALIGN(4)
    {
        __copy_table_start__ = .;
        LONG (LOADADDR(.TFM_DATA))
        LONG (ADDR(.TFM_DATA))
        LONG (SIZEOF(.TFM_DATA))
        LONG (LOADADDR(.TFM_UNPRIV_DATA))
        LONG (ADDR(.TFM_UNPRIV_DATA))
        LONG (SIZEOF(.TFM_UNPRIV_DATA))
        LONG (LOADADDR(.TFM_SP_STORAGE_LINKER_DATA))
        LONG (ADDR(.TFM_SP_STORAGE_LINKER_DATA))
        LONG (SIZEOF(.TFM_SP_STORAGE_LINKER_DATA))
        LONG (LOADADDR(.TFM_SP_ITS_LINKER_DATA))
        LONG (ADDR(.TFM_SP_ITS_LINKER_DATA))
        LONG (SIZEOF(.TFM_SP_ITS_LINKER_DATA))
        LONG (LOADADDR(.TFM_SP_CRYPTO_LINKER_DATA))
        LONG (ADDR(.TFM_SP_CRYPTO_LINKER_DATA))
        LONG (SIZEOF(.TFM_SP_CRYPTO_LINKER_DATA))
        LONG (LOADADDR(.TFM_SP_PLATFORM_LINKER_DATA))
        LONG (ADDR(.TFM_SP_PLATFORM_LINKER_DATA))
        LONG (SIZEOF(.TFM_SP_PLATFORM_LINKER_DATA))
        LONG (LOADADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA))
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA))
        LONG (LOADADDR(.ER_CODE_SRAM))
        LONG (ADDR(.ER_CODE_SRAM))
        LONG (SIZEOF(.ER_CODE_SRAM))
        __copy_table_end__ = .;
    } > FLASH
    .zero.table : ALIGN(4)
    {
        __zero_table_start__ = .;
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS))
        LONG (ADDR(.TFM_SP_STORAGE_LINKER_BSS))
        LONG (SIZEOF(.TFM_SP_STORAGE_LINKER_BSS))
        LONG (ADDR(.TFM_SP_STORAGE_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_STORAGE_LINKER_STACK))
        LONG (ADDR(.TFM_SP_ITS_LINKER_BSS))
        LONG (SIZEOF(.TFM_SP_ITS_LINKER_BSS))
        LONG (ADDR(.TFM_SP_ITS_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_ITS_LINKER_STACK))
        LONG (ADDR(.TFM_SP_CRYPTO_LINKER_BSS))
        LONG (SIZEOF(.TFM_SP_CRYPTO_LINKER_BSS))
        LONG (ADDR(.TFM_SP_CRYPTO_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_CRYPTO_LINKER_STACK))
        LONG (ADDR(.TFM_SP_PLATFORM_LINKER_BSS))
        LONG (SIZEOF(.TFM_SP_PLATFORM_LINKER_BSS))
        LONG (ADDR(.TFM_SP_PLATFORM_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_PLATFORM_LINKER_STACK))
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_BSS))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_BSS))
        LONG (ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK))
        LONG (SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK))
        __zero_table_end__ = .;
    } > FLASH
    .TFM_UNPRIV_CODE : ALIGN(32)
    {
        *libc_nano*:*(.text*)
        *libc_nano*:*(.rodata*)
        *tfm_spm_services.o(.text*)
        *tfm_spm_services.o(.rodata*)
        *platform_retarget_dev.o(.text*)
        *platform_retarget_dev.o(.rodata*)
        *device_definition.o(.text*)
        *device_definition.o(.rodata*)
        *(SFN)
        *libgcc*:*(.text*)
        *libgcc*:*(.rodata*)
        *libtfmsprt*:*(.text*)
        *libtfmsprt*:*(.rodata*)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_UNPRIV_CODE$$RO$$Base = ADDR(.TFM_UNPRIV_CODE);
    Image$$TFM_UNPRIV_CODE$$RO$$Limit = ADDR(.TFM_UNPRIV_CODE) + SIZEOF(.TFM_UNPRIV_CODE);
    Image$$TFM_PSA_CODE_START$$Base = .;
    .TFM_SP_STORAGE_LINKER : ALIGN(32)
    {
        *tfm_storage*:*(.text*)
        *tfm_storage*:*(.rodata*)
        *(TFM_SP_STORAGE_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_STORAGE_LINKER$$RO$$Base = ADDR(.TFM_SP_STORAGE_LINKER);
    Image$$TFM_SP_STORAGE_LINKER$$RO$$Limit = ADDR(.TFM_SP_STORAGE_LINKER) + SIZEOF(.TFM_SP_STORAGE_LINKER);
    Image$$TFM_SP_STORAGE_LINKER$$Base = ADDR(.TFM_SP_STORAGE_LINKER);
    Image$$TFM_SP_STORAGE_LINKER$$Limit = ADDR(.TFM_SP_STORAGE_LINKER) + SIZEOF(.TFM_SP_STORAGE_LINKER);
    .TFM_SP_ITS_LINKER : ALIGN(32)
    {
        *tfm_internal_trusted_storage*:*(.text*)
        *tfm_internal_trusted_storage*:*(.rodata*)
        *(TFM_SP_ITS_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_ITS_LINKER$$RO$$Base = ADDR(.TFM_SP_ITS_LINKER);
    Image$$TFM_SP_ITS_LINKER$$RO$$Limit = ADDR(.TFM_SP_ITS_LINKER) + SIZEOF(.TFM_SP_ITS_LINKER);
    Image$$TFM_SP_ITS_LINKER$$Base = ADDR(.TFM_SP_ITS_LINKER);
    Image$$TFM_SP_ITS_LINKER$$Limit = ADDR(.TFM_SP_ITS_LINKER) + SIZEOF(.TFM_SP_ITS_LINKER);
    .TFM_SP_CRYPTO_LINKER : ALIGN(32)
    {
        *tfm_crypto*:*(.text*)
        *tfm_crypto*:*(.rodata*)
        *(TFM_SP_CRYPTO_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_CRYPTO_LINKER$$RO$$Base = ADDR(.TFM_SP_CRYPTO_LINKER);
    Image$$TFM_SP_CRYPTO_LINKER$$RO$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER) + SIZEOF(.TFM_SP_CRYPTO_LINKER);
    Image$$TFM_SP_CRYPTO_LINKER$$Base = ADDR(.TFM_SP_CRYPTO_LINKER);
    Image$$TFM_SP_CRYPTO_LINKER$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER) + SIZEOF(.TFM_SP_CRYPTO_LINKER);
    .TFM_SP_PLATFORM_LINKER : ALIGN(32)
    {
        *tfm_platform*:*(.text*)
        *tfm_platform*:*(.rodata*)
        *(TFM_SP_PLATFORM_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_PLATFORM_LINKER$$RO$$Base = ADDR(.TFM_SP_PLATFORM_LINKER);
    Image$$TFM_SP_PLATFORM_LINKER$$RO$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER) + SIZEOF(.TFM_SP_PLATFORM_LINKER);
    Image$$TFM_SP_PLATFORM_LINKER$$Base = ADDR(.TFM_SP_PLATFORM_LINKER);
    Image$$TFM_SP_PLATFORM_LINKER$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER) + SIZEOF(.TFM_SP_PLATFORM_LINKER);
    .TFM_SP_INITIAL_ATTESTATION_LINKER : ALIGN(32)
    {
        *tfm_attest*:*(.text*)
        *tfm_attest*:*(.rodata*)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_FN)
        . = ALIGN(32);
    } > FLASH
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER$$RO$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER$$RO$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER);
    Image$$TFM_PSA_CODE_END$$Base = .;
    Image$$TFM_APP_CODE_START$$Base = .;
    Image$$TFM_APP_CODE_END$$Base = .;
    .ER_CODE_SRAM : ALIGN(4)
    {
        *Driver_GFC100_EFlash.o(.text*)
        *Driver_GFC100_EFlash.o(.rodata*)
        *gfc100_eflash_drv.o(.text*)
        *gfc100_eflash_drv.o(.rodata*)
        *musca_b1_eflash_drv.o(.text*)
        *musca_b1_eflash_drv.o(.rodata*)
        . = ALIGN(4);
    } > CODE_RAM AT > FLASH
    Image$$ER_CODE_SRAM$$RO$$Base = ADDR(.ER_CODE_SRAM);
    Image$$ER_CODE_SRAM$$RO$$Limit = ADDR(.ER_CODE_SRAM) + SIZEOF(.ER_CODE_SRAM);
    Image$$ER_CODE_SRAM$$Base = ADDR(.ER_CODE_SRAM);
    Image$$ER_CODE_SRAM$$Limit = ADDR(.ER_CODE_SRAM) + SIZEOF(.ER_CODE_SRAM);
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
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
         *crtbegin.o(.dtors)
         *crtbegin?.o(.dtors)
         *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
         *(SORT(.dtors.*))
         *(.dtors)
        *(.rodata*)
        KEEP(*(.eh_frame*))
    } > FLASH
    .tfm_secure_data_start :
    {
        . = ABSOLUTE((((0x20000000) + (0x0)))) ;
    } > RAM
    .tfm_bl2_shared_data : ALIGN(32)
    {
        . += (0x400);
    } > RAM
    .msp_stack : ALIGN(32)
    {
        . += __msp_init_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK_MSP$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK_MSP$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);
    .psp_stack : ALIGN(32)
    {
        . += __psp_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.psp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.psp_stack) + SIZEOF(.psp_stack);
    .TFM_UNPRIV_DATA : ALIGN(32)
    {
        */tfm_spm_services.o(.data*)
        */platform_retarget_dev.o(.data*)
        */device_definition.o(.data*)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_UNPRIV_DATA$$RW$$Base = ADDR(.TFM_UNPRIV_DATA);
    Image$$TFM_UNPRIV_DATA$$RW$$Limit = ADDR(.TFM_UNPRIV_DATA) + SIZEOF(.TFM_UNPRIV_DATA);
    .TFM_UNPRIV_BSS : ALIGN(32)
    {
        */tfm_spm_services.o(.bss*)
        */platform_retarget_dev.o(.bss*)
        */device_definition.o(.bss*)
        */tfm_spm_services.o(COMMON)
        */platform_retarget_dev.o(COMMON)
        */device_definition.o(COMMON)
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_UNPRIV_DATA$$ZI$$Base = ADDR(.TFM_UNPRIV_BSS);
    Image$$TFM_UNPRIV_DATA$$ZI$$Limit = ADDR(.TFM_UNPRIV_BSS) + SIZEOF(.TFM_UNPRIV_BSS);
    Image$$TFM_APP_RW_STACK_START$$Base = .;
    Image$$TFM_APP_RW_STACK_END$$Base = .;
    .heap : ALIGN(8)
    {
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += __heap_size__;
        __HeapLimit = .;
        __heap_limit = .;
    } > RAM AT> RAM
    Image$$TFM_PSA_RW_STACK_START$$Base = .;
    .TFM_SP_STORAGE_LINKER_DATA : ALIGN(32)
    {
        *tfm_storage*:*(.data*)
        *(TFM_SP_STORAGE_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_SP_STORAGE_LINKER_DATA$$RW$$Base = ADDR(.TFM_SP_STORAGE_LINKER_DATA);
    Image$$TFM_SP_STORAGE_LINKER_DATA$$RW$$Limit = ADDR(.TFM_SP_STORAGE_LINKER_DATA) + SIZEOF(.TFM_SP_STORAGE_LINKER_DATA);
    .TFM_SP_STORAGE_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_SP_STORAGE_LINKER = .;
        *tfm_storage*:*(.bss*)
        *tfm_storage*:*(COMMON)
        *(TFM_SP_STORAGE_ATTR_ZI)
        . += (. - start_of_TFM_SP_STORAGE_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_SP_STORAGE_LINKER_DATA$$ZI$$Base = ADDR(.TFM_SP_STORAGE_LINKER_BSS);
    Image$$TFM_SP_STORAGE_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_SP_STORAGE_LINKER_BSS) + SIZEOF(.TFM_SP_STORAGE_LINKER_BSS);
    .TFM_SP_STORAGE_LINKER_STACK : ALIGN(128)
    {
        . += 0x600;
    } > RAM
    Image$$TFM_SP_STORAGE_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_STORAGE_LINKER_STACK);
    Image$$TFM_SP_STORAGE_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_STORAGE_LINKER_STACK) + SIZEOF(.TFM_SP_STORAGE_LINKER_STACK);
    .TFM_SP_ITS_LINKER_DATA : ALIGN(32)
    {
        *tfm_internal_trusted_storage*:*(.data*)
        *(TFM_SP_ITS_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_SP_ITS_LINKER_DATA$$RW$$Base = ADDR(.TFM_SP_ITS_LINKER_DATA);
    Image$$TFM_SP_ITS_LINKER_DATA$$RW$$Limit = ADDR(.TFM_SP_ITS_LINKER_DATA) + SIZEOF(.TFM_SP_ITS_LINKER_DATA);
    .TFM_SP_ITS_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_SP_ITS_LINKER = .;
        *tfm_internal_trusted_storage*:*(.bss*)
        *tfm_internal_trusted_storage*:*(COMMON)
        *(TFM_SP_ITS_ATTR_ZI)
        . += (. - start_of_TFM_SP_ITS_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_SP_ITS_LINKER_DATA$$ZI$$Base = ADDR(.TFM_SP_ITS_LINKER_BSS);
    Image$$TFM_SP_ITS_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_SP_ITS_LINKER_BSS) + SIZEOF(.TFM_SP_ITS_LINKER_BSS);
    .TFM_SP_ITS_LINKER_STACK : ALIGN(128)
    {
        . += 0x680;
    } > RAM
    Image$$TFM_SP_ITS_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_ITS_LINKER_STACK);
    Image$$TFM_SP_ITS_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_ITS_LINKER_STACK) + SIZEOF(.TFM_SP_ITS_LINKER_STACK);
    .TFM_SP_CRYPTO_LINKER_DATA : ALIGN(32)
    {
        *tfm_crypto*:*(.data*)
        *(TFM_SP_CRYPTO_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_SP_CRYPTO_LINKER_DATA$$RW$$Base = ADDR(.TFM_SP_CRYPTO_LINKER_DATA);
    Image$$TFM_SP_CRYPTO_LINKER_DATA$$RW$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER_DATA) + SIZEOF(.TFM_SP_CRYPTO_LINKER_DATA);
    .TFM_SP_CRYPTO_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_SP_CRYPTO_LINKER = .;
        *tfm_crypto*:*(.bss*)
        *tfm_crypto*:*(COMMON)
        *(TFM_SP_CRYPTO_ATTR_ZI)
        . += (. - start_of_TFM_SP_CRYPTO_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_SP_CRYPTO_LINKER_DATA$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_LINKER_BSS);
    Image$$TFM_SP_CRYPTO_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER_BSS) + SIZEOF(.TFM_SP_CRYPTO_LINKER_BSS);
    .TFM_SP_CRYPTO_LINKER_STACK : ALIGN(128)
    {
        . += 0x2000;
    } > RAM
    Image$$TFM_SP_CRYPTO_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_CRYPTO_LINKER_STACK);
    Image$$TFM_SP_CRYPTO_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_CRYPTO_LINKER_STACK) + SIZEOF(.TFM_SP_CRYPTO_LINKER_STACK);
    .TFM_SP_PLATFORM_LINKER_DATA : ALIGN(32)
    {
        *tfm_platform*:*(.data*)
        *(TFM_SP_PLATFORM_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_SP_PLATFORM_LINKER_DATA$$RW$$Base = ADDR(.TFM_SP_PLATFORM_LINKER_DATA);
    Image$$TFM_SP_PLATFORM_LINKER_DATA$$RW$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER_DATA) + SIZEOF(.TFM_SP_PLATFORM_LINKER_DATA);
    .TFM_SP_PLATFORM_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_SP_PLATFORM_LINKER = .;
        *tfm_platform*:*(.bss*)
        *tfm_platform*:*(COMMON)
        *(TFM_SP_PLATFORM_ATTR_ZI)
        . += (. - start_of_TFM_SP_PLATFORM_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_SP_PLATFORM_LINKER_DATA$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_LINKER_BSS);
    Image$$TFM_SP_PLATFORM_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER_BSS) + SIZEOF(.TFM_SP_PLATFORM_LINKER_BSS);
    .TFM_SP_PLATFORM_LINKER_STACK : ALIGN(128)
    {
        . += 0x0400;
    } > RAM
    Image$$TFM_SP_PLATFORM_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_PLATFORM_LINKER_STACK);
    Image$$TFM_SP_PLATFORM_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_PLATFORM_LINKER_STACK) + SIZEOF(.TFM_SP_PLATFORM_LINKER_STACK);
    .TFM_SP_INITIAL_ATTESTATION_LINKER_DATA : ALIGN(32)
    {
        *tfm_attest*:*(.data*)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_DATA$$RW$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_DATA$$RW$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_DATA);
    .TFM_SP_INITIAL_ATTESTATION_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_SP_INITIAL_ATTESTATION_LINKER = .;
        *tfm_attest*:*(.bss*)
        *tfm_attest*:*(COMMON)
        *(TFM_SP_INITIAL_ATTESTATION_ATTR_ZI)
        . += (. - start_of_TFM_SP_INITIAL_ATTESTATION_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_DATA$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_BSS);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_BSS) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_BSS);
    .TFM_SP_INITIAL_ATTESTATION_LINKER_STACK : ALIGN(128)
    {
        . += 0x0A80;
    } > RAM
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_STACK$$ZI$$Base = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK);
    Image$$TFM_SP_INITIAL_ATTESTATION_LINKER_STACK$$ZI$$Limit = ADDR(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK) + SIZEOF(.TFM_SP_INITIAL_ATTESTATION_LINKER_STACK);
    Image$$TFM_PSA_RW_STACK_END$$Base = .;
    .TFM_DATA : ALIGN(4)
    {
        *(.data*)
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
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
    .gnu.sgstubs : ALIGN(32)
    {
        *(.gnu.sgstubs*)
        . = ALIGN(32);
    } > VENEERS AT> VENEERS
    Load$$LR$$LR_VENEER$$Base = ADDR(.gnu.sgstubs);
    Load$$LR$$LR_VENEER$$Limit = ADDR(.gnu.sgstubs) + SIZEOF(.gnu.sgstubs);
    Load$$LR$$LR_NS_PARTITION$$Base = (((0x00000000) + ((((0x0) + (0x20000)) + (0x30800)))));
    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);
}
