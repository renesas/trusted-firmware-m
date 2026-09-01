#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# RA6E1 platform. Consumes the e2 studio SOLUTION project set:
#
#   cmake -DTFM_PLATFORM=renesas/ra6e1 \
#         -DFSP_BL2_APP_DIR=<repo>/fsp_cmake/ra6e1_mcuboot   \
#         -DFSP_S_APP_DIR=<repo>/fsp_cmake/ra6e1_secure      \
#         -DFSP_NS_APP_DIR=<repo>/fsp_cmake/ra6e1_nonsecure  ...
#
# DESIGN NOTE - why this globs instead of including a per-project cmake file:
# the ra6m4 port required each RASC project to carry a hand-written
# tfm_integration/CMakeLists_tfm.cmake with explicit source lists. e2-generated
# solution projects have no cmake scaffolding at all, and hand-maintained lists
# drift from the generated tree (that is exactly what broke the ra6m4 external BL2
# path: it named files that FSP had moved). So all selection lives HERE and works
# by directory glob, the same way RASC's own GeneratedSrc.cmake does.
#
# Consequence, and the point of the exercise: a project regenerated in e2 after
# adding or removing modules needs NO hand-edited files. Re-run cmake and the new
# driver sources are picked up. Flash layout changes flow through the same way via
# bsp_linker_info.h (see flash_layout.h).
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

set(FSP_BL2_APP_DIR "" CACHE PATH "e2 solution bootloader project (ra6e1_mcuboot)")
set(FSP_S_APP_DIR   "" CACHE PATH "e2 solution secure project (ra6e1_secure)")
set(FSP_NS_APP_DIR  "" CACHE PATH "e2 solution non-secure project (ra6e1_nonsecure)")

#-------------------------------------------------------------------------------
# Validation + source selection helpers
#-------------------------------------------------------------------------------

# The generated layout (bsp_linker_info.h, memory_regions.ld, fsp_gen.ld) is emitted
# into <project>/Debug/ by the e2 build, and is deliberately NOT committed. So the
# solution must have been built in e2 at least once. Fail loudly rather than let the
# user hit a confusing "bsp_linker_info.h: No such file" deep in the compile.
function(fsp_validate_project dir role)
    if(NOT EXISTS "${dir}")
        message(FATAL_ERROR "RA6E1: FSP ${role} project not found: ${dir}")
    endif()
    foreach(sub ra ra_gen ra_cfg)
        if(NOT EXISTS "${dir}/${sub}")
            message(FATAL_ERROR
                "RA6E1: ${dir} does not look like a generated FSP project "
                "(missing ${sub}/). Generate it in e2 studio first.")
        endif()
    endforeach()
    if(NOT EXISTS "${dir}/Debug/bsp_linker_info.h")
        message(FATAL_ERROR
            "RA6E1: ${dir}/Debug/bsp_linker_info.h is missing.\n"
            "The solution carries the flash layout and emits it during the e2 build. "
            "Build the ${role} project in e2 studio once, then re-run cmake.")
    endif()
endfunction()

