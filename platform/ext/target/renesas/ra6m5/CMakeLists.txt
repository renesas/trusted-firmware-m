#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# RA6M5 platform. Consumes the e2 studio SOLUTION project set:
#
#   cmake -DTFM_PLATFORM=renesas/ra6m5 \
#         -DFSP_BL2_APP_DIR=<repo>/ra_fsp_cmake_tfm/ra6m5_mcuboot   \
#         -DFSP_S_APP_DIR=<repo>/ra_fsp_cmake_tfm/ra6m5_secure      \
#         -DFSP_NS_APP_DIR=<repo>/ra_fsp_cmake_tfm/ra6m5_nonsecure  ...
#
# Why no per-project cmake file: e2-generated projects carry none, and hand-kept source lists
# drift from the generated tree (that drift broke the ra6m4 external BL2 path). This file
# selects the sources itself: by directory glob for the NS image, as RASC's GeneratedSrc.cmake
# does, and through the FSP module registry below for S and BL2.
#
# After regenerating in e2, re-run cmake. The NS image and the flash layout (through
# bsp_linker_info.h, see flash_layout.h) follow with no edits. A module newly enabled for S or
# BL2 also needs an entry in the FSP module registry; see ADDING_FSP_MODULES.md.
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

set(FSP_BL2_APP_DIR "" CACHE PATH "e2 solution bootloader project (ra6m5_mcuboot)")
set(FSP_S_APP_DIR   "" CACHE PATH "e2 solution secure project (ra6m5_secure)")
set(FSP_NS_APP_DIR  "" CACHE PATH "e2 solution non-secure project (ra6m5_nonsecure)")

#-------------------------------------------------------------------------------
# Validation + source selection helpers
#-------------------------------------------------------------------------------

# The e2 build emits the generated layout (bsp_linker_info.h, memory_regions.ld, fsp_gen.ld)
# into <project>/Debug/, which is deliberately not committed. Fail at configure time if the
# project has not been built in e2, rather than with a missing-header error deep in the compile.
function(fsp_validate_project dir role)
    if(NOT EXISTS "${dir}")
        message(FATAL_ERROR "RA6M5: FSP ${role} project not found: ${dir}")
    endif()
    foreach(sub ra ra_gen ra_cfg)
        if(NOT EXISTS "${dir}/${sub}")
            message(FATAL_ERROR
                "RA6M5: ${dir} does not look like a generated FSP project "
                "(missing ${sub}/). Generate it in e2 studio first.")
        endif()
    endforeach()
    if(NOT EXISTS "${dir}/Debug/bsp_linker_info.h")
        message(FATAL_ERROR
            "RA6M5: ${dir}/Debug/bsp_linker_info.h is missing.\n"
            "The solution carries the flash layout and emits it during the e2 build. "
            "Build the ${role} project in e2 studio once, then re-run cmake.")
    endif()
endfunction()

# Glob the FSP sources of a generated project. Non-secure only.
#
# S and BL2 are built from the module libraries in the FSP module registry below. The NS image
# is a complete FSP application: it keeps FSP's main, startup.c, bsp_linker.c and linker script,
# and links no TF-M platform library. To add a driver to S or BL2, add a module, not a glob.
#
#   out_var : list of .c files
#   dir     : project root
#   role    : NS (the S and BL2 branches remain only to document the exclusions)
function(fsp_collect_sources out_var dir role)
    file(GLOB_RECURSE _src
        "${dir}/ra/fsp/src/bsp/*.c"
        "${dir}/ra/fsp/src/r_*/*.c"
        "${dir}/ra/fsp/src/rm_*/*.c"
        "${dir}/ra_gen/*.c"
    )
    if(role STREQUAL "NS")
        # The NS project is the application, so take its src/ too.
        file(GLOB_RECURSE _app "${dir}/src/*.c")
        list(APPEND _src ${_app})
        file(GLOB_RECURSE _rtos "${dir}/ra/aws/*.c" "${dir}/ra/arm/*.c")
        list(APPEND _src ${_rtos})
    endif()

    # FSP modules TF-M replaces. The e2 projects enable them for the standalone bootloader,
    # which must keep working, so they are dropped here rather than reconfigured there. The
    # reason for each is under FSP_MODULES_NEVER_BUILT below. r_sce is dropped from the NS glob
    # too; in S and BL2 the sce module builds it. The headers stay on the include path,
    # because the generated ra_gen/common_data.h and hal_data.h include them; nothing is
    # linked from them.
    set(FSP_EXCLUDED_MODULES r_sce rm_psa_crypto rm_mcuboot_port)
    foreach(m ${FSP_EXCLUDED_MODULES})
        list(FILTER _src EXCLUDE REGEX "/ra/fsp/src/${m}/")
    endforeach()
    list(FILTER _src EXCLUDE REGEX "/ra/arm/mbedtls/")

    # Files TF-M supplies for S and BL2; FSP's copies would collide at link time. The NS image
    # has no TF-M equivalent and keeps all three:
    #   ra_gen/main.c        - TF-M has its own main for S and BL2
    #   .../Source/startup.c - FSP's Reset_Handler, which FSP's linker script expects
    #   bsp_linker.c         - emits the OFS option-setting sections and provides the real
    #                          g_init_info for C-runtime init. TF-M emits OFS only from BL2's
    #                          bl2_option_setting.c, with discrete MEMORY regions. FSP's copy
    #                          would put option memory in the signed secure image; in the NS
    #                          image BSP_TZ_NONSECURE_BUILD compiles those sections out.
    set(_excl "")
    if(NOT role STREQUAL "NS")
        list(APPEND _excl "${dir}/ra_gen/main.c")
        list(APPEND _excl "${dir}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c")
        file(GLOB_RECURSE _linker_c "${dir}/ra/fsp/src/bsp/mcu/*/bsp_linker.c")
        list(APPEND _excl ${_linker_c})
    endif()

    foreach(f ${_excl})
        list(REMOVE_ITEM _src "${f}")
    endforeach()

    list(REMOVE_DUPLICATES _src)
    if(NOT _src)
        message(FATAL_ERROR "RA6M5: no FSP sources found under ${dir} - is it generated?")
    endif()
    set(${out_var} "${_src}" PARENT_SCOPE)
endfunction()

function(fsp_collect_includes out_var dir)
    set(_inc
        "${dir}"
        "${dir}/Debug"                       # bsp_linker_info.h - generated layout
        "${dir}/ra/fsp/inc"
        "${dir}/ra/fsp/inc/api"
        "${dir}/ra/fsp/inc/instances"
        "${dir}/ra/arm/CMSIS_6/CMSIS/Core/Include"
        "${dir}/ra_cfg/fsp_cfg"
        "${dir}/ra_cfg/fsp_cfg/bsp"
        "${dir}/ra_gen"
    )
    # MCU-specific BSP headers - glob so a device change does not need an edit.
    file(GLOB _mcu_dirs "${dir}/ra/fsp/src/bsp/mcu/*")
    foreach(d ${_mcu_dirs})
        if(IS_DIRECTORY "${d}")
            list(APPEND _inc "${d}")
        endif()
    endforeach()
    set(${out_var} "${_inc}" PARENT_SCOPE)
endfunction()

