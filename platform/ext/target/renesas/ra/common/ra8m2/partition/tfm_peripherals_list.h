/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_LIST_H__
#define __TFM_PERIPHERALS_LIST_H__

/*
 * Secure peripherals that a Secure Partition can map with "mmio_regions".
 *
 * RA_PERIPHERALS(X, SEP) is the list of the peripherals: it calls the function-like
 * macro X once for each of them, with the fields as arguments, each call followed
 * by SEP (a ';' to declare or define, empty for a list of values):
 *
 *   X(name, secure, bank, bit, base, size)
 *
 *   name       - platform data tfm_peripheral_<name>, manifest name
 *                TFM_PERIPHERAL_<NAME>
 *   secure     - 1 for the peripheral to be mapped, with
 *                RA_PSAR_SECURE(<X>) = 1 when RENESAS_RA_PSAR_NS_<X> is OFF
 *   bank, bit  - PPC bank (PPC_SP_PERIPH_<bank>) and bit of the module
 *   base, size - 32 byte aligned base address and size of the register block
 *
 * A peripheral is added both to the manifest names and to RA_PERIPHERALS, in
 * the order of bank and bit.
 */

/* Manifest names */
#define TFM_PERIPHERAL_I3C0 (&tfm_peripheral_i3c0)
#define TFM_PERIPHERAL_IIC2 (&tfm_peripheral_iic2)
#define TFM_PERIPHERAL_IIC1 (&tfm_peripheral_iic1)
#define TFM_PERIPHERAL_IIC0 (&tfm_peripheral_iic0)
#define TFM_PERIPHERAL_USBFS0 (&tfm_peripheral_usbfs0)
#define TFM_PERIPHERAL_USBHS0 (&tfm_peripheral_usbhs0)
#define TFM_PERIPHERAL_OSPI0 (&tfm_peripheral_ospi0)
#define TFM_PERIPHERAL_DOTF0 (&tfm_peripheral_dotf0)
#define TFM_PERIPHERAL_OSPI1 (&tfm_peripheral_ospi1)
#define TFM_PERIPHERAL_DOTF1 (&tfm_peripheral_dotf1)
#define TFM_PERIPHERAL_RSPI1 (&tfm_peripheral_rspi1)
#define TFM_PERIPHERAL_RSPI0 (&tfm_peripheral_rspi0)
#define TFM_PERIPHERAL_SCI9 (&tfm_peripheral_sci9)
#define TFM_PERIPHERAL_SCI8 (&tfm_peripheral_sci8)
#define TFM_PERIPHERAL_SCI7 (&tfm_peripheral_sci7)
#define TFM_PERIPHERAL_SCI6 (&tfm_peripheral_sci6)
#define TFM_PERIPHERAL_SCI5 (&tfm_peripheral_sci5)
#define TFM_PERIPHERAL_SCI4 (&tfm_peripheral_sci4)
#define TFM_PERIPHERAL_SCI3 (&tfm_peripheral_sci3)
#define TFM_PERIPHERAL_SCI2 (&tfm_peripheral_sci2)
#define TFM_PERIPHERAL_SCI1 (&tfm_peripheral_sci1)
#define TFM_PERIPHERAL_SCI0 (&tfm_peripheral_sci0)
#define TFM_PERIPHERAL_CAC (&tfm_peripheral_cac)
#define TFM_PERIPHERAL_CRC (&tfm_peripheral_crc)
#define TFM_PERIPHERAL_SSIE1 (&tfm_peripheral_ssie1)
#define TFM_PERIPHERAL_SSIE0 (&tfm_peripheral_ssie0)
#define TFM_PERIPHERAL_SDHI1 (&tfm_peripheral_sdhi1)
#define TFM_PERIPHERAL_SDHI0 (&tfm_peripheral_sdhi0)
#define TFM_PERIPHERAL_DOC (&tfm_peripheral_doc)
#define TFM_PERIPHERAL_CEU (&tfm_peripheral_ceu)
#define TFM_PERIPHERAL_PDMIF (&tfm_peripheral_pdmif)
#define TFM_PERIPHERAL_CANFD1 (&tfm_peripheral_canfd1)
#define TFM_PERIPHERAL_CANFD0 (&tfm_peripheral_canfd0)
#define TFM_PERIPHERAL_ESWM_MFWD (&tfm_peripheral_eswm_mfwd)
#define TFM_PERIPHERAL_ESWM (&tfm_peripheral_eswm)
#define TFM_PERIPHERAL_ESWM_COMA (&tfm_peripheral_eswm_coma)
#define TFM_PERIPHERAL_ESWM_ETHA0 (&tfm_peripheral_eswm_etha0)
#define TFM_PERIPHERAL_ESWM_RMAC0 (&tfm_peripheral_eswm_rmac0)
#define TFM_PERIPHERAL_ESWM_ETHA1 (&tfm_peripheral_eswm_etha1)
#define TFM_PERIPHERAL_ESWM_RMAC1 (&tfm_peripheral_eswm_rmac1)
#define TFM_PERIPHERAL_ESWM_GWCA0 (&tfm_peripheral_eswm_gwca0)
#define TFM_PERIPHERAL_ESWM_GPTP (&tfm_peripheral_eswm_gptp)
#define TFM_PERIPHERAL_AGT1 (&tfm_peripheral_agt1)
#define TFM_PERIPHERAL_AGT0 (&tfm_peripheral_agt0)
#define TFM_PERIPHERAL_PDG (&tfm_peripheral_pdg)
#define TFM_PERIPHERAL_POEG3 (&tfm_peripheral_poeg3)
#define TFM_PERIPHERAL_POEG2 (&tfm_peripheral_poeg2)
#define TFM_PERIPHERAL_POEG1 (&tfm_peripheral_poeg1)
#define TFM_PERIPHERAL_POEG0 (&tfm_peripheral_poeg0)
#define TFM_PERIPHERAL_DAC1 (&tfm_peripheral_dac1)
#define TFM_PERIPHERAL_DAC0 (&tfm_peripheral_dac0)
#define TFM_PERIPHERAL_ADC (&tfm_peripheral_adc)
#define TFM_PERIPHERAL_TSN (&tfm_peripheral_tsn)
#define TFM_PERIPHERAL_ACMPHS3 (&tfm_peripheral_acmphs3)
#define TFM_PERIPHERAL_ACMPHS2 (&tfm_peripheral_acmphs2)
#define TFM_PERIPHERAL_ACMPHS1 (&tfm_peripheral_acmphs1)
#define TFM_PERIPHERAL_ACMPHS0 (&tfm_peripheral_acmphs0)
#define TFM_PERIPHERAL_RTC (&tfm_peripheral_rtc)
#define TFM_PERIPHERAL_ULPT1 (&tfm_peripheral_ulpt1)
#define TFM_PERIPHERAL_ULPT0 (&tfm_peripheral_ulpt0)
#define TFM_PERIPHERAL_GPT_COMMON (&tfm_peripheral_gpt_common)
#define TFM_PERIPHERAL_GPT13 (&tfm_peripheral_gpt13)
#define TFM_PERIPHERAL_GPT12 (&tfm_peripheral_gpt12)
#define TFM_PERIPHERAL_GPT11 (&tfm_peripheral_gpt11)
#define TFM_PERIPHERAL_GPT10 (&tfm_peripheral_gpt10)
#define TFM_PERIPHERAL_GPT4 (&tfm_peripheral_gpt4)
#define TFM_PERIPHERAL_GPT3 (&tfm_peripheral_gpt3)
#define TFM_PERIPHERAL_GPT2 (&tfm_peripheral_gpt2)
#define TFM_PERIPHERAL_GPT1 (&tfm_peripheral_gpt1)
#define TFM_PERIPHERAL_GPT0 (&tfm_peripheral_gpt0)

