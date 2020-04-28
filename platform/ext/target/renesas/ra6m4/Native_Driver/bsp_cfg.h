/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef BSP_CFG_H
#define BSP_CFG_H

#ifdef RA2L1_EEK_LPM_BUILD
 #define RA2L1_EEK_BUILD
#endif

#include "bsp_clock_cfg.h"

#if defined(RA2A1_ADK_BUILD) || defined(RA2A1_TBB_BUILD) || defined(RA2A1_EK_BUILD)
 #define BSP_MCU_R7FA2A1AB3CFM
 #include "bsp_mcu_family_cfg_ra2a1.h"
 #include "bsp_cfg_ra2a1.h"
 #if defined(RA2A1_ADK_BUILD)
  #include "../../../board/ra2a1_adk/board.h"
 #endif
 #if defined(RA2A1_TBB_BUILD)
  #include "../../../board/ra2a1_tbb/board.h"
 #endif
 #if defined(RA2A1_EK_BUILD)
  #include "../../../board/ra2a1_ek/board.h"
 #endif
#endif
#if defined(RA2L1_EEK_BUILD)
 #define BSP_MCU_R7FA2L1783A01CFP
 #include "bsp_mcu_family_cfg_ra2l1.h"
 #include "bsp_cfg_ra2l1.h"
 #include "../../../board/ra2l1_eek/board.h"
#endif
#if defined(RA4M1_TBB_BUILD) || defined(RA4M1_EK_BUILD)
 #define BSP_MCU_R7FA4M1AB3CFP
 #include "bsp_mcu_family_cfg_ra4m1.h"
 #include "bsp_cfg_ra4m1.h"
 #if defined(RA4M1_TBB_BUILD)
  #include "../../../board/ra4m1_tbb/board.h"
 #endif
 #if defined(RA4M1_EK_BUILD)
  #include "../../../board/ra4m1_ek/board.h"
 #endif
#endif
#if defined(RA4W1_ADK_BUILD)
 #define BSP_MCU_R7FA4W1AB3CFP
 #include "bsp_mcu_family_cfg_ra4w1.h"
 #include "../../../board/ra4w1_adk/board.h"
 #include "bsp_cfg_ra4w1.h"
#endif
#if defined(RA6M1_TBB_BUILD) || defined(RA6M1_EK_BUILD)
 #define BSP_MCU_R7FA6M1AD3CFP
 #include "bsp_mcu_family_cfg_ra6m1.h"
 #include "bsp_cfg_ra6m1.h"
 #if defined(RA6M1_TBB_BUILD)
  #include "../../../board/ra6m1_tbb/board.h"
 #endif
 #if defined(RA6M1_EK_BUILD)
  #include "../../../board/ra6m1_ek/board.h"
 #endif
#endif
#if defined(RA6M2_TBB_BUILD)
 #define BSP_MCU_R7FA6M2AF3CFB
 #include "bsp_mcu_family_cfg_ra6m2.h"
 #include "../../../board/ra6m2_tbb/board.h"
 #include "bsp_cfg_ra6m2.h"
#endif
#if defined(RA6M3_DK_BUILD) || defined(RA6M3_PK_BUILD) || defined(RA6M3_EK_BUILD)
 #define BSP_MCU_R7FA6M3AH3CFC
 #include "bsp_mcu_family_cfg_ra6m3.h"
 #include "bsp_cfg_ra6m3.h"
 #if defined(RA6M3_DK_BUILD)
  #include "../../../board/ra6m3_dk/board.h"
 #endif
 #if defined(RA6M3_PK_BUILD)
  #include "../../../board/ra6m3_pk/board.h"
 #endif
 #if defined(RA6M3_EK_BUILD)
  #include "../../../board/ra6m3_ek/board.h"
 #endif
#endif
#if defined(RA6M4_EK_BUILD)
 #include "bsp_mcu_family_cfg_ra6m4.h"
 #include "bsp_cfg_ra6m4.h"
 #if defined(RA6M4_EK_BUILD)
  #include "../../../board/ra6m4_ek/board.h"
 #endif
#endif

#ifndef BSP_CFG_SOFT_RESET_SUPPORTED
 #define BSP_CFG_SOFT_RESET_SUPPORTED               (0)
#endif

#ifndef BSP_CLOCK_CFG_MAIN_OSC_POPULATED
 #define BSP_CLOCK_CFG_MAIN_OSC_POPULATED           (0)
#endif
#ifndef BSP_CLOCK_CFG_MAIN_OSC_WAIT
 #define BSP_CLOCK_CFG_MAIN_OSC_WAIT                (9)
#endif
#ifndef BSP_CLOCK_CFG_MAIN_OSC_CLOCK_SOURCE
 #define BSP_CLOCK_CFG_MAIN_OSC_CLOCK_SOURCE        (0)
#endif
#ifndef BSP_CLOCK_CFG_SUBCLOCK_POPULATED
 #define BSP_CLOCK_CFG_SUBCLOCK_POPULATED           (0)
#endif
#ifndef BSP_CLOCK_CFG_SUBCLOCK_DRIVE
 #define BSP_CLOCK_CFG_SUBCLOCK_DRIVE               (0)
#endif
#ifndef BSP_CLOCK_CFG_SUBCLOCK_STABILIZATION_MS
 #define BSP_CLOCK_CFG_SUBCLOCK_STABILIZATION_MS    (1000U)
#endif

#endif
