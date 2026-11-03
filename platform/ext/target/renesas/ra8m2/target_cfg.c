/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2018-2020 Arm Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "target_cfg.h"
#include "common_target_cfg.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "cmsis.h"
#include "bsp_api.h"
/* Not hal_data.h, deliberately: nothing here uses it, and with the Crypto stack in the e2
 * project its ra_gen/common_data.h includes FSP's psa/crypto.h, which would shadow TF-M's
 * for everything in platform_s. */

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Region limits from region_defs.h. */
const struct memory_region_limits memory_regions = {
    .non_secure_code_start = NS_CODE_START,
    .non_secure_partition_base = NS_CODE_START,
    .non_secure_partition_limit = NS_CODE_LIMIT,
    .veneer_base = CMSE_VENEER_REGION_START,
    .veneer_limit = CMSE_VENEER_REGION_START + CMSE_VENEER_REGION_SIZE - 1,
#ifdef BL2
    .secondary_partition_base = SECONDARY_PARTITION_START,
    .secondary_partition_limit = SECONDARY_PARTITION_START + SECONDARY_PARTITION_SIZE - 1,
#endif
};

/*
 * Empty by design: FSP programmes the SAU, not TF-M.
 *
 * TF-M's common code calls this hook and expects the platform to programme the SAU; it has no
 * generic implementation. On this port FSP's R_BSP_SecurityInit()
 * (ra/fsp/src/bsp/mcu/all/bsp_security.c) does it during BSP init, before TF-M's platform
 * init, and derives every boundary from the generated partition macros. flash_layout.h and
 * region_defs.h are built from the same solution.xml partitioning, so the memory map is
 * described once.
 *
 * Verify that R_BSP_SecurityInit is in the linked secure image:
 *     arm-none-eabi-nm bin/tfm_s.axf | grep R_BSP_SecurityInit
 * If it is absent, nothing programmes the SAU and the non-secure image faults on its first
 * access; the replacement then belongs in this hook.
 *
 * ra8m2_ddsc.c supplies the four gp_ddsc_{I,D}TCM_* symbols R_BSP_SecurityInit() needs for
 * M85 TCM attribution, because the port excludes FSP's bsp_linker.c.
 *
 * RA8M2 has no IDAU in the Armv8-M sense. Attribution is the SAU (8 regions, per the device
 * rzone) plus the Renesas security-attribution registers: PSCU, and TZF/MSAOAD for
 * operation-after-detection. bsp_security.c programmes all of them from the solution.
 */
