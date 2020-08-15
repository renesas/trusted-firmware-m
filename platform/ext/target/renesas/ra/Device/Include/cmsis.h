/*
<<<<<<< HEAD:platform/ext/target/renesas/ra6m4/CMSIS_Driver/Config/cmsis_driver_config.h
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
=======
 * Copyright (c) 2017-2019 Arm Limited. All rights reserved.
>>>>>>> add_ra_bl2_support:platform/ext/target/renesas/ra/Device/Include/cmsis.h
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

#ifndef __RA_CMSIS_H__
#define __RA_CMSIS_H__

/* This file is only included to prevent build errors since the ARM 
 * code includes this file. All the definitions expected here are already 
 * provided by other CMSIS files that are provided via the FSP BSP. */

<<<<<<< HEAD:platform/ext/target/renesas/ra6m4/CMSIS_Driver/Config/cmsis_driver_config.h
#define UART0_DEV           UART0_PL011_DEV_NS
#define UART1_DEV           UART1_PL011_DEV_NS

#define MUSCA_B1_SCC_DEV    MUSCA_B1_SCC_DEV_S

#define DFLASH0_DEV         FLASH0_DEV_S
#define CFLASH0_DEV         FLASH0_DEV_S
#define FLASH0_DEV          MT25QL_DEV_S

#endif  /* __CMSIS_DRIVER_CONFIG_H__ */
=======

#endif /*__RA_CMSIS_H__ */
>>>>>>> add_ra_bl2_support:platform/ext/target/renesas/ra/Device/Include/cmsis.h