# Glob the FSP sources of a generated project.
#
# NON-SECURE ONLY. The S and BL2 images are built from the per-module libraries declared
# in the registry below; this whole-tree glob remains because the NS image is a different
# animal - a complete FSP application that keeps FSP's own main, startup.c, bsp_linker.c
# and linker script, and links no TF-M platform library. Do not reach for this when adding
# a driver to the secure image or BL2; add a module instead.
#
#   out_var : list of .c files
#   dir     : project root
#   role    : NS (the S and BL2 branches are kept only so the exclusions stay documented)
function(fsp_collect_sources out_var dir role)
    file(GLOB_RECURSE _src
        "${dir}/ra/fsp/src/bsp/*.c"
        "${dir}/ra/fsp/src/r_*/*.c"
        "${dir}/ra/fsp/src/rm_*/*.c"
        "${dir}/ra_gen/*.c"
    )
    if(role STREQUAL "NS")
        # The NS project IS the application - take its src/ too.
        file(GLOB_RECURSE _app "${dir}/src/*.c")
        list(APPEND _src ${_app})
        file(GLOB_RECURSE _rtos "${dir}/ra/aws/*.c" "${dir}/ra/arm/*.c")
        list(APPEND _src ${_rtos})
    endif()

    # Whole FSP modules TF-M replaces with its own implementation. These are enabled
    # in the e2 projects because the STANDALONE bootloader needs them, and that project
    # set must keep working standalone - so drop them here rather than reconfiguring it.
    #   r_sce, rm_psa_crypto, ra/arm/mbedtls - FSP's SCE9 hardware crypto and its
    #       mbedTLS. TF-M uses its own mbedcrypto with CRYPTO_HW_ACCELERATOR OFF; FSP's
    #       mbedTLS is entangled with the FSP MCUboot config and was never grafted
    #       (DESIGN.md 6). Re-enabling the hardware path is the future HW-crypto switch.
    #   rm_mcuboot_port - FSP's flash_map/MCUboot glue. It carries FSP's whole flash
    #       identity (its own FLASH_AREA_*_ID, single-image assumptions) out of
    #       bsp_linker_info.h and conflicts with TF-M's dual-image flash_map. Grafting
    #       it is what failed on ra6m4 (DESIGN.md 4, 5); TF-M's own Driver_Flash.c
    #       carries the RA 32 KB region-1 geometry instead.
    # Their HEADERS stay on the include path - the generated ra_gen/common_data.h and
    # hal_data.h include them unconditionally. Nothing is linked from them.
    set(FSP_EXCLUDED_MODULES r_sce rm_psa_crypto rm_mcuboot_port)
    foreach(m ${FSP_EXCLUDED_MODULES})
        list(FILTER _src EXCLUDE REGEX "/ra/fsp/src/${m}/")
    endforeach()
    list(FILTER _src EXCLUDE REGEX "/ra/arm/mbedtls/")

    # Individual files TF-M provides itself for S and BL2; taking FSP's would collide at
    # link time. The NS image is a whole FSP application built here, with no TF-M
    # equivalent to take their place, so it keeps all three:
    #   ra_gen/main.c    - TF-M has its own main for the S and BL2 images
    #   .../Source/startup.c - FSP's Reset_Handler, which FSP's linker script expects
    #   bsp_linker.c     - emits the OFS option-setting sections, and provides the real
    #                      g_init_info for C-runtime init. TF-M emits OFS from
    #                      bl2_option_setting.c, BL2 only, with discrete MEMORY regions.
    #                      Linking FSP's version into the secure image would put option
    #                      memory in a signed image; it is safe in the NS image because
    #                      BSP_TZ_NONSECURE_BUILD compiles those sections out.
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
        message(FATAL_ERROR "RA6E1: no FSP sources found under ${dir} - is it generated?")
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
set(FSP_COMPILE_OPTIONS -Wno-conversion -Wno-sign-conversion -Wno-unused-parameter)
set(FSP_COMPILE_DEFS _RA_CORE=CM33 _RENESAS_RA_)

# TrustZone build flavour, PER ROLE. The secure image declares itself secure to FSP; BL2
# is deliberately FLAT, matching how the e2 solution template defines the bootloader.
#
# This is not cosmetic - it selects the OFS1_SEL formula. Generated bsp_mcu_ofs_cfg.h:
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
# BL2 is the only image that emits OFS (DESIGN.md 8.4). Declaring it _RA_TZ_SECURE took
# the TrustZone branch, which reads BSP_CFG_CLOCKS_SECURE - set to (0) by the generated
# ra_gen/bsp_clock_cfg.h - and ORed in 0xF00, giving 0xFFFFFFF8: the clock-related OFS1
# fields marked NON-SECURE. On a part with the TZ boundaries programmed that attribution
# mismatch can lock out the debug interface. Flat takes the #else branch and emits
# 0xFFFFF8F8, byte-identical to the e2 bootloader.
#
# Do NOT reach for -DBSP_CFG_CLOCKS_SECURE=1 instead: bsp_clock_cfg.h defines it unguarded,
# so the header wins and all you get is a redefinition warning.
#
# What BL2 gives up by being flat (BSP_TZ_SECURE_BUILD 1 -> 0; BSP_TZ_NONSECURE_BUILD is 0
# either way, so the OFS sections still emit):
#   R_BSP_NonSecureEnter()  - not compiled. BL2 never calls it; MCUboot chainloads the
#                             secure image and TF-M performs the S->NS jump itself.
#   r_flash_hp callbacks    - no cmse_nsfptr_create wrapping. BL2's flash callback is
#                             never a non-secure function pointer.
#   r_ioport.c:857          - inactive either way, BSP_FEATURE_TZ_NS_OFFSET is 0 here.
#   bsp_sdram NSC entries   - already off, they need BSP_CFG_CLOCKS_SECURE == 1.
#
# Verify: arm-none-eabi-objdump -s -j .option_setting_ofs1_sel bin/bl2.axf -> f8f8ffff
set(FSP_TZ_DEFS_S   _RA_TZ_SECURE=1)
set(FSP_TZ_DEFS_BL2 )

