#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# RA8M2 SoC-level CPU/architecture parameters, shared by every board built on
# this SoC.

set(TFM_SYSTEM_PROCESSOR cortex-m85)
set(TFM_SYSTEM_ARCHITECTURE armv8.1-m.main)
set(TFM_SYSTEM_DSP ON)
set(CONFIG_TFM_FP_ARCH "fpv5-d16") # Double precision FPU
set(CONFIG_TFM_FP_ARCH_ASM "FPv5_D16") # Instruction for FPU with double precision