/* Peripherals: X(name, secure, bank, bit, base, size) SEP for each one */
#define RA_PERIPHERALS(X, SEP)                                                                     \
    /* Bank B (PSARB) */                                                                           \
    X(i3c0, RA_PSAR_SECURE(I3C), REG_B, 4, R_I3C0_BASE, sizeof(R_I3C0_Type)) SEP                   \
    X(iic2, RA_PSAR_SECURE(IIC2), REG_B, 7, R_IIC2_BASE, sizeof(R_IIC0_Type)) SEP                  \
    X(iic1, RA_PSAR_SECURE(IIC1), REG_B, 8, R_IIC1_BASE, sizeof(R_IIC0_Type)) SEP                  \
    X(iic0, RA_PSAR_SECURE(IIC0), REG_B, 9, R_IIC0_BASE, sizeof(R_IIC0_Type)) SEP                  \
    X(usbfs0, RA_PSAR_SECURE(USBFS), REG_B, 11, R_USB_FS0_BASE, sizeof(R_USB_FS0_Type)) SEP        \
    X(usbhs0, RA_PSAR_SECURE(USBHS), REG_B, 12, R_USB_HS0_BASE, sizeof(R_USB_HS0_Type)) SEP        \
    X(ospi0, RA_PSAR_SECURE(OSPI0), REG_B, 16, R_XSPI0_BASE, sizeof(R_XSPI0_Type)) SEP             \
    X(dotf0, RA_PSAR_SECURE(OSPI0), REG_B, 16, R_DOTF_BASE, sizeof(R_DOTF_Type)) SEP               \
    X(ospi1, RA_PSAR_SECURE(OSPI1), REG_B, 17, R_XSPI1_BASE, sizeof(R_XSPI0_Type)) SEP             \
    X(dotf1, RA_PSAR_SECURE(OSPI1), REG_B, 17, R_DOTF1_BASE, sizeof(R_DOTF_Type)) SEP              \
    X(rspi1, RA_PSAR_SECURE(RSPI1), REG_B, 18, R_SPI_B1_BASE, sizeof(R_SPI_B0_Type)) SEP           \
    X(rspi0, RA_PSAR_SECURE(RSPI0), REG_B, 19, R_SPI_B0_BASE, sizeof(R_SPI_B0_Type)) SEP           \
    X(sci9, RA_PSAR_SECURE(SCI9), REG_B, 22, R_SCI_B9_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci8, RA_PSAR_SECURE(SCI8), REG_B, 23, R_SCI_B8_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci7, RA_PSAR_SECURE(SCI7), REG_B, 24, R_SCI_B7_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci6, RA_PSAR_SECURE(SCI6), REG_B, 25, R_SCI_B6_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci5, RA_PSAR_SECURE(SCI5), REG_B, 26, R_SCI_B5_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci4, RA_PSAR_SECURE(SCI4), REG_B, 27, R_SCI_B4_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci3, RA_PSAR_SECURE(SCI3), REG_B, 28, R_SCI_B3_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci2, RA_PSAR_SECURE(SCI2), REG_B, 29, R_SCI_B2_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci1, RA_PSAR_SECURE(SCI1), REG_B, 30, R_SCI_B1_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    X(sci0, RA_PSAR_SECURE(SCI0), REG_B, 31, R_SCI_B0_BASE, sizeof(R_SCI_B0_Type)) SEP             \
    /* Bank C (PSARC) */                                                                           \
    X(cac, RA_PSAR_SECURE(CAC), REG_C, 0, R_CAC_BASE, sizeof(R_CAC_Type)) SEP                      \
    X(crc, RA_PSAR_SECURE(CRC), REG_C, 1, R_CRC_BASE, sizeof(R_CRC_Type)) SEP                      \
    X(ssie1, RA_PSAR_SECURE(SSIE1), REG_C, 7, R_SSI1_BASE, sizeof(R_SSI0_Type)) SEP                \
    X(ssie0, RA_PSAR_SECURE(SSIE0), REG_C, 8, R_SSI0_BASE, sizeof(R_SSI0_Type)) SEP                \
    X(sdhi1, RA_PSAR_SECURE(SDHI1), REG_C, 11, R_SDHI1_BASE, sizeof(R_SDHI0_Type)) SEP             \
    X(sdhi0, RA_PSAR_SECURE(SDHI0), REG_C, 12, R_SDHI0_BASE, sizeof(R_SDHI0_Type)) SEP             \
    X(doc, RA_PSAR_SECURE(DOC), REG_C, 13, R_DOC_B_BASE, sizeof(R_DOC_B_Type)) SEP                 \
    X(ceu, RA_PSAR_SECURE(CEU), REG_C, 16, R_CEU_BASE, sizeof(R_CEU_Type)) SEP                     \
    X(pdmif, RA_PSAR_SECURE(PDMIF), REG_C, 24, R_PDM_BASE, sizeof(R_PDM_Type)) SEP                 \
    X(canfd1, RA_PSAR_SECURE(CANFD1), REG_C, 26, R_CANFD1_BASE, sizeof(R_CANFD_Type)) SEP          \
    X(canfd0, RA_PSAR_SECURE(CANFD0), REG_C, 27, R_CANFD_BASE, sizeof(R_CANFD_Type)) SEP           \
    /* One security bit for all the blocks of the switch */                                        \
    X(eswm_mfwd, RA_PSAR_SECURE(ESWM), REG_C, 30, R_MFWD_BASE, sizeof(R_MFWD_Type)) SEP            \
    X(eswm, RA_PSAR_SECURE(ESWM), REG_C, 30, R_ESWM_BASE, 0x1000U) SEP                             \
    X(eswm_coma, RA_PSAR_SECURE(ESWM), REG_C, 30, R_COMA_BASE, sizeof(R_COMA_Type)) SEP            \
    X(eswm_etha0, RA_PSAR_SECURE(ESWM), REG_C, 30, R_ETHA0_BASE, sizeof(R_ETHA0_Type)) SEP         \
    X(eswm_rmac0, RA_PSAR_SECURE(ESWM), REG_C, 30, R_RMAC0_BASE, sizeof(R_RMAC0_Type)) SEP         \
    X(eswm_etha1, RA_PSAR_SECURE(ESWM), REG_C, 30, R_ETHA1_BASE, sizeof(R_ETHA0_Type)) SEP         \
    X(eswm_rmac1, RA_PSAR_SECURE(ESWM), REG_C, 30, R_RMAC1_BASE, sizeof(R_RMAC0_Type)) SEP         \
    /* sizeof(R_GWCA0_Type) is 0x12AC, but GWIDCi (i = 0 to 63) at offset 0xC000 ends at 0xC0FF */ \
    X(eswm_gwca0, RA_PSAR_SECURE(ESWM), REG_C, 30, R_GWCA0_BASE, 0xC100U) SEP                      \
    X(eswm_gptp, RA_PSAR_SECURE(ESWM), REG_C, 30, R_GPTP_BASE, sizeof(R_GPTP_Type)) SEP            \
    /* Bank D (PSARD) */                                                                           \
    X(agt1, RA_PSAR_SECURE(AGT1), REG_D, 4, R_AGTX1_BASE, sizeof(R_AGTX0_Type)) SEP                \
    X(agt0, RA_PSAR_SECURE(AGT0), REG_D, 5, R_AGTX0_BASE, sizeof(R_AGTX0_Type)) SEP                \
    X(pdg, RA_PSAR_SECURE(PDG), REG_D, 6, R_GPT_ODC_BASE, sizeof(R_GPT_ODC_Type)) SEP              \
    X(poeg3, RA_PSAR_SECURE(PGI3), REG_D, 11, R_GPT_POEG3_BASE, sizeof(R_GPT_POEG0_Type)) SEP      \
    X(poeg2, RA_PSAR_SECURE(PGI2), REG_D, 12, R_GPT_POEG2_BASE, sizeof(R_GPT_POEG0_Type)) SEP      \
    X(poeg1, RA_PSAR_SECURE(PGI1), REG_D, 13, R_GPT_POEG1_BASE, sizeof(R_GPT_POEG0_Type)) SEP      \
    X(poeg0, RA_PSAR_SECURE(PGI0), REG_D, 14, R_GPT_POEG0_BASE, sizeof(R_GPT_POEG0_Type)) SEP      \
    X(dac1, RA_PSAR_SECURE(DAC121), REG_D, 19, R_DAC_B1_BASE, sizeof(R_DAC_B0_Type)) SEP           \
    X(dac0, RA_PSAR_SECURE(DAC120), REG_D, 20, R_DAC_B0_BASE, sizeof(R_DAC_B0_Type)) SEP           \
    X(adc, RA_PSAR_SECURE(ADC16H), REG_D, 21, R_ADC_B0_BASE, sizeof(R_ADC_B0_Type)) SEP            \
    X(tsn, RA_PSAR_SECURE(TSN), REG_D, 22, R_TSN_CTRL_BASE, sizeof(R_TSN_CTRL_Type)) SEP           \
    X(acmphs3, RA_PSAR_SECURE(ACMPHS3), REG_D, 25, R_ACMPHS3_BASE, sizeof(R_ACMPHS0_Type)) SEP     \
    X(acmphs2, RA_PSAR_SECURE(ACMPHS2), REG_D, 26, R_ACMPHS2_BASE, sizeof(R_ACMPHS0_Type)) SEP     \
    X(acmphs1, RA_PSAR_SECURE(ACMPHS1), REG_D, 27, R_ACMPHS1_BASE, sizeof(R_ACMPHS0_Type)) SEP     \
    X(acmphs0, RA_PSAR_SECURE(ACMPHS0), REG_D, 28, R_ACMPHS0_BASE, sizeof(R_ACMPHS0_Type)) SEP     \
    /* Bank E (PSARE) */                                                                           \
    X(rtc, RA_PSAR_SECURE(RTC), REG_E, 3, R_RTC_BASE, sizeof(R_RTC_Type)) SEP                      \
    X(ulpt1, RA_PSAR_SECURE(ULPT1), REG_E, 8, R_ULPT1_BASE, sizeof(R_ULPT0_Type)) SEP              \
    X(ulpt0, RA_PSAR_SECURE(ULPT0), REG_E, 9, R_ULPT0_BASE, sizeof(R_ULPT0_Type)) SEP              \
    /* The 32 byte block also holds another timer register, so both must be secure */              \
    X(gpt_common, RA_PSAR_SECURE_AND(GPT_COMMON, GPT0), REG_E, 15, R_GPT_OPS_BASE, 0x20U) SEP      \
    X(gpt13, RA_PSAR_SECURE(GPT13), REG_E, 18, R_GPT13_BASE, sizeof(R_GPT0_Type)) SEP              \
    X(gpt12, RA_PSAR_SECURE(GPT12), REG_E, 19, R_GPT12_BASE, sizeof(R_GPT0_Type)) SEP              \
    X(gpt11, RA_PSAR_SECURE(GPT11), REG_E, 20, R_GPT11_BASE, sizeof(R_GPT0_Type)) SEP              \
    X(gpt10, RA_PSAR_SECURE(GPT10), REG_E, 21, R_GPT10_BASE, sizeof(R_GPT0_Type)) SEP              \
    X(gpt4, RA_PSAR_SECURE(GPT4), REG_E, 27, R_GPT4_BASE, sizeof(R_GPT0_Type)) SEP                 \
    X(gpt3, RA_PSAR_SECURE(GPT3), REG_E, 28, R_GPT3_BASE, sizeof(R_GPT0_Type)) SEP                 \
    X(gpt2, RA_PSAR_SECURE(GPT2), REG_E, 29, R_GPT2_BASE, sizeof(R_GPT0_Type)) SEP                 \
    X(gpt1, RA_PSAR_SECURE(GPT1), REG_E, 30, R_GPT1_BASE, sizeof(R_GPT0_Type)) SEP                 \
    X(gpt0, RA_PSAR_SECURE(GPT0), REG_E, 31, R_GPT0_BASE, sizeof(R_GPT0_Type)) SEP

#endif /* __TFM_PERIPHERALS_LIST_H__ */
