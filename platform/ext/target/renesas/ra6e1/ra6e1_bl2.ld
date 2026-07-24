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

    /* RA6E1 option-setting (OFS) memory - ONE region PER WORD, exactly as FSP's
     * generated fsp_gen.ld does it. This is safety-critical: a separate, exactly-
     * sized MEMORY region per option word forces GNU ld to emit a SEPARATE PT_LOAD
     * segment for each (4 or 12 bytes), with NO span across the gaps between them.
     * A tool that flashes by program header then writes ONLY the configured words.
     * The previous (bricking) layout placed these at absolute addresses in the
     * default region, so ld coalesced them into one 0x184 segment spanning
     * 0x0100A100-0x0100A284 and ZERO-FILLED the gaps - flashing that programmed
     * PBPS(0x0100A1E0)=0 => permanent block protect => bricked two boards.
     * See DESIGN.md 8.4. Addresses are the fixed RA6E1 config-area map. */
    OPTION_SETTING_OFS0        (r) : ORIGIN = 0x0100A100, LENGTH = 0x04
    OPTION_SETTING_DUALSEL     (r) : ORIGIN = 0x0100A110, LENGTH = 0x04
    OPTION_SETTING_OFS1        (r) : ORIGIN = 0x0100A180, LENGTH = 0x04
    OPTION_SETTING_BANKSEL     (r) : ORIGIN = 0x0100A190, LENGTH = 0x04
    OPTION_SETTING_BPS         (r) : ORIGIN = 0x0100A1C0, LENGTH = 0x0C
    OPTION_SETTING_PBPS        (r) : ORIGIN = 0x0100A1E0, LENGTH = 0x0C
    OPTION_SETTING_OFS1_SEC    (r) : ORIGIN = 0x0100A200, LENGTH = 0x04
    OPTION_SETTING_BANKSEL_SEC (r) : ORIGIN = 0x0100A210, LENGTH = 0x04
    OPTION_SETTING_BPS_SEC     (r) : ORIGIN = 0x0100A240, LENGTH = 0x0C
    OPTION_SETTING_PBPS_SEC    (r) : ORIGIN = 0x0100A260, LENGTH = 0x0C
    OPTION_SETTING_OFS1_SEL    (r) : ORIGIN = 0x0100A280, LENGTH = 0x04
    OPTION_SETTING_BANKSEL_SEL (r) : ORIGIN = 0x0100A290, LENGTH = 0x04
    OPTION_SETTING_BPS_SEL     (r) : ORIGIN = 0x0100A2C0, LENGTH = 0x0C
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

    .data : ALIGN(4)
    {
        *(vtable)
        *(.data*)

        KEEP(*(.jcr*))
        . = ALIGN(4);

    } > RAM AT > FLASH
    Image$$ER_DATA$$Base = ADDR(.data);

    /* FSP uninitialised RAM - handled the same way FSP's own linker does.
     *
     * FSP places state that must survive the C-runtime init in .ram_noinit via
     * BSP_SECTION_NOINIT / BSP_SECTION_EARLY_INIT (e.g. SystemCoreClock,
     * g_clock_freq[], g_protect_counters[]). TF-M's startup calls SystemInit()
     * (bsp_clock_init) BEFORE __PROGRAM_START() does the C-runtime init, so
     * anything that lands in .bss is zeroed immediately after FSP computed it -
     * which made R_FLASH_HP_Open() see FCLK == 0 and fail with FSP_ERR_FCLK.
     *
     * Declared BEFORE .bss so the prefixed variant (.bss.ram_noinit, used by
     * toolchains where BSP_UNINIT_SECTION_PREFIX is ".bss") is matched here and
     * not swallowed by *(.bss*), and NOLOAD so no flash image is emitted and the
     * C-runtime init neither copies nor zeroes it. Because the zero table only
     * covers ADDR(.bss)..SIZEOF(.bss), this region is never cleared.
     *
     * NOTE: for SystemCoreClock et al. to actually carry the .ram_noinit
     * attribute, the RASC config must have BSP_CFG_EARLY_INIT = 1. With
     * BSP_CFG_EARLY_INIT = 0 they have no section attribute and still land in
     * .bss - the platform code additionally calls SystemCoreClockUpdate()
     * defensively so the port is correct either way. */
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

    /* RA6E1 option-setting (OFS) - BL2 ONLY, placed EXACTLY as FSP's generated
     * fsp_gen.ld does: each option word into its OWN, exactly-sized MEMORY region
     * (declared above), so ld emits a SEPARATE PT_LOAD segment per word and NEVER
     * spans/zero-fills the gaps. Only words actually emitted by bl2_option_setting.c
     * (i.e. configured via BSP_CFG_OPTION_SETTING_* in RASC) produce a segment;
     * unconfigured ones stay empty and emit nothing - matching FSP, where fsp_gen.ld
     * only contains entries for configured OFS. DO NOT collapse these into one region
     * or place them at absolute addresses in a shared region - that reintroduces the
     * gap-filled span that bricked two boards (DESIGN.md 8.4). */
    .option_setting_ofs0        : { KEEP(*(.option_setting_ofs0))        } > OPTION_SETTING_OFS0
    .option_setting_dualsel     : { KEEP(*(.option_setting_dualsel))     } > OPTION_SETTING_DUALSEL
    .option_setting_ofs1        : { KEEP(*(.option_setting_ofs1))        } > OPTION_SETTING_OFS1
    .option_setting_banksel     : { KEEP(*(.option_setting_banksel))     } > OPTION_SETTING_BANKSEL
    .option_setting_bps         : { KEEP(*(.option_setting_bps))         } > OPTION_SETTING_BPS
    .option_setting_pbps        : { KEEP(*(.option_setting_pbps))        } > OPTION_SETTING_PBPS
    .option_setting_ofs1_sec    : { KEEP(*(.option_setting_ofs1_sec))    } > OPTION_SETTING_OFS1_SEC
    .option_setting_banksel_sec : { KEEP(*(.option_setting_banksel_sec)) } > OPTION_SETTING_BANKSEL_SEC
    .option_setting_bps_sec     : { KEEP(*(.option_setting_bps_sec))     } > OPTION_SETTING_BPS_SEC
    .option_setting_pbps_sec    : { KEEP(*(.option_setting_pbps_sec))    } > OPTION_SETTING_PBPS_SEC
    .option_setting_ofs1_sel    : { KEEP(*(.option_setting_ofs1_sel))    } > OPTION_SETTING_OFS1_SEL
    .option_setting_banksel_sel : { KEEP(*(.option_setting_banksel_sel)) } > OPTION_SETTING_BANKSEL_SEL
    .option_setting_bps_sel     : { KEEP(*(.option_setting_bps_sel))     } > OPTION_SETTING_BPS_SEL
}
