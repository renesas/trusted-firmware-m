/*
 * Copyright (c) 2013-2020 Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdint.h>

#include "Driver_USART.h"
#include "RTE_Device.h"
#include "bsp_api.h"
#include "device_cfg.h"
#include "tfm_hal_device_header.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)arg
#endif

#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    .asynchronous = 1,
};

/* BRR max value */
#define SCI_B_UART_BRR_MAX        255U

/* SCI channel size */
#define SCI_B_REG_SIZE            (R_SCI1_BASE - R_SCI0_BASE)

/* Reset value of FCR (HWM 38.2.11 FCR), written explicitly here since
 * FIFO mode stays disabled on all channels used by this driver.
 */
#define SCI_B_UART_FCR_DEFAULT    0x1F1F0000U

/*
 * Baud rate divisor table, same coefficients as the SCI_B baud rate
 * generator supports: divisor = 2^(2n+1) x G x (BRR + 1), where G depends
 * on BGDM/ABCS/ABCSE. Ordered from the smallest divisor to the largest so
 * that the highest achievable baud rates are found first.
 *
 * The table mirrors the divisors used by the FSP baud rate calculator
 * (g_async_baud / g_div_coefficient in r_sci_b_uart.c). Bitrate modulation
 * (BRME/MDDR) is intentionally not used by this driver.
 */
typedef struct {
    uint16_t coefficient;
    uint8_t  bgdm  : 1;
    uint8_t  abcs  : 1;
    uint8_t  abcse : 1;
    uint8_t  cks   : 2;
} sci_b_uart_baud_divisor_t;

static const sci_b_uart_baud_divisor_t g_sci_b_uart_baud_divisors[] = {
    {   6U, 0U, 0U, 1U, 0U },
    {   8U, 1U, 1U, 0U, 0U },
    {  16U, 1U, 0U, 0U, 0U },
    {  24U, 0U, 0U, 1U, 1U },
    {  32U, 0U, 0U, 0U, 0U },
    {  64U, 1U, 0U, 0U, 1U },
    {  96U, 0U, 0U, 1U, 2U },
    { 128U, 0U, 0U, 0U, 1U },
    { 256U, 1U, 0U, 0U, 2U },
    { 384U, 0U, 0U, 1U, 3U },
    { 512U, 0U, 0U, 0U, 2U },
    {1024U, 1U, 0U, 0U, 3U },
    {2048U, 0U, 0U, 0U, 3U },
};

/* Maximum tolerated baud rate error, in units of 0.001%. 5000 = 5%. */
#define SCI_B_UART_BAUD_ERROR_MAX_X1000   (5000U)

/* MDDR is only used when BRME is 1, but the hardware manual defines its
 * valid range as 128-255; write the same safe default as FSP
 * (SCI_B_UART_MDDR_MIN = 128) even though BRME stays 0 in this driver,
 * instead of leaving MDDR at 0 (out of spec per HWM).
 */
#define SCI_B_UART_CCR2_MDDR_DEFAULT \
    ((128UL << R_SCI_B0_CCR2_MDDR_Pos) & R_SCI_B0_CCR2_MDDR_Msk)

/* Single raw CMSIS control mask (mode/data bits/parity/stop bits/flow
 * control) instead of separate data_bits/parity/stop_bits fields.
 */

typedef struct {
    uint8_t  channel;
    uint32_t baudrate;
    uint32_t control;   /* raw CMSIS control mask (mode/data bits/parity/stop bits/flow control) */
} sci_b_uart_config_t;

typedef struct {
    volatile R_SCI_B0_Type *p_reg;
    uint32_t tx_count;
    uint32_t rx_count;
} sci_b_uart_data_t;

/*
 * Compute the CCR2 value for the requested baud rate.
 *
 * The divisor table is searched for the setting with the smallest baud
 * rate error, using the same divisors as the SCI_B baud rate generator
 * supports. A baud rate that cannot be reached within
 * SCI_B_UART_BAUD_ERROR_MAX_X1000 is rejected instead of being silently
 * approximated.
 */
