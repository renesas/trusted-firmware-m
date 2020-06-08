/* generated HAL source file - do not edit */
#include "hal_data.h"
sci_uart_instance_ctrl_t g_uart_ctrl;

baud_setting_t g_uart_baud_setting =
{
/* Baud rate calculated with 0.160% error. */ .abcse = 0,
    .abcs                                            = 0,.bgdm= 1, .cks = 0, .brr = 64, .mddr = (uint8_t) 256, .brme = false
};

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart_cfg_extend =
{
    .clock           = SCI_UART_CLOCK_INT,
    .rx_edge_start   = SCI_UART_START_BIT_FALLING_EDGE,
    .noise_cancel    = SCI_UART_NOISE_CANCELLATION_DISABLE,
    .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX,
    .p_baud_setting  = &g_uart_baud_setting,
    .uart_mode       = UART_MODE_RS232,
    .ctsrts_en       = SCI_UART_CTSRTS_RTS_OUTPUT,
#if 0
    .flow_control_pin = IOPORT_PORT_00_PIN_00,
#else
    .flow_control_pin = (bsp_io_port_pin_t) (0xFFFFU),
#endif
};

/** UART interface configuration */
const uart_cfg_t g_uart_cfg =
{
    .channel    = 7,
    .data_bits  = UART_DATA_BITS_8,
    .parity     = UART_PARITY_OFF,
    .stop_bits  = UART_STOP_BITS_1,
    .p_callback =
        user_uart_callback,
    .p_context = NULL,
    .p_extend  = &g_uart_cfg_extend,
#define RA_NOT_DEFINED    (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
    .p_transfer_tx = NULL,
#else
    .p_transfer_tx = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
    .p_transfer_rx = NULL,
#else
    .p_transfer_rx = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
    .rxi_ipl = (12),
    .txi_ipl = (12),
    .tei_ipl = (12),
    .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI7_RXI)
    .rxi_irq       = VECTOR_NUMBER_SCI7_RXI,
#else
    .rxi_irq       = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_TXI)
    .txi_irq       = VECTOR_NUMBER_SCI7_TXI,
#else
    .txi_irq       = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_TEI)
    .tei_irq       = VECTOR_NUMBER_SCI7_TEI,
#else
    .tei_irq       = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI7_ERI)
    .eri_irq       = VECTOR_NUMBER_SCI7_ERI,
#else
    .eri_irq       = FSP_INVALID_VECTOR,
#endif
};

/* Instance structure to use this module. */
const uart_instance_t g_uart =
{.p_ctrl = &g_uart_ctrl, .p_cfg = &g_uart_cfg, .p_api = &g_uart_on_sci};
flash_hp_instance_ctrl_t g_flash_ctrl;
const flash_cfg_t        g_flash_cfg =
{
    .data_flash_bgo = false,
    .p_callback     = NULL,
    .p_context      = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq            = VECTOR_NUMBER_FCU_FRDYI,
#else
    .irq            = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq        = VECTOR_NUMBER_FCU_FIFERR,
#else
    .err_irq        = FSP_INVALID_VECTOR,
#endif
    .err_ipl = (BSP_IRQ_DISABLED),
    .ipl     = (BSP_IRQ_DISABLED),
};

/* Instance structure to use this module. */
const flash_instance_t g_flash =
{.p_ctrl = &g_flash_ctrl, .p_cfg = &g_flash_cfg, .p_api = &g_flash_on_flash_hp};

// void g_hal_init(void)
// {
// g_common_init ();
// }