#-------------------------------------------------------------------------------
# FSP module registry
#
# The S and BL2 images are built from per-module libraries, not from one glob of the
# project tree. A module is OPT-IN: it enters the build only by appearing below and having
# a cmake/modules/fsp_<name>.cmake. That is the point - the e2 projects enable modules the
# STANDALONE bootloader needs (SCE, PSA crypto, the MCUboot port) that must not be dragged
# into a signed TF-M image just because they are present.
#
# The NS image is not built this way: it is a whole FSP application, keeps FSP's own
# startup and linker script, and is still collected by fsp_collect_sources() below.
#-------------------------------------------------------------------------------
set(FSP_MODULES_S    bsp flash sce)   # sce: SCE9 TRNG backs the PSA external RNG
set(FSP_MODULES_BL2  bsp flash)       # BL2 needs no randomness - see fsp_sce.cmake

# Which ra/fsp/src/<dir> each module accounts for, so an unclaimed module in the project
# can be reported rather than silently dropped.
set(FSP_MODULE_DIRS_bsp    bsp r_ioport)
set(FSP_MODULE_DIRS_flash  r_flash_hp)
set(FSP_MODULE_DIRS_sce    r_sce)

# Present in the e2 projects, deliberately never built: TF-M supplies its own. Their
# HEADERS still resolve (see fsp_bsp.cmake) because the generated ra_gen files include
# them unconditionally.
#   r_sce, rm_psa_crypto, ra/arm/mbedtls - FSP's SCE9 cipher stack and its mbedTLS.
#       TF-M uses its own mbedcrypto; FSP's config wires PSA ITS to littlefs and fights
#       TFM_PARTITION_INTERNAL_TRUSTED_STORAGE (DESIGN.md 6). Only the TRNG is taken, via
#       the sce module.
#   rm_mcuboot_port - FSP's flash_map/MCUboot glue, carrying FSP's own FLASH_AREA_*_ID and
#       single-image assumptions out of bsp_linker_info.h. Conflicts with TF-M's dual-image
#       flash_map; grafting it is what failed on ra6m4 (DESIGN.md 4, 5).
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

    # Anything in the project that no module accounts for is NOT in the image. Say so -
    # a silently missing driver surfaces much later as a confusing link error.
    file(GLOB _present RELATIVE "${dir}/ra/fsp/src" "${dir}/ra/fsp/src/*")
    foreach(d ${_present})
        if(IS_DIRECTORY "${dir}/ra/fsp/src/${d}")
            if((NOT d IN_LIST _claimed) AND (NOT d IN_LIST FSP_MODULES_NEVER_BUILT))
                message(WARNING
                    "RA6E1 [${role}]: FSP module '${d}' is in the project but no module "
                    "declares it, so it is NOT in the image. Add a "
                    "cmake/modules/fsp_<name>.cmake and list it in FSP_MODULES_${_role_uc} "
                    "- see TFM_INTEGRATION_COMPLETE.md, \"Adding New FSP Modules\".")
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
# flash_layout.h and region_defs.h need the BSP_PARTITION_* values, and they are
# consumed by C code AND by ra6e1_bl2.ld, which the toolchain preprocesses with -xc.
# bsp_linker_info.h cannot serve the second: alongside the macros it declares C types
# and externs (typedef enum e_bsp_init_mem, gp_ddsc_*), which land verbatim in the
# preprocessed linker script and fail to parse. It also carries FSP's own MCUboot flash
# identity, which conflicts with TF-M's dual-image flash_map (DESIGN.md 4).
#
# So filter the file down to its #define BSP_PARTITION_* lines at configure time. Still
# fully derived - CMAKE_CONFIGURE_DEPENDS re-runs cmake when the solution regenerates it,
# so repartitioning in e2 and rebuilding is all it takes.
#
# The secure project's copy is the source: the three agree on the shared partitions, and
# it is a strict superset - it also carries the RAM_BL / DATA_FLASH_BL / QSPI_FLASH_BL
# entries the bootloader's own copy omits.
#-------------------------------------------------------------------------------
set(FSP_PARTITION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(FSP_PARTITION_HEADER "${FSP_PARTITION_HEADER_DIR}/bsp_partitions.h")
set(FSP_LINKER_INFO_SRC "${FSP_S_APP_DIR}/Debug/bsp_linker_info.h")

file(STRINGS "${FSP_LINKER_INFO_SRC}" _partition_defs
     REGEX "^[ \t]*#define[ \t]+BSP_PARTITION_[A-Za-z0-9_]+[ \t]")
if(NOT _partition_defs)
    message(FATAL_ERROR
        "RA6E1: no BSP_PARTITION_* defines in ${FSP_LINKER_INFO_SRC}. "
        "The solution emits them during the e2 build - rebuild the secure project there.")
endif()
list(JOIN _partition_defs "\n" _partition_body)
file(CONFIGURE OUTPUT "${FSP_PARTITION_HEADER}" CONTENT
"/* GENERATED - do not edit. Extracted by the ra6e1 platform CMakeLists from
 * ${FSP_LINKER_INFO_SRC}
 * Macros only: this header is preprocessed into ra6e1_bl2.ld as well as compiled.
 */
#ifndef __BSP_PARTITIONS_H__
#define __BSP_PARTITIONS_H__

${_partition_body}

#endif /* __BSP_PARTITIONS_H__ */
")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FSP_LINKER_INFO_SRC}")

# platform_region_defs is what the scatter targets link, so these are the paths that make
# region_defs.h and its includes resolvable when a linker script is preprocessed.
#
# The platform's own directory is needed for the SECURE script specifically: TF-M generates
# tfm_isolation_s.ld into ${CMAKE_BINARY_DIR}/generated/, so its #include "region_defs.h"
# has no relative path to find. ra6e1_bl2.ld happens to work without it, being in this
# directory itself. Same approach as the rse/tc and stm32wba65i-dk platforms.
target_include_directories(platform_region_defs
    INTERFACE
        "${FSP_PARTITION_HEADER_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

# FSP module libraries for the secure image.
fsp_add_modules(s "${FSP_S_APP_DIR}" "${FSP_S_INCLUDES}" FSP_S_MODULES)
target_link_libraries(platform_s PUBLIC ${FSP_S_MODULES})

target_sources(platform_s PRIVATE
    # PSA external RNG over the SCE9 TRNG - the entropy source for the whole secure image
    ${CMAKE_CURRENT_SOURCE_DIR}/sce_trng.c
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_peripherals_def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    # Static assertions only, no code. Catches an OTP area too small for the backend's
    # region struct, which otherwise folds init_otp_nv_counters_flash() into an
    # unconditional error return with no diagnostic. See the file for the detail.
    ${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_layout_checks.c
    # bsp_init_stub.c is NOT here - it lives in the fsp_bsp module, beside the FSP
    # system.c that needs it. See the note in cmake/modules/fsp_bsp.cmake.
    # Common ARMv8-M isolation HAL: tfm_hal_set_up_static_boundaries,
    # tfm_hal_bind_boundary, tfm_hal_memory_check. Nothing else provides them, and the
    # port has no reason to fork it - attribution here is IDAU/SAU, which this handles.
    ${PLATFORM_DIR}/ext/common/tfm_hal_isolation_v8m.c
)
target_include_directories(platform_s PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    ${FSP_S_INCLUDES}
)
# Scoped to the one file rather than added to platform_s: the template directory holds
# TF-M's default platform implementations, and putting it on the whole target's include
# path invites a header there shadowing one of ours.
#
# TARGET_DIRECTORY is required. Source file properties are directory scoped, and
# platform_s is defined in platform/CMakeLists.txt, not here - without it the property is
# set in this directory's scope, where nothing consumes it, and the include is silently
# never applied.
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_layout_checks.c
    TARGET_DIRECTORY platform_s
    PROPERTIES INCLUDE_DIRECTORIES "${PLATFORM_DIR}/ext/common/template")

# attest_hal.c needs tfm_strnlen.h. The port sets PLATFORM_DEFAULT_ATTEST_HAL OFF, so
# platform_s does not inherit tfm_sprt's include paths the way the default HAL does.
#
# ext/common: tfm_hal_platform.c needs uart_stdout.h for the stdio_* backend API. PRIVATE,
# and NOT ext/common/template - that directory holds TF-M's default platform
# implementations and would shadow ours. Matches what the ra6m4 port does.
target_include_directories(platform_s PRIVATE
    ${CMAKE_SOURCE_DIR}/secure_fw/partitions/lib/runtime/include
    ${PLATFORM_DIR}/ext/common
)
target_compile_definitions(platform_s PUBLIC ${FSP_COMPILE_DEFS} ${FSP_TZ_DEFS_S})
target_compile_options(platform_s PRIVATE ${FSP_COMPILE_OPTIONS})

# -mcmse, PUBLIC. It must reach every target that links platform_s, not just the FSP
# sources: tfm_s compiles secure_fw/partitions/ns_agent_tz/psa_api_veneers_v80m.c, and
# without -mcmse the compiler does not define __ARM_FEATURE_CMSE, so security_defs.h
# compiles out __tz_naked_veneer and the veneers fail to parse.
target_compile_options(platform_s PUBLIC ${COMPILER_CMSE_FLAG})

target_sources(tfm_s PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c)

# TF-M generates the secure linker script from its template using our region_defs.h
# (that is how TFM_LINKER_VENEERS_START pins the NSC window - DESIGN.md 7). It does not
# attach it to tfm_s for us: without this the link has no -T at all and every script
# symbol is undefined (Image$$ARM_LIB_STACK$$ZI$$Limit, __copy_table_start__,
# Image$$TFM_SP_LOAD_LIST$$RO$$Base, ...). No custom secure linker - only the generated one.
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:GNU>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/gcc/tfm_isolation_s.ld>
    $<$<C_COMPILER_ID:ARMClang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/armclang/tfm_isolation_s.sct>
    $<$<C_COMPILER_ID:IAR>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/iar/tfm_isolation_s.icf>
    $<$<C_COMPILER_ID:Clang>:${CMAKE_BINARY_DIR}/generated/platform/ext/common/llvm/tfm_isolation_s.ld>
)

# Point FSP's g_main_stack at the image's real stack. FSP's SystemInit() sets MSPLIM from
# &g_main_stack[0] and writes its stack seal at &g_main_stack[BSP_CFG_STACK_MAIN_BYTES],
# both of which describe the stack the CPU is actually running on - and that stack belongs
# to TF-M, not to FSP. See the long note in bsp_init_stub.c: a private array there put
# MSPLIM 44 KB above the live SP and hard-faulted the first push inside SystemInit().
#
# Done with --defsym rather than in C: the target symbol is defined by the linker script,
# so __attribute__((alias)) is rejected and an assembler .set leaves it undefined.
# NOTE the SINGLE '$' in each separator. The real symbol is Image$$ARM_LIB_STACK$$ZI$$Base
# with doubled dollars, but CMake escapes '$' twice on the way into build.ninja, so one
# here arrives as two on the link line. Verify after any change with:
#   grep -o 'defsym=g_main_stack=[^ "]*' build_ra6e1/build.ninja
# which should show four '$' per separator (ninja halves them), and then confirm with
#   arm-none-eabi-nm bin/tfm_s.axf | grep g_main_stack
# that the symbol resolved to Image$$ARM_LIB_STACK$$ZI$$Base rather than being absent.
set(RA6E1_MAIN_STACK_DEFSYM "-Wl,--defsym=g_main_stack=Image$ARM_LIB_STACK$ZI$Base")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_options(tfm_s PRIVATE ${RA6E1_MAIN_STACK_DEFSYM})
endif()

if(RA6E1_STDOUT_RTT)
    target_sources(platform_s PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
        ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c)
    target_include_directories(platform_s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rtt)
else()
    target_sources(platform_s PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c)
endif()

#-------------------------------------------------------------------------------
# BL2 (MCUboot)
#-------------------------------------------------------------------------------
if(BL2)
    fsp_validate_project("${FSP_BL2_APP_DIR}" "bootloader")
    fsp_collect_includes(FSP_BL2_INCLUDES "${FSP_BL2_APP_DIR}")

    # FSP module libraries for the bootloader. Its e2 project also contains r_sce,
    # rm_psa_crypto and rm_mcuboot_port for the standalone build; none of them are
    # declared here, so none reach the image.
    fsp_add_modules(bl2 "${FSP_BL2_APP_DIR}" "${FSP_BL2_INCLUDES}" FSP_BL2_MODULES)
    target_link_libraries(platform_bl2 PUBLIC ${FSP_BL2_MODULES})

    # bsp_init_stub.c is NOT here - it lives in the fsp_bsp_bl2 module, beside the FSP
    # system.c that needs it. See the note in cmake/modules/fsp_bsp.cmake.
    target_sources(platform_bl2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    )
    target_include_directories(platform_bl2 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
        ${FSP_BL2_INCLUDES}
    )
    # No _RA_TZ_ macro: BL2 is a flat build, like the e2 solution's bootloader.
    target_compile_definitions(platform_bl2 PUBLIC ${FSP_COMPILE_DEFS} ${FSP_TZ_DEFS_BL2})
    target_compile_options(platform_bl2 PRIVATE ${FSP_COMPILE_OPTIONS})
    target_compile_options(platform_bl2 PUBLIC ${COMPILER_CMSE_FLAG})

    # OFS goes in BL2 only, and ONLY with the discrete per-group MEMORY regions in
    # ra6e1_bl2.ld. Emitting it as one coalesced PT_LOAD zero-fills the FCU config
    # including FSPR and permanently bricks the part - it cost two EK-RA6M4 boards.
    # See DESIGN.md 8.4 and verify with: arm-none-eabi-readelf -l bin/bl2.axf
    target_sources(bl2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
    )

    # MCUboot's bl2_main.c calls stdio_init/stdio_uninit, so BL2 needs a stdout backend
    # of its own - it is a separate binary from the secure image, with its own RTT
    # control block. config.cmake forces PLATFORM_DEFAULT_UART_STDOUT off when RTT is on,
    # so nothing else provides them.
    if(RA6E1_STDOUT_RTT)
        target_sources(platform_bl2 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_stdout.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c)
        target_include_directories(platform_bl2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rtt)
    else()
        target_sources(platform_bl2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_USART.c)
    endif()

    if(RA6E1_BL2_HALT_AT_MAIN)
        message(WARNING "RA6E1_BL2_HALT_AT_MAIN=ON - BL2 spins at main(); not bootable")
        target_compile_definitions(bl2 PRIVATE BL2_HALT_AT_MAIN=1)
    endif()

    target_add_scatter_file(bl2
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_bl2.ld>
        $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
        $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
    )

    # Same g_main_stack aliasing as the secure image - see the note above. BL2 is flat, so
    # BSP_TZ_SECURE_BUILD is 0 and it skips the seal write; only MSPLIM applies. It did not
    # fault before only because the private array happened to land below BL2's stack.
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_options(bl2 PRIVATE ${RA6E1_MAIN_STACK_DEFSYM})
        # ra6e1_bl2.ld relocates FSP's .ram_from_flash (the code-flash program/erase
        # routines) into the RAM region, which is executable code inside an otherwise
        # read/write segment - so ld reports "LOAD segment with RWX permissions". That is
        # exactly what code-in-RAM is, and TF-M links with -Wl,-fatal-warnings, so without
        # this the link fails. Scoped to bl2: it is the only image that programs code flash,
        # and the only one with a relocated text section. FSP's own linker scripts produce
        # the same RWX segment for the same reason.
        target_link_options(bl2 PRIVATE -Wl,--no-warn-rwx-segments)
    endif()
endif()

#-------------------------------------------------------------------------------
# Orphan-section check (post-link, GNU only)
#
# The images built here do not use FSP's linker script, so nothing carries FSP's memory
# section contract across - see cmake/ra6e1_check_orphans.cmake for the full reasoning and
# for why this is a post-link ELF check rather than a diff against fsp_gen.ld.
#
# $<TARGET_OBJECTS:<target>_scatter> is the PREPROCESSED script. target_add_scatter_file()
# in toolchain_GNUARM.cmake compiles the .ld with -E -P -xc into that object library and
# passes it to the linker as -T, so this is the same text ld saw - macros expanded, and
# with the #if branches that actually applied. Reading the source .ld instead would give
# the wrong answer for bl2, whose script has both a CMSE and a non-CMSE arm.
#
# Known-benign orphans are listed per image rather than silenced globally, so a NEW one is
# still reported. Each entry needs a reason.
#
#   tfm_s had .ram_noinit here until 2026-08-30. It is now handled properly, via
#       S_DATA_EXTRA_NOINIT_SECTION_NAME in region_defs.h -> .TFM_NOINIT. Deliberately NOT
#       allowlisted any more: if that placement ever regresses, this check should say so.
#
#   bl2 .msp_stack_seal_res - bl2 is a flat build, so __ARM_FEATURE_CMSE != 3 and
#       ra6e1_bl2.ld deliberately takes the non-CMSE arm, which has no seal section.
#       startup_ra6e1.c still emits the 8-byte __StackSeal, which then orphans away from
#       the stack top. Inert: FSP writes the seal only when BSP_TZ_SECURE_BUILD, and TF-M's
#       startup only under CMSE. Both are false here.
#
# Warning, not error, by default: a TF-M or FSP update can introduce a new section that is
# perfectly benign, and that should not block a build. Set RA6E1_ORPHAN_CHECK_STRICT for CI.
#-------------------------------------------------------------------------------
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(RA6E1_ORPHAN_CHECK_STRICT OFF CACHE BOOL
        "Fail the build on an unexpected orphan section instead of warning")

    # A custom TARGET, not add_custom_command(TARGET ... POST_BUILD): that form is
    # directory-scoped, and tfm_s and bl2 are created in secure_fw/ and bl2/, not here.
    function(ra6e1_add_orphan_check target allow)
        add_custom_target(ra6e1_orphan_check_${target} ALL
            COMMAND ${CMAKE_COMMAND}
                    -DIMAGE=${target}
                    -DAXF=$<TARGET_FILE:${target}>
                    -DSCATTER=$<TARGET_OBJECTS:${target}_scatter>
                    -DOBJDUMP=${CMAKE_OBJDUMP}
                    -DALLOW=${allow}
                    -DSTRICT=${RA6E1_ORPHAN_CHECK_STRICT}
                    -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/cmake/ra6e1_check_orphans.cmake
            DEPENDS $<TARGET_FILE:${target}>
            VERBATIM COMMAND_EXPAND_LISTS
            COMMENT "RA6E1 [${target}]: checking for orphan sections")
        add_dependencies(ra6e1_orphan_check_${target} ${target})
    endfunction()

    ra6e1_add_orphan_check(tfm_s "")
    if(BL2)
        ra6e1_add_orphan_check(bl2 ".msp_stack_seal_res")
    endif()
endif()

#-------------------------------------------------------------------------------
# Non-secure application - built in-tree
#
# TF-M v2.2's canonical model is a split SPE/NSPE build: the SPE installs api_ns/ and
# the NS app is configured separately against it. This port defines tfm_ns inside the
# SPE build instead, as the ra6m4 port did, because BL2's signing step references
# $<TARGET_FILE_DIR:tfm_ns> and this yields a complete, bootable image set from one
# configure. Moving to the split build is a follow-up, and is the prerequisite for
# running the official tf-m-tests regression suite.
#
# The NS app is a full FSP application: it keeps FSP's startup and linker script
# (script/fsp.ld, which INCLUDEs the generated memory_regions.ld), so its vectors and
# RAM land in the non-secure partitions the solution defined. Nothing here hardcodes
# an address.
#-------------------------------------------------------------------------------
# RA6E1_NS_IN_SPE_BUILD - build the NS image HERE, in the secure build.
#
# OFF by default since 2026-08-30, when the split SPE/NSPE build started working. The NS
# image is now built by ns_app/ against the installed api_ns/ tree, which is the only
# arrangement in which it can call PSA APIs: tfm_api_ns is created by the NSPE build from
# that tree and does not exist here, so linking it in this build yields
# "cannot find -ltfm_api_ns".
#
# Kept, rather than deleted, for the one thing it is still good for: producing a complete
# bootable image set from a single configure, with an NS image that makes no secure calls.
# That is useful for bisecting a boot problem down to "does the S->NS jump still work",
# without the install step in between. It cannot run the service tests.
#
# Turning it ON alongside the NSPE build gives two tfm_ns images from two build trees that
# can silently disagree; the SPE one is not signed into anything the NSPE build produces.
# Pick one.
#-------------------------------------------------------------------------------
set(RA6E1_NS_IN_SPE_BUILD OFF CACHE BOOL
    "Build the NS image in the secure build instead of the NSPE build (no PSA calls)")

# Keyed off FSP_NS_APP_DIR, not NS: in the v2.2 SPE build NS is not set. BL2's NS signing
# step is guarded on the tfm_ns target existing, so it follows this switch.
if(FSP_NS_APP_DIR AND RA6E1_NS_IN_SPE_BUILD)
    fsp_validate_project("${FSP_NS_APP_DIR}" "non-secure")
    fsp_collect_sources(FSP_NS_SOURCES "${FSP_NS_APP_DIR}" "NS")
    fsp_collect_includes(FSP_NS_INCLUDES "${FSP_NS_APP_DIR}")

    # ns/syscalls_ns.c supplies the newlib _close/_lseek/_read/_write stubs. Without it
    # nosys.specs' versions warn at link, and TF-M links NS with -Wl,-fatal-warnings.
    # It is also installed for the NSPE build, which needs it for the same reason.
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
    target_compile_definitions(tfm_ns PRIVATE
        ${FSP_COMPILE_DEFS}
        _RA_TZ_NONSECURE=1
        BSP_TZ_NONSECURE_BUILD=1
    )
    target_compile_options(tfm_ns PRIVATE ${FSP_COMPILE_OPTIONS})

    # No tfm_api_ns here. That target is created by the NSPE build from the installed
    # api_ns/ tree and does not exist in an SPE build, so linking it just yields
    # "cannot find -ltfm_api_ns". The solution's NS app is a plain FSP application that
    # makes no PSA calls - it exists to prove the S->NS jump - so it needs nothing from
    # the secure interface. The moment NS code calls a PSA API, this has to become a
    # real split SPE/NSPE build; see the note above.

    if(RA6E1_STDOUT_RTT)
        # rtt_ns_init.c is what makes the NS image visible at all - see the file. Without a
        # reference into SEGGER_RTT.c, --gc-sections drops the control block and the NS side
        # is silent whether it ran or hung.
        #
        # NOT rtt_stdout.c. That file defines _write(), and so does ns/syscalls_ns.c, which
        # tfm_ns always links - adding it here fails with a duplicate symbol. The NS side
        # gets its printf() redirection from syscalls_ns.c's own _write() instead, guarded on
        # the RA6E1_STDOUT_RTT definition below. The secure side keeps rtt_stdout.c because
        # it also needs the stdio_* backend API that TF-M's SPM logging calls.
        target_sources(tfm_ns PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt/rtt_ns_init.c)
        target_include_directories(tfm_ns PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/rtt
            ${CMAKE_CURRENT_SOURCE_DIR}/ns)
        target_compile_definitions(tfm_ns PRIVATE RA6E1_STDOUT_RTT)
        # Redirects main()'s call to hal_entry() into __wrap_hal_entry() in rtt_ns_init.c,
        # which emits the NS banner and then calls __real_hal_entry(). This pairs with that
        # file and the two must move together: the flag without the definition is an
        # undefined-symbol error, the definition without the flag is dead code. A
        # constructor cannot be used instead - this image has no .init_array. See the file.
        target_link_options(tfm_ns PRIVATE -Wl,--wrap=hal_entry)
    endif()

    # No PSA client here, and no service tests. Linking the interface in this build was a
    # spike, retired on 2026-08-30 when the split SPE/NSPE build started working: the NS
    # image now gets tfm_api_ns from the installed api_ns/ tree, which is the only place it
    # can come from. What remains of this path is a plain FSP application that makes no
    # secure calls - useful for answering "does the S->NS jump still work" without the
    # install step, and nothing more. See ns_app/.

    # Match tfm_s and bl2: .axf in bin/. Not cosmetic - add_convert_to_bin_target() reads
    # RUNTIME_OUTPUT_DIRECTORY, and with it unset the objcopy rule writes to a literal
    # "bin_dir-NOTFOUND" directory while BL2's signing step looks for tfm_ns.bin next to
    # the executable, so signing failed with "Input file not found".
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
string(REGEX REPLACE "-gcc(\.exe)?$" "" RA6E1_SPE_CROSS_COMPILE "${CMAKE_C_COMPILER}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ns/ra6e1_ns_config.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/ra6e1_ns_config.cmake @ONLY)
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/ra6e1_ns_config.cmake
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# flash_layout.h's first act is #include "bsp_partitions.h", which is GENERATED from the
# solution during this build. Without it the NS side cannot compile either layout header.
install(FILES       ${CMAKE_BINARY_DIR}/platform/target/generated/bsp_partitions.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})

# ns/syscalls_ns.c is compiled into platform_ns by the installed platform/CMakeLists.txt -
# it supplies the newlib _close/_lseek/_read/_write stubs, and NS links with
# -Wl,-fatal-warnings so nosys.specs' versions are not an option. Its _write() is also the
# NS printf backend when RA6E1_STDOUT_RTT is on, hence the RTT sources beside it.
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

# uart_stdout.h is only the stdio_init/stdio_output_string/stdio_uninit API - no UART in it.
# TF-M installs it only when PLATFORM_DEFAULT_UART_STDOUT is on, which this port turns OFF
# because it logs over RTT. But tf-m-tests' NS log library includes it unconditionally, so
# an RTT port cannot build the regression tests without it. rtt/rtt_stdout.c implements
# exactly these four functions, so exporting the header is enough - it names the contract,
# not the transport.
if(NOT PLATFORM_DEFAULT_UART_STDOUT)
    install(FILES       ${PLATFORM_DIR}/ext/common/uart_stdout.h
            DESTINATION ${INSTALL_PLATFORM_NS_DIR}/ext/common)
endif()

# The NS application project itself. Not part of platform_ns - it is the app, not the
# platform - but shipped so the NSPE build has something to point CONFIG_SPE_PATH at.
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/ns_app/CMakeLists.txt
                    ${CMAKE_CURRENT_SOURCE_DIR}/ns_app/README.md
        DESTINATION ${INSTALL_PLATFORM_NS_DIR}/ns_app)