static int32_t sci_b_ccr2_compute(uint32_t baudrate, uint32_t *p_ccr2)
{
    uint32_t best_ccr2 = 0U;
    uint32_t best_err  = UINT32_MAX;
    uint32_t freq_hz;

    if ((baudrate == 0U) || (p_ccr2 == NULL)) {
        return ARM_USART_ERROR_BAUDRATE;
    }

#if defined(BSP_FEATURE_SCI_HAS_SCISPI_CLOCK) && \
    (BSP_FEATURE_SCI_HAS_SCISPI_CLOCK != 0)
    freq_hz = R_FSP_SciSpiClockHzGet();
#else
    freq_hz = R_FSP_SciClockHzGet();
#endif

    for (uint32_t reduced_cycles = 0U; reduced_cycles <= 1U; reduced_cycles++) {
        for (uint32_t i = 0U;
             i < (sizeof(g_sci_b_uart_baud_divisors) /
                  sizeof(g_sci_b_uart_baud_divisors[0]));
             i++) {
            const sci_b_uart_baud_divisor_t *p_div =
                &g_sci_b_uart_baud_divisors[i];

            uint64_t divisor;
            uint64_t brr_plus_one;
            uint32_t brr;
            uint32_t err_x1000;

            /* Pass 0: only 16 cycles/bit entries (abcs|abcse == 0). */
            /* Pass 1: only 8/6 cycles/bit entries (abcs|abcse == 1). */
            if (reduced_cycles != (uint32_t)(p_div->abcs | p_div->abcse)) {
                continue;
            }

            divisor      = (uint64_t)p_div->coefficient * baudrate;
            brr_plus_one = (uint64_t)freq_hz / divisor;

            if ((brr_plus_one == 0U) ||
                (brr_plus_one > ((uint64_t)SCI_B_UART_BRR_MAX + 1U))) {
                continue;
            }

            brr = (uint32_t)(brr_plus_one - 1U);

            /* Baud rate error in units of 0.001%, computed the same way
             * as the FSP calculator (avoids double rounding through an
             * intermediate integer baud rate):
             *   err = (freq x 100000) / (coefficient x baud x (BRR + 1))
             *         - 100000
             */
            {
                uint64_t err_divisor = divisor * (uint64_t)(brr + 1U);
                int64_t  signed_err  =
                    (int64_t)(((uint64_t)freq_hz * 100000U) / err_divisor) -
                    (int64_t)100000;

                if (signed_err < 0) {
                    signed_err = -signed_err;
                }

                err_x1000 = (uint32_t)signed_err;
            }

            if (err_x1000 < best_err) {
                best_err = err_x1000;

                best_ccr2 =
                    ((uint32_t)p_div->bgdm  << R_SCI_B0_CCR2_BGDM_Pos)
                    | ((uint32_t)p_div->abcs  << R_SCI_B0_CCR2_ABCS_Pos)
                    | ((uint32_t)p_div->abcse << R_SCI_B0_CCR2_ABCSE_Pos)
                    | (((uint32_t)p_div->cks  << R_SCI_B0_CCR2_CKS_Pos)
                       & R_SCI_B0_CCR2_CKS_Msk)
                    | ((brr << R_SCI_B0_CCR2_BRR_Pos)
                       & R_SCI_B0_CCR2_BRR_Msk)
                    | SCI_B_UART_CCR2_MDDR_DEFAULT;  /* BRME left clear (0) */
            }
        }

        /* Stop after pass 0 if 16 cycles/bit already meets the accuracy
         * requirement - only fall back to 8/6 cycles/bit if it doesn't.
         */
        if (best_err <= SCI_B_UART_BAUD_ERROR_MAX_X1000) {
            break;
        }
    }

    if (best_err > SCI_B_UART_BAUD_ERROR_MAX_X1000) {
        return ARM_USART_ERROR_BAUDRATE;
    }

    *p_ccr2 = best_ccr2;

    return ARM_DRIVER_OK;
}