# FSP is built with warnings TF-M does not use; keep them off the TF-M targets.
#
# GCC spellings, hence the gate: iccarm rejects the whole command line on an unknown -W option,
#     Command line error: Unexpected command line arguments: -Wno-conversion ...
# and every FSP source fails to compile. IAR needs nothing here: TF-M's IAR toolchain file
# already passes --diag_suppress=Pe546,Pe940,Pa082,Pa084, and warnings are fatal only when
# CONFIG_TFM_WARNINGS_ARE_ERRORS adds --warnings_are_errors.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(FSP_COMPILE_OPTIONS -Wno-conversion -Wno-sign-conversion -Wno-unused-parameter)
else()
    set(FSP_COMPILE_OPTIONS "")
endif()
set(FSP_COMPILE_DEFS _RA_CORE=CM33 _RENESAS_RA_)

# TrustZone build flavour, per role. The secure image declares itself secure to FSP. BL2 is
# deliberately flat, as the e2 solution template defines the bootloader.
#
# The flavour selects the OFS1_SEL formula in the generated bsp_mcu_ofs_cfg.h:
#
#   #ifndef BSP_CFG_OPTION_SETTING_OFS1_SEL
#   #if defined(_RA_TZ_SECURE) || defined(_RA_TZ_NONSECURE)
#     #define BSP_CFG_OPTION_SETTING_OFS1_SEL \
#         (0xFFFFF8F8 | ... | ((BSP_CFG_CLOCKS_SECURE == 0) ? 0xF00 : 0U))
#   #else
#     #define BSP_CFG_OPTION_SETTING_OFS1_SEL (0xFFFFF8F8)
#   #endif
#   #endif
#
# BL2 is the only image that emits OFS (DESIGN.md 8.4). Declared _RA_TZ_SECURE, it takes the
# TrustZone branch, reads BSP_CFG_CLOCKS_SECURE (set to (0) by the generated
# ra_gen/bsp_clock_cfg.h) and ORs in 0xF00, giving 0xFFFFFFF8: the clock-related OFS1 fields
# marked non-secure. On a part with the TZ boundaries programmed, that attribution mismatch can
# lock out the debug interface. Flat takes the #else branch and emits 0xFFFFF8F8,
# byte-identical to the e2 bootloader.
#
# Do not pass -DBSP_CFG_CLOCKS_SECURE=1 instead: bsp_clock_cfg.h defines it unguarded, so the
# header wins and the only effect is a redefinition warning.
#
# What flat BL2 gives up (BSP_TZ_SECURE_BUILD 1 -> 0; BSP_TZ_NONSECURE_BUILD is 0 either way,
# so the OFS sections still emit):
#   R_BSP_NonSecureEnter()  - not compiled. BL2 never calls it; MCUboot chainloads the
#                             secure image and TF-M performs the S->NS jump itself.
#   r_flash_hp callbacks    - no cmse_nsfptr_create wrapping. BL2's flash callback is
#                             never a non-secure function pointer.
#   r_ioport.c:857          - inactive either way; BSP_FEATURE_TZ_NS_OFFSET is 0 here.
#   bsp_sdram NSC entries   - already off; they need BSP_CFG_CLOCKS_SECURE == 1.
#
# Verify: arm-none-eabi-objdump -s -j .option_setting_ofs1_sel bin/bl2.axf -> f8f8ffff
set(FSP_TZ_DEFS_S   _RA_TZ_SECURE=1)
set(FSP_TZ_DEFS_BL2 )

#-------------------------------------------------------------------------------
# FSP module registry
#
# S and BL2 are built from per-module libraries, not from one glob of the project tree. A module
# is opt-in: it enters the build only by being listed below and having a
# cmake/modules/fsp_<name>.cmake. The e2 projects enable modules for the standalone bootloader
# (SCE, PSA crypto, the MCUboot port) that must not enter a signed TF-M image only because they
# are present.
#
# The NS image is a whole FSP application with FSP's own startup and linker script; it is still
# collected by fsp_collect_sources().
#-------------------------------------------------------------------------------
set(FSP_MODULES_S    bsp flash sce agt)
#                    sce: SCE9 TRNG backs the PSA external RNG
#                    agt: the FLIH/SLIH test timer, and agt_int_isr() which the
#                         generated vector table names - see fsp_agt.cmake
set(FSP_MODULES_BL2  bsp flash)
# BL2 needs no randomness, but with the SCE9 accelerator on its image hash (SHA-256) runs on
# the engine - platform/ext/accelerator/renesas/sce9. See fsp_sce.cmake.
if(CRYPTO_HW_ACCELERATOR)
    list(APPEND FSP_MODULES_BL2 sce)
endif()

# Which ra/fsp/src/<dir> each module accounts for, so an unclaimed module in the project
# can be reported rather than silently dropped.
set(FSP_MODULE_DIRS_bsp    bsp r_ioport)
set(FSP_MODULE_DIRS_flash  r_flash_hp)
set(FSP_MODULE_DIRS_sce    r_sce)
set(FSP_MODULE_DIRS_agt    r_agt)

# In the e2 projects but never built here; TF-M supplies its own. Their headers still resolve
# (see fsp_bsp.cmake) because the generated ra_gen files include them unconditionally.
#   rm_psa_crypto, ra/arm/mbedtls - FSP's PSA core and its mbedTLS config, which wires PSA ITS
#       to littlefs and conflicts with TFM_PARTITION_INTERNAL_TRUSTED_STORAGE (DESIGN.md 6).
#       FSP's crypto code still reaches the image by two routes: the *_ALT sources from
#       platform/ext/accelerator/renesas/sce9, and the library from the
#       cmake/fsp_mbedtls.cmake overlay, which FORCEs MBEDCRYPTO_PATH. With
#       CRYPTO_HW_ACCELERATOR ON (the default) the SCE9 ciphers are live.
#   r_sce - built by the sce module (FSP_MODULE_DIRS_sce). Listed here only to keep the
#       unclaimed-directory audit quiet for BL2 without the accelerator, which declares no sce
#       module. See cmake/modules/fsp_sce.cmake.
#   rm_mcuboot_port - FSP's flash_map/MCUboot glue. TF-M keeps its own flash_map and area IDs,
#       whose numbering differs from FSP's (see "bsp_partitions.h" below, D013). Grafting the
#       FSP port is what failed on ra6m4 (DESIGN.md 4, 5); TF-M's Driver_Flash.c carries the RA
#       32 KB region-1 geometry instead.
set(FSP_MODULES_NEVER_BUILT r_sce rm_psa_crypto rm_mcuboot_port)

