/***********************************************************************************************************************
 * Copyright [2020] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "bsp_api.h"

#if BSP_TZ_SECURE_BUILD

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
 #define BSP_PRV_TZ_REG_KEY       (0xA500U)
 #define BSP_PRV_AIRCR_VECTKEY    (0x05FA0000U)
 #define RA_NOT_DEFINED           (0)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/
void R_BSP_SecurityInit(void);

/*******************************************************************************************************************//**
 * Initialize security features for TrustZone.
 *
 * This function initializes ARM security register and Renesas SAR registers for secure projects.
 *
 * @note IDAU settings must be configured to match project settings with a separate configuration tool.
 **********************************************************************************************************************/
void R_BSP_SecurityInit (void)
{
    /* Setting SAU_CTRL.ALLNS to 1 allows the security attribution of all addresses to be set by the IDAU in the
     * system. */
    SAU->CTRL = SAU_CTRL_ALLNS_Msk;

    /* The following section of code to configure SCB->AIRCR, SCB->NSACR, and FPU->FPCCR is taken from
     * system_ARMCM33.c in the CMSIS_5 repository. SCB->SCR SLEEPDEEPS bit is not configured because the
     * SCB->SCR SLEEPDEEP bit is ignored on RA MCUs. */
 #if defined(SCB_CSR_AIRCR_INIT) && (SCB_CSR_AIRCR_INIT == 1U)

    /* Configure whether non-secure projects have access to system reset, whether bus fault, hard fault, and NMI target
     * secure or non-secure, and whether non-secure interrupt priorities are reduced to the lowest 8 priority levels. */
    SCB->AIRCR = (SCB->AIRCR & ~(SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_SYSRESETREQS_Msk |
                                 SCB_AIRCR_BFHFNMINS_Msk | SCB_AIRCR_PRIS_Msk)) |
                 ((0x05FAU << SCB_AIRCR_VECTKEY_Pos) & SCB_AIRCR_VECTKEY_Msk) |
                 ((SCB_AIRCR_SYSRESETREQS_VAL << SCB_AIRCR_SYSRESETREQS_Pos) & SCB_AIRCR_SYSRESETREQS_Msk) |
                 ((SCB_AIRCR_PRIS_VAL << SCB_AIRCR_PRIS_Pos) & SCB_AIRCR_PRIS_Msk) |
                 ((SCB_AIRCR_BFHFNMINS_VAL << SCB_AIRCR_BFHFNMINS_Pos) & SCB_AIRCR_BFHFNMINS_Msk);
 #endif

 #if defined(__FPU_USED) && (__FPU_USED == 1U) && \
    defined(TZ_FPU_NS_USAGE) && (TZ_FPU_NS_USAGE == 1U)

    /* Configure whether the FPU can be accessed in the non-secure project. */
    SCB->NSACR = (SCB->NSACR & ~(SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk)) |
                 ((SCB_NSACR_CP10_11_VAL << SCB_NSACR_CP10_Pos) & (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk));

    /* Configure whether FPU registers are always treated as non-secure (and therefore not preserved on the stack when
     * switching from secure to non-secure), and whether the FPU registers should be cleared on exception return. */
    FPU->FPCCR = (FPU->FPCCR & ~(FPU_FPCCR_TS_Msk | FPU_FPCCR_CLRONRETS_Msk | FPU_FPCCR_CLRONRET_Msk)) |
                 ((FPU_FPCCR_TS_VAL << FPU_FPCCR_TS_Pos) & FPU_FPCCR_TS_Msk) |
                 ((FPU_FPCCR_CLRONRETS_VAL << FPU_FPCCR_CLRONRETS_Pos) & FPU_FPCCR_CLRONRETS_Msk) |
                 ((FPU_FPCCR_CLRONRET_VAL << FPU_FPCCR_CLRONRET_Pos) & FPU_FPCCR_CLRONRET_Msk);
 #endif

    /* Disable PRCR for SARs. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    /* Set TrustZone filter to Secure. */
    R_TZF->TZFSAR = ~R_TZF_TZFSAR_TZFSA0_Msk;

    /* Set TrustZone filter exception response. */
    R_TZF->TZFPT  = BSP_PRV_TZ_REG_KEY + 1U;
    R_TZF->TZFOAD = BSP_PRV_TZ_REG_KEY + BSP_TZ_CFG_EXCEPTION_RESPONSE;
    R_TZF->TZFPT  = BSP_PRV_TZ_REG_KEY + 0U;

    /* Initialize PSARs. */
    R_PSCU->PSARB = BSP_TZ_CFG_PSARB;
    R_PSCU->PSARC = BSP_TZ_CFG_PSARC;
    R_PSCU->PSARD = BSP_TZ_CFG_PSARD;
    R_PSCU->PSARE = BSP_TZ_CFG_PSARE;
    R_PSCU->MSSAR = BSP_TZ_CFG_MSSAR;

    /* Reenable PRCR for SARs. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);
}

#endif
