#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# cpuarch.cmake is used to set things that related to the platform that are both
# immutable and global, which is to say they should apply to any kind of project
# that uses this platform. In practice this is normally CPU architecture and
# architecture extensions.

# Renesas RA6E1 - Cortex-M33 with TrustZone, FPU, DSP
set(TFM_SYSTEM_PROCESSOR cortex-m33)
set(TFM_SYSTEM_ARCHITECTURE armv8-m.main)
set(TFM_SYSTEM_DSP ON)
set(TFM_SYSTEM_FP ON)
set(CONFIG_TFM_FP_ARCH "fpv5-sp-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5_SP")