# Declare this role's module libraries. Returns their target names in out_targets.
function(fsp_add_modules role dir includes out_targets)
    string(TOUPPER "${role}" _role_uc)
    set(_modules ${FSP_MODULES_${_role_uc}})

    set(FSP_MODULE_ROLE     "${role}")
    set(FSP_MODULE_BASE_DIR "${dir}")
    set(FSP_MODULE_INCLUDES "${includes}")
    set(FSP_MODULE_TZ_DEFS  ${FSP_TZ_DEFS_${_role_uc}})
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/fsp_module_common.cmake)

    set(_targets "")
    set(_claimed "")
    foreach(m ${_modules})
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/fsp_${m}.cmake)
        list(APPEND _targets "fsp_${m}_${role}")
        list(APPEND _claimed ${FSP_MODULE_DIRS_${m}})
    endforeach()

    # A project module no module claims is not in the image. Report it: a silently missing
    # driver surfaces much later as a confusing link error.
    file(GLOB _present RELATIVE "${dir}/ra/fsp/src" "${dir}/ra/fsp/src/*")
    foreach(d ${_present})
        if(IS_DIRECTORY "${dir}/ra/fsp/src/${d}")
            if((NOT d IN_LIST _claimed) AND (NOT d IN_LIST FSP_MODULES_NEVER_BUILT))
                message(WARNING
                    "RA6M5 [${role}]: FSP module '${d}' is in the project but no module "
                    "declares it, so it is NOT in the image. Add a "
                    "cmake/modules/fsp_<name>.cmake and list it in FSP_MODULES_${_role_uc} "
                    "- see ADDING_FSP_MODULES.md.")
            endif()
        endif()
    endforeach()

    set(${out_targets} "${_targets}" PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
# Secure side
#-------------------------------------------------------------------------------
fsp_validate_project("${FSP_S_APP_DIR}" "secure")
fsp_collect_includes(FSP_S_INCLUDES "${FSP_S_APP_DIR}")

#-------------------------------------------------------------------------------
# bsp_partitions.h - the partition macros, extracted from the solution
#
# flash_layout.h and region_defs.h need the BSP_PARTITION_* values, in C code and in
# ra6m5_bl2.ld, which the toolchain preprocesses with -xc. bsp_linker_info.h cannot serve the
# linker script: it also declares C types and externs (typedef enum e_bsp_init_mem, gp_ddsc_*),
# which land verbatim in the preprocessed script and fail to parse. Under #ifdef FLASH_MAP_C
# it also defines a `static const struct flash_area flash_map[]` that would collide with
# TF-M's own.
#
# That map describes the same layout as this port: the solution's bootloader is
# MCUBOOT_IMAGE_NUMBER 2, and its four entries carry the same offsets, derived from the same
# BSP_PARTITION___BL_* macros. Only the ID numbering differs, so never mix the two conventions
# in one image (D013):
#
#   area              FSP id                TF-M id
#   secure primary    FLASH_AREA_0P_ID 1    FLASH_AREA_0_ID 1
#   secure secondary  FLASH_AREA_0S_ID 2    FLASH_AREA_2_ID 3
#   NS primary        FLASH_AREA_1P_ID 3    FLASH_AREA_1_ID 2
#   NS secondary      FLASH_AREA_1S_ID 4    FLASH_AREA_3_ID 4
#
# So configure filters the file down to its #define BSP_PARTITION_* lines.
# CMAKE_CONFIGURE_DEPENDS re-runs cmake when the solution regenerates the file, so
# repartitioning in e2 needs only a rebuild.
#
# The secure project's copy is the source. The three copies agree on the shared partitions, and
# the secure one also carries the RAM_BL / DATA_FLASH_BL / QSPI_FLASH_BL entries the
# bootloader's copy omits.
#-------------------------------------------------------------------------------
set(FSP_PARTITION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(FSP_PARTITION_HEADER "${FSP_PARTITION_HEADER_DIR}/bsp_partitions.h")
set(FSP_LINKER_INFO_SRC "${FSP_S_APP_DIR}/Debug/bsp_linker_info.h")

file(STRINGS "${FSP_LINKER_INFO_SRC}" _partition_defs
     REGEX "^[ \t]*#define[ \t]+BSP_PARTITION_[A-Za-z0-9_]+[ \t]")
if(NOT _partition_defs)
    message(FATAL_ERROR
        "RA6M5: no BSP_PARTITION_* defines in ${FSP_LINKER_INFO_SRC}. "
        "The solution emits them during the e2 build - rebuild the secure project there.")
endif()
list(JOIN _partition_defs "\n" _partition_body)
file(CONFIGURE OUTPUT "${FSP_PARTITION_HEADER}" CONTENT
"/* GENERATED - do not edit. Extracted by the ra6m5 platform CMakeLists from
 * ${FSP_LINKER_INFO_SRC}
 * Macros only: this header is preprocessed into ra6m5_bl2.ld as well as compiled.
 */
#ifndef __BSP_PARTITIONS_H__
#define __BSP_PARTITIONS_H__

${_partition_body}

#endif /* __BSP_PARTITIONS_H__ */
")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FSP_LINKER_INFO_SRC}")

# platform_region_defs is what the scatter targets link, so these paths make region_defs.h and
# its includes resolvable when a linker script is preprocessed.
#
# The platform directory is needed for the secure script: TF-M generates tfm_isolation_s.ld into
# ${CMAKE_BINARY_DIR}/generated/, where its #include "region_defs.h" has no relative path to
# follow. ra6m5_bl2.ld, in this directory, works without it. The rse/tc and stm32wba65i-dk
# platforms do the same.
target_include_directories(platform_region_defs
    INTERFACE
        "${FSP_PARTITION_HEADER_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

# FSP module libraries for the secure image.
fsp_add_modules(s "${FSP_S_APP_DIR}" "${FSP_S_INCLUDES}" FSP_S_MODULES)
target_link_libraries(platform_s PUBLIC ${FSP_S_MODULES})

target_sources(platform_s PRIVATE
    # PSA external RNG over the SCE9 TRNG: the entropy source for the whole secure image
    ${CMAKE_CURRENT_SOURCE_DIR}/sce_trng.c
    # One-time SCE bring-up, shared by the TRNG and the cipher accelerator
    ${CMAKE_CURRENT_SOURCE_DIR}/ra_sce_init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_peripherals_def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    # Static assertions only, no code. Catches an OTP area too small for the backend's region
    # struct, which otherwise folds init_otp_nv_counters_flash() into an unconditional error
    # return with no diagnostic. See the file.
    ${CMAKE_CURRENT_SOURCE_DIR}/ra6m5_layout_checks.c
    # bsp_init_stub.c is in the fsp_bsp module, beside the FSP system.c that needs it; see
    # cmake/modules/fsp_bsp.cmake.
    # The common ARMv8-M isolation HAL is deliberately not here: it goes in tfm_spm. See the
    # note above target_sources(tfm_spm) below. DECISIONS D120.

    # tfm_hal_irq_enable/_disable/_clear_pending, required once any partition manifest declares
    # an "irqs" entry. The common file is the whole HAL.
    #
    # Not ext/common/tfm_interrupts.c: that is Arm's TIMER0 test fixture. It defines
    # TFM_TIMER0_IRQ_Handler and none of the HAL, and references TFM_TIMER0_IRQ, which no RA
    # port defines. DECISIONS D061.
    $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${PLATFORM_DIR}/ext/common/tfm_hal_nvic.c>

    # The TIMER0 fixture the FLIH/SLIH suites need: the handler and the SPM registration hook.
    # RA-specific; the header of tfm_timer0_irq.c explains why ext/common/tfm_interrupts.c
    # cannot be used on this family. DECISIONS D061.
    #
    # This half stays in platform_s: the handler runs in Handler mode, which is always
    # privileged, and the SPM calls tfm_timer0_irq_init(). plat_test.c, the other half, must
    # not be here; see target_sources(tfm_sprt) below.
    $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${CMAKE_CURRENT_SOURCE_DIR}/tfm_timer0_irq.c>

)

# plat_test.c goes in tfm_sprt, not platform_s. The difference shows only above isolation 1.
#
# The FLIH and SLIH test services are Application RoT partitions, so from isolation 2 they run
# unprivileged, and they call tfm_plat_test_secure_timer_{start,stop,clear_intr} directly.
# platform_s code lands in PT_PSA_ROT_CODE, which the static MPU table in
# tfm_hal_isolation_v8m.c marks ARM_MPU_PRIVILEGED, so the call takes an instruction access
# violation (MMFSR 0x01, IACCVIOL; measured on RA6M5 at isolation 2, D113). tfm_sprt, the Secure
# Partition RunTime library, links into the unprivileged code region, where code an ARoT
# partition calls has to be. an521 does the same for timer_cmsdk.c.
#
# At isolation 1, which this port ships, CONFIG_TFM_ENABLE_MEMORY_PROTECT is undefined, no MPU
# region is programmed, and the call works.
#
# The data side is already covered: the FLIH service declares TFM_PERIPHERAL_TIMER0 as a named
# MMIO region, so tfm_hal_bind_boundary() programs an MPU region for the AGT0 registers.
target_sources(tfm_sprt
    PRIVATE
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c>
)

# plat_test.c takes platform_s's resolved include path, not a hand-written subset.
#
# It includes hal_data.h for the generated g_timer0_ctrl / g_timer0_cfg instance. That header
# pulls in ra_gen/common_data.h, which includes rm_psa_crypto.h from an FSP module directory
# (ra/fsp/src/rm_psa_crypto/inc) that fsp_module_library() adds to platform_s and that is not
# in FSP_S_INCLUDES. A hand-written list breaks whenever the generated common_data.h or the e2
# project's module set changes:
#
#     ra_gen/common_data.h:9:10: fatal error: rm_psa_crypto.h: No such file or directory
#
# platform_s's list stays right as modules come and go.
#
# Scoped to the one file: tfm_sprt is linked into every partition, and the FSP tree must not
# reach the whole target, where an FSP header could shadow a partition's.
#
# TARGET_DIRECTORY must name tfm_sprt. Source file properties are directory-scoped, and
# tfm_sprt is defined under secure_fw/, not here. Without it the property lands in this
# directory's scope, where nothing reads it, and the build fails on a missing bsp_api.h with no
# indication why. ra6m5_layout_checks.c below needs the same for platform_s.
if(CONFIG_TFM_FLIH_API OR CONFIG_TFM_SLIH_API)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c
        TARGET_DIRECTORY tfm_sprt
        PROPERTIES INCLUDE_DIRECTORIES
            "$<TARGET_PROPERTY:platform_s,INCLUDE_DIRECTORIES>")
endif()
# The shared ARMv8-M isolation HAL goes in tfm_spm, not platform_s. The placement is
# load-bearing; do not move it back. DECISIONS D120, D121.
#
# tfm_hal_isolation_v8m.c provides tfm_hal_set_up_static_boundaries, tfm_hal_bind_boundary and
# tfm_hal_memory_check, and switches on TFM_ISOLATION_LEVEL and CONFIG_TFM_PARTITION_META
# throughout. psa_interface is the only target carrying those (interface/CMakeLists.txt:37).
# tfm_spm links psa_interface; platform_s links tfm_config and does not. Every upstream user of
# this file (an519, an524, corstone300/310, mps4, musca_b1/s1, rse, alcor) adds it to tfm_spm
# for this reason.
#
# What each missing macro costs, measured on RA6M5 (D120):
#   TFM_ISOLATION_LEVEL undefined, at isolation 2 - the named-MMIO region is never programmed.
#       The post-partition dump showed 5 enabled regions where 6 were expected, nothing
#       covering AGT0, and an ARoT partition touching its own mmio_region took a DACCVIOL.
#   TFM_ISOLATION_LEVEL=3 defined but CONFIG_TFM_PARTITION_META missing, at isolation 3 -
#       TFM_SP_META_PTR is never covered. Four static regions instead of five, then
#       MMFSR 0x82 / MMFAR 0x20000400, the metadata pointer itself.
# With both undefined, "#if TFM_ISOLATION_LEVEL != 3" reads 0 != 3 = true, so the App RoT data
# arm fires and level 3 builds five regions, but the wrong ones.
#
# Taking the macros from psa_interface fixes the class, not the instances: anything
# psa_interface gains later arrives with it, and the port defines neither macro by hand.
target_sources(tfm_spm
    PRIVATE
        ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
)

# TFM_PXN_ENABLE, which the same file tests (#ifdef), is plumbed by TF-M:
# platform/CMakeLists.txt:126 emits it PUBLIC on platform_s, and tfm_spm links platform_s, so
# it reaches the HAL in tfm_spm after the D121 move. It is absent from default compile lines
# only because TFM_PXN_ENABLE defaults OFF, which collapses the generator expression to
# nothing. The architecture gate is in config/check_config.cmake, not in the generator
# expression; its provenance (d839593bf, ec27c23f5, d90aa0c44) is in D117.
#
# PXN cannot be enabled on this part:
#
#   config/check_config.cmake:91
#     tfm_invalid_config(TFM_PXN_ENABLE AND NOT TFM_SYSTEM_ARCHITECTURE STREQUAL "armv8.1-m.main")
#
# RA6M5 is a Cortex-M33, and cpuarch.cmake:15 sets TFM_SYSTEM_ARCHITECTURE armv8-m.main, so
# -DTFM_PXN_ENABLE=ON fails the configure by design: PXN is an Armv8.1-M feature. The
# plumbing is verified on RA8M2 (armv8.1-m.main, cpuarch.cmake:24); see that port's copy of
# this comment.
# See D115, D117, D121, D128.

target_include_directories(platform_s PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    ${FSP_S_INCLUDES}
)
# Scoped to the one file rather than added to platform_s: the template directory holds TF-M's
# default platform implementations, and on the whole target's include path a header there could
# shadow one of ours. TARGET_DIRECTORY is required, as for plat_test.c above: platform_s is
# defined in platform/CMakeLists.txt, and without it the include is silently never applied.
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/ra6m5_layout_checks.c
    TARGET_DIRECTORY platform_s
    PROPERTIES INCLUDE_DIRECTORIES "${PLATFORM_DIR}/ext/common/template")

# attest_hal.c needs tfm_strnlen.h. With PLATFORM_DEFAULT_ATTEST_HAL OFF, platform_s does not
# inherit tfm_sprt's include paths as the default HAL does.
#
# ext/common: tfm_hal_platform.c needs uart_stdout.h for the stdio_* backend API. PRIVATE, and
# not ext/common/template, whose default implementations would shadow ours. Same as the ra6m4
# port.
target_include_directories(platform_s PRIVATE
    ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/include
    ${PLATFORM_DIR}/ext/common
)
target_compile_definitions(platform_s PUBLIC ${FSP_COMPILE_DEFS} ${FSP_TZ_DEFS_S})
target_compile_options(platform_s PRIVATE ${FSP_COMPILE_OPTIONS})

# -mcmse, PUBLIC: it must reach every target that links platform_s, not only the FSP sources.
# tfm_s compiles secure_fw/partitions/ns_agent_tz/psa_api_veneers_v80m.c; without -mcmse the
# compiler does not define __ARM_FEATURE_CMSE, security_defs.h compiles out __tz_naked_veneer,
# and the veneers fail to parse.
target_compile_options(platform_s PUBLIC ${COMPILER_CMSE_FLAG})

target_sources(tfm_s PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6m5.c)

# TF-M generates the secure linker script from its template using our region_defs.h (which is
# how TFM_LINKER_VENEERS_START pins the NSC window, DESIGN.md 7), but does not attach it to
# tfm_s. Without this the link has no -T and every script symbol is undefined
# (Image$$ARM_LIB_STACK$$ZI$$Limit, __copy_table_start__, Image$$TFM_SP_LOAD_LIST$$RO$$Base,
# ...). There is no custom secure linker script, only the generated one.
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:GNU>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/gcc/tfm_isolation_s.ld>
    $<$<C_COMPILER_ID:ARMClang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/armclang/tfm_isolation_s.sct>
    $<$<C_COMPILER_ID:IAR>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/iar/tfm_isolation_s.icf>
    $<$<C_COMPILER_ID:Clang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/llvm/tfm_isolation_s.ld>
)

