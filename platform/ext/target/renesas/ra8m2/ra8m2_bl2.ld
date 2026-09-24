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

    /* RA8M2 option-setting memory - one DISCRETE region per register group, the
     * way FSP's own generated linker does it (fsp_gen.ld MEMORY block). Each
     * region is sized to exactly its register group, so ld emits a separate
     * PT_LOAD per group and CANNOT zero-fill the FCU config bytes in between.
     * Merging these into one region/segment permanently bricks the part - see
     * the OPTION_SETTING_* block in region_defs.h for the full explanation. */
    /* Exactly the groups this solution emits - see bl2_option_setting.c. Addresses come
     * from the generated option_settings.h, never from here.
     *
     * The device has 26 groups; the others are deliberately absent because a region with no
     * section is noise and a section with no region is a hard ld error, which is how a
     * carried-over script should announce itself. To add one: define BSP_CFG_OPTION_SETTING_*
     * in e2, add the emission in bl2_option_setting.c, then a region HERE and a placement
     * below - and the same three in ra8m2_bl2.icf for IAR. The unused ones are
     *   OFS1        0x12c9f4c0   OFS3        0x12c9f4c4   SAS             0x02c9f074
     *   BPS         0x12c9f600   BPS_SEC     0x02c9f200   (0x80 each, not RA6's 0x10)
     *   OTP_FSBLCTRL0..2  0x02e07600/604/608   OTP_SAMR   0x02e07614
     *   OTP_SACC00..03    0x02e07620..680      OTP_SACC10..13  0x02e07630..690
     *   OTP_PBPS_SEC      0x02e17700   OTP_PBPS  0x12e17780   OTP_ZHUK  0x02e17920
     * Note OFS1, OFS3 and OTP_PBPS are NON-SECURE alias addresses (0x12.../0x12e...);
     * BL2 is built flat, so think before placing those from this image.
     *
     * RA6E1/RA6M5 have DUALSEL, BANKSEL and a non-OTP PBPS. THIS PART HAS NONE OF THEM. */
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

    /* FSP flash driver routines that MUST execute from RAM.
     *
     * INERT ON RA8M2 - kept, not removed. This part uses r_mram, and NOTHING in it carries
     * PLACE_IN_RAM_SECTION: r_mram.h defines the macro (copied verbatim from r_flash_hp.h,
     * including a comment about a "Code Flash programming" option r_mram_cfg.h does not
     * have) but r_mram.c never applies it, and the e2-built image shows the block empty
     * ("__ram_from_flash 0x2200'0000 0x0"). MRAM does not go unreadable while being
     * programmed, so FSP's driver executes in place. See region_defs.h.
     *
     * The wildcards below therefore collect nothing and the section costs no RAM, and the
     * copy-table entry copies zero bytes. Both stay so that the mechanism is one commit away
     * if a future MRAM revision needs it - and because the reasoning below is why it exists
     * at all, which is worth keeping rather than rediscovering.
     *
     * WHAT IT IS FOR, on the parts that need it (RA6E1, RA6M5):
     *
     * r_flash_hp.c marks flash_hp_erase, flash_hp_cf_write, flash_hp_pe_mode_exit,
     * flash_hp_status_clear and the delay loop PLACE_IN_RAM_SECTION, which resolves to
     * BSP_PLACE_IN_SECTION(".ram_from_flash"). The reason is hardware: while the FCU is in
     * program/erase mode for CODE flash, code flash is not readable. Any instruction fetched
     * from it in that window returns garbage, so the routines driving the operation cannot
     * live there.
     *
     * Without this section the linker gave .ram_from_flash a plain flash VMA next to .text
     * (0x0000D160), because this file never named it and FSP's own bsp_linker.c - which would
     * have relocated it - is excluded from the build. Data flash was unaffected, since
     * data-flash P/E leaves code flash readable; that is why NV counters, ITS and PS all
     * worked. Code flash P/E is undefined behaviour in that state: on 2026-08-30 BL2 erased
     * image 1's slot, then died partway through image 0's erase, leaving 0x58000 blank and
     * the CPU executing erased flash. Surviving one erase and not the next is the expected
     * signature - it depends on what is in the prefetch buffer and flash cache at the time.
     *
     * Kept out of .data because it is executable (the RAM region is declared rwx above), and
     * copied by the CMSIS copy-table entry added beside .data's, which TF-M's startup already
     * runs. That deliberately avoids adding a MEMORY region or PT_LOAD - see the
     * OPTION_SETTING note below for why segment layout in this file is handled with care.
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

    /* RA8M2 option-setting memory (OFS) - BL2 ONLY.
     *
     * Places the RASC-configured OFS words emitted by bl2_option_setting.c. Each
     * section goes to its OWN MEMORY region (see the MEMORY block above) - this is
     * the whole point, and it mirrors FSP's generated fsp_gen.ld exactly.
     *
     * ⚠ DO NOT rewrite these as bare addressed sections:
     *       .option_setting_ofs0 0x02c9f040 : { KEEP(*(.option_setting_ofs0)) }
     *   With no '> REGION', ld coalesces them into ONE PT_LOAD spanning the whole
     *   range and zero-fills the FCU configuration bytes in the gaps, including the
     *   block-protect permanence word. Flashing that from a debugger permanently
     *   bricks the part - it cost two EK-RA6M4 boards on 2026-07-21. The gaps are
     *   wider here than on RA6: OFS0 is 0x02c9f040 and OFS3_SEL 0x02c9f124, with
     *   BPS_SEC's 0x80 and the whole OTP block in between.
     *   The zero fill is invisible in the srec; check readelf -l. Full rationale
     *   and the verification command are in region_defs.h (OPTION_SETTING_* block).
     *
     * BL2-only: the secure/NS images are MCUboot-signed and imgtool needs a
     * contiguous payload, and two images programming OFS would collide. */
    .option_setting_ofs0     : { KEEP(*(.option_setting_ofs0))     } > OPTION_SETTING_OFS0
    .option_setting_ofs2     : { KEEP(*(.option_setting_ofs2))     } > OPTION_SETTING_OFS2
    .option_setting_ofs1_sec : { KEEP(*(.option_setting_ofs1_sec)) } > OPTION_SETTING_OFS1_SEC
    .option_setting_ofs3_sec : { KEEP(*(.option_setting_ofs3_sec)) } > OPTION_SETTING_OFS3_SEC
    .option_setting_ofs1_sel : { KEEP(*(.option_setting_ofs1_sel)) } > OPTION_SETTING_OFS1_SEL
    .option_setting_ofs3_sel : { KEEP(*(.option_setting_ofs3_sel)) } > OPTION_SETTING_OFS3_SEL
}
