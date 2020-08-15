<<<<<<< HEAD:platform/ext/target/renesas/ra/nv_counters.c
/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* NOTE: This API should be implemented by platform vendor. For the security of
 * the secure storage system's and the bootloader's rollback protection etc. it
 * is CRITICAL to use a internal (in-die) persistent memory for multiple time
 * programmable (MTP) non-volatile counters or use a One-time Programmable (OTP)
 * non-volatile counters solution.
 *
 * This dummy implementation assumes that the NV counters are the only data in
 * the flash sector. To use it, one flash sector should be allocated exclusively
 * for the NV counters.
 *
 * The current software dummy implementation is not resistant to asynchronous
 * power failures and should not be used in production code. It is exclusively
 * for testing purposes.
 */

#include "platform/include/tfm_plat_nv_counters.h"

#include <limits.h>
#include "Driver_Flash.h"
#include "flash_layout.h"

/* Compilation time checks to be sure the defines are well defined */
#ifndef TFM_NV_COUNTERS_AREA_ADDR
#error "TFM_NV_COUNTERS_AREA_ADDR must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_AREA_SIZE
#error "TFM_NV_COUNTERS_AREA_SIZE must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_SECTOR_ADDR
#error "TFM_NV_COUNTERS_SECTOR_ADDR must be defined in flash_layout.h"
#endif

#ifndef TFM_NV_COUNTERS_SECTOR_SIZE
#error "TFM_NV_COUNTERS_SECTOR_SIZE must be defined in flash_layout.h"
#endif

/* End of compilation time checks to be sure the defines are well defined */

#define NV_COUNTER_SIZE  sizeof(uint32_t)
#define INIT_VALUE_SIZE  NV_COUNTER_SIZE
#define NUM_NV_COUNTERS  ((TFM_NV_COUNTERS_AREA_SIZE - INIT_VALUE_SIZE) \
                          / NV_COUNTER_SIZE)

#define NV_COUNTERS_INITIALIZED 0xC0DE0042U

/**
 * \brief Struct representing the NV counter data in flash.
 */
struct nv_counters_t {
    uint32_t counters[NUM_NV_COUNTERS]; /**< Array of NV counters */
    uint32_t init_value; /**< Watermark to indicate if the NV counters have been
                          *   initialised
                          */
};

/* Import the CMSIS flash device driver */
extern ARM_DRIVER_FLASH Driver_DFLASH0;

enum tfm_plat_err_t tfm_plat_init_nv_counter(void)
{
    int32_t err;
    uint32_t i;
    struct nv_counters_t nv_counters = {{0}};