/*
 * Build CCR1 value from parity configuration.
 * SPB2DT=1, SPB2IO=1: TXD_MOSI pin output, driven high idle when TE=0.
 */
static uint32_t sci_b_ccr1_val(const sci_b_uart_config_t *cfg)
{
    uint32_t ccr1;

    ccr1 = R_SCI_B0_CCR1_SPB2DT_Msk | R_SCI_B0_CCR1_SPB2IO_Msk;

    /* Mask the PARITY field first, then compare for equality - these
     * macros are encoded values within ARM_USART_PARITY_Msk, not
     * independent flag bits, so a bare `control & VALUE` test is not
     * reliable (e.g. some encoded values can share bits).
     */

    switch (cfg->control & ARM_USART_PARITY_Msk) {
    case ARM_USART_PARITY_ODD:
        /* Odd parity: PE=1, PM=1 */
        ccr1 |= R_SCI_B0_CCR1_PE_Msk | R_SCI_B0_CCR1_PM_Msk;
        break;
    case ARM_USART_PARITY_EVEN:
        /* Even parity: PE=1, PM=0 */
        ccr1 |= R_SCI_B0_CCR1_PE_Msk;
        break;
    case ARM_USART_PARITY_NONE:
    default:
        /* PE stays 0, nothing else to set */
        break;
    }

    return ccr1;
}

/*
 * Build CCR3 value from data_bits and stop_bits.
 * LSBF=1: LSB-first; MOD=000: Asynchronous mode.
 *
 * CHR[1:0] encoding per HWM 38.2.11 CCR3 (character length in
 * Asynchronous/Manchester mode):
 *   00 / 01 : 9-bit
 *   10      : 8-bit (reset value)
 *   11      : 7-bit
 */

static uint32_t sci_b_ccr3_val(const sci_b_uart_config_t *cfg)
{
    uint32_t ccr3;

    ccr3 = R_SCI_B0_CCR3_LSBF_Msk;

    /* CHR[1:0] - character length. Masked switch/compare for the same
     * reason as parity above (encoded field, not independent bits).
     */

    switch (cfg->control & ARM_USART_DATA_BITS_Msk) {
    case ARM_USART_DATA_BITS_7:
        ccr3 |= (3U << R_SCI_B0_CCR3_CHR_Pos);      /* CHR=11: 7-bit */
        break;
    case ARM_USART_DATA_BITS_9:
        ccr3 |= (0U << R_SCI_B0_CCR3_CHR_Pos);      /* CHR=00: 9-bit */
        break;
    case ARM_USART_DATA_BITS_8:
    default:
        ccr3 |= (2U << R_SCI_B0_CCR3_CHR_Pos);      /* CHR=10: 8-bit */
        break;
    }

    /* STP - stop bits */
    if ((cfg->control & ARM_USART_STOP_BITS_Msk) == ARM_USART_STOP_BITS_2) {
        ccr3 |= R_SCI_B0_CCR3_STP_Msk;              /* 2 stop bits */
    }
    /* ARM_USART_STOP_BITS_1: STP stays 0 -> 1 stop bit (default) */

    return ccr3;
}

