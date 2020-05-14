/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This file is derivative of CMSIS V5.01 Device\_Template_Vendor\Vendor\Device\Include\Device.h
 * Git SHA: 8a1d9d6ee18b143ae5befefa14d89fb5b3f99c75
 */

#ifndef __RA6M4_BASE_ADDRESS_H__
#define __RA6M4_BASE_ADDRESS_H__

#ifdef __cplusplus
extern "C" {
#endif



/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */

/** @addtogroup Device_Peripheral_peripheralAddr
 * @{
 */

 #define R_MPU_BASE            0x40000000
 #define R_TZF_BASE            0x40000E00
 #define R_SRAM_BASE           0x40002000
 #define R_BUS_BASE            0x40003000
 #define R_DMAC0_BASE          0x40005000
 #define R_DMAC1_BASE          0x40005040
 #define R_DMAC2_BASE          0x40005080
 #define R_DMAC3_BASE          0x400050C0
 #define R_DMAC4_BASE          0x40005100
 #define R_DMAC5_BASE          0x40005140
 #define R_DMAC6_BASE          0x40005180
 #define R_DMAC7_BASE          0x400051C0
 #define R_DMA_BASE            0x40005200
 #define R_DTC_BASE            0x40005400
 #define R_ICU_BASE            0x40006000
 #define R_CPSCU_BASE          0x40008000
 #define R_DBG_BASE            0x4001B000
 #define R_FCACHE_BASE         0x4001C000
 #define R_SYSC_BASE           0x4001E000
 #define R_PFS_BASE            0x40080800
 #define R_ELC_BASE            0x40082000
 #define R_TC_BASE             0x40083000
 #define R_IWDT_BASE           0x40083200
 #define R_WDT_BASE            0x40083400
 #define R_CAC_BASE            0x40083600
 #define R_MSTP_BASE           0x40084004

// #define R_MSTP_BASE 0x40084000
 #define R_POEG_BASE           0x4008A000
 #define R_USBFS_BASE          0x40090000
 #define R_SDHI0_BASE          0x40092000
 #define R_SSI0_BASE           0x4009D000
 #define R_IIC0_BASE           0x4009F000
 #define R_IIC0WU_BASE         0x4009F014
 #define R_IIC1_BASE           0x4009F100
 #define R_OSPI_BASE           0x400A6000
 #define R_CAN0_BASE           0x400A8000
 #define R_CAN1_BASE           0x400A9000
 #define R_CTSU_BASE           0x400D0000
 #define R_PSCU_BASE           0x400E0000
 #define R_AGT0_BASE           0x400E8000
 #define R_AGT1_BASE           0x400E8100
 #define R_AGT2_BASE           0x400E8200
 #define R_AGT3_BASE           0x400E8300
 #define R_AGT4_BASE           0x400E8400
 #define R_AGT5_BASE           0x400E8500
 #define R_TSN_CTRL_BASE       0x400F3000
 #define R_CRC_BASE            0x40108000
 #define R_DOC_BASE            0x40109000
 #define R_EDMAC0_BASE         0x40114000
 #define R_ETHERC0_BASE        0x40114100
 #define R_SCI0_BASE           0x40118000
 #define R_SCI1_BASE           0x40118100
 #define R_SCI2_BASE           0x40118200
 #define R_SCI3_BASE           0x40118300
 #define R_SCI4_BASE           0x40118400
 #define R_SCI5_BASE           0x40118500
 #define R_SCI6_BASE           0x40118600
 #define R_SCI7_BASE           0x40118700
 #define R_SCI8_BASE           0x40118800
 #define R_SCI9_BASE           0x40118900
 #define R_SPI0_BASE           0x4011A000
 #define R_SPI1_BASE           0x4011A100
 #define R_GPT320_BASE         0x40169000
 #define R_GPT321_BASE         0x40169100
 #define R_GPT322_BASE         0x40169200
 #define R_GPT323_BASE         0x40169300
 #define R_GPT164_BASE         0x40169400
 #define R_GPT165_BASE         0x40169500
 #define R_GPT166_BASE         0x40169600
 #define R_GPT167_BASE         0x40169700
 #define R_GPT168_BASE         0x40169800
 #define R_GPT169_BASE         0x40169900
 #define R_GPT_OPS_BASE        0x40169A00
 #define R_ADC120_BASE         0x40170000
 #define R_ADC121_BASE         0x40170200
 #define R_DAC12_BASE          0x40171000
 #define R_FLAD_BASE           0x407FC000
 #define R_FACI_HP_CMD_BASE    0x407E0000
 #define R_FACI_HP_BASE        0x407FE000
 #define R_QSPI_BASE           0x64000000

/* Not included in SVD */
 #define R_PORT0_BASE          0x40080000
 #define R_PORT1_BASE          0x40080020
 #define R_PORT2_BASE          0x40080040
 #define R_PORT3_BASE          0x40080060
 #define R_PORT4_BASE          0x40080080
 #define R_PORT5_BASE          0x400800A0
 #define R_PORT6_BASE          0x400800C0
 #define R_PORT7_BASE          0x400800E0
 #define R_PORT8_BASE          0x40080100
 #define R_PORT9_BASE          0x40080120
 #define R_PORT10_BASE         0x40080140
 #define R_PORT11_BASE         0x40080160
 #define R_PFS_BASE            0x40080800
 #define R_PMISC_BASE          0x40080D00 // does not exist but FSP will not build without this

 #define R_GPT_POEG0_BASE      0x4008A000
 #define R_GPT_POEG1_BASE      0x4008A100
 #define R_GPT_POEG2_BASE      0x4008A200
 #define R_GPT_POEG3_BASE      0x4008A300

 #define R_RTC_BASE            0x40083000

/** @} */ /* End of group Device_Peripheral_peripheralAddr */

/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */

/** @addtogroup Device_Peripheral_declaration
 * @{
 */

// #define R_MPU ((R_MPU_Type *) R_MPU_BASE)
 #define R_TZF            ((R_TZF_Type *) R_TZF_BASE)
 #define R_SRAM           ((R_SRAM_Type *) R_SRAM_BASE)
 #define R_BUS            ((R_BUS_Type *) R_BUS_BASE)
 #define R_DMAC0          ((R_DMAC0_Type *) R_DMAC0_BASE)
 #define R_DMAC1          ((R_DMAC0_Type *) R_DMAC1_BASE)
 #define R_DMAC2          ((R_DMAC0_Type *) R_DMAC2_BASE)
 #define R_DMAC3          ((R_DMAC0_Type *) R_DMAC3_BASE)
 #define R_DMAC4          ((R_DMAC0_Type *) R_DMAC4_BASE)
 #define R_DMAC5          ((R_DMAC0_Type *) R_DMAC5_BASE)
 #define R_DMAC6          ((R_DMAC0_Type *) R_DMAC6_BASE)
 #define R_DMAC7          ((R_DMAC0_Type *) R_DMAC7_BASE)
 #define R_DMA            ((R_DMA_Type *) R_DMA_BASE)
 #define R_DTC            ((R_DTC_Type *) R_DTC_BASE)
 #define R_ICU            ((R_ICU_Type *) R_ICU_BASE)
 #define R_CPSCU          ((R_CPSCU_Type *) R_CPSCU_BASE)
 #define R_DEBUG          ((R_DEBUG_Type *) R_DBG_BASE)
 #define R_FCACHE         ((R_FCACHE_Type *) R_FCACHE_BASE)
 #define R_SYSTEM         ((R_SYSTEM_Type *) R_SYSC_BASE)
 #define R_PFS            ((R_PFS_Type *) R_PFS_BASE)
 #define R_ELC            ((R_ELC_Type *) R_ELC_BASE)
 #define R_TC             ((R_TC_Type *) R_TC_BASE)
 #define R_IWDT           ((R_IWDT_Type *) R_IWDT_BASE)
 #define R_WDT            ((R_WDT_Type *) R_WDT_BASE)
 #define R_CAC            ((R_CAC_Type *) R_CAC_BASE)
 #define R_MSTP           ((R_MSTP_Type *) R_MSTP_BASE)
 #define R_POEG           ((R_POEG_Type *) R_POEG_BASE)
 #define R_USBFS          ((R_USBFS_Type *) R_USBFS_BASE)
 #define R_SDHI0          ((R_SDHI0_Type *) R_SDHI0_BASE)
 #define R_SSI0           ((R_SSI0_Type *) R_SSI0_BASE)
 #define R_IIC0           ((R_IIC0_Type *) R_IIC0_BASE)
 #define R_IIC0WU         ((R_IIC0WU_Type *) R_IIC0WU_BASE)
 #define R_IIC1           ((R_IIC0_Type *) R_IIC1_BASE)
 #define R_OSPI           ((R_OSPI_Type *) R_OSPI_BASE)
 #define R_CAN0           ((R_CAN0_Type *) R_CAN0_BASE)
 #define R_CAN1           ((R_CAN0_Type *) R_CAN1_BASE)
 #define R_CTSU           ((R_CTSU_Type *) R_CTSU_BASE)
 #define R_PSCU           ((R_PSCU_Type *) R_PSCU_BASE)
 #define R_AGT0           ((R_AGT0_Type *) R_AGT0_BASE)
 #define R_AGT1           ((R_AGT0_Type *) R_AGT1_BASE)
 #define R_AGT2           ((R_AGT0_Type *) R_AGT2_BASE)
 #define R_AGT3           ((R_AGT0_Type *) R_AGT3_BASE)
 #define R_AGT4           ((R_AGT0_Type *) R_AGT4_BASE)
 #define R_AGT5           ((R_AGT0_Type *) R_AGT5_BASE)
 #define R_TSN_CTRL       ((R_TSN_CTRL_Type *) R_TSN_CTRL_BASE)
 #define R_CRC            ((R_CRC_Type *) R_CRC_BASE)
 #define R_DOC            ((R_DOC_Type *) R_DOC_BASE)
 #define R_EDMAC0         ((R_EDMAC0_Type *) R_EDMAC0_BASE)
 #define R_ETHERC0        ((R_ETHERC0_Type *) R_ETHERC0_BASE)
 #define R_SCI0           ((R_SCI0_Type *) R_SCI0_BASE)
 #define R_SCI1           ((R_SCI0_Type *) R_SCI1_BASE)
 #define R_SCI2           ((R_SCI0_Type *) R_SCI2_BASE)
 #define R_SCI3           ((R_SCI0_Type *) R_SCI3_BASE)
 #define R_SCI4           ((R_SCI0_Type *) R_SCI4_BASE)
 #define R_SCI5           ((R_SCI0_Type *) R_SCI5_BASE)
 #define R_SCI6           ((R_SCI0_Type *) R_SCI6_BASE)
 #define R_SCI7           ((R_SCI0_Type *) R_SCI7_BASE)
 #define R_SCI8           ((R_SCI0_Type *) R_SCI8_BASE)
 #define R_SCI9           ((R_SCI0_Type *) R_SCI9_BASE)
 #define R_SPI0           ((R_SPI0_Type *) R_SPI0_BASE)
 #define R_SPI1           ((R_SPI0_Type *) R_SPI1_BASE)
 #define R_GPT0           ((R_GPT0_Type *) R_GPT320_BASE)
 #define R_GPT1           ((R_GPT0_Type *) R_GPT321_BASE)
 #define R_GPT2           ((R_GPT0_Type *) R_GPT322_BASE)
 #define R_GPT3           ((R_GPT0_Type *) R_GPT323_BASE)
 #define R_GPT4           ((R_GPT0_Type *) R_GPT164_BASE)
 #define R_GPT5           ((R_GPT0_Type *) R_GPT165_BASE)
 #define R_GPT6           ((R_GPT0_Type *) R_GPT166_BASE)
 #define R_GPT7           ((R_GPT0_Type *) R_GPT167_BASE)
 #define R_GPT8           ((R_GPT0_Type *) R_GPT168_BASE)
 #define R_GPT9           ((R_GPT0_Type *) R_GPT169_BASE)
 #define R_GPT_OPS        ((R_GPT_OPS_Type *) R_GPT_OPS_BASE)
 #define R_ADC0           ((R_ADC0_Type *) R_ADC120_BASE)
 #define R_ADC1           ((R_ADC0_Type *) R_ADC121_BASE)
 #define R_DAC            ((R_DAC_Type *) R_DAC12_BASE)
 #define R_FLAD           ((R_FLAD_Type *) R_FLAD_BASE)
 #define R_FACI_HP_CMD    ((R_FACI_HP_CMD_Type *) R_FACI_HP_CMD_BASE)
 #define R_FACI_HP        ((R_FACI_HP_Type *) R_FACI_HP_BASE)
 #define R_QSPI           ((R_QSPI_Type *) R_QSPI_BASE)

/* Not in SVD. */

 #define R_PORT0          ((R_PORT0_Type *) R_PORT0_BASE)
 #define R_PORT1          ((R_PORT0_Type *) R_PORT1_BASE)
 #define R_PORT2          ((R_PORT0_Type *) R_PORT2_BASE)
 #define R_PORT3          ((R_PORT0_Type *) R_PORT3_BASE)
 #define R_PORT4          ((R_PORT0_Type *) R_PORT4_BASE)
 #define R_PORT5          ((R_PORT0_Type *) R_PORT5_BASE)
 #define R_PORT6          ((R_PORT0_Type *) R_PORT6_BASE)
 #define R_PORT7          ((R_PORT0_Type *) R_PORT7_BASE)
 #define R_PORT8          ((R_PORT0_Type *) R_PORT8_BASE)
 #define R_PORT9          ((R_PORT0_Type *) R_PORT9_BASE)
 #define R_PORT10         ((R_PORT0_Type *) R_PORT10_BASE)
 #define R_PORT11         ((R_PORT0_Type *) R_PORT11_BASE)
 #define R_PFS            ((R_PFS_Type *) R_PFS_BASE)
 #define R_PMISC          ((R_PMISC_Type *) R_PMISC_BASE)

 #define R_GPT_POEG0      ((R_GPT_POEG0_Type *) R_GPT_POEG0_BASE)
 #define R_GPT_POEG1      ((R_GPT_POEG0_Type *) R_GPT_POEG1_BASE)
 #define R_GPT_POEG2      ((R_GPT_POEG0_Type *) R_GPT_POEG2_BASE)
 #define R_GPT_POEG3      ((R_GPT_POEG0_Type *) R_GPT_POEG3_BASE)

 #define R_RTC            ((R_RTC_Type *) R_RTC_BASE)

/** @} */ /* End of group Device_Peripheral_declaration */



#ifdef __cplusplus
}
#endif

#endif  /* __RA6M4_BASE_ADDRESS_H__ */
