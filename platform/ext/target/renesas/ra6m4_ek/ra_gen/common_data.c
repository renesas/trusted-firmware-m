/* generated common source file - do not edit */
#include "common_data.h"
flash_hp_instance_ctrl_t g_flash1_ctrl;
const flash_cfg_t g_flash1_cfg =
{ .data_flash_bgo = false, .p_callback = NULL, .p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
  .err_irq = FSP_INVALID_VECTOR,
#endif
  .err_ipl = (BSP_IRQ_DISABLED),
  .ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const flash_instance_t g_flash1 =
{ .p_ctrl = &g_flash1_ctrl, .p_cfg = &g_flash1_cfg, .p_api = &g_flash_on_flash_hp };

void *const gp_mcuboot_flash_ctrl = &g_flash1_ctrl;
flash_cfg_t const *const gp_mcuboot_flash_cfg = &g_flash1_cfg;
flash_instance_t const *const gp_mcuboot_flash_instance = &g_flash1;
flash_hp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg =
{ .data_flash_bgo = false, .p_callback = NULL, .p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
  .err_irq = FSP_INVALID_VECTOR,
#endif
  .err_ipl = (BSP_IRQ_DISABLED),
  .ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const flash_instance_t g_flash0 =
{ .p_ctrl = &g_flash0_ctrl, .p_cfg = &g_flash0_cfg, .p_api = &g_flash_on_flash_hp };
void *const gp_tfm_flash_ctrl = &g_flash0_ctrl;
flash_cfg_t const *const gp_tfm_flash_cfg = &g_flash0_cfg;
flash_instance_t const *const gp_tfm_flash_instance = &g_flash0;

ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };
void g_common_init(void)
{
}
