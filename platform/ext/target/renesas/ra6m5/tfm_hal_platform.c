/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tfm_hal_platform.h"
#include <string.h>
#include "target_cfg.h"
#include "region_defs.h"
#include "bsp_api.h"
#include "cmsis.h"
#include "tfm_platform_system.h"
#include "fih.h"
#include "uart_stdout.h"

/* The linker script provides __StackSeal; TF-M's SPM expects __STACK_SEAL. */
extern uint64_t __StackSeal;

/* Weak, and in the same section as __StackSeal. */
__attribute__((weak, section(".msp_stack_seal_res")))
uint64_t __STACK_SEAL = 0xFEF5EDA5FEF5EDA5ULL;

/* Everything below is secure-side HAL. BL2 links this file only for __STACK_SEAL above,
 * which ra6m5_bl2.ld needs to resolve FSP's __StackSeal, and calls none of it.
 *
 * The guard is required. tfm_hal_platform_init() returns fih_int_encode() as FIH_RET_TYPE,
 * which compiles only while fih_ret and fih_int are one type. In BL2 the mcuboot fih.h is in
 * scope, and at MCUBOOT_FIH_PROFILE MEDIUM (selected by TFM_PROFILE=profile_large) they differ:
 *     error: incompatible types when returning type 'fih_int' but 'fih_ret'
 *            {aka 'volatile int'} was expected
 *
 * Guard on RA6M5_BUILDING_BL2, which CMakeLists.txt sets on platform_bl2 only. The plain BL2
 * macro is defined for every image when BL2 is enabled, so guarding on it would also remove
 * these symbols from tfm_s:
 *     undefined reference to `tfm_hal_platform_init' (secure_fw/spm/core/main.c)
 */
#ifndef RA6M5_BUILDING_BL2

#if defined(__ICCARM__)
/*
 * Copy .ram_from_flash to RAM. DECISIONS D048, which closes D029.
 *
 * FSP's code-flash program/erase routines must run from RAM: the FCU makes code flash
 * unreadable during a P/E operation, so a routine fetching from flash hangs mid-erase with the
 * FCU still in P/E mode, without a fault.
 *
 * The ICF declares the section 'initialize manually' and the copy is done here, identically at
 * every isolation level. Asked to copy-init the section, ILINK declined at isolation 3
 * ("excluded because they were marked as possibly needed for init", --log initialization) and
 * left the routines in flash, with nothing in the build to say so.
 *
 * The section pragmas give the RAM destination and the initialiser ILINK leaves in ER_RO_DATA.
 * The length is taken from the destination: ER_CODE_SRAM has maximum size = S_RAM_CODE_SIZE,
 * so an overflow is a link error rather than a run-time overrun. The GNU build does the copy
 * through the linker script.
 */
#pragma section = ".ram_from_flash"
#pragma section = ".ram_from_flash_init"

static void ra_ram_code_init(void)
{
    const uint8_t *src = (const uint8_t *)__section_begin(".ram_from_flash_init");
    uint8_t       *dst = (uint8_t *)__section_begin(".ram_from_flash");
    size_t         len = (size_t)__section_size(".ram_from_flash");

    if ((src != dst) && (len != 0U)) {
        (void)memcpy(dst, src, len);
        /* The copied bytes are executed: make them visible to the instruction side. */
        __DSB();
        __ISB();
    }
}
#endif /* __ICCARM__ */

#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
/* Included here, not at the top of the file: tfm_spm_log.h has
 *     #error "TFM_SPM_LOG_LEVEL not defined!"
 * and BL2, which also compiles this file, does not define that symbol. Inside these guards the
 * include is secure-only (RA6M5_BUILDING_BL2) and isolation 2 or above only.
 */
#include "tfm_spm_log.h"
/* Declares tfm_hal_post_partition_init_hook(), implemented at the end of this block. */
#include "tfm_hal_isolation.h"

