/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "target_cfg.h"
#include "cmsis.h"
#include "Driver_Flash.h"
#include "flash_layout.h"
#include "bootutil/bootutil_log.h"

#include "r_flash_hp.h"
#include "r_flash_api.h"

#include "psa/crypto.h"

#if defined(__ARMCC_VERSION)
__attribute__((naked)) void boot_clear_bl2_ram_area (void)
{
    __asm volatile (
        ".syntax unified                             \n"
        "mov     r0, #0                              \n"
        "ldr     r1, =Image$$ER_DATA$$Base           \n"
        "ldr     r2, =Image$$ARM_LIB_HEAP$$ZI$$Limit \n"
        "subs    r2, r2, r1                          \n"
        "Loop:                                       \n"
        "subs    r2, #4                              \n"
        "itt     ge                                  \n"
        "strge   r0, [r1, r2]                        \n"
        "bge     Loop                                \n"
        "bx      lr                                  \n"
        : : : "r0", "r1", "r2", "memory"
        );
}

#elif defined(__GNUC__)
__attribute__((naked)) void boot_clear_bl2_ram_area (void)
{
    __asm volatile (
        ".syntax unified                             \n"
        "mov     r0, #0                              \n"
        "ldr     r1, =__data_start__                 \n"
        "ldr     r2, =__HeapLimit                    \n"
        "subs    r2, r2, r1                          \n"
        "Loop:                                       \n"
        "subs    r2, #4                              \n"
        "itt     ge                                  \n"
        "strge   r0, [r1, r2]                        \n"
        "bge     Loop                                \n"
        "bx      lr                                  \n"
        : : : "r0", "r1", "r2", "memory"
        );
}
#endif

#define FAW_START_ADDR    (0xFFFC)
#define FAW_END_ADDR      (0x200000)

/* Flash device name must be specified by target */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME;

/* FSP structures required by uart and flash drivers */
extern flash_hp_instance_ctrl_t g_tfm_fsp_flash_ctrl;
extern const flash_cfg_t        g_tfm_fsp_flash_cfg;

static void flash_FAW_Set (uint32_t start_addr, uint32_t end_addr)
{
    int ret_val = 0;

    volatile uint32_t faws = (uint32_t) ((R_FACI_HP->FAWMON_b.FAWS) << 13);
    volatile uint32_t fawe = (uint32_t) ((R_FACI_HP->FAWMON_b.FAWE) << 13);

    if (faws == fawe)
    {
        BOOT_LOG_INF("Configuring FAW settings");

        FSP_CRITICAL_SECTION_DEFINE;
        FSP_CRITICAL_SECTION_ENTER;
        ret_val = R_FLASH_HP_AccessWindowSet(&g_tfm_fsp_flash_ctrl, start_addr, end_addr);

        FSP_CRITICAL_SECTION_EXIT;
        if (ret_val)
        {
            BOOT_LOG_ERR("Failed to set Flash Access Window: 0x%x", ret_val);
        }
    }
}

/* bootloader platform-specific HW intialization */
int32_t boot_platform_init (void)
{
    int32_t result;

    result = FLASH_DEV_NAME.Initialize(NULL);
    if (ARM_DRIVER_OK != result)
    {
        BOOT_LOG_ERR("Flash driver initialization failed!!!");
    }

    /* Set the FAW to lock the Secure code and data region */

    // flash_FAW_Set(FAW_START_ADDR, FAW_END_ADDR);

    result = mbedtls_platform_setup(NULL);
    if (result != 0)
    {
        BOOT_LOG_ERR("mbedtls_platform_setup failed!!!");
    }

    result = psa_crypto_init();
    if (result != 0)
    {
        BOOT_LOG_ERR("psa_crypto_init failed!!!");
    }

    return result;
}