void sau_and_idau_cfg(void)
{
}

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* No MPC on this part. Memory attribution is the SAU (see sau_and_idau_cfg above) and,
     * at isolation 2/3, the MPU via TF-M's own tfm_hal_isolation_v8m.c. */
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t ppc_init_cfg(void)
{
    /* No PPC on this part. Peripheral attribution is the Renesas PSAR/PSCU registers, which
     * bsp_security.c programmes from the solution's peripheral slot assignments. See
     * target_cfg.h. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*
 * No-ops, present only so the shared isolation HAL links.
 *
 * tfm_hal_bind_boundary() calls these only if periph_ppc_bank != PPC_SP_DO_NOT_CONFIGURE, and
 * every platform_data_t on this port sets that sentinel, so none runs. The call sites still
 * compile, so the symbols must resolve.
 *
 * Per-peripheral attribution belongs in PSAR/PSCU, set from the solution, not in a TF-M-side
 * emulation of a controller the part does not have.
 */
void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                  | SCB_SHCSR_BUSFAULTENA_Msk
                  | SCB_SHCSR_MEMFAULTENA_Msk
                  | SCB_SHCSR_SECUREFAULTENA_Msk;
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
/*
 * Both functions below are no-ops and neither is called: upstream platforms call them from
 * tfm_hal_platform_init(), and this port's does not. They exist because common_target_cfg.h
 * declares them.
 *
 * ---------------------------------------------------------------------------
 * How secure interrupts work on RA, and why the upstream recipe does not apply
 * ---------------------------------------------------------------------------
 * Arm reference ports make an IRQ secure at run time, in the partition's <source>_init():
 *
 *     NVIC_SetPriority(irq, DEFAULT_IRQ_PRIORITY);
 *     NVIC_ClearTargetState(irq);          <-- do not do this on RA
 *     NVIC_DisableIRQ(irq);
 *
 * On RA the attribution is in two registers, NVIC->ITNS and R_CPSCU->ICUSARG. FSP writes both
 * in bsp_irq_cfg() (bsp_irq.c), under R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR), and
 * they must match: the ICU does not deliver an event whose ICU-side attribution disagrees with
 * the NVIC's. NVIC_ClearTargetState() writes only ITNS, so it desynchronises the pair.
 *
 * There is no runtime API to change it: R_BSP_IrqCfg/IrqEnable/IrqDisable/IrqStatusClear/
 * IrqClearPending set priority, context and pending only. ICUSARG is written once, during
 * SystemInit(), from the generated g_interrupt_event_link_select[]. Therefore:
 *
 *     An interrupt is secure if and only if the secure e2 project configures it.
 *
 * bsp_irq_cfg() clears the ITNS/ICUSARG bit for every slot with a non-zero entry in that array
 * and leaves all others non-secure. The secure projects configure one interrupt, AGT0 in slot 0
 * (VECTOR_DATA_IRQ_COUNT 1), for the FLIH/SLIH tests.
 *
 * To add one (DECISIONS D061); AGT0 (TFM_TIMER0_IRQ) is the worked example:
 *   1. Add the peripheral and its interrupt to the secure e2 project and regenerate. FSP then
 *      sets IELSR, ITNS and ICUSARG at boot.
 *   2. Override that slot in startup_ra8m2.c's __VECTOR_TABLE with a designated initialiser
 *      after the [16 ... N] = Default_Handler range; the later initialiser wins. RASC assigns
 *      slots densely from 0 in the order interrupts were added, so adding an unrelated one
 *      renumbers the rest. Key the index on the generated VECTOR_NUMBER_* macro from
 *      vector_data.h, or write it out and assert it against that macro as plat_test.c does.
 *   3. Write the ISR and the init hook; see tfm_timer0_irq.c. The ISR acks the ICU with
 *      R_BSP_IrqStatusClear() and calls spm_handle_interrupt(). The init sets the priority,
 *      clears any pending state BL2 left, and leaves the IRQ disabled (FF-M 1.1: the partition
 *      enables it). The init name comes from the manifest: "source": "TFM_TIMER0_IRQ" gives
 *      tfm_timer0_irq_init. Use a symbolic source; a numeric one hits a stray-brace bug in
 *      tools/templates/partition_load_info.template.
 *   4. Define the IRQ (from vector_data.h) and DEFAULT_IRQ_PRIORITY in tfm_peripherals_def.h.
 *      Keep the priority in IPL 1-7: SECURE_THREAD_EXECUTION_PRIORITY is 0x80 == IPL 8, so
 *      IPL >= 8 is masked if CONFIG_TFM_SECURE_THREAD_MASK_NS_INTERRUPT is ever enabled.
 *   5. Declare "irqs" in the owning partition's manifest. Under the SFN backend use FLIH: it
 *      runs entirely in handler mode and needs no thread. Calling any psa_* API from a FLIH
 *      panics (psa_interface_sfn.c checks __get_active_exc_num()).
 *
 * Do not reach TF-M through FSP's p_callback: spm_handle_interrupt() must run in the exception
 * context, and no upstream port routes through a vendor callback registry.
 */
/*
 * Forces ra_gen/vector_data.o out of libfsp_bsp_s.a. Do not delete this, and do not make it
 * static.
 *
 * vector_data.c holds the strong g_interrupt_event_link_select[], the table bsp_irq_cfg()
 * reads to programme R_ICU->IELSR and the ITNS/ICUSARG attribution. bsp_irq.c holds a
 * BSP_WEAK_REFERENCE all-zero fallback of the same array in the same archive. The linker
 * extracts bsp_irq.o, the weak definition satisfies the reference, and vector_data.o is never
 * extracted, because nothing else refers to it: the secure image uses TF-M's __VECTOR_TABLE,
 * not FSP's g_vector_table.
 *
 * With the zero table, bsp_irq_cfg() skips both writes: IELSR is never programmed, so a
 * configured interrupt never reaches the NVIC, and the vector stays non-secure. No tool
 * reports either; the only signs are the symbol binding (V vs T) and the array contents in
 * the linked image.
 *
 * g_vector_table is defined only in vector_data.c, so referencing it pulls the member in. A
 * file-scope non-static const pointer emits a real relocation, which archive extraction keys
 * on. Moving the file into platform_s does not help, because that is a static library too.
 *
 * The guard mirrors vector_data.c's own "#if VECTOR_DATA_IRQ_COUNT > 0": with no interrupt
 * configured in the secure e2 project neither symbol exists, and the weak all-zero table is
 * then correct. DECISIONS D069.
 */
#if VECTOR_DATA_IRQ_COUNT > 0
extern const fsp_vector_t g_vector_table[];
const fsp_vector_t * const ra8m2_icu_vector_data_anchor = g_vector_table;
#endif

enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Nothing to do: FSP's bsp_irq_cfg() writes ITNS and ICUSARG from the secure project's
     * generated event-link table before main(), and writing ITNS here would undo it. That
     * holds only because the anchor above links the real table. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* Nothing to do. RA has no MPC/PPC fault IRQ for the SPM to own, and a partition-owned IRQ
     * must not be enabled here: load_irqs_assuredly() deliberately leaves FF-M 1.1 interrupts
     * disabled so the partition enables its own with psa_irq_enable(). */
    return TFM_PLAT_ERR_SUCCESS;
}