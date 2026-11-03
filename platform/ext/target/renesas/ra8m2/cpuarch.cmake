#-------------------------------------------------------------------------------
# Copyright (c) 2026, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# cpuarch.cmake is used to set things that related to the platform that are both
# immutable and global, which is to say they should apply to any kind of project
# that uses this platform. In practice this is normally CPU architecture and
# architecture extensions.

# Renesas RA8M2 (R7KA8M2JFLCAC): CPU0 is a Cortex-M85 with TrustZone, DSP and a
# double-precision FPU, at 1 GHz. It is the port's first Armv8.1-M part; RA6E1 and RA6M5 are
# Armv8-M.main Cortex-M33.
#
# CPU1, a Cortex-M33 at 250 MHz, is not built by this platform: RA8x2 is a single-core part
# for this project. The secure project is ra8m2_iar_CPU0_secure for the same reason.
#
# Values match platform/ext/target/arm/mps3/corstone310, TF-M's own M85 platform, rather than
# being derived independently.
set(TFM_SYSTEM_PROCESSOR    cortex-m85)
set(TFM_SYSTEM_ARCHITECTURE armv8.1-m.main)
set(TFM_SYSTEM_DSP          ON)
set(TFM_SYSTEM_FP           ON)

# Double precision, unlike the M33 parts' fpv5-sp-d16. The rzone reports Dfpu="1" for CPU0
# and FSP's BSP builds with the DP FPU enabled; a single-precision setting here would
# disagree with the FSP objects at link time.
set(CONFIG_TFM_FP_ARCH      "fpv5-d16")
set(CONFIG_TFM_FP_ARCH_ASM  "FPv5_D16")

# Helium (MVE) and PACBTI are deliberately left at their defaults. Both are M85 features
# FSP's BSP can enable, and both change the ABI, so they are left for a step after bring-up,
# which keeps the configuration as close as possible to the validated RA6M5 one.
