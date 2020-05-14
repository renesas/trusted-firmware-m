/* generated configuration header file - do not edit */
#ifndef BSP_MCU_FAMILY_CFG_RA6M4_H
#define BSP_MCU_FAMILY_CFG_RA6M4_H
#include "bsp_mcu_info.h"
#include "bsp_clock_cfg.h"
#define BSP_MCU_GROUP_RA6M4                    (1)
#define BSP_LOCO_HZ                            (32768)
#define BSP_MOCO_HZ                            (8000000)
#define BSP_SUB_CLOCK_HZ                       (32768)
#if   BSP_CFG_HOCO_FREQUENCY == 0
 #define BSP_HOCO_HZ                           (16000000)
#elif BSP_CFG_HOCO_FREQUENCY == 1
 #define BSP_HOCO_HZ                           (18000000)
#elif BSP_CFG_HOCO_FREQUENCY == 2
 #define BSP_HOCO_HZ                           (20000000)
#else
 #error "Invalid HOCO frequency chosen (BSP_CFG_HOCO_FREQUENCY) in bsp_clock_cfg.h"
#endif

#define BSP_CORTEX_VECTOR_TABLE_ENTRIES        (16U)
#define BSP_VECTOR_TABLE_MAX_ENTRIES           (112U)
#define BSP_MCU_VBATT_SUPPORT                  (1)

#define _RA_TZ_SECURE 1

#if defined(_RA_TZ_SECURE)
#define BSP_TZ_CFG_SECURE_BUILD           (1)
#define BSP_TZ_CFG_NONSECURE_BUILD        (0)
#define BSP_TZ_SECURE_BUILD           (1)
#define BSP_TZ_NONSECURE_BUILD        (0)
#elif defined(_RA_TZ_NONSECURE)
#define BSP_TZ_CFG_SECURE_BUILD           (0)
#define BSP_TZ_CFG_NONSECURE_BUILD        (1)
#else
#define BSP_TZ_CFG_SECURE_BUILD           (0)
#define BSP_TZ_CFG_NONSECURE_BUILD        (0)
#endif

            /* TrustZone Settings */
            #define BSP_TZ_CFG_SECURE_PROJECT         (BSP_TZ_CFG_SECURE_BUILD && BSP_FEATURE_TZ_HAS_TRUSTZONE)
            #define BSP_TZ_CFG_SECURE_BSP             (1)
            #define BSP_TZ_CFG_SKIP_INIT              !((BSP_TZ_CFG_SECURE_BUILD && BSP_TZ_CFG_SECURE_BSP) || (BSP_TZ_CFG_NONSECURE_BUILD && !BSP_TZ_CFG_SECURE_BSP) || (!BSP_TZ_CFG_SECURE_BUILD && !BSP_TZ_CFG_NONSECURE_BUILD))
            #define BSP_TZ_CFG_USER_INIT              (1)
            #define BSP_TZ_CFG_EXCEPTION_RESPONSE     (1) //nmi
            #define BSP_TZ_CFG_NMI_NON_SECURE_ONLY    (0)
            #define BSP_TZ_CFG_SECURE_PRIORITY_BOOST  (1)
            #define BSP_TZ_CFG_FPU_SECURE_ONLY        (3)
            #define BSP_TZ_CFG_FPU_SECURE_REGS        (0)

            /* Peripheral Security Attribution Register (PSAR) Settings */
            
            #define BSP_TZ_CFG_PSARB               (0xFFFFFFFF)
            #define BSP_TZ_CFG_PSARC               (0xFFFFFFFF)
            #define BSP_TZ_CFG_PSARD               (0xFFFFFFFF)
            #define BSP_TZ_CFG_PSARE               (0xFFFFFFFF)
            #define BSP_TZ_CFG_MSSAR               (0xFFFFFFFF)

            #define BSP_CFG_ROM_REG_DUALSEL (0xFFFFFFFFU)

            #define BSP_CFG_ROM_REG_OFS1_SECURITY_ATTRIBUTION (0xFFFFFFFE)


#ifndef OFS_SEQ1
 #define OFS_SEQ1                              (0xA001A001U) | (1U << 1) | (3U << 2)
#endif
#ifndef OFS_SEQ2
 #define OFS_SEQ2                              (15U << 4) | (3U << 8) | (3U << 10)
#endif
#ifndef OFS_SEQ3
 #define OFS_SEQ3                              (1U << 12) | (1U << 14) | (1U << 17)
#endif
#ifndef OFS_SEQ4
 #define OFS_SEQ4                              (3U << 18) | (15U << 20) | (3U << 24) | (3U << 26)
#endif
#ifndef OFS_SEQ5
 #define OFS_SEQ5                              (1U << 28) | (1U << 30)
#endif
#ifndef BSP_CFG_ROM_REG_OFS0
 #define BSP_CFG_ROM_REG_OFS0                  (OFS_SEQ1 | OFS_SEQ2 | OFS_SEQ3 | OFS_SEQ4 | OFS_SEQ5)
#endif
#ifndef BSP_CFG_ROM_REG_OFS1
 #define BSP_CFG_ROM_REG_OFS1                  (0xFFFFFEF8U | (1U << 2) | (3U) | (1U << 8))
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC0_ENABLE
 #define BSP_CFG_ROM_REG_MPU_PC0_ENABLE        (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC0_START
 #define BSP_CFG_ROM_REG_MPU_PC0_START         (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC0_END
 #define BSP_CFG_ROM_REG_MPU_PC0_END           (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC1_ENABLE
 #define BSP_CFG_ROM_REG_MPU_PC1_ENABLE        (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC1_START
 #define BSP_CFG_ROM_REG_MPU_PC1_START         (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_PC1_END
 #define BSP_CFG_ROM_REG_MPU_PC1_END           (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION0_ENABLE
 #define BSP_CFG_ROM_REG_MPU_REGION0_ENABLE    (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION0_START
 #define BSP_CFG_ROM_REG_MPU_REGION0_START     (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION0_END
 #define BSP_CFG_ROM_REG_MPU_REGION0_END       (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION1_ENABLE
 #define BSP_CFG_ROM_REG_MPU_REGION1_ENABLE    (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION1_START
 #define BSP_CFG_ROM_REG_MPU_REGION1_START     (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION1_END
 #define BSP_CFG_ROM_REG_MPU_REGION1_END       (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION2_ENABLE
 #define BSP_CFG_ROM_REG_MPU_REGION2_ENABLE    (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION2_START
 #define BSP_CFG_ROM_REG_MPU_REGION2_START     (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION2_END
 #define BSP_CFG_ROM_REG_MPU_REGION2_END       (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION3_ENABLE
 #define BSP_CFG_ROM_REG_MPU_REGION3_ENABLE    (1)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION3_START
 #define BSP_CFG_ROM_REG_MPU_REGION3_START     (0xFFFFFFFFU)
#endif
#ifndef BSP_CFG_ROM_REG_MPU_REGION3_END
 #define BSP_CFG_ROM_REG_MPU_REGION3_END       (0xFFFFFFFFU)
#endif

/* Used to create IELS values for the interrupt initialization table g_interrupt_event_link_select. */
#define BSP_PRV_IELS_ENUM(vector)    (ELC_ ## vector)
#endif
