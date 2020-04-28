/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef BSP_CLOCK_CFG_H
#define BSP_CLOCK_CFG_H

#ifdef RA2L1_EEK_LPM_BUILD
 #define RA2L1_EEK_BUILD
#endif

/***********************************************************************************************************************
 * Choose pin configuration file for current build
 **********************************************************************************************************************/
#if defined(RA2A1_TBB_BUILD) || defined(RA2A1_EK_BUILD)
 #include "bsp_clock_cfg_ra2a1.h"
#endif
#if defined(RA2L1_EEK_BUILD)
 #include "bsp_clock_cfg_ra2l1.h"
#endif
#if defined(RA4M1_TBB_BUILD) || defined(RA4M1_EK_BUILD)
 #include "bsp_clock_cfg_ra4m1.h"
#endif
#if defined(RA4W1_ADK_BUILD)
 #include "bsp_clock_cfg_ra4w1.h"
#endif
#if defined(RA6M1_TBB_BUILD) || defined(RA6M1_EK_BUILD)
 #include "bsp_clock_cfg_ra6m1.h"
#endif
#if defined(RA6M2_TBB_BUILD)
 #include "bsp_clock_cfg_ra6m2.h"
#endif
#if defined(RA6M3_DK_BUILD) || defined(RA6M3_PK_BUILD) || defined(RA6M3_EK_BUILD)
 #include "bsp_clock_cfg_ra6m3.h"
#endif
#if defined(RA6M4_EK_BUILD)
 #include "bsp_clock_cfg_ra6m4.h"
#endif

#ifndef BSP_CFG_CLKOUT_SOURCE
 #define BSP_CFG_CLKOUT_SOURCE    BSP_CLOCKS_SOURCE_CLOCK_MOCO
 #define BSP_CFG_CLKOUT_DIV       BSP_CLOCKS_SYS_CLOCK_DIV_2
#endif
#endif
