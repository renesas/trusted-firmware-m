/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef BSP_PIN_CFG_H
#define BSP_PIN_CFG_H

#ifdef RA2L1_EEK_LPM_BUILD
 #define RA2L1_EEK_BUILD
#endif

/***********************************************************************************************************************
 * Choose pin configuration file for current build
 **********************************************************************************************************************/
#if defined(RA2A1_EK_BUILD)
 #include "bsp_pin_cfg_ra2a1_tbb.h"
#endif
#if defined(RA2A1_TBB_BUILD)
 #include "bsp_pin_cfg_ra2a1_tbb.h"
#endif
#if defined(RA2L1_EEK_BUILD)
 #include "bsp_pin_cfg_ra2l1_eek.h"
#endif
#if defined(RA4M1_TBB_BUILD) || defined(RA4M1_EK_BUILD)
 #include "bsp_pin_cfg_ra4m1_tbb.h"
#endif
#if defined(RA4W1_ADK_BUILD)
 #include "bsp_pin_cfg_ra4w1_adk.h"
#endif
#if defined(RA6M1_TBB_BUILD) || defined(RA6M1_EK_BUILD)
 #include "bsp_pin_cfg_ra6m1_tbb.h"
#endif
#if defined(RA6M2_TBB_BUILD)
 #include "bsp_pin_cfg_ra6m2_tbb.h"
#endif
#if defined(RA6M3_DK_BUILD)
 #include "bsp_pin_cfg_ra6m3_dk.h"
#endif
#if defined(RA6M3_PK_BUILD) || defined(RA6M3_EK_BUILD)
 #include "bsp_pin_cfg_ra6m3_pk.h"
#endif

#endif
