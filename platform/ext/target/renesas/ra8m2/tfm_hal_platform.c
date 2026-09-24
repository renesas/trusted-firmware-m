/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tfm_hal_platform.h"
/* <string.h> was here for the .ram_from_flash memcpy, which this part does not need - see
 * the note below. Removed rather than left: an unused include is a claim about what the file
 * does. */
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
 * all; here it cannot simply be dropped, because __STACK_SEAL would go with it and
 * ra8m2_bl2.ld resolves FSP's __StackSeal from the section it anchors.
 *
 * The discriminator is RA8M2_BUILDING_BL2, set by CMakeLists.txt on platform_bl2 alone -
 * NOT the plain BL2 macro, which is defined for every image in the build whenever BL2 is
 * enabled, so guarding on it would delete these symbols from tfm_s as well:
 *     undefined reference to `tfm_hal_platform_init' (secure_fw/spm/core/main.c)
 */
#ifndef RA8M2_BUILDING_BL2

/*
 * NO .ram_from_flash COPY ON THIS PART - deliberately, unlike RA6E1/RA6M5.
 *
 * Those ports need a ra_ram_code_init() here: FSP's r_flash_hp marks its code-flash
 * program/erase routines PLACE_IN_RAM_SECTION, ILINK declined to copy-init the section at
 * isolation 3 while doing so correctly at isolation 1, and the copy had to become ours
 * (DECISIONS D029/D048).
 *
 * RA8M2 has MRAM and nothing in r_mram carries PLACE_IN_RAM_SECTION, so the section is empty
 * in the linked image and there is nothing to copy. region_defs.h therefore defines no
 * S_RAM_CODE_START, TF-M's template creates no ER_CODE_SRAM, and ra8m2_fsp_sections.icf
 * places nothing - all three have to agree, and they do. See the restore recipe at the top of
 * ra8m2_fsp_sections.icf if that ever changes.
 */

FIH_RET_TYPE(enum tfm_hal_status_t) tfm_hal_platform_init(void)
{
    /* FSP BSP clock initialization (bsp_clock_init) is called automatically from
     * SystemInit() in system.c during Reset_Handler, before main() is entered.
     *
     * From ra_gen/bsp_clock_cfg.h on this part - NOT RA6M5's 200 MHz:
     *   - XTAL: 24 MHz
     *   - PLL: div /3, mul x250, source PLL1P
     *   - CPUCLK: /1   (CPU0 is specified at 1 GHz - see the device rzone, Dclock)
     *   - ICLK:   /4   (the peripheral clock FSP drivers derive from)
     * R_FSP_SystemClockHzGet() applies the per-peripheral divider to SystemCoreClock, so a
     * wrong SystemCoreClock scales every peripheral clock, not just one.
     *
     * The M85 CACHES need nothing from TF-M: FSP enables both during clock init -
     * SCB_EnableICache() in bsp_clocks.c and SCB_EnableDCache() in the CMSIS system.c, with
     * MEMSYSCTL->MSCR ICACTIVE/DCACTIVE set alongside. Worth stating because an M85 port is
     * where a reader would expect to find cache bring-up, and its absence here is a decision
     * rather than an omission.
     *
     * No additional clock *configuration* is needed here, and SystemCoreClockUpdate() below
     * is now belt-and-braces rather than load-bearing: the port REQUIRES
     * BSP_CFG_EARLY_INIT=1 (ra8m2_layout_checks.c fails the build otherwise), which puts
     * SystemCoreClock somewhere the C-runtime init does not zero. It is kept because it is
     * idempotent and cheap, and because the failure it guards against is silent.
     *
     * The history, since the guard is what makes this safe: with BSP_CFG_EARLY_INIT=0
     * SystemInit() runs before __PROGRAM_START() does the C-runtime init and the
     * SystemCoreClock variable lives in plain .bss, so it is zeroed immediately after
     * SystemInit set it. Any FSP driver that
     * derives a peripheral clock via R_FSP_SystemClockHzGet() (== SystemCoreClock
     * >> divider) would then see 0 - e.g. R_MRAM_Open() rejects it with
     * FSP_ERR_FCLK (FCLK below the 4 MHz minimum). g_clock_freq[] lives in
     * .ram_noinit and survives, so SystemCoreClockUpdate() restores the real
     * value. Idempotent.
     *
     * NOTE: this is the SECURE path only - tfm_hal_platform_init() is an SPM hook and
     * BL2 never calls it. BL2 runs TF-M's startup too and is the image that actually
     * hit FSP_ERR_FCLK in July, so it needs its own call; see DESIGN.md 8.1.
     */
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
     * Deliberately after SystemCoreClockUpdate(): the UART backend derives its baud
     * divisor from SystemCoreClock, which is 0 until that call. RTT does not care.
     */
    stdio_init();

    /* Note: target_cfg.h functions are called by TF-M framework */
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
