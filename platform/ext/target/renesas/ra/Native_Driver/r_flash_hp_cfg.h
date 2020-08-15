/* ${REA_DISCLAIMER_PLACEHOLDER} */

/*******************************************************************************************************************//**
 * @addtogroup FLASH
 * @{
 **********************************************************************************************************************/

#ifndef R_FLASH_HP_CFG_H
#define R_FLASH_HP_CFG_H

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Configuration Options
 **********************************************************************************************************************/

/* SPECIFY WHETHER TO INCLUDE CODE FOR API PARAMETER CHECKING */

/* Setting to BSP_CFG_PARAM_CHECKING_ENABLE utilizes the system default setting
 * Setting to 1 includes parameter checking; 0 compiles out parameter checking
 */
#ifndef FLASH_HP_CFG_PARAM_CHECKING_ENABLE
 #define FLASH_HP_CFG_PARAM_CHECKING_ENABLE    (BSP_CFG_PARAM_CHECKING_ENABLE)
#endif

/* SPECIFY IF CODE FLASH PROGRAMMING WILL BE ENABLED */

/* Setting to 1 allows Code Flash programming
 * Setting to 0 disables Code Flash programming
 */
#ifndef FLASH_HP_CFG_CODE_FLASH_PROGRAMMING_ENABLE
 #define FLASH_HP_CFG_CODE_FLASH_PROGRAMMING_ENABLE    (1)
#endif

/* Setting to 1 allows Data Flash programming
 * Setting to 0 disables Data Flash programming
 */
#ifndef FLASH_HP_CFG_DATA_FLASH_PROGRAMMING_ENABLE
 #define FLASH_HP_CFG_DATA_FLASH_PROGRAMMING_ENABLE    (1)
#endif

#if TEST_STATIC
 #define static
#endif

#endif

/*******************************************************************************************************************//**
 * @} (end addtogroup FLASH)
 **********************************************************************************************************************/
