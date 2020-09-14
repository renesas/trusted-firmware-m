/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "target_cfg.h"
#include "Driver_Common.h"
#include "platform_description.h"
#include "device_definition.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "region.h"

/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);
#ifdef BL2
REGION_DECLARE(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base);
#endif /* BL2 */

const struct memory_region_limits memory_regions = {
    .non_secure_code_start =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        BL2_HEADER_SIZE,

    .non_secure_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        NS_PARTITION_SIZE - 1,

    .veneer_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Base),

    .veneer_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit),

#ifdef BL2
    .secondary_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base),

    .secondary_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_SECONDARY_PARTITION, $$Base) +
        SECONDARY_PARTITION_SIZE - 1,
#endif /* BL2 */
};

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

#define All_SEL_STATUS (SPNIDEN_SEL_STATUS | SPIDEN_SEL_STATUS | \
                        NIDEN_SEL_STATUS | DBGEN_SEL_STATUS)

//struct tfm_spm_partition_platform_data_t tfm_peripheral_std_uart = {
//        MUSCA_B1_UART1_NS_BASE,
//        MUSCA_B1_UART1_NS_BASE + 0xFFF,
//        PPC_SP_DO_NOT_CONFIGURE,
//        -1
//};

//struct tfm_spm_partition_platform_data_t tfm_peripheral_timer0 = {
//        MUSCA_B1_CMSDK_TIMER0_S_BASE,
//        MUSCA_B1_CMSDK_TIMER1_S_BASE - 1,
//        PPC_SP_APB_PPC0,
//        CMSDK_TIMER0_APB_PPC_POS
//};

#ifdef PSA_API_TEST_IPC

/* Below data structure are only used for PSA FF tests, and this pattern is
 * definitely not to be followed for real life use cases, as it can break
 * security.
 */

struct tfm_spm_partition_platform_data_t
    tfm_peripheral_FF_TEST_UART_REGION = {
        MUSCA_B1_UART1_NS_BASE,
        MUSCA_B1_UART1_NS_BASE + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct tfm_spm_partition_platform_data_t
    tfm_peripheral_FF_TEST_WATCHDOG_REGION = {
        MUSCA_B1_CMSDK_WATCHDOG_S_BASE,
        MUSCA_B1_CMSDK_WATCHDOG_S_BASE + 0xFFF,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

#define FF_TEST_NVMEM_REGION_START            0x3003F800
#define FF_TEST_NVMEM_REGION_END              0x3003FBFF
#define FF_TEST_SERVER_PARTITION_MMIO_START   0x3003FC00
#define FF_TEST_SERVER_PARTITION_MMIO_END     0x3003FD00
#define FF_TEST_DRIVER_PARTITION_MMIO_START   0x3003FE00
#define FF_TEST_DRIVER_PARTITION_MMIO_END     0x3003FF00

struct tfm_spm_partition_platform_data_t
    tfm_peripheral_FF_TEST_NVMEM_REGION = {
        FF_TEST_NVMEM_REGION_START,
        FF_TEST_NVMEM_REGION_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct tfm_spm_partition_platform_data_t
    tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO = {
        FF_TEST_SERVER_PARTITION_MMIO_START,
        FF_TEST_SERVER_PARTITION_MMIO_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};

struct tfm_spm_partition_platform_data_t
    tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO = {
        FF_TEST_DRIVER_PARTITION_MMIO_START,
        FF_TEST_DRIVER_PARTITION_MMIO_END,
        PPC_SP_DO_NOT_CONFIGURE,
        -1
};
#endif

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
//    struct sysctrl_t *sysctrl = (struct sysctrl_t *)CMSDK_SYSCTRL_BASE_S;
    uint32_t reg_value = SCB->AIRCR;

    /* Enable system reset request for CPU 0, to be triggered via
     * NVIC_SystemReset function.
     */
//    sysctrl->resetmask |= ENABLE_CPU0_SYSTEM_RESET_REQUEST;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{

#if defined(DAUTH_NONE)


#elif defined(DAUTH_NS_ONLY)


#elif defined(DAUTH_FULL)

#else

#if !defined(DAUTH_CHIP_DEFAULT)
#error "No debug authentication setting is provided."
#endif

#endif
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be WI */
//    for (uint8_t i=0; i<sizeof(NVIC->ITNS)/sizeof(NVIC->ITNS[0]); i++) {
//        NVIC->ITNS[i] = 0xFFFFFFFF;
//    }

    /* Make sure that MPC and PPC are targeted to S state */
//    NVIC_ClearTargetState(S_MPC_COMBINED_IRQn);
//    NVIC_ClearTargetState(S_PPC_COMBINED_IRQn);

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* MPC interrupt enabling */
//    NVIC_EnableIRQ(S_MPC_COMBINED_IRQn);

    /* PPC interrupt enabling */
    /* Clear pending PPC interrupts */
    /* In the PPC configuration function, we have used the Non-Secure
     * Privilege Control Block to grant unprivilged NS access to some
     * peripherals used by NS. That triggers a PPC0 exception as that
     * register is meant for NS privileged access only. Clear it here
     */
//    /* Enable PPC interrupts */
//    NVIC_EnableIRQ(S_PPC_COMBINED_IRQn);

#ifdef PSA_API_TEST_IPC
    NVIC_EnableIRQ(FF_TEST_UART_IRQ);
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
return;
}

/*------------------- Memory configuration functions -------------------------*/

void mpc_revert_non_secure_to_secure_cfg(void)
{

}

/*---------------------- PPC configuration functions -------------------------*/

int32_t ppc_init_cfg(void)
{

    return ARM_DRIVER_OK;
}

void ppc_configure_to_non_secure(enum ppc_bank_e bank, uint16_t pos)
{
    /* Setting NS flag for peripheral to enable NS access */
}

void ppc_configure_to_secure(enum ppc_bank_e bank, uint16_t pos)
{
    /* Clear NS flag for peripheral to prevent NS access */
}