# Point FSP's g_main_stack at the image's real stack, which belongs to TF-M. FSP's SystemInit()
# sets MSPLIM from &g_main_stack[0] and writes its stack seal at
# &g_main_stack[BSP_CFG_STACK_MAIN_BYTES]. A private array instead put MSPLIM 44 KB above the
# live SP and hard-faulted the first push inside SystemInit(); see bsp_init_stub.c.
#
# --defsym, not C: the target symbol is defined by the linker script, so
# __attribute__((alias)) is rejected and an assembler .set leaves it undefined.
#
# Passed through a response file, deliberately (D010). The symbol Image$$ARM_LIB_STACK$$ZI$$Base
# contains '$$', and how many '$' survive from a CMake string to the link line depends on the
# CMake/Ninja version. CMake 4.1.1 / Ninja 1.13.1 halves them:
#     ld: --defsym:1: undefined symbol `Image$ARM_LIB_STACK$ZI$Base'
# file(WRITE) emits the bytes verbatim and ld reads them verbatim.
#
# Verify after any change, and after any toolchain upgrade:
#   cat build_ra6m5/platform/target/g_main_stack.rsp     # two '$' per separator
#   arm-none-eabi-nm bin/tfm_s.axf | grep g_main_stack   # resolved, not absent
set(RA6M5_MAIN_STACK_RSP "${CMAKE_CURRENT_BINARY_DIR}/g_main_stack.rsp")
file(WRITE "${RA6M5_MAIN_STACK_RSP}"
     "--defsym=g_main_stack=Image$$ARM_LIB_STACK$$ZI$$Base\n")
