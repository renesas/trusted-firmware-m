/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
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

#ifndef __CMSIS_DRIVER_CONFIG_H__
#define __CMSIS_DRIVER_CONFIG_H__

#include "device_cfg.h"
#include "device_definition.h"
#include "platform_description.h"
#include "RTE_Device.h"
#include "target_cfg.h"

//#define UART0_DEV           UART0_PL011_DEV_NS
#define UART1_DEV           UART1_RA6M4_DEV_S

#define MUSCA_B1_SCC_DEV    MUSCA_B1_SCC_DEV_S

#define DFLASH0_DEV         FLASH0_DEV_S
#define CFLASH0_DEV         FLASH0_DEV_S
#define FLASH0_DEV          MT25QL_DEV_S

#endif  /* __CMSIS_DRIVER_CONFIG_H__ */