/*
 * MPU inventory, reported at the end of platform init and again from
 * tfm_hal_post_partition_init_hook(). It reads two numbers off the hardware:
 *
 *   1. MPU regions the part provides (MPU->TYPE). A successful boot proves only ">= the number
 *      needed", because tfm_hal_set_up_static_boundaries() panics when over budget.
 *   2. Regions TF-M enabled (RLAR.EN). The static table in tfm_hal_isolation_v8m.c is five
 *      entries on an M33 (PXN needs armv8.1-m.main, so the combined-code branch is taken),
 *      plus one reserved for private data at isolation 3, plus one per named MMIO region.
 *
 * The SPM calls tfm_hal_platform_init() after tfm_hal_set_up_static_boundaries(), so the
 * static regions are live when the first report runs. Per-partition regions are programmed
 * later, in tfm_hal_activate_boundary(), so at isolation 3 the count is the static set only.
 *
 * Costs nothing below isolation 2: the function compiles out with
 * CONFIG_TFM_ENABLE_MEMORY_PROTECT, and SPMLOG_INFMSGVAL expands to (void)(val) when SPM
 * logging is off, as at the MinSizeRel default. Secure-slot headroom is small on both parts
 * (DECISIONS D087).
 *
 * RNR is saved and restored. Nothing in the SPM relies on it here, but ARM_MPU_SetRegion()
 * also writes it, and a logging build should leave the MPU as a non-logging build does.
 */
static void ra_mpu_report(void)
{
    const uint32_t dregion = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    const uint32_t saved_rnr = MPU->RNR;
    uint32_t enabled = 0U;
    uint32_t i;

    for (i = 0U; i < dregion; i++) {
        MPU->RNR = i;
        if ((MPU->RLAR & MPU_RLAR_EN_Msk) != 0U) {
            enabled++;
        }
    }
    MPU->RNR = saved_rnr;

    SPMLOG_INFMSGVAL("[MPU] regions in hardware: ", dregion);
    SPMLOG_INFMSGVAL("[MPU] enabled after static boundaries: ", enabled);
    SPMLOG_INFMSGVAL("[MPU] CTRL: ", MPU->CTRL);
}

/*
 * Per-region dump, printed only after partition init, where the question is which ranges the
 * regions cover.
 *
 * RBAR[31:5] is the base and RLAR[31:5] the inclusive limit; the low 5 bits hold attributes
 * and are masked off. A limit reads back with its low 5 bits clear: 0x400E80FF programmes as
 * 0x400E80E0, so add 0x1F when comparing against a datasheet range.
 */
static void ra_mpu_dump_regions(void)
{
    const uint32_t dregion = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    const uint32_t saved_rnr = MPU->RNR;
    uint32_t i;

    for (i = 0U; i < dregion; i++) {
        MPU->RNR = i;
        if ((MPU->RLAR & MPU_RLAR_EN_Msk) != 0U) {
            SPMLOG_INFMSGVAL("[MPU]   idx ", i);
            SPMLOG_INFMSGVAL("[MPU]     base  ", MPU->RBAR & MPU_RBAR_BASE_Msk);
            SPMLOG_INFMSGVAL("[MPU]     limit ", MPU->RLAR & MPU_RLAR_LIMIT_Msk);
        }
    }
    MPU->RNR = saved_rnr;
}

/*
 * Called from spm_ipc.c once every partition is bound, the only point at which per-partition
 * MMIO regions are visible. Enabled by CONFIG_TFM_POST_PARTITION_INIT_HOOK in
 * config_tfm_target.h.
 *
 * At isolation 2 a named MMIO region is added permanently in tfm_hal_bind_boundary(), so an
 * ARoT partition's peripheral appears as an extra enabled region beyond the five static ones.
 * At isolation 3 it is programmed per context switch and does not appear here.
 */
FIH_RET_TYPE(enum tfm_hal_status_t) tfm_hal_post_partition_init_hook(void)
{
    SPMLOG_INFMSG("[MPU] --- after partition init ---\r\n");
    ra_mpu_report();
    ra_mpu_dump_regions();

    FIH_RET(fih_int_encode(TFM_HAL_SUCCESS));
}
#endif /* CONFIG_TFM_ENABLE_MEMORY_PROTECT */

