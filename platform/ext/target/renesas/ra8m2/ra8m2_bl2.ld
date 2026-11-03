;/*
; * Copyright (c) 2022-2024 Arm Limited. All rights reserved.
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

/* Linker script to configure memory regions. */
/* This file will be run trough the pre-processor. */

#include "region_defs.h"

MEMORY
{
    FLASH (rx)  : ORIGIN = BL2_CODE_START, LENGTH = BL2_CODE_SIZE
    RAM   (rwx) : ORIGIN = BL2_DATA_START, LENGTH = BL2_DATA_SIZE

    /* RA8M2 option-setting memory: one discrete region per register group, as in FSP's
     * generated fsp_gen.ld MEMORY block. Each region is exactly its group's size, so ld emits a
     * separate PT_LOAD per group and cannot zero-fill the registers between them. Merging them
     * into one region or segment bricks the part, with no known recovery; see region_defs.h,
     * OPTION_SETTING_* block. */
    /* Exactly the groups this solution emits; see bl2_option_setting.c. Addresses come from
     * the generated option_settings.h, never from here.
     *
     * The device has 26 groups. The others are left out because a region with no section is
     * noise, and a section with no region is a hard ld error, which is how a carried-over
     * script should announce itself. To add a group: define BSP_CFG_OPTION_SETTING_* in e2,
     * add the emission in bl2_option_setting.c, then add a region here and a placement below,
     * and the same three in ra8m2_bl2.icf for IAR. The unused groups are
     *   OFS1        0x12c9f4c0   OFS3        0x12c9f4c4   SAS             0x02c9f074
     *   BPS         0x12c9f600   BPS_SEC     0x02c9f200   (0x80 each, not RA6's 0x10)
     *   OTP_FSBLCTRL0..2  0x02e07600/604/608   OTP_SAMR   0x02e07614
     *   OTP_SACC00..03    0x02e07620..680      OTP_SACC10..13  0x02e07630..690
     *   OTP_PBPS_SEC      0x02e17700   OTP_PBPS  0x12e17780   OTP_ZHUK  0x02e17920
     * OFS1, OFS3 and OTP_PBPS are non-secure alias addresses (0x12.../0x12e...). BL2 is built
     * flat, so check the alias before placing those from this image.
     *
     * RA6E1/RA6M5 have DUALSEL, BANKSEL and a non-OTP PBPS; this part has none of them. */
    OPTION_SETTING_OFS0        (r) : ORIGIN = OPTION_SETTING_OFS0_START,        LENGTH = OPTION_SETTING_OFS0_LENGTH
    OPTION_SETTING_OFS2        (r) : ORIGIN = OPTION_SETTING_OFS2_START,        LENGTH = OPTION_SETTING_OFS2_LENGTH
    OPTION_SETTING_OFS1_SEC    (r) : ORIGIN = OPTION_SETTING_OFS1_SEC_START,    LENGTH = OPTION_SETTING_OFS1_SEC_LENGTH
    OPTION_SETTING_OFS3_SEC    (r) : ORIGIN = OPTION_SETTING_OFS3_SEC_START,    LENGTH = OPTION_SETTING_OFS3_SEC_LENGTH
    OPTION_SETTING_OFS1_SEL    (r) : ORIGIN = OPTION_SETTING_OFS1_SEL_START,    LENGTH = OPTION_SETTING_OFS1_SEL_LENGTH
    OPTION_SETTING_OFS3_SEL    (r) : ORIGIN = OPTION_SETTING_OFS3_SEL_START,    LENGTH = OPTION_SETTING_OFS3_SEL_LENGTH
}

__heap_size__  = BL2_HEAP_SIZE;
__msp_stack_size__ = BL2_MSP_STACK_SIZE;

ENTRY(Reset_Handler)

