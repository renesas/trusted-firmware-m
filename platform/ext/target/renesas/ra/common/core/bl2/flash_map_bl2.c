/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* Platform MCUboot flash map. The default TF-M flash map handles at most two
 * images; the RA platform also manages the CPU1 (Cortex-M33) NS image.
 *
 * As in the default flash map, FLASH_DEV_NAME_<n> and FLASH_DEVICE_ID_<n> can
 * be defined by the platform (flash_layout.h) to place FLASH_AREA_<n> on
 * another flash device, e.g. external flash for the NS images. FLASH_DRIVER_LIST
 * must then list every driver to initialize, each one once.
 */

#include <stdbool.h>
#include "flash_map/flash_map.h"
#include "target.h"
#include "Driver_Flash.h"
#include "tfm_utils.h"

/* target.h only checks areas 0 to 3 */
#if defined(FLASH_DEV_NAME_4) != defined(FLASH_DEVICE_ID_4)
#error "FLASH DEV_NAME_4 and DEVICE_ID_4 must be simultaneously defined or not by target"
#endif

#if defined(FLASH_DEV_NAME_5) != defined(FLASH_DEVICE_ID_5)
#error "FLASH DEV_NAME_5 and DEVICE_ID_5 must be simultaneously defined or not by target"
#endif

/* When undefined FLASH_DEV_NAME_0 or FLASH_DEVICE_ID_0 , default */
#if !defined(FLASH_DEV_NAME_0) || !defined(FLASH_DEVICE_ID_0)
#define FLASH_DEV_NAME_0  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_0 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_1 or FLASH_DEVICE_ID_1 , default */
#if !defined(FLASH_DEV_NAME_1) || !defined(FLASH_DEVICE_ID_1)
#define FLASH_DEV_NAME_1  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_1 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_2 or FLASH_DEVICE_ID_2 , default */
#if !defined(FLASH_DEV_NAME_2) || !defined(FLASH_DEVICE_ID_2)
#define FLASH_DEV_NAME_2  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_2 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_3 or FLASH_DEVICE_ID_3 , default */
#if !defined(FLASH_DEV_NAME_3) || !defined(FLASH_DEVICE_ID_3)
#define FLASH_DEV_NAME_3  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_3 FLASH_DEVICE_ID
#endif

#if (MCUBOOT_IMAGE_NUMBER == 3)
/* When undefined FLASH_DEV_NAME_4 or FLASH_DEVICE_ID_4 , default */
#if !defined(FLASH_DEV_NAME_4) || !defined(FLASH_DEVICE_ID_4)
#define FLASH_DEV_NAME_4  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_4 FLASH_DEVICE_ID
#endif

/* When undefined FLASH_DEV_NAME_5 or FLASH_DEVICE_ID_5 , default */
#if !defined(FLASH_DEV_NAME_5) || !defined(FLASH_DEVICE_ID_5)
#define FLASH_DEV_NAME_5  FLASH_DEV_NAME
#define FLASH_DEVICE_ID_5 FLASH_DEVICE_ID
#endif
#endif /* (MCUBOOT_IMAGE_NUMBER == 3) */

/* Flash device names must be specified by target */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_0;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_1;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_2;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_3;
#if (MCUBOOT_IMAGE_NUMBER == 3)
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_4;
extern ARM_DRIVER_FLASH FLASH_DEV_NAME_5;
#endif

#if !defined(FLASH_DRIVER_LIST)
/* Default Drivers list */
const ARM_DRIVER_FLASH *flash_driver[] = {
    &FLASH_DEV_NAME,
};
#else
/* Platform driver list */
const ARM_DRIVER_FLASH *flash_driver[] = FLASH_DRIVER_LIST;
#endif /* !defined(FLASH_DRIVER_LIST) */
const int flash_driver_entry_num = ARRAY_SIZE(flash_driver);

const struct flash_area flash_map[] = {
    {
        .fa_id = FLASH_AREA_0_ID,
        .fa_device_id = FLASH_DEVICE_ID_0,
        .fa_driver = &FLASH_DEV_NAME_0,
        .fa_off = FLASH_AREA_0_OFFSET,
        .fa_size = FLASH_AREA_0_SIZE,
    },
    {
        .fa_id = FLASH_AREA_1_ID,
        .fa_device_id = FLASH_DEVICE_ID_1,
        .fa_driver = &FLASH_DEV_NAME_1,
        .fa_off = FLASH_AREA_1_OFFSET,
        .fa_size = FLASH_AREA_1_SIZE,
    },
    {
        .fa_id = FLASH_AREA_2_ID,
        .fa_device_id = FLASH_DEVICE_ID_2,
        .fa_driver = &FLASH_DEV_NAME_2,
        .fa_off = FLASH_AREA_2_OFFSET,
        .fa_size = FLASH_AREA_2_SIZE,
    },
    {
        .fa_id = FLASH_AREA_3_ID,
        .fa_device_id = FLASH_DEVICE_ID_3,
        .fa_driver = &FLASH_DEV_NAME_3,
        .fa_off = FLASH_AREA_3_OFFSET,
        .fa_size = FLASH_AREA_3_SIZE,
    },
#if (MCUBOOT_IMAGE_NUMBER == 3)
    {
        .fa_id = FLASH_AREA_4_ID,
        .fa_device_id = FLASH_DEVICE_ID_4,
        .fa_driver = &FLASH_DEV_NAME_4,
        .fa_off = FLASH_AREA_4_OFFSET,
        .fa_size = FLASH_AREA_4_SIZE,
    },
    {
        .fa_id = FLASH_AREA_5_ID,
        .fa_device_id = FLASH_DEVICE_ID_5,
        .fa_driver = &FLASH_DEV_NAME_5,
        .fa_off = FLASH_AREA_5_OFFSET,
        .fa_size = FLASH_AREA_5_SIZE,
    },
#endif /* (MCUBOOT_IMAGE_NUMBER == 3) */
};

const int flash_map_entry_num = ARRAY_SIZE(flash_map);