set(RA6M5_MAIN_STACK_DEFSYM "-Wl,@${RA6M5_MAIN_STACK_RSP}")

# IAR does the same job by a different mechanism, on the same two images. ILINK has no
# block-address expression to alias against, so the reference is redirected instead of a symbol
# being defined. Defined here so tfm_s and bl2 share one response file; see the IAR note in the
# BL2 block below.
set(RA6M5_IAR_STACK_RSP "${CMAKE_CURRENT_BINARY_DIR}/g_main_stack_iar.rsp")
file(WRITE "${RA6M5_IAR_STACK_RSP}"
     "--redirect g_main_stack=ARM_LIB_STACK$$Base
")
set(RA6M5_IAR_STACK_OPT "SHELL:-f ${RA6M5_IAR_STACK_RSP}")

# TF-M's IAR toolchain links with "--redirect __write=__write_buffered", which leaves no
# reference to __write. The RTT-backed __write() in rtt/rtt_stdout.c is then garbage-collected,
# and printf() (which MCUboot's BOOT_LOG_* expand to) falls through to the semihosting stub,
# silent without a debugger: the image boots and prints nothing.
#
# --keep forces the definition in, so __write_buffered binds to ours (D025). Verify after any
# toolchain change:
#   arm-none-eabi-readelf -sW bin/bl2.axf | grep " __write$"      # present, not absent
#   arm-none-eabi-readelf -sW bin/bl2.axf | grep SEGGER_RTT_Write # pulled in behind it
set(RA6M5_IAR_KEEP_WRITE "SHELL:--keep __write")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_options(tfm_s PRIVATE ${RA6M5_MAIN_STACK_DEFSYM})
elseif(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    target_link_options(tfm_s PRIVATE "${RA6M5_IAR_STACK_OPT}" "${RA6M5_IAR_KEEP_WRITE}")
endif()

if(RA6M5_STDOUT_RTT)
    target_sources(platform_s PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
        ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c)
    target_include_directories(platform_s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rtt)
    if(RA6M5_RTT_BLOCKING)
        target_compile_definitions(platform_s PRIVATE SEGGER_RTT_MODE_DEFAULT=2)
    endif()
else()
    target_sources(platform_s PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c)
endif()

#-------------------------------------------------------------------------------
# BL2 (MCUboot)
#-------------------------------------------------------------------------------
if(BL2)
    fsp_validate_project("${FSP_BL2_APP_DIR}" "bootloader")
    fsp_collect_includes(FSP_BL2_INCLUDES "${FSP_BL2_APP_DIR}")

    # FSP module libraries for the bootloader. Its e2 project also contains r_sce, rm_psa_crypto
    # and rm_mcuboot_port for the standalone build. rm_psa_crypto and rm_mcuboot_port are never
    # declared; r_sce enters only through the sce module, when CRYPTO_HW_ACCELERATOR is ON.
    fsp_add_modules(bl2 "${FSP_BL2_APP_DIR}" "${FSP_BL2_INCLUDES}" FSP_BL2_MODULES)
    target_link_libraries(platform_bl2 PUBLIC ${FSP_BL2_MODULES})

    # bsp_init_stub.c is in the fsp_bsp_bl2 module, beside the FSP system.c that needs it; see
    # cmake/modules/fsp_bsp.cmake.
    #
    # tfm_hal_platform.c is here only for __STACK_SEAL, which anchors .msp_stack_seal_res.
    # Its secure-only half is guarded on RA6M5_BUILDING_BL2; the note there explains why the
    # plain BL2 macro cannot serve.
    target_sources(platform_bl2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    )
    target_compile_definitions(platform_bl2 PRIVATE RA6M5_BUILDING_BL2)
    target_include_directories(platform_bl2 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
        ${FSP_BL2_INCLUDES}
    )
    # No _RA_TZ_ macro: BL2 is a flat build, like the e2 solution's bootloader.
    target_compile_definitions(platform_bl2 PUBLIC ${FSP_COMPILE_DEFS} ${FSP_TZ_DEFS_BL2})
    target_compile_options(platform_bl2 PRIVATE ${FSP_COMPILE_OPTIONS})
    target_compile_options(platform_bl2 PUBLIC ${COMPILER_CMSE_FLAG})

    # OFS goes in BL2 only, and only with the discrete per-group MEMORY regions in
    # ra6m5_bl2.ld. Emitted as one coalesced PT_LOAD, it zero-fills the configuration memory
    # between the option groups and bricks the part, with no known recovery; it cost two
    # EK-RA6M4 boards. The fill writes 0x00 into eight defined option registers (DUALSEL, OFS1,
    # BANKSEL, BPS, PBPS, BANKSEL_SEC, BPS_SEC, PBPS_SEC) and the reserved bytes between them
    # (DECISIONS D133). Which of the eight kills the part is not established (D094).
    # See DESIGN.md 8.4 and verify with: arm-none-eabi-readelf -l bin/bl2.axf
    target_sources(bl2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6m5.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
    )

    # SCE9 accelerator: BL2's SHA-256 runs on the engine, so BL2 brings it up itself in
    # boot_platform_post_init(), before MCUboot hashes the first slot.
    if(CRYPTO_HW_ACCELERATOR)
        target_sources(bl2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ra_sce_init.c
            ${CMAKE_CURRENT_SOURCE_DIR}/bl2_boot_hal.c
        )
    endif()

    # MCUboot's bl2_main.c calls stdio_init/stdio_uninit, so BL2, a separate binary with its own
    # RTT control block, needs its own stdout backend. config.cmake forces
    # PLATFORM_DEFAULT_UART_STDOUT off when RTT is on, so nothing else provides them.
    if(RA6M5_STDOUT_RTT)
        target_sources(platform_bl2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c)
        target_include_directories(platform_bl2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rtt)
        if(RA6M5_RTT_BLOCKING)
            target_compile_definitions(platform_bl2 PRIVATE SEGGER_RTT_MODE_DEFAULT=2)
        endif()
    else()
        target_sources(platform_bl2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c)
    endif()

    if(RA6M5_BL2_HALT_AT_MAIN)
        message(WARNING "RA6M5_BL2_HALT_AT_MAIN=ON - BL2 spins at main(); not bootable")
        target_compile_definitions(bl2 PRIVATE BL2_HALT_AT_MAIN=1)
    endif()

    target_add_scatter_file(bl2
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6m5_bl2.ld>
        $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
        $<$<C_COMPILER_ID:IAR>:${CMAKE_CURRENT_SOURCE_DIR}/ra6m5_bl2.icf>
    )

    # Same g_main_stack aliasing as the secure image; see the g_main_stack note above. BL2 is
    # flat, so BSP_TZ_SECURE_BUILD is 0 and the seal write is skipped; only MSPLIM applies.
    if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
        # IAR equivalent of the GNU --defsym below. FSP's system.c reads &g_main_stack[0] for
        # MSPLIM, but g_main_stack is defined in FSP's startup.c, which this port excludes for S
        # and BL2 (TF-M supplies its own startup). ILINK cannot alias a symbol to a
        # linker-created block from the .icf (there is no block-address expression), so the
        # reference is redirected instead.
        #
        # ILINK creates ARM_LIB_STACK$$Base for the .icf block of that name. TF-M's own
        # cmsis_override.h maps __STACK_LIMIT onto the same symbol for IAR, so FSP sees exactly
        # the stack TF-M runs on.
        #
        # Through a response file for the reason in D010: the symbol contains '$$', and how
        # many '$' CMake/Ninja rewrite is version-dependent. file(WRITE) emits it verbatim.
        target_link_options(bl2 PRIVATE "${RA6M5_IAR_STACK_OPT}" "${RA6M5_IAR_KEEP_WRITE}")
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_options(bl2 PRIVATE ${RA6M5_MAIN_STACK_DEFSYM})
        # ra6m5_bl2.ld relocates FSP's .ram_from_flash (the code-flash program/erase routines)
        # into RAM: executable code in a read/write segment, so ld reports "LOAD segment with
        # RWX permissions". TF-M links with -Wl,-fatal-warnings, so without this the link
        # fails. Scoped to bl2, the only image that programs code flash and the only one with a
        # relocated text section. FSP's own linker scripts produce the same RWX segment.
        target_link_options(bl2 PRIVATE -Wl,--no-warn-rwx-segments)
    endif()
endif()

#-------------------------------------------------------------------------------
# Orphan-section check (post-link, GNU only)
#
# These images do not use FSP's linker script, so nothing carries FSP's memory section contract
# across. cmake/ra6m5_check_orphans.cmake has the full reasoning, including why this is a
# post-link ELF check rather than a diff against fsp_gen.ld.
#
# $<TARGET_OBJECTS:<target>_scatter> is the preprocessed script: target_add_scatter_file() in
# toolchain_GNUARM.cmake compiles the .ld with -E -P -xc into that object library and passes it
# to the linker as -T. This is the text ld saw, with macros expanded and only the #if branches
# that applied. The source .ld would give the wrong answer for bl2, whose script has a CMSE and
# a non-CMSE arm.
#
# Known-benign orphans are allowlisted per image, each with a reason, so a new one is still
# reported:
#
#   tfm_s - none, deliberately. .ram_noinit is placed in .TFM_NOINIT through
#       S_DATA_EXTRA_NOINIT_SECTION_NAME in region_defs.h; if that placement regresses, this
#       check reports it.
#
#   bl2 .msp_stack_seal_res - bl2 is a flat build, so __ARM_FEATURE_CMSE != 3 and
#       ra6m5_bl2.ld deliberately takes the non-CMSE arm, which has no seal section.
#       startup_ra6m5.c still emits the 8-byte __StackSeal, which then orphans away from the
#       stack top. Inert: FSP writes the seal only when BSP_TZ_SECURE_BUILD, and TF-M's
#       startup only under CMSE. Both are false here.
#
# A warning by default: a TF-M or FSP update can add a benign section, which should not block a
# build. Set RA6M5_ORPHAN_CHECK_STRICT for CI.
#-------------------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(RA6M5_ORPHAN_CHECK_STRICT OFF CACHE BOOL
        "Fail the build on an unexpected orphan section instead of warning")

    # A custom target, not add_custom_command(TARGET ... POST_BUILD): that form is
    # directory-scoped, and tfm_s and bl2 are created in secure_fw/ and bl2/, not here.
    function(ra6m5_add_orphan_check target allow)
        add_custom_target(ra6m5_orphan_check_${target} ALL
            COMMAND ${CMAKE_COMMAND}
                    -DIMAGE=${target}
                    -DAXF=$<TARGET_FILE:${target}>
                    -DSCATTER=$<TARGET_OBJECTS:${target}_scatter>
                    -DOBJDUMP=${CMAKE_OBJDUMP}
                    -DALLOW=${allow}
                    -DSTRICT=${RA6M5_ORPHAN_CHECK_STRICT}
                    -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/cmake/ra6m5_check_orphans.cmake
            DEPENDS $<TARGET_FILE:${target}>
            VERBATIM COMMAND_EXPAND_LISTS
            COMMENT "RA6M5 [${target}]: checking for orphan sections")
        add_dependencies(ra6m5_orphan_check_${target} ${target})
    endfunction()

    ra6m5_add_orphan_check(tfm_s "")
    if(BL2)
        ra6m5_add_orphan_check(bl2 ".msp_stack_seal_res")
    endif()
endif()

#-------------------------------------------------------------------------------
# OFS brick guard (post-link)
#
# Two EK-RA6M4 boards were destroyed by an image whose option-setting words ld had coalesced
# into one PT_LOAD spanning 0x0100A100-0x0100A284, with the gaps zero-filled. A debugger
# flashes by program header, so it wrote 0x00000000 over eight defined option registers, BPS
# and PBPS among them. DESIGN.md 8.4, DECISIONS D133.
#
# The defence is in ra6m5_bl2.ld: thirteen discrete MEMORY regions, one per option word, each
# with its own '> REGION', giving one small PT_LOAD per word instead of a span. check_ofs.py
# asserts the outcome on the linked ELF, the only place the mistake is visible: it does not
# show in the srec, and a plausible-looking linker script can still emit a spanning segment.
#
# A hard failure, deliberately not switchable, unlike the orphan check: the failure mode is
# silent, instant and unrecoverable. Nothing this build produces is worth flashing if it trips.
#
# tfm_s is checked too and must report CLEAN: it carries no OFS. The guard is not wired into the
# NSPE build; tfm_ns has no option-setting sections and ns_app/ does not include this file.
#
# The script is shared with ra6m4 from renesas/common: the option-setting map is identical on
# both parts, and divergent copies of this check are a hazard in themselves.
#-------------------------------------------------------------------------------
# Not gated on the compiler. The guard defends against a linker-script mistake, so it matters
# most when a script is new and unproven, such as the .icf an IAR build uses. check_ofs.py
# reads program headers from the linked ELF, which both toolchains emit.
#
# find_package() is directory-scoped; bl2/ and spe-CMakeLists call it for their own
# directories, and this one has to as well. Not REQUIRED: a missing interpreter is reported by
# the guard's own failure below, not by aborting configure.
find_package(Python3 COMPONENTS Interpreter)

# CMAKE_READELF is set by CMake's GNU toolchain support and may be empty elsewhere. Fall back to
# find_program() for arm-none-eabi-readelf, then readelf. check_ofs.py fails loudly if none
# works, which is correct: a build that cannot run this check must not produce a flashable bl2.
set(RA6M5_OFS_READELF "${CMAKE_READELF}")
if(NOT RA6M5_OFS_READELF)
    # unset() first: the set() above created an empty normal variable, which shadows the cache
    # entry find_program() writes, so without it the fallback does nothing. This matters under
    # IAR, where the CMAKE_* tool variables are unset.
    unset(RA6M5_OFS_READELF)
    find_program(RA6M5_OFS_READELF NAMES arm-none-eabi-readelf readelf)
endif()



function(ra6m5_add_ofs_check target)
    # ARGN: extra guard flags, e.g. --require-segments for the image that carries the words.
    add_custom_target(ra6m5_ofs_check_${target} ALL
        COMMAND ${CMAKE_COMMAND} -E env READELF=${RA6M5_OFS_READELF}
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../common/check_ofs.py
                --ra6-default --max-word 0x10 --quarantine
                ${ARGN}
                $<TARGET_FILE:${target}>
        DEPENDS $<TARGET_FILE:${target}>
        VERBATIM
        COMMENT "RA6M5 [${target}]: OFS brick guard")
    add_dependencies(ra6m5_ofs_check_${target} ${target})
endfunction()

# WARNING: the flat .bin is a second way to brick the part, and the guard above cannot see it.
#
# The ELF can be correct (discrete per-word OFS PT_LOADs, the check above passing) and
# `objcopy -O binary` still yields an image that bricks the part. objcopy lays the image out
# from the lowest load address to the highest and zero-fills everything between, so a bl2 whose
# last option word sits at 0x0100A280 gives a ~16 MB .bin spanning 0x00000000-0x0100A284:
# 16,818,820 bytes of .bin for 26,912 bytes of real content (D058). The fill covers eight
# defined option registers and the reserved bytes between them, the write that destroyed two
# EK-RA6M4 boards (D133). Flashing such a .bin at 0 reproduces the brick from a build that
# passed.
#
# .hex and .srec are record-based and skip the gaps, so they are safe; only the flat binary
# coalesces. This target re-emits it without the option sections, and --check-flat-bin below
# verifies that the file on disk is what this target wrote.
#
# add_convert_to_bin_target() is generic upstream code shared by every platform, and
# add_custom_command(TARGET) works only on a target created in the same directory; bl2_bin is
# made in bl2/. So this is a separate target, sequenced after bl2_bin, that rewrites the .bin
# bl2_bin produced.
function(ra6m5_strip_ofs_from_bin target)
    if(NOT TARGET ${target}_bin)
        message(FATAL_ERROR
            "ra6m5_strip_ofs_from_bin(${target}): no ${target}_bin target. The flat binary "
            "would keep its zero fill across the option memory and nothing would catch it.")
    endif()
    get_target_property(_bindir ${target} RUNTIME_OUTPUT_DIRECTORY)
    add_custom_target(ra6m5_safe_bin_${target} ALL
        COMMAND ${CMAKE_COMMAND} -E env READELF=${RA6M5_OFS_READELF}
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../common/check_ofs.py
                --ra6-default --max-word 0x10
                --emit-safe-bin ${_bindir}/${target}.bin
                $<TARGET_FILE:${target}>
        VERBATIM
        COMMENT "RA6M5 [${target}]: re-emit flat .bin without the option sections")
    add_dependencies(ra6m5_safe_bin_${target} ${target}_bin)
endfunction()

ra6m5_add_ofs_check(tfm_s --forbid-segments)
if(BL2)
    # --require-segments: BL2 is the image that carries the option words, so an empty result
    # is a failure, not a pass. Without it, a dropped section or a wrong region window prints
    # as CLEAN.
    #
    # --check-flat-bin: verifies that the stripped binary above is what landed on disk. It
    # checks the file, so the guard is sequenced after the strip.
    ra6m5_strip_ofs_from_bin(bl2)
    ra6m5_add_ofs_check(bl2 --require-segments --check-flat-bin)
    add_dependencies(ra6m5_ofs_check_bl2 ra6m5_safe_bin_bl2)
endif()

#-------------------------------------------------------------------------------
# Non-secure application - optional in-tree build
#
# By default the NS image is built by ns_app/ in the split SPE/NSPE build, against the installed
# api_ns/ tree. RA6M5_NS_IN_SPE_BUILD instead defines tfm_ns inside the SPE build, as the ra6m4
# port did, so that one configure yields a complete, bootable image set (BL2's signing step
# references $<TARGET_FILE_DIR:tfm_ns>).
#
# That NS app is a full FSP application: it keeps FSP's startup and linker script
# (script/fsp.ld, which INCLUDEs the generated memory_regions.ld), so its vectors and RAM land
# in the non-secure partitions the solution defined. Nothing here hardcodes an address.
#-------------------------------------------------------------------------------
# RA6M5_NS_IN_SPE_BUILD - build the NS image here, in the secure build. OFF by default.
#
# PSA calls need the NSPE build: tfm_api_ns is created there from the installed api_ns/ tree
# and does not exist here, so linking it in this build yields "cannot find -ltfm_api_ns". This
# path gives an NS image that makes no secure calls, which is useful for bisecting a boot
# problem down to "does the S->NS jump still work" without the install step. It cannot run the
# service tests.
#
# Do not turn it on alongside the NSPE build: that gives two tfm_ns images from two build trees
# that can silently disagree, and the SPE one is not signed into anything the NSPE build
# produces.
#-------------------------------------------------------------------------------
set(RA6M5_NS_IN_SPE_BUILD OFF CACHE BOOL
    "Build the NS image in the secure build instead of the NSPE build (no PSA calls)")

# Keyed off FSP_NS_APP_DIR, not NS: in the v2.2 SPE build NS is not set. BL2's NS signing
# step is guarded on the tfm_ns target existing, so it follows this switch.
if(FSP_NS_APP_DIR AND RA6M5_NS_IN_SPE_BUILD)
    fsp_validate_project("${FSP_NS_APP_DIR}" "non-secure")
    fsp_collect_sources(FSP_NS_SOURCES "${FSP_NS_APP_DIR}" "NS")
    fsp_collect_includes(FSP_NS_INCLUDES "${FSP_NS_APP_DIR}")

    # ns/syscalls_ns.c supplies the newlib stubs; see its install rule below.
    add_executable(tfm_ns
        ${FSP_NS_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/ns/syscalls_ns.c
    )

    target_include_directories(tfm_ns PRIVATE
        ${FSP_NS_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated/interface/include
        ${CMAKE_SOURCE_DIR}/interface/include
    )
    # BSP_TZ_NONSECURE_BUILD is not defined here - bsp_mcu_family_cfg.h derives it from
    # _RA_TZ_NONSECURE and defines it as `(1)`. See the longer note in ns/CMakeLists.txt.
    target_compile_definitions(tfm_ns PRIVATE
        ${FSP_COMPILE_DEFS}
        _RA_TZ_NONSECURE=1
    )
    target_compile_options(tfm_ns PRIVATE ${FSP_COMPILE_OPTIONS})

    # No tfm_api_ns here; see RA6M5_NS_IN_SPE_BUILD above.

    if(RA6M5_STDOUT_RTT)
        # rtt_ns_init.c makes the NS image visible at all: without a reference into
        # SEGGER_RTT.c, --gc-sections drops the control block and the NS side is silent whether
        # it ran or hung.
        #
        # Not rtt_stdout.c: it defines _write(), and so does ns/syscalls_ns.c, which tfm_ns
        # always links, so adding it fails with a duplicate symbol. NS printf() goes through
        # syscalls_ns.c's own _write(), guarded on the RA6M5_STDOUT_RTT definition below. The
        # secure side keeps rtt_stdout.c because TF-M's SPM logging also needs its stdio_* API.
        target_sources(tfm_ns PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_ns_init.c)
        target_include_directories(tfm_ns PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt
            ${CMAKE_CURRENT_SOURCE_DIR}/ns)
        target_compile_definitions(tfm_ns PRIVATE RA6M5_STDOUT_RTT)
        if(RA6M5_RTT_BLOCKING)
            target_compile_definitions(tfm_ns PRIVATE SEGGER_RTT_MODE_DEFAULT=2)
        endif()
        # Redirects main()'s call to hal_entry() into __wrap_hal_entry() in rtt_ns_init.c, which
        # emits the NS banner and then calls __real_hal_entry(). The flag and that file must
        # move together: the flag without the definition is an undefined-symbol error, the
        # definition without the flag is dead code. A constructor cannot be used instead; this
        # image has no .init_array.
        target_link_options(tfm_ns PRIVATE -Wl,--wrap=hal_entry)
    endif()

    # Match tfm_s and bl2: .axf in bin/. add_convert_to_bin_target() reads
    # RUNTIME_OUTPUT_DIRECTORY; unset, the objcopy rule writes to a literal "bin_dir-NOTFOUND"
    # directory while BL2's signing step looks for tfm_ns.bin next to the executable, and
    # signing fails with "Input file not found".
    set_target_properties(tfm_ns PROPERTIES
        SUFFIX ".axf"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    # FSP's own linker script - it INCLUDEs Debug/memory_regions.ld, so the NS image
    # follows a repartitioned solution with no edit here.
    target_add_scatter_file(tfm_ns ${FSP_NS_APP_DIR}/script/fsp.ld)
    target_link_options(tfm_ns PRIVATE -L${FSP_NS_APP_DIR}/Debug -L${FSP_NS_APP_DIR})
    add_convert_to_bin_target(tfm_ns)
endif()

#-------------------------------------------------------------------------------
# Install for the NS build (split SPE/NSPE model)
#-------------------------------------------------------------------------------
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/ns/cpuarch_ns.cmake
                    ${CMAKE_CURRENT_SOURCE_DIR}/ns/CMakeLists.txt
                    ${CMAKE_CURRENT_SOURCE_DIR}/cpuarch.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# Port-specific config the NSPE build must not re-decide. TF-M's own spe_config.cmake is
# generated from an upstream template and has no room for a platform's settings.
# CMAKE_C_COMPILER is a full path here; the NS toolchain file wants the prefix without the
# tool name, the same shape as a CROSS_COMPILE setting.
string(REGEX REPLACE "-gcc(\.exe)?$" "" RA6M5_SPE_CROSS_COMPILE "${CMAKE_C_COMPILER}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ns/ra6m5_ns_config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/ra6m5_ns_config.cmake @ONLY)
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/ra6m5_ns_config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# flash_layout.h first includes "bsp_partitions.h", which this build generates from the
# solution. Without it the NS side cannot compile either layout header.
install(FILES       ${CMAKE_BINARY_DIR}/platform/target/generated/bsp_partitions.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# Install the test-capability configs. tf-m-tests reads them from the installed tree:
# tests_reg/CMakeLists.txt:38 includes ${CONFIG_SPE_PATH}/platform/tests/tfm_tests_config.cmake,
# and tests_psa_arch/CMakeLists.txt:29 the psa_arch one, both OPTIONAL. Without this rule the
# OPTIONAL include silently finds nothing. DECISIONS D062.
install(DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/tests
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# ns/syscalls_ns.c is compiled into platform_ns by the installed platform/CMakeLists.txt. It
# supplies the newlib _close/_lseek/_read/_write stubs; nosys.specs' versions warn at link, and
# NS links with -Wl,-fatal-warnings. Its _write() is also the NS printf backend when
# RA6M5_STDOUT_RTT is on, hence the RTT sources beside it.
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/ns/syscalls_ns.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/ns/tfm_service_tests.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/ns/tfm_service_tests.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/ns)

install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT_Conf.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_ns_init.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/rtt)

# uart_stdout.h declares only the stdio_* API (init, uninit, output_string, is_initialized,
# is_initialized_reset), no UART. TF-M installs it only when PLATFORM_DEFAULT_UART_STDOUT is on,
# which this port turns off because it logs over RTT, but tf-m-tests' NS log library includes
# it unconditionally. rtt/rtt_stdout.c implements all five functions, so exporting the header is
# enough: it names the contract, not the transport.
if(NOT PLATFORM_DEFAULT_UART_STDOUT)
    install(FILES       ${PLATFORM_DIR}/ext/common/uart_stdout.h
            DESTINATION ${INSTALL_PLATFORM_NS_DIR}/ext/common)
endif()

# The NS application project itself. Not part of platform_ns - it is the app, not the
# platform - but shipped so the NSPE build has something to point CONFIG_SPE_PATH at.
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/ns_app/CMakeLists.txt
                    ${CMAKE_CURRENT_SOURCE_DIR}/ns_app/README.md
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/ns_app)
