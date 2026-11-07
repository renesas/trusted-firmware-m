#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# RA8P1 SoC-level CPU/architecture parameters, shared by every board built on
# this SoC.

# RA_CORE allows to check active core
add_compile_definitions(RA_CM85=85)
add_compile_definitions(RA_CM33=33)

# Set architecture and CPU
if(RA_CORE STREQUAL "cm85")
    # CM85 core for SPE/NSPE
    set(TFM_SYSTEM_PROCESSOR cortex-m85)
    set(TFM_SYSTEM_ARCHITECTURE armv8.1-m.main)
    set(TFM_SYSTEM_DSP ON)
    set(CONFIG_TFM_FP_ARCH "fpv5-d16") # Double precision FPU
    set(CONFIG_TFM_FP_ARCH_ASM "FPv5_D16") # Instruction for FPU with double precision

    # RA_CORE allows to check active core
    add_compile_definitions(RA_CORE=85)
elseif(RA_CORE STREQUAL "cm33")
    # CM33 core for NSPE
    set(TFM_SYSTEM_PROCESSOR cortex-m33)
    set(TFM_SYSTEM_ARCHITECTURE armv8-m.main)
    set(TFM_SYSTEM_DSP ON)
    set(CONFIG_TFM_FP_ARCH "fpv5-sp-d16")
    set(CONFIG_TFM_FP_ARCH_ASM "FPv5_SP_D16")

    # RA_CORE allows to check active core
    add_compile_definitions(RA_CORE=33)
else()
    message(FATAL_ERROR
            " \nPlease select the core for this build targets:\n"
            "    -DRA_CORE=cm85   CPU0, TrustZone\n"
            "    -DRA_CORE=cm33   CPU1, Mailbox NS client\n")
endif()
