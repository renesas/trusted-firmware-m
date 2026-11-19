#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Regression-test only (same condition as plat_test.c, see common.cmake).
# Not part of the FSP core build (fsp/fsp_s_lib.cmake).
#
# Static library for the FSP driver sources used by the SLIH/FLIH IRQ test
# partitions (r_agt.c), which call into them unprivileged.
# TF-M's isolation linker scripts (scripts/{gcc,atfe,armclang}) place every
# "*libtfm_sprt*" object in ER_UNPRIV_CODE: code that is executable both
# privileged and unprivileged (not PXN, unlike ER_APP_ROT_CODE). Naming the
# library with "libtfm_sprt" therefore puts its functions in that region
# without a linker script change. Privileged-only entry points (e.g. the
# *_Open() and ISR paths) also run from there, which is allowed.
# Do not link fsp_library here: an OBJECT library would copy every FSP object
# into this archive, and so into the unprivileged region.
if(TFM_S_REG_TEST OR TFM_NS_REG_TEST)
    add_library(tfm_sprt_fsp_agt STATIC)

    target_sources(tfm_sprt_fsp_agt
        PRIVATE
        ${RENESAS_RA_FSP_LIB_PATH}/ra/fsp/src/r_agt/r_agt.c
    )

    target_include_directories(tfm_sprt_fsp_agt
        PRIVATE
        $<TARGET_PROPERTY:fsp_library,INTERFACE_INCLUDE_DIRECTORIES>
    )

    target_compile_definitions(tfm_sprt_fsp_agt
        PRIVATE
        $<TARGET_PROPERTY:fsp_library,INTERFACE_COMPILE_DEFINITIONS>
    )

    target_compile_options(tfm_sprt_fsp_agt
        PRIVATE
        ${COMPILER_CP_FLAG}
        ${COMPILER_CMSE_FLAG}
    )

    target_link_libraries(tfm_sprt_fsp_agt
        PRIVATE
        cmsis
        fsp_library_config
        cmsis_stack_override
    )
endif()
