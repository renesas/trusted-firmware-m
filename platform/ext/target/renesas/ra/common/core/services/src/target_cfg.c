/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <common_target_cfg.h>
#include "target_cfg.h"

#include "tfm_hal_device_header.h"
#include "tfm_plat_defs.h"
#include "tfm_utils.h"
#include "utilities.h"

/* Registers and valid (module) bits of one PPC bank */
struct ra_ppc_bank {
    volatile uint32_t *ppar;
    volatile uint32_t *psar;
    uint32_t valid_mask;
};

static const struct ra_ppc_bank ra_ppc_banks[] = {
    [PPC_SP_PERIPH_REG_B] = { (volatile uint32_t *)&R_PSCU->PPARB,
                              (volatile uint32_t *)&R_PSCU->PSARB, RA_PPC_VALID_B },
    [PPC_SP_PERIPH_REG_C] = { (volatile uint32_t *)&R_PSCU->PPARC,
                              (volatile uint32_t *)&R_PSCU->PSARC, RA_PPC_VALID_C },
    [PPC_SP_PERIPH_REG_D] = { (volatile uint32_t *)&R_PSCU->PPARD,
                              (volatile uint32_t *)&R_PSCU->PSARD, RA_PPC_VALID_D },
    [PPC_SP_PERIPH_REG_E] = { (volatile uint32_t *)&R_PSCU->PPARE,
                              (volatile uint32_t *)&R_PSCU->PSARE, RA_PPC_VALID_E },
};

/* Returns NULL for an invalid bank, including PPC_SP_DO_NOT_CONFIGURE (-1) */
static const struct ra_ppc_bank *ra_ppc_bank_get(ppc_bank_t bank)
{
    if (((int)bank < 0) || ((size_t)bank >= ARRAY_SIZE(ra_ppc_banks))) {
        return NULL;
    }

    return &ra_ppc_banks[bank];
}

/* True when 'pos' is a module bit of the bank (reserved bits are not) */
static bool ra_ppc_pos_valid(const struct ra_ppc_bank *p_bank, uint32_t pos)
{
    return (p_bank != NULL) && (pos <= 31U) && ((p_bank->valid_mask & (1U << pos)) != 0U);
}

static void ra_ppar_set_bit(ppc_bank_t bank, uint32_t pos, bool unpriv)
{
    const struct ra_ppc_bank *p_bank = ra_ppc_bank_get(bank);
    uint32_t mask;
    uint32_t val;

    /* Only module bits may be changed; a reserved bit must stay 1 */
    if (!ra_ppc_pos_valid(p_bank, pos)) {
        tfm_core_panic();
    }

    mask = 1U << pos;

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    if (unpriv) {
        *p_bank->ppar |= mask;
    } else {
        *p_bank->ppar &= ~mask;
    }

    val = *p_bank->ppar;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);

    /* Read back: the privilege bit must have taken the requested value */
    if (((val & mask) != 0U) != unpriv) {
        tfm_core_panic();
    }
}

/* Sets (non-secure) or clears (secure) the security attribution bit of a module */
static void ra_psar_set_bit(ppc_bank_t bank, uint32_t pos, bool nonsecure)
{
    const struct ra_ppc_bank *p_bank = ra_ppc_bank_get(bank);
    uint32_t mask;
    uint32_t val;

    /* Only module bits may be changed; a reserved bit must stay 0 */
    if (!ra_ppc_pos_valid(p_bank, pos)) {
        tfm_core_panic();
    }

    mask = 1U << pos;

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    if (nonsecure) {
        *p_bank->psar |= mask;
    } else {
        *p_bank->psar &= ~mask;
    }

    val = *p_bank->psar;

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);

    /* Read back: the security bit must have taken the requested value (51.9.1) */
    if (((val & mask) != 0U) != nonsecure) {
        tfm_core_panic();
    }
}