/* Register-level initialization */
static int32_t sci_b_uart_init(sci_b_uart_data_t *data,
                                const sci_b_uart_config_t *cfg)
{
    volatile R_SCI_B0_Type *p_reg;
    uint32_t ccr0;
    uint32_t ccr2;
    int32_t  err;

    R_BSP_MODULE_START(FSP_IP_SCI, cfg->channel);

    p_reg = (volatile R_SCI_B0_Type *)(R_SCI0_BASE
                                       + (SCI_B_REG_SIZE * cfg->channel));

    /* CCR0: IDSEL=1, interrupts disabled, TE=0, RE=0 */
    ccr0 = R_SCI_B0_CCR0_IDSEL_Msk;

    /* CCR2: baud rate generator - SCICLK from FSP clock tree */
    err = sci_b_ccr2_compute(cfg->baudrate, &ccr2);
    if (err != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }
    p_reg->CCR2 = ccr2;

    /* CCR1: TXD pin idle + parity */
    p_reg->CCR1 = sci_b_ccr1_val(cfg);

    /* CCR3: async mode, LSB-first, char length, stop bits */
    p_reg->CCR3 = sci_b_ccr3_val(cfg);

    /* CCR4: disable extended serial mode */
    p_reg->CCR4 = 0U;

    /* FCR: FIFO disabled. Write FSP-compatible default for FIFO-capable
     * channels, clear to 0 otherwise.
     */
#if defined(BSP_FEATURE_SCI_UART_FIFO_CHANNELS_MASK)
#if BSP_FEATURE_SCI_UART_FIFO_CHANNELS_MASK != 0
    if ((BSP_FEATURE_SCI_UART_FIFO_CHANNELS_MASK) & (1U << cfg->channel)) {
        p_reg->FCR = SCI_B_UART_FCR_DEFAULT;
    } else
#endif
#endif
    {
        p_reg->FCR = 0U;
    }

    /* Clear all status and error flags */
    p_reg->CFCLR = R_SCI_B0_CFCLR_TDREC_Msk
                   | R_SCI_B0_CFCLR_RDRFC_Msk
                   | R_SCI_B0_CFCLR_ORERC_Msk
                   | R_SCI_B0_CFCLR_PERC_Msk
                   | R_SCI_B0_CFCLR_FERC_Msk
                   | R_SCI_B0_CFCLR_ERSC_Msk
                   | R_SCI_B0_CFCLR_MFFC_Msk;

    /* Enable transmitter and receiver */
    ccr0 |= R_SCI_B0_CCR0_RE_Msk | R_SCI_B0_CCR0_TE_Msk;
    p_reg->CCR0 = ccr0;

    /* Wait for RE internal state to stabilize */
    while (p_reg->CESR_b.RIST == 0U) {
        /* Wait until the receiver state machine is ready */
    }

    data->p_reg    = p_reg;
    data->tx_count = 0u;
    data->rx_count = 0u;

    return ARM_DRIVER_OK;
}

/* Register-level de-initialization */
/* Idempotent: safe to call before init (or twice in a row) - returns OK
 * without touching hardware when there is nothing to tear down.
 */

static int32_t sci_b_uart_deinit(sci_b_uart_data_t *data, uint8_t channel)
{
    volatile R_SCI_B0_Type *p_reg;

    if (data == NULL) {
        return ARM_DRIVER_ERROR;
    }

    if (data->p_reg == NULL) {
        /* Already de-initialized (or never initialized) - not an error. */
        return ARM_DRIVER_OK;
    }

    p_reg = data->p_reg;

    /* Make sure no transmission is in progress before clearing TE.
     * Setting CCR0_b.TE to 0 while CSR_b.TEND is 0 causes the SCI
     * peripheral to work abnormally (HWM / FSP r_sci_b_uart.c).
     */
    while ((p_reg->CSR & R_SCI_B0_CSR_TEND_Msk) == 0U) {
        /* Wait until the current transmission completes */
    }

    /* Disable transmitter and wait for internal state inactive */
    p_reg->CCR0 &= ~R_SCI_B0_CCR0_TE_Msk;
    while (p_reg->CESR_b.TIST != 0U) {
        /* Wait for the TX state machine to become inactive */
    }

    /* Disable receiver */
    p_reg->CCR0 &= ~R_SCI_B0_CCR0_RE_Msk;

    /* Clear all flags */
    p_reg->CFCLR = R_SCI_B0_CFCLR_TDREC_Msk
                   | R_SCI_B0_CFCLR_RDRFC_Msk
                   | R_SCI_B0_CFCLR_ORERC_Msk
                   | R_SCI_B0_CFCLR_PERC_Msk
                   | R_SCI_B0_CFCLR_FERC_Msk
                   | R_SCI_B0_CFCLR_ERSC_Msk
                   | R_SCI_B0_CFCLR_MFFC_Msk;

    /* Gate SCI channel clock off */
    R_BSP_MODULE_STOP(FSP_IP_SCI, channel);

    data->p_reg = NULL;

    return ARM_DRIVER_OK;
}

