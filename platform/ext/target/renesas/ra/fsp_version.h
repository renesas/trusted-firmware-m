/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef FSP_VERSION_H
#define FSP_VERSION_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

/* Includes board and MCU related header files. */
#include "bsp_api.h"

/*******************************************************************************************************************//**
 * @addtogroup RENESAS_COMMON
 * @{
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** FSP pack major version. */
#define FSP_VERSION_MAJOR           (0U)

/** FSP pack minor version. */
#define FSP_VERSION_MINOR           (0U)

/** FSP pack patch version. */
#define FSP_VERSION_PATCH           (0U)

/** FSP pack version build number (currently unused). */
#define FSP_VERSION_BUILD           (0U)

/** Public FSP version name. */
#define FSP_VERSION_STRING          ("##FSP_VERSION##")

/** Unique FSP version ID. */
#define FSP_VERSION_BUILD_STRING    ("##FSP_VERSION_BUILD_STRING##")

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** FSP Pack version structure */
typedef union st_fsp_pack_version
{
    /** Version id */
    uint32_t version_id;

    /** Code version parameters, little endian order. */
    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in FSP code. */
    struct
    {
        uint8_t build;                 ///< Build version of FSP Pack
        uint8_t patch;                 ///< Patch version of FSP Pack
        uint8_t minor;                 ///< Minor version of FSP Pack
        uint8_t major;                 ///< Major version of FSP Pack
    };
} fsp_pack_version_t;

/** @} */

#endif
