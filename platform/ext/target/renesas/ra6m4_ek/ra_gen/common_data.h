/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "r_flash_hp.h"
#include "r_flash_api.h"
#include "mbedtls/platform.h"
#include "psa/crypto.h"
#include "psa/crypto_extra.h"
#include "rm_psa_crypto.h"

#include "rm_mcuboot_port.h"
#if defined(MCUBOOT_USE_MBED_TLS)
#include "mbedtls/platform.h"
#endif

#include "r_ioport.h"
#include "bsp_pin_cfg.h"
FSP_HEADER
/* Flash on Flash HP Instance */
extern const flash_instance_t g_flash1;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_flash1_ctrl;
extern const flash_cfg_t g_flash1_cfg;

#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif
void mcuboot_quick_setup();
/* Flash on Flash HP Instance */
extern const flash_instance_t g_flash0;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t g_flash0_cfg;

#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif
/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */
