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

/* Linker script provides __StackSeal. TF-M SPM code expects __STACK_SEAL.
 * We provide __STACK_SEAL as a weak symbol in the same section. */
extern uint64_t __StackSeal;

/* Weak definition placed in msp_stack_seal_res section - same as __StackSeal */
__attribute__((weak, section(".msp_stack_seal_res")))
uint64_t __STACK_SEAL = 0xFEF5EDA5FEF5EDA5ULL;

/* Everything below is secure-side (SPM and platform-service) HAL. BL2 links this file
 * only for the __STACK_SEAL definition above, and calls none of it.
 *
 * The guard is not cosmetic: tfm_hal_platform_init() is declared FIH_RET_TYPE(...) and
 * returns fih_int_encode(...), which agree only while fih_ret and fih_int are the same
 * type. In BL2 the real mcuboot fih.h is in scope, and at MCUBOOT_FIH_PROFILE MEDIUM -
 * selected by TFM_PROFILE=profile_large, which the PSA Arch crypto tests require - they
 * are not:
 *     error: incompatible types when returning type 'fih_int' but 'fih_ret'
 *            {aka 'volatile int'} was expected
 * The default FIH profile is OFF, which is why this compiled until the first
 * profile_large build. Upstream platforms (an521) never compile this file into BL2 at
 * all; here it cannot be dropped, because __STACK_SEAL would go with it and
 * ra8m2_bl2.ld resolves FSP's __StackSeal from the section it anchors.
 *
 * The discriminator is RA8M2_BUILDING_BL2, set by CMakeLists.txt on platform_bl2 alone -
 * NOT the plain BL2 macro, which is defined for every image in the build whenever BL2 is
 * enabled, so guarding on it would delete these symbols from tfm_s as well:
 *     undefined reference to `tfm_hal_platform_init' (secure_fw/spm/core/main.c)
 */
#ifndef RA8M2_BUILDING_BL2

#if defined(__ICCARM__)
/*
 * Copy .ram_from_flash to RAM. DECISIONS D029.
 *
 * FSP's code-flash program/erase routines must run from RAM: the FCU makes code flash
 * unreadable for the duration of a P/E operation, so a routine fetching from flash aborts
 * mid-erase with the FCU still in P/E mode - a hang, not a fault.
 *
 * ILINK will not be trusted with the copy. Asked to copy-init the section it decides for
 * itself whether it may, and at isolation 3 it declined - "excluded because they were marked
 * as possibly needed for init" (--log initialization) - leaving the routines in flash while
 * isolation 1 relocated them correctly, with nothing in the build to say so. The ICF now
 * declares the section 'initialize manually' and the copy happens here, identically at every
 * isolation level.
 *
 * The section pragmas give the RAM half and the initialiser ILINK left in ER_RO_DATA.
 * Sizes are taken from the destination: it is what the members hook placed, and ER_CODE_SRAM
 * has maximum size = S_RAM_CODE_SIZE, so an overflow is a link error rather than a run-time
 * overrun. The GNU build reaches the same result through the linker script and needs none of
 * this.
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
/* Included HERE, not with the headers at the top of the file. tfm_spm_log.h has
 *     #error "TFM_SPM_LOG_LEVEL not defined!"
 * and this file is compiled into platform_bl2 as well as platform_s - BL2 links it for the
 * __STACK_SEAL definition above and calls none of the rest - where that symbol does not
 * exist. Including it at the top broke the BL2 object only, which is a confusing failure
 * because nothing in BL2 uses the logging. Inside this guard it is both secure-only (the
 * enclosing RA8M2_BUILDING_BL2 check) and isolation-2-or-above only.
 */
#include "tfm_spm_log.h"
/* Declares tfm_hal_post_partition_init_hook(), implemented at the end of this block. */
#include "tfm_hal_isolation.h"