/*
 * Default-deny for secure peripherals, run once by the SPM before any
 * partition is loaded. Security (PSARx) is set by R_BSP_SecurityInit() in
 * security.c; the privilege side (PPARx) is set here:
 *  - every module whose PSAR bit is 0 (secure) becomes privileged (PPAR bit 0);
 *  - non-secure modules keep their PPAR value (S-TYPE-2: owned by the NS side);
 *  - only module bits are written, so reserved PPAR bits stay 1.
 * Then every named MMIO the platform lists for partitions must be secure.
 *
 * The NS check stays here on purpose: the FSP configuration (BSP_TZ_CFG_PSARx)
 * is the main source of security attribution, and ppc_configure_to_secure()
 * must not be used at runtime to take over a peripheral that another side
 * (or, on RA8P1, the CM33 core sharing PSARx) is using.
 */
enum tfm_plat_err_t ppc_init_cfg(void)
{
    const uintptr_t *mmio_list;
    size_t mmio_count;
    size_t i;
    bool ok = true;

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    for (i = 0; i < ARRAY_SIZE(ra_ppc_banks); i++) {
        const struct ra_ppc_bank *p_bank = &ra_ppc_banks[i];
        uint32_t secure = ~(*p_bank->psar) & p_bank->valid_mask;

        *p_bank->ppar &= ~secure;

        /* Read back: every secure module bit must now be privileged */
        if ((*p_bank->ppar & secure) != 0U) {
            ok = false;
            break;
        }
    }

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);

    if (!ok) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    get_partition_named_mmio_list(&mmio_list, &mmio_count);
    for (i = 0; i < mmio_count; i++) {
        const struct platform_data_t *p_data = (const struct platform_data_t *)mmio_list[i];
        const struct ra_ppc_bank *p_bank;
        uint32_t pos;

        /* The entry of a peripheral that is not secure is 0 */
        if (mmio_list[i] == 0U) {
            continue;
        }

        if (p_data->periph_ppc_bank == PPC_SP_DO_NOT_CONFIGURE) {
            continue;
        }

        pos = (uint32_t)p_data->periph_ppc_mask;
        p_bank = ra_ppc_bank_get(p_data->periph_ppc_bank);
        if (!ra_ppc_pos_valid(p_bank, pos)) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        /* PSAR bit 1 = non-secure: a secure partition cannot be given it */
        if ((*p_bank->psar & (1U << pos)) != 0U) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }
    }

    return TFM_PLAT_ERR_SUCCESS;
}

void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
{
    const struct ra_ppc_bank *p_bank = ra_ppc_bank_get(bank);
    bool was_nonsecure;

    if (!ra_ppc_pos_valid(p_bank, pos)) {
        tfm_core_panic();
    }

    was_nonsecure = (*p_bank->psar & (1U << pos)) != 0U;

    ra_psar_set_bit(bank, pos, false);

    /*
     * A module that becomes secure must not keep an unprivileged grant left
     * over from when it was non-secure. The common code sets the privilege
     * right after this call anyway; this is only a safe default.
     */
    if (was_nonsecure) {
        ra_ppar_set_bit(bank, pos, false);
    }
}

void ppc_configure_to_non_secure(ppc_bank_t bank, uint32_t pos)
{
    /* Only the security attribution: privilege (PPAR) belongs to the caller */
    ra_psar_set_bit(bank, pos, true);
}

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    ra_ppar_set_bit(bank, pos, true);
}

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    ra_ppar_set_bit(bank, pos, false);
}

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* MPC is initialized in R_BSP_SecurityInit() in security.c */
    return TFM_PLAT_ERR_SUCCESS;
}

void sau_and_idau_cfg(void)
{
    /* SAU and IDAU is initialized in R_BSP_SecurityInit() in security.c */
}

void platform_icu_secure_irq_setup(IRQn_Type irq, uint32_t elc_event)
{
    const uint32_t index = (uint32_t)irq / 32U;
    const uint32_t bit = (uint32_t)irq % 32U;
    uint32_t volatile *p_icusarg = &R_CPSCU->ICUSARG;

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);
    p_icusarg[index] &= ~(1U << bit);
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);
    R_ICU->IELSR[irq] = elc_event;
    R_ICU->INTSELR[elc_event >> 5U] &= ~(1U << (elc_event % 32U));
}
