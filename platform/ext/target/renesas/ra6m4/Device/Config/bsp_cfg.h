/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef BSP_CFG_H
#define BSP_CFG_H

#include "bsp_clock_cfg.h"

 #include "bsp_mcu_family_cfg_ra6m4.h"
 #include "bsp_cfg_ra6m4.h"

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