SECTIONS
{
    .text (READONLY) :
    {
        KEEP(*(.vectors))
        __Vectors_End = .;
        __Vectors_Size = __Vectors_End - __Vectors;
        __end__ = .;

        *(.text*)

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

        /* .copy.table */
        . = ALIGN(4);
        __copy_table_start__ = .;
#ifdef CODE_SHARING
        LONG (LOADADDR(.tfm_shared_symbols))
        LONG (ADDR(.tfm_shared_symbols))
        LONG (SIZEOF(.tfm_shared_symbols) / 4)
#endif
        LONG (LOADADDR(.data))
        LONG (ADDR(.data))
        LONG (SIZEOF(.data) / 4)
        LONG (LOADADDR(.ram_from_flash))
        LONG (ADDR(.ram_from_flash))
        LONG (SIZEOF(.ram_from_flash) / 4)
        __copy_table_end__ = .;

        /* .zero.table */
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (ADDR(.bss))
        LONG (SIZEOF(.bss) / 4)
        __zero_table_end__ = .;

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

#ifdef CODE_SHARING
    /* The code sharing between bootloader and runtime firmware requires to
     * share the global variables. Section size must be equal with
     * SHARED_SYMBOL_AREA_SIZE defined in region_defs.h
     */
    .tfm_shared_symbols : ALIGN(4)
    {
        *(.data.mbedtls_calloc_func)
        *(.data.mbedtls_free_func)
        *(.data.mbedtls_exit)
        *(.data.memset_func)
        . = ALIGN(SHARED_SYMBOL_AREA_SIZE);
    } > RAM AT > FLASH

    ASSERT(SHARED_SYMBOL_AREA_SIZE % 4 == 0, "SHARED_SYMBOL_AREA_SIZE must be divisible by 4")
#endif

    .tfm_bl2_shared_data : ALIGN(32)
    {
        . += BOOT_TFM_SHARED_DATA_SIZE;
    } > RAM
    Image$$SHARED_DATA$$RW$$Base = ADDR(.tfm_bl2_shared_data);
    Image$$SHARED_DATA$$RW$$Limit = ADDR(.tfm_bl2_shared_data) + SIZEOF(.tfm_bl2_shared_data);

    /* FSP flash driver routines that must execute from RAM: inert on RA8M2.
     *
     * Nothing in r_mram carries PLACE_IN_RAM_SECTION. r_mram.h defines the macro (copied from
     * r_flash_hp.h, including a comment about a "Code Flash programming" option that
     * r_mram_cfg.h does not have), but r_mram.c never applies it, and the e2-built image shows
     * the block empty ("__ram_from_flash 0x2200'0000 0x0"). MRAM stays readable while it is
     * programmed, so FSP's driver executes in place. See region_defs.h, "Secure RAM".
     *
     * The wildcards below collect nothing, the section costs no RAM, and the copy-table entry
     * copies zero bytes. Both are kept so that the mechanism is ready if a future MRAM revision
     * needs it. For what the section does on RA6E1/RA6M5, see ra6m5_bl2.ld, .ram_from_flash.
     */
    .ram_from_flash : ALIGN(4)
    {
        __ram_from_flash_start__ = .;
        *(.ram_from_flash)
        *(.ram_from_flash.*)
        . = ALIGN(4);
        __ram_from_flash_end__ = .;
    } > RAM AT > FLASH

    .data : ALIGN(4)
    {
        *(vtable)
        *(.data*)

        KEEP(*(.jcr*))
        . = ALIGN(4);

    } > RAM AT > FLASH
    Image$$ER_DATA$$Base = ADDR(.data);

    /* FSP uninitialised RAM, handled as in FSP's own linker script.
     *
     * FSP places state that must survive the C-runtime init in .ram_noinit through
     * BSP_SECTION_NOINIT / BSP_SECTION_EARLY_INIT (for example SystemCoreClock,
     * g_clock_freq[], g_protect_counters[]). TF-M's startup calls SystemInit()
     * (bsp_clock_init) before __PROGRAM_START() runs the C-runtime init, so anything in .bss
     * is zeroed right after FSP computes it, and R_MRAM_Open() then sees FCLK == 0 and
     * fails with FSP_ERR_FCLK.
     *
     * Declared before .bss so that the prefixed variant (.bss.ram_noinit, used by toolchains
     * where BSP_UNINIT_SECTION_PREFIX is ".bss") matches here and not in *(.bss*). NOLOAD, so
     * no flash image is emitted and the C-runtime init neither copies nor zeroes it. The zero
     * table covers only ADDR(.bss)..SIZEOF(.bss), so this region is never cleared.
     *
     * SystemCoreClock and the other clock state carry the .ram_noinit attribute only when the
     * bootloader e2 project sets BSP_CFG_EARLY_INIT = 1; bl2_option_setting.c fails the build
     * otherwise (DESIGN.md §8.1). */
    .ram_noinit (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram_noinit_start__ = .;
        *(.bss.ram_noinit*)
        *(.ram_noinit*)
        . = ALIGN(4);
        __ram_noinit_end__ = .;
    } > RAM

    .bss : ALIGN(4)
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    .msp_stack (NOLOAD) : ALIGN(32)
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

#else
    .msp_stack (NOLOAD) : ALIGN(32)
    {
        . += __msp_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);

#endif /* defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */

    .heap (NOLOAD): ALIGN(8)
    {
        . = ALIGN(8);
        __end__ = .;
        PROVIDE(end = .);
        __HeapBase = .;
        . += __heap_size__;
        __HeapLimit = .;
        __heap_limit = .; /* Add for _sbrk */
    } > RAM
    Image$$ARM_LIB_HEAP$$ZI$$Limit = ADDR(.heap) + SIZEOF(.heap);

    PROVIDE(__stack = Image$$ARM_LIB_STACK$$ZI$$Limit);

    /* RA8M2 option-setting memory (OFS), BL2 only.
     *
     * Places the RASC-configured OFS words emitted by bl2_option_setting.c. Each section goes
     * to its own MEMORY region (see the MEMORY block above), exactly as FSP's generated
     * fsp_gen.ld does.
     *
     * WARNING: do not rewrite these as bare addressed sections:
     *       .option_setting_ofs0 0x02c9f040 : { KEEP(*(.option_setting_ofs0)) }
     *   With no '> REGION', ld coalesces them into one PT_LOAD spanning the whole range and
     *   zero-fills every register and reserved byte in the gaps. Flashing that from a debugger
     *   writes option words nobody chose (D002). The six groups placed here run from OFS0 at
     *   0x02c9f040 to OFS3_SEL at 0x02c9f124, 24 bytes placed in a 232-byte span, so the gaps
     *   are 208 bytes - SAS at 0x02c9f074 and 204 reserved. On this part the block-protect and
     *   permanence words are outside that span (BPS_SEC 0x02c9f200, BPS 0x12c9f600, PBPS_SEC
     *   0x02e17700, PBPS 0x12e17780); it was RA6M4's layout that put them in range (D133).
     *   The fill is invisible in the srec; check readelf -l.
     *   See region_defs.h, OPTION_SETTING_* block, and DESIGN.md §8.4.
     *
     * BL2 only: the secure and NS images are MCUboot-signed and imgtool needs a contiguous
     * payload, and two images programming OFS would collide. */
    .option_setting_ofs0     : { KEEP(*(.option_setting_ofs0))     } > OPTION_SETTING_OFS0
    .option_setting_ofs2     : { KEEP(*(.option_setting_ofs2))     } > OPTION_SETTING_OFS2
    .option_setting_ofs1_sec : { KEEP(*(.option_setting_ofs1_sec)) } > OPTION_SETTING_OFS1_SEC
    .option_setting_ofs3_sec : { KEEP(*(.option_setting_ofs3_sec)) } > OPTION_SETTING_OFS3_SEC
    .option_setting_ofs1_sel : { KEEP(*(.option_setting_ofs1_sel)) } > OPTION_SETTING_OFS1_SEL
    .option_setting_ofs3_sel : { KEEP(*(.option_setting_ofs3_sel)) } > OPTION_SETTING_OFS3_SEL
}
