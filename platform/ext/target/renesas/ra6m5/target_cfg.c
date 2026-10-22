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
/* Deliberately NOT hal_data.h: nothing here uses it, and once the Crypto stack is in the
 * e2 project the generated ra_gen/common_data.h it pulls in includes FSP's own
 * psa/crypto.h, which would shadow TF-M's for everything in platform_s. */

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Memory region limits - initialized with region_defs.h values */
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

void sau_and_idau_cfg(void)
{
    /*
     * Deliberately empty, and NOT because the generic framework does it elsewhere.
     *
     * RA6M5 has an SAU with ZERO REGIONS, which is not the same as having no SAU.
     * __SAUREGION_PRESENT is 0 in the device header, so SAU_TYPE.SREGION reads 0 and there
     * is nothing to programme - but an Armv8-M core with the Security Extension always
     * implements SAU_CTRL and SAU_TYPE, and their state still matters.
     *
     * The IDAU decides attribution here ONLY because FSP sets SAU_CTRL.ALLNS = 1 in
     * bsp_security.c (R_BSP_SecurityInit, "allows the security attribution of all addresses
     * to be set by the IDAU"). At reset ALLNS is 0 with the SAU disabled, and in that state
     * Armv8-M attributes EVERY address Secure - the part could not run non-secure code at
     * all. **Anyone replacing FSP's startup must keep that write.** Saying "no SAU" hides
     * that dependency, which is why this comment no longer does.
     *
     * The attribution itself comes from the Renesas Device Partition Manager - the
     * BSP_PARTITION_* values in the generated bsp_linker_info.h - and is fixed before this
     * code runs.
     *
     * An earlier revision of this comment said the reverse: "RA6M5 doesn't have IDAU -
     * security attribution is done entirely via SAU". That is backwards on both halves. The
     * part in this port with __SAUREGION_PRESENT 1 is RA8M2; ra8m2/target_cfg.c states it
     * correctly and ra8m2/bl2_boot_hal.c is where an SAU actually gets programmed.
     * DECISIONS D117.
     */
}

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* No MPC on this part. Memory attribution is the IDAU (see sau_and_idau_cfg above -
     * the SAU here has no regions), and the MPU subdivides the secure side. */
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t ppc_init_cfg(void)
{
    /* RA6 has no PPC - peripheral protection is done by the IDAU (the SAU here has no
     * regions - see sau_and_idau_cfg above) and the Renesas PSAR registers, which FSP programs from the solution.
     * See target_cfg.h. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*
 * No-ops, present only so the shared isolation HAL links.
 *
 * tfm_hal_bind_boundary() guards these calls with
 * "if (periph_ppc_bank != PPC_SP_DO_NOT_CONFIGURE)", and every platform_data_t on this
 * port sets exactly that sentinel, so none is ever reached at run time. The call sites are
 * still COMPILED, so the symbols must resolve.
 *
 * If a future RA6 partition genuinely needs per-peripheral attribution, this is the wrong
 * place for it: that attribution lives in PSAR and belongs in the solution, not in a
 * TF-M-side emulation of a controller the part does not have.
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
 * Both functions below are no-ops on this port, and NEITHER IS CALLED - upstream
 * platforms invoke them from tfm_hal_platform_init(), and this port's replacement
 * does not. They are kept because common_target_cfg.h declares them.
 *
 * ---------------------------------------------------------------------------
 * HOW SECURE INTERRUPTS WORK ON RA, AND WHY THE UPSTREAM RECIPE DOES NOT APPLY
 * ---------------------------------------------------------------------------
 * Every Arm reference port makes an IRQ secure at run time, in the partition's
 * <source>_init() callback:
 *
 *     NVIC_SetPriority(irq, DEFAULT_IRQ_PRIORITY);
 *     NVIC_ClearTargetState(irq);          <-- DO NOT DO THIS ON RA
 *     NVIC_DisableIRQ(irq);
 *
 * On RA the attribution lives in TWO registers, not one: NVIC->ITNS and
 * R_CPSCU->ICUSARG. FSP writes them together in bsp_irq_cfg() (bsp_irq.c), under
 * R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR), and its own comment states
 * they MUST match - the ICU will not deliver an event whose ICU-side attribution
 * disagrees with the NVIC's. NVIC_ClearTargetState() writes only ITNS, so on RA it
 * desynchronises the pair.
 *
 * There is also no runtime API to change it: R_BSP_IrqCfg/IrqEnable/IrqDisable/
 * IrqStatusClear/IrqClearPending set priority, context and pending only. ICUSARG is
 * written exactly once, during SystemInit(), from the generated
 * g_interrupt_event_link_select[]. The rule is therefore:
 *
 *     AN INTERRUPT IS SECURE IF AND ONLY IF THE SECURE e2 PROJECT CONFIGURES IT.
 *
 * bsp_irq_cfg() clears the ITNS/ICUSARG bit for every slot with a non-zero entry in
 * that array and leaves all others non-secure. With VECTOR_DATA_IRQ_COUNT == 0 in
 * the secure project today, all 96 ICU slots are non-secure.
 *
 * To add one (DECISIONS D061):
 *   1. Add the peripheral + its interrupt to the SECURE e2 project and regenerate.
 *      FSP then sets IELSR, ITNS and ICUSARG for you at boot.
 *   2. Override that slot in startup_ra6m5.c's __VECTOR_TABLE, keyed on the
 *      generated macro so the configurator stays the source of truth:
 *          #include "vector_data.h"
 *          [16 + VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW] = TFM_TIMER0_IRQ_Handler,
 *      A later designated initialiser overrides the [16 ... N] = Default_Handler
 *      range. Do NOT hardcode the index: RASC assigns slots densely from 0 in the
 *      order interrupts were added, so adding an unrelated one renumbers the rest.
 *   3. Write the ISR and the init in a new tfm_interrupts.c for this port:
 *          static struct irq_t timer0_irq;
 *          void TFM_TIMER0_IRQ_Handler(void) {
 *              R_BSP_IrqStatusClear(R_FSP_CurrentIrqGet());   // ack in the ICU
 *              spm_handle_interrupt(timer0_irq.p_pt, timer0_irq.p_ildi);
 *          }
 *          enum tfm_hal_status_t tfm_timer0_irq_init(void *p_pt,
 *                                    const struct irq_load_info_t *p_ildi) {
 *              timer0_irq.p_pt = p_pt; timer0_irq.p_ildi = p_ildi;
 *              NVIC_SetPriority(TFM_TIMER0_IRQ, DEFAULT_IRQ_PRIORITY);
 *              NVIC_ClearPendingIRQ(TFM_TIMER0_IRQ);   // BL2 may have left it set
 *              NVIC_DisableIRQ(TFM_TIMER0_IRQ);        // FF-M 1.1: partition enables
 *              return TFM_HAL_SUCCESS;
 *          }
 *      The init name is derived from the manifest: "source": "TFM_TIMER0_IRQ" gives
 *      tfm_timer0_irq_init. Use a SYMBOLIC source - a numeric one hits a stray-brace
 *      bug in tools/templates/partition_load_info.template.
 *   4. Define TFM_TIMER0_IRQ (from vector_data.h) and DEFAULT_IRQ_PRIORITY in
 *      tfm_peripherals_def.h. Keep the priority in IPL 1-7: SECURE_THREAD_EXECUTION_
 *      PRIORITY is 0x80 == IPL 8, so IPL >= 8 is masked if
 *      CONFIG_TFM_SECURE_THREAD_MASK_NS_INTERRUPT is ever enabled.
 *   5. Declare "irqs" in the owning partition's manifest. Under the SFN backend use
 *      FLIH: it runs entirely in handler mode and needs no thread. Calling ANY psa_*
 *      API from a FLIH panics (psa_interface_sfn.c checks __get_active_exc_num()).
 *
 * Do not use FSP's p_callback to reach TF-M. spm_handle_interrupt() must run in the
 * exception context; no upstream port routes through a vendor callback registry.
 */
/*
 * FORCES ra_gen/vector_data.o OUT OF libfsp_bsp_s.a. Do not delete this, and do not make
 * it static.
 *
 * vector_data.c holds the STRONG g_interrupt_event_link_select[] - the table bsp_irq_cfg()
 * reads to program R_ICU->IELSR and the ITNS/ICUSARG attribution. bsp_irq.c holds a
 * BSP_WEAK_REFERENCE all-zero fallback of the same array, and both are members of the same
 * archive. The linker extracts bsp_irq.o (something calls bsp_irq_cfg()), the weak
 * definition satisfies the reference, and the member with the real table is never extracted
 * - because nothing else refers to it: the secure image uses TF-M's own __VECTOR_TABLE, not
 * FSP's g_vector_table.
 *
 * With the zeros, bsp_irq_cfg()'s two guarded writes both skip: IELSR is never programmed,
 * so a configured peripheral interrupt never reaches the NVIC at all, and the vector is
 * left attributed NON-SECURE. Both silently, with no diagnostic from any tool - the only
 * tell is the symbol binding (V vs T) and the array contents in the linked image. That cost
 * two hardware runs and two rewrites of the RA6M5 timer code before it was found.
 *
 * g_vector_table is defined ONLY in vector_data.c, so referencing it is what pulls the
 * member in. A file-scope non-static const pointer emits a real relocation, which is what
 * archive extraction keys on; moving the file into platform_s does NOT work, because that
 * is a static library too.
 *
 * THE GUARD MIRRORS THE GENERATED FILE'S OWN. vector_data.c wraps both symbols in
 * "#if VECTOR_DATA_IRQ_COUNT > 0", so with no interrupt configured in the secure e2 project
 * neither exists and an unconditional anchor is an undefined reference. That case needs no
 * anchor anyway: with nothing configured the weak all-zero table is the correct answer.
 * DECISIONS D069.
 */
#if VECTOR_DATA_IRQ_COUNT > 0
extern const fsp_vector_t g_vector_table[];
const fsp_vector_t * const ra6m5_icu_vector_data_anchor = g_vector_table;
#endif

enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Nothing to do. FSP's bsp_irq_cfg() has already written ITNS and ICUSARG from
     * the secure project's generated event-link table, before main() ran. Writing
     * ITNS here would undo it.
     *
     * That is only true because the anchor above got the real table linked - see it. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* Nothing to do. RA has no MPC/PPC fault IRQ for the SPM to own, and a
     * partition-owned IRQ must NOT be enabled here: FF-M 1.1 interrupts are
     * deliberately left disabled by load_irqs_assuredly() so the partition can
     * enable its own with psa_irq_enable(). */
    return TFM_PLAT_ERR_SUCCESS;
}