FIH_RET_TYPE(enum tfm_hal_status_t) tfm_hal_platform_init(void)
{
    /* SystemInit() sets SystemCoreClock before the C runtime zeroes .bss. Early BSP
     * initialisation (BSP_CFG_EARLY_INIT = 1, enforced by ra6m5_layout_checks.c) keeps the
     * clock state in .ram_noinit, and this idempotent call also recomputes SystemCoreClock from
     * g_clock_freq[]. FSP drivers derive peripheral clocks from it through
     * R_FSP_SystemClockHzGet(), and see 0 if it is lost:
     * R_FLASH_HP_Open() then fails with FSP_ERR_FCLK. See DESIGN.md 8.1.
     *
     * Secure image only: BL2 never calls this SPM hook. BL2 relies on early initialisation,
     * checked in bl2_option_setting.c.
     */
#if defined(__ICCARM__)
    /* Before anything can reach the flash driver. DECISIONS D048. */
    ra_ram_code_init();
#endif

    SystemCoreClockUpdate();

    /* Clear PRIMASK. Reset_Handler in startup_ra6m5.c sets it with __disable_irq(), as every
     * TF-M port does, and tfm_hal_platform_init() is where the reference ports clear it. Under
     * the SFN backend nothing else does: the SPM's only cpsie i sites,
     * backend_abi_leaving_spm() and the PendSV exit path, are IPC-backend only, and
     * arch_clean_stack_and_launch() does not touch PRIMASK.
     *
     * With PRIMASK set, SVCall is masked, so the first SVC escalates to HardFault with
     * HFSR.FORCED set and every CFSR/BFSR/MMFSR/UFSR/SFSR bit clear. The first SVC is often
     * partition logging ("svc 2" in tfm_output_unpriv_string()), so the fault is easily misread
     * as a fault in the code being logged from.
     *
     * Must precede stdio_init(), as in the reference ports, and follow the boundary and
     * peripheral setup the SPM does before calling this hook.
     */
    __enable_irq();

    /* Bring up the stdout backend, as every TF-M port does here.
     *   - RTT: stdio_init() is the only caller of SEGGER_RTT_Init(). Without this call
     *     --gc-sections drops it, the control block is never initialised, and J-Link RTT
     *     Viewer cannot find it. BL2 is unaffected: bl2_main.c calls stdio_init() itself.
     *   - UART: Driver_USART is never opened, so output goes nowhere.
     *
     * Must follow SystemCoreClockUpdate(): the UART backend derives its baud divisor from
     * SystemCoreClock, which is 0 until then.
     */
    stdio_init();

#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
    /* After stdio_init(), or the report goes nowhere. */
    ra_mpu_report();
#endif

    /* NOTE: target_cfg.c's fault, reset and debug hooks have no caller on this port (D134). */
    FIH_RET(fih_int_encode(TFM_HAL_SUCCESS));
}

void tfm_hal_system_reset(void)
{
    NVIC_SystemReset();
    while(1);
}

void tfm_hal_system_halt(void)
{
    __disable_irq();
    while(1) {
        __WFI();
    }
}

/* Non-Secure context initialization functions */
uint32_t tfm_hal_get_ns_VTOR(void)
{
    /* Non-secure vector table is at the start of non-secure code */
    return NS_CODE_START;
}

uint32_t tfm_hal_get_ns_MSP(void)
{
    /* Read the initial stack pointer from NS vector table */
    return *((uint32_t *)NS_CODE_START);
}

uint32_t tfm_hal_get_ns_entry_point(void)
{
    /* Read the reset handler address from NS vector table */
    return *((uint32_t *)(NS_CODE_START + 4));
}
/* Platform service implementations */
void tfm_platform_hal_system_reset(void)
{
    NVIC_SystemReset();
    while(1);
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                                psa_invec *in_vec,
                                                psa_outvec *out_vec)
{
    /* No platform IOCTLs are implemented. */
    (void)request;
    (void)in_vec;
    (void)out_vec;

    return TFM_PLATFORM_ERR_NOT_SUPPORTED;
}

#endif /* !RA6M5_BUILDING_BL2 */