/*
 * MPU inventory, reported once at the end of platform init.
 *
 * WHY THIS EXISTS. Two numbers about this port were asserted for months and never read off
 * the hardware. BOTH ARE NOW MEASURED - this block records what they turned out to be, and
 * the dump stays because it is the only thing that can prove either one:
 *
 *   1. How many MPU regions the part provides. The region budget is checked at run time in
 *      tfm_hal_set_up_static_boundaries() against MPU->TYPE, and over-budget is a panic, so
 *      a successful boot only ever proved ">= the number needed" - never the actual count.
 *      ra8m2/config.cmake justified its budget with "the RA8M2 Cortex-M33 has the 8-region
 *      ARMv8-M MPU it assumes" - and CPU0 is a Cortex-M85, so that sentence had been carried
 *      over from RA6M5 and the count had never been checked against this silicon.
 *      MEASURED: MPU->TYPE reads 8. The number was right, the core name was not; both have
 *      been corrected in config.cmake.
 *   2. How many regions TF-M programmed. On an M85 TFM_PXN_ENABLE is permitted -
 *      check_config.cmake:91 allows it only for armv8.1-m.main - and with it the combined
 *      unprivileged/ARoT code region splits in two, giving six static entries rather than
 *      five. PXN defaults OFF, so the shipping configuration should still report five.
 *      MEASURED: five static at both isolation 2 and 3, so the combined-code branch is
 *      taken here exactly as on the M33 - because PXN is OFF, not because PXN is broken.
 *      platform/CMakeLists.txt:126 does emit TFM_PXN_ENABLE PUBLIC on platform_s, so setting
 *      it ON for this part would split that region and report six static entries.
 *      D115 corrects D114, which claimed the macro never reaches the HAL.
 *      At isolation 2 a sixth appears when the FLIH service binds AGT0 (0x40221000); at
 *      isolation 3 MMIO is reprogrammed per context switch, so the post-partition hook
 *      cannot see it and five is the correct reading, not a regression.
 *      Counting source branches is not the same as reading RLAR.EN.
 *
 * ORDERING. The SPM calls this hook AFTER tfm_hal_set_up_static_boundaries(), so the static
 * regions are already live and the enabled count is meaningful. Per-partition regions are
 * programmed later still, on each tfm_hal_activate_boundary(), so at isolation 3 the figure
 * here is the static set only - the dynamic ones have not been bound yet.
 *
 * COST. Nothing below isolation 2: the whole function is compiled out with
 * CONFIG_TFM_ENABLE_MEMORY_PROTECT, and SPMLOG_INFMSGVAL expands to (void)(val) whenever SPM
 * logging is off, which it is at the MinSizeRel default. That matters - the secure slot has
 * 1,693 B spare on RA6M5 and 670 B on RA8M2 (DECISIONS D087).
 *
 * RNR is saved and restored. It is only a selector, and nothing in the SPM holds a value
 * across this point, but ARM_MPU_SetRegion() writes it for its own use and leaving it moved
 * would be a gratuitous difference between a logging and a non-logging build.
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
 * Per-region dump. Separate from the summary above because it is only worth printing at the
 * second call site - after the SPM has bound every partition - where the question is not how
 * many regions exist but which ranges they cover.
 *
 * RBAR[31:5] is the base and RLAR[31:5] the limit, both with the low 5 bits used for
 * attributes, so each is masked before printing. The limit is inclusive and reads back with
 * its low 5 bits clear, i.e. 0x400E80FF programmes as 0x400E80E0 - add 0x1F when comparing
 * against a datasheet range.
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
 * Called by the SPM from spm_ipc.c once every partition has been bound, which is the only
 * point at which per-partition MMIO regions are visible. Enabled by
 * CONFIG_TFM_POST_PARTITION_INIT_HOOK in config_tfm_target.h.
 *
 * At isolation 2 a named MMIO region is appended permanently during tfm_hal_bind_boundary(),
 * so an ARoT partition's peripheral should appear here as an extra enabled region beyond the
 * five static ones. At isolation 3 it is programmed per context switch instead and will NOT
 * appear - absence there means nothing.
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
    /* FSP BSP clock initialization (bsp_clock_init) is called automatically from
     * SystemInit() in system.c during Reset_Handler, before main() is entered.
     * This configures the system to 200 MHz from the 24 MHz XTAL:
     *   - XTAL: 24 MHz
     *   - PLL: (24 MHz / 3) * 25 = 200 MHz
     *   - ICLK (system clock): 200 MHz
     *
     * No additional clock *configuration* is needed here - BUT the cached
     * SystemCoreClock value must be recomputed: SystemInit() runs before
     * __PROGRAM_START() does the C-runtime init, and with the RASC config's
     * BSP_CFG_EARLY_INIT=0 the SystemCoreClock variable lives in plain .bss,
     * so it is zeroed immediately after SystemInit set it. Any FSP driver that
     * derives a peripheral clock via R_FSP_SystemClockHzGet() (== SystemCoreClock
     * >> divider) would then see 0 - e.g. R_FLASH_HP_Open() rejects it with
     * FSP_ERR_FCLK (FCLK below the 4 MHz minimum). g_clock_freq[] lives in
     * .ram_noinit and survives, so SystemCoreClockUpdate() restores the real
     * value. Idempotent.
     *
     * NOTE: this is the SECURE path only - tfm_hal_platform_init() is an SPM hook and
     * BL2 never calls it. BL2 runs TF-M's startup too and is the image that
     * hit FSP_ERR_FCLK in July, so it needs its own call; see DESIGN.md 8.1.
     */