    err = Driver_DFLASH0.Initialize(NULL);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Read the NV counter area to be able to erase the sector and write later
     * in the flash.
     */
    err = Driver_DFLASH0.ReadData(TFM_NV_COUNTERS_AREA_ADDR, &nv_counters,
                                  TFM_NV_COUNTERS_AREA_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    if (nv_counters.init_value == NV_COUNTERS_INITIALIZED) {
        return TFM_PLAT_ERR_SUCCESS;
    }

    /* Add watermark, at the end of the NV counters area, to indicate that NV
     * counters have been initialized.
     */
    nv_counters.init_value = NV_COUNTERS_INITIALIZED;

    /* Initialize all counters to 0 */
    for (i = 0; i < NUM_NV_COUNTERS; i++) {
        nv_counters.counters[i] = 0;
    }

    /* Erase sector before write in it */
    err = Driver_DFLASH0.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Write in flash the in-memory NV counter content after modification */
    err = Driver_DFLASH0.ProgramData(TFM_NV_COUNTERS_AREA_ADDR, &nv_counters,
                                     TFM_NV_COUNTERS_AREA_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_read_nv_counter(enum tfm_nv_counter_t counter_id,
                                             uint32_t size, uint8_t *val)
{
    int32_t  err;
    uint32_t flash_addr;

    if (size != NV_COUNTER_SIZE) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    flash_addr = TFM_NV_COUNTERS_AREA_ADDR + (counter_id * NV_COUNTER_SIZE);

    err = Driver_DFLASH0.ReadData(flash_addr, val, NV_COUNTER_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_set_nv_counter(enum tfm_nv_counter_t counter_id,
                                            uint32_t value)
{
    int32_t err;
    struct nv_counters_t nv_counters = {{0}};

    /* Read the NV counter area to be able to erase the sector and write later
     * in the flash.
     */
    err = Driver_DFLASH0.ReadData(TFM_NV_COUNTERS_AREA_ADDR, &nv_counters,
                                  TFM_NV_COUNTERS_AREA_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    if (value != nv_counters.counters[counter_id]) {

        if (value > nv_counters.counters[counter_id]) {
            nv_counters.counters[counter_id] = value;
        } else {
            return TFM_PLAT_ERR_INVALID_INPUT;
        }

        /* Erase sector before write in it */
        err = Driver_DFLASH0.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        /* Write in flash the in-memory NV counter content after modification */
        err = Driver_DFLASH0.ProgramData(TFM_NV_COUNTERS_AREA_ADDR,
                                         &nv_counters,
                                         TFM_NV_COUNTERS_AREA_SIZE);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_increment_nv_counter(
                                           enum tfm_nv_counter_t counter_id)
{
    uint32_t security_cnt;
    enum tfm_plat_err_t err;

    err = tfm_plat_read_nv_counter(counter_id,
                                   sizeof(security_cnt),
                                   (uint8_t *)&security_cnt);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (security_cnt == UINT32_MAX) {
        return TFM_PLAT_ERR_MAX_VALUE;
    }

    return tfm_plat_set_nv_counter(counter_id, security_cnt + 1u);
}
=======
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#include <Driver_Flash.h>
#include <flash_layout.h>
#include <limits.h>
#include "tfm_plat_nv_counters.h"

#define SECTOR_OFFSET               0
#define NV_COUNTER_SIZE             sizeof(uint32_t)
#define INIT_VALUE_SIZE             NV_COUNTER_SIZE
#define NV_COUNTERS_AREA_OFFSET     (TFM_NV_COUNTERS_AREA_ADDR - \
                                     TFM_NV_COUNTERS_SECTOR_ADDR)

#define NV_COUNTERS_INITIALIZED     0xC0DE0042


extern ARM_DRIVER_FLASH FLASH_DEV_NAME;

enum tfm_plat_err_t tfm_plat_init_nv_counter(void)
{
    int32_t err;
    uint32_t i;
    uint32_t nbr_counters = ((TFM_NV_COUNTERS_AREA_SIZE - INIT_VALUE_SIZE)
                             / NV_COUNTER_SIZE);
    uint32_t *p_nv_counter;
    uint8_t sector_data[TFM_NV_COUNTERS_SECTOR_SIZE] = {0};

    /* Read the whole sector to be able to erase and write later in the flash */
    err = FLASH_DEV_NAME.ReadData(TFM_NV_COUNTERS_SECTOR_ADDR, sector_data,
                                  TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Set the pointer to nv counters position */
    p_nv_counter = (uint32_t *)(sector_data + NV_COUNTERS_AREA_OFFSET);

    if (p_nv_counter[nbr_counters] == NV_COUNTERS_INITIALIZED) {
        return TFM_PLAT_ERR_SUCCESS;
    }

    /* Add watermark, at the end of the NV counters area, to indicate that NV
     * counters have been initialized.
     */
    p_nv_counter[nbr_counters] = NV_COUNTERS_INITIALIZED;

    /* Initialize all counters to 0 */
    for (i = 0; i < nbr_counters; i++) {
        p_nv_counter[i] = 0;
    }

    /* Erase sector before write in it */
    err = FLASH_DEV_NAME.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Write in flash the in-memory block content after modification */
    err = FLASH_DEV_NAME.ProgramData(TFM_NV_COUNTERS_SECTOR_ADDR, sector_data,
                                     TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_read_nv_counter(enum tfm_nv_counter_t counter_id,
                                             uint32_t size, uint8_t *val)
{
    int32_t  err;
    uint32_t flash_addr;

    if (size != NV_COUNTER_SIZE) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    flash_addr = TFM_NV_COUNTERS_AREA_ADDR + (counter_id * NV_COUNTER_SIZE);

    err = FLASH_DEV_NAME.ReadData(flash_addr, val, NV_COUNTER_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_set_nv_counter(enum tfm_nv_counter_t counter_id,
                                            uint32_t value)
{
    int32_t  err;
    uint32_t *p_nv_counter;
    uint8_t  sector_data[TFM_NV_COUNTERS_SECTOR_SIZE];

    /* Read the whole sector to be able to erase and write later in the flash */
    err = FLASH_DEV_NAME.ReadData(TFM_NV_COUNTERS_SECTOR_ADDR, sector_data,
                                  TFM_NV_COUNTERS_SECTOR_SIZE);
    if (err != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    /* Set the pointer to nv counter position */
    p_nv_counter = (uint32_t *)(sector_data + NV_COUNTERS_AREA_OFFSET +
                                (counter_id * NV_COUNTER_SIZE));

    if (value != *p_nv_counter) {

        if (value > *p_nv_counter) {
            *p_nv_counter = value;
        } else {
            return TFM_PLAT_ERR_INVALID_INPUT;
        }

        /* Erase sector before write in it */
        err = FLASH_DEV_NAME.EraseSector(TFM_NV_COUNTERS_SECTOR_ADDR);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }

        /* Write in flash the in-memory block content after modification */
        err = FLASH_DEV_NAME.ProgramData(TFM_NV_COUNTERS_SECTOR_ADDR,
                                         sector_data,
                                         TFM_NV_COUNTERS_SECTOR_SIZE);
        if (err != ARM_DRIVER_OK) {
            return TFM_PLAT_ERR_SYSTEM_ERR;
        }
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_plat_increment_nv_counter(
                                           enum tfm_nv_counter_t counter_id)
{
    uint32_t security_cnt;
    enum tfm_plat_err_t err;

    err = tfm_plat_read_nv_counter(counter_id,
                                   sizeof(security_cnt),
                                   (uint8_t *)&security_cnt);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (security_cnt == UINT32_MAX) {
        return TFM_PLAT_ERR_MAX_VALUE;
    }

    return tfm_plat_set_nv_counter(counter_id, security_cnt + 1u);
}
>>>>>>> add_ra_bl2_support:platform/ext/target/renesas/ra/dummy_nv_counters.c