/* Polling TX */
static int32_t sci_b_uart_send(sci_b_uart_data_t *data,
                                const uint8_t *buf, uint32_t num)
{
    volatile R_SCI_B0_Type *p_reg;

    if ((data == NULL) || (data->p_reg == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    if ((num == 0U) || (buf == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    p_reg = data->p_reg;

    for (uint32_t i = 0U; i < num; i++) {
        while ((p_reg->CSR & R_SCI_B0_CSR_TDRE_Msk) == 0U) {
            /* Wait until the transmit data register is ready */
        }
        p_reg->TDR_BY = buf[i];
    }

    while ((p_reg->CSR & R_SCI_B0_CSR_TEND_Msk) == 0U) {
        /* Wait until the final transmission completes */
    }

    data->tx_count = num;

    return ARM_DRIVER_OK;
}

/* Polling RX */
static int32_t sci_b_uart_receive(sci_b_uart_data_t *data,
                                   uint8_t *buf, uint32_t num)
{
    volatile R_SCI_B0_Type *p_reg;

    if ((data == NULL) || (data->p_reg == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    if ((num == 0U) || (buf == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    p_reg = data->p_reg;

    for (uint32_t i = 0U; i < num; i++) {
        while ((p_reg->CSR & R_SCI_B0_CSR_RDRF_Msk) == 0U) {
            /* Wait until receive data is available */
        }
        buf[i] = (uint8_t)p_reg->RDR;
    }

    data->rx_count = num;

    return ARM_DRIVER_OK;
}

/* Shared functions (channel-independent) */
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_USART_Transfer(const void *data_out, void *data_in,
                                   uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    ARM_USART_STATUS status = {0};
    return status;
}

static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = {0};
    return modem_status;
}

static int32_t sci_b_uart_control(sci_b_uart_data_t *data,
                                   uint32_t control, uint32_t arg)
{
    volatile R_SCI_B0_Type *p_reg;
    uint32_t operation;
    uint32_t ccr2;
    int32_t  err;
    sci_b_uart_config_t cfg;

    if ((data == NULL) || (data->p_reg == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    p_reg     = data->p_reg;
    operation = control & ARM_USART_CONTROL_Msk;

    switch (operation) {
    case ARM_USART_MODE_ASYNCHRONOUS:
            /* arg == 0 is only invalid for the baudrate here, not for
             * ARM_USART_CONTROL_TX/RX below - scoped to this branch only.
             */
        if (arg == 0U) {
            return ARM_USART_ERROR_BAUDRATE;
        }

        /* Validate data bits (return error for anything unsupported) */
        switch (control & ARM_USART_DATA_BITS_Msk) {
        case ARM_USART_DATA_BITS_7:
        case ARM_USART_DATA_BITS_8:
            break;
        case ARM_USART_DATA_BITS_9:
        default:
            /* 9-bit is rejected: the TX/RX data path is byte-oriented
             * (uint8_t buffers, TDR_BY/RDR 8-bit access), so the 9th bit
             * would silently be lost if accepted here.
             */
            return ARM_USART_ERROR_DATA_BITS;
        }


        /* Validate parity (return error for anything unsupported) */
        switch (control & ARM_USART_PARITY_Msk) {
        case ARM_USART_PARITY_NONE:
        case ARM_USART_PARITY_EVEN:
        case ARM_USART_PARITY_ODD:
            break;
        default:
            return ARM_USART_ERROR_PARITY;
        }

        /* Validate stop bits - only 1 and 2 stop bits are supported by
         * this hardware/driver; 1.5 and 0.5 stop bits are rejected.
         */
        switch (control & ARM_USART_STOP_BITS_Msk) {
        case ARM_USART_STOP_BITS_1:
        case ARM_USART_STOP_BITS_2:
            break;
        default:
            return ARM_USART_ERROR_STOP_BITS;
        }

        /* Validate flow control - RTS/CTS are not wired up in this
         * driver yet, so only "none" is accepted.
         */
        switch (control & ARM_USART_FLOW_CONTROL_Msk) {
        case ARM_USART_FLOW_CONTROL_NONE:
            break;
        default:
            return ARM_USART_ERROR_FLOW_CONTROL;
        }

        /* Fully initialize cfg (avoids reading an uninitialized
         * `channel` even though no helper currently reads it).
         */
        cfg = (sci_b_uart_config_t){
            .channel  = 0U,   /* Not used while reconfiguring. */
            .baudrate = arg,
            .control  = control,
        };

        /* Compute the new baud rate setting before touching any hardware
         * state, so a rejected baud rate leaves TE/RE untouched.
         */
        err = sci_b_ccr2_compute(cfg.baudrate, &ccr2);
        if (err != ARM_DRIVER_OK) {
            return err;
        }

        /* Make sure no transmission is in progress before clearing TE.
         * Setting CCR0_b.TE to 0 while CSR_b.TEND is 0 causes the SCI
         * peripheral to work abnormally (HWM / FSP r_sci_b_uart.c).
         */
        while ((p_reg->CSR & R_SCI_B0_CSR_TEND_Msk) == 0U) {
            /* Wait until the current transmission stops */
        }

        /* RA8P1 HW manual 39.3.7: disable TE/RE before writing CCRx */
        p_reg->CCR0 &= ~(R_SCI_B0_CCR0_TE_Msk | R_SCI_B0_CCR0_RE_Msk);
        while (p_reg->CESR_b.TIST != 0U) {
            /* Wait for the TX state machine to become inactive */
        }

        p_reg->CCR3 = sci_b_ccr3_val(&cfg);
        p_reg->CCR2 = ccr2;
        p_reg->CCR1 = sci_b_ccr1_val(&cfg);

        /* Re-enable transmitter and receiver */
        p_reg->CCR0 |= R_SCI_B0_CCR0_RE_Msk | R_SCI_B0_CCR0_TE_Msk;
        while (p_reg->CESR_b.RIST == 0U) {
            /* Wait for the receiver state machine to become ready */
        }
        return ARM_DRIVER_OK;

    case ARM_USART_CONTROL_TX:
        /* CMSIS contract: arg is 0=disabled, 1=enabled (exactly two
         * valid values) - reject anything else instead of silently
         * treating it as enable or disable.
         */
        if (arg > 1U) {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
        if (arg == 1U) {
            p_reg->CCR0 |= R_SCI_B0_CCR0_TE_Msk;
        } else {
            /* Make sure no transmission is in progress before clearing
             * TE (see note above).
             */
            while ((p_reg->CSR & R_SCI_B0_CSR_TEND_Msk) == 0U) {
                /* Wait until the transmission has actually ended */
            }
            p_reg->CCR0 &= ~R_SCI_B0_CCR0_TE_Msk;
            while (p_reg->CESR_b.TIST != 0U) {
                /* Wait for the TX hardware state to go idle */
            }
        }
        return ARM_DRIVER_OK;

    case ARM_USART_CONTROL_RX:
        /* Same {0,1}-only contract as TX above. */
        if (arg > 1U) {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
        if (arg == 1U) {
            p_reg->CCR0 |= R_SCI_B0_CCR0_RE_Msk;
        } else {
            p_reg->CCR0 &= ~R_SCI_B0_CCR0_RE_Msk;
        }
        return ARM_DRIVER_OK;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

/* X-Macro: generate one CMSIS Driver_USART instance per SCI_B channel */
#define SCI_B_UART_INSTANCE(ch)                                                \
                                                                               \
    static sci_b_uart_data_t sci_b_uart##ch##_data = {                         \
        .p_reg    = NULL,                                                      \
        .tx_count = 0u,                                                        \
        .rx_count = 0u,                                                        \
    };                                                                         \
                                                                               \
    static const sci_b_uart_config_t sci_b_uart##ch##_cfg = {                  \
        .channel   = (uint8_t)(ch),                                            \
        .baudrate  = DEFAULT_UART_BAUDRATE,                                    \
        .control   =  ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE            \
                    | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE,     \
    };                                                                         \
                                                                               \
    static int32_t ARM_USART##ch##_Initialize(                                 \
                                ARM_USART_SignalEvent_t cb_event)              \
    {                                                                          \
        ARG_UNUSED(cb_event);                                                  \
        return sci_b_uart_init(&sci_b_uart##ch##_data,                         \
                               &sci_b_uart##ch##_cfg);                         \
    }                                                                          \
                                                                               \
    static int32_t ARM_USART##ch##_Uninitialize(void)                          \
    {                                                                          \
        return sci_b_uart_deinit(&sci_b_uart##ch##_data,                       \
                                 sci_b_uart##ch##_cfg.channel);                \
    }                                                                          \
                                                                               \
    static int32_t ARM_USART##ch##_PowerControl(ARM_POWER_STATE state)         \
    {                                                                          \
        switch (state) {                                                       \
        case ARM_POWER_FULL:                                                   \
            return ARM_DRIVER_OK;                                              \
        case ARM_POWER_OFF:                                                    \
        case ARM_POWER_LOW:                                                    \
        default:                                                               \
            return ARM_DRIVER_ERROR_UNSUPPORTED;                               \
        }                                                                      \
    }                                                                          \
                                                                               \
    static int32_t ARM_USART##ch##_Send(const void *data, uint32_t num)        \
    {                                                                          \
        return sci_b_uart_send(&sci_b_uart##ch##_data,                         \
                               (const uint8_t *)data, num);                    \
    }                                                                          \
                                                                               \
    static int32_t ARM_USART##ch##_Receive(void *data, uint32_t num)           \
    {                                                                          \
        return sci_b_uart_receive(&sci_b_uart##ch##_data,                      \
                                  (uint8_t *)data, num);                       \
    }                                                                          \
                                                                               \
    static uint32_t ARM_USART##ch##_GetTxCount(void)                           \
    {                                                                          \
        return sci_b_uart##ch##_data.tx_count;                                 \
    }                                                                          \
                                                                               \
    static uint32_t ARM_USART##ch##_GetRxCount(void)                           \
    {                                                                          \
        return sci_b_uart##ch##_data.rx_count;                                 \
    }                                                                          \
                                                                               \
    static int32_t ARM_USART##ch##_Control(uint32_t control, uint32_t arg)     \
    {                                                                          \
        return sci_b_uart_control(&sci_b_uart##ch##_data, control, arg);       \
    }                                                                          \
                                                                               \
    ARM_DRIVER_USART Driver_USART##ch = {                                      \
        .GetVersion      = ARM_USART_GetVersion,                               \
        .GetCapabilities = ARM_USART_GetCapabilities,                          \
        .Initialize      = ARM_USART##ch##_Initialize,                         \
        .Uninitialize    = ARM_USART##ch##_Uninitialize,                       \
        .PowerControl    = ARM_USART##ch##_PowerControl,                       \
        .Send            = ARM_USART##ch##_Send,                               \
        .Receive         = ARM_USART##ch##_Receive,                            \
        .Transfer        = ARM_USART_Transfer,                                 \
        .GetTxCount      = ARM_USART##ch##_GetTxCount,                         \
        .GetRxCount      = ARM_USART##ch##_GetRxCount,                         \
        .Control         = ARM_USART##ch##_Control,                            \
        .GetStatus       = ARM_USART_GetStatus,                                \
        .SetModemControl = ARM_USART_SetModemControl,                          \
        .GetModemStatus  = ARM_USART_GetModemStatus                            \
    }

/* Conditional instantiation */
#if RTE_USART0
SCI_B_UART_INSTANCE(0);
#endif

#if RTE_USART7
SCI_B_UART_INSTANCE(7);
#endif

#if RTE_USART8
SCI_B_UART_INSTANCE(8);
#endif