#if defined(__ICCARM__)
    /* Before anything can reach the flash driver. DECISIONS D029. */
    ra_ram_code_init();
#endif

    SystemCoreClockUpdate();

    /* Clear PRIMASK. Reset_Handler in startup_ra8m2.c does __disable_irq() - standard,
     * every TF-M port's startup does - and tfm_hal_platform_init() is where the canonical
     * ports undo it. Both Renesas ports omitted it, and under the SFN backend nothing else
     * ever does: the only cpsie i sites in the SPM are backend_abi_leaving_spm() and the
     * PendSV exit path, and both are IPC-backend only. arch_clean_stack_and_launch(), which
     * is how SFN reaches the partition init loop, does not touch PRIMASK.
     *
     * With PRIMASK still set, SVCall - priority 0, but a configurable priority - is masked,
     * so the first SVC executed escalates to HardFault. That is what killed the ITS
     * partition init on 2026-08-29: LOG_INFFMT -> printf -> tfm_hal_output_sp_log ->
     * "svc 2" in tfm_output_unpriv_string(), faulting with HFSR.FORCED set and every
     * CFSR/BFSR/MMFSR/UFSR/SFSR bit clear - the textbook signature of a masked SVCall,
     * and easily misread as a fault in the code being logged from.
     *
     * Ordering: must precede stdio_init() to match the reference ports, and must follow
     * the boundary/peripheral setup already done by the SPM before this hook is called.
     */
    __enable_irq();

    /* Bring up the stdout backend. Every other TF-M port calls this from its
     * tfm_hal_platform_init(); this one did not, and the omission is not benign:
     *
     *   - RTT backend: stdio_init() is the ONLY caller of SEGGER_RTT_Init(), so
     *     --gc-sections dropped both symbols from tfm_s entirely. _SEGGER_RTT lives
     *     in .bss, so the control block stayed 64 bytes of zeros and J-Link RTT
     *     Viewer - which finds the block by searching for the "SEGGER RTT" ID string -
     *     could not locate it at any address or search range. BL2 was unaffected
     *     because bl2_main.c calls stdio_init() itself.
     *   - UART backend: Driver_USART is never opened, so output goes nowhere.
     *
     * After SystemCoreClockUpdate() by design: the UART backend derives its baud
     * divisor from SystemCoreClock, which is 0 until that call. RTT does not care.
     */
    stdio_init();

#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
    /* After stdio_init(), or the report goes nowhere. */
    ra_mpu_report();
#endif

    /* NOTE: the fault, reset and debug hooks have NO caller on this port (D134) */
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
    /* Use the same system reset as tfm_hal_system_reset */
    NVIC_SystemReset();
    while(1);
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                                psa_invec *in_vec,
                                                psa_outvec *out_vec)
{
    /* Platform-specific IOCTL stub - can be extended for platform services */
    (void)request;
    (void)in_vec;
    (void)out_vec;

    /* Return not supported for now */
    return TFM_PLATFORM_ERR_NOT_SUPPORTED;
}

#endif /* !RA8M2_BUILDING_BL2 */
