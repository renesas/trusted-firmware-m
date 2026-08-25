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
#   out_var : list of .c files
#   dir     : project root
#   role    : BL2 | S | NS  - controls what TF-M supplies itself and must exclude
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

    # TF-M provides these itself; taking FSP's would collide at link time.
    #   ra_gen/main.c    - TF-M has its own main for every image
    #   .../Source/startup.c, system.c handled below
    #   bsp_linker.c     - emits the OFS option-setting sections. TF-M emits them
    #                      from bl2_option_setting.c, BL2 only, with discrete
    #                      MEMORY regions. Linking FSP's version into the secure or
    #                      NS image would put option memory in a signed image.
    set(_excl "${dir}/ra_gen/main.c")
    if(NOT role STREQUAL "NS")
        # S and BL2 use TF-M's startup and TF-M's own OFS emitter. The NS image keeps
        # FSP's: startup.c gives the Reset_Handler its linker script expects, and
        # bsp_linker.c provides the real g_init_info for C-runtime init. bsp_linker.c
        # is safe there because its option-setting sections are compiled out by
        # BSP_TZ_NONSECURE_BUILD - the NS image must never carry option memory.
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

#-------------------------------------------------------------------------------
# Secure side
#-------------------------------------------------------------------------------
fsp_validate_project("${FSP_S_APP_DIR}" "secure")
fsp_collect_sources(FSP_S_SOURCES "${FSP_S_APP_DIR}" "S")
fsp_collect_includes(FSP_S_INCLUDES "${FSP_S_APP_DIR}")

target_sources(platform_s PRIVATE
    ${FSP_S_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tfm_peripherals_def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/attest_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp_init_stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
)
target_include_directories(platform_s PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
    ${FSP_S_INCLUDES}
)
target_compile_definitions(platform_s PUBLIC ${FSP_COMPILE_DEFS} _RA_TZ_SECURE=1)
target_compile_options(platform_s PRIVATE ${FSP_COMPILE_OPTIONS})

target_sources(tfm_s PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c)

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
    fsp_collect_sources(FSP_BL2_SOURCES "${FSP_BL2_APP_DIR}" "BL2")
    fsp_collect_includes(FSP_BL2_INCLUDES "${FSP_BL2_APP_DIR}")

    target_sources(platform_bl2 PRIVATE
        ${FSP_BL2_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/target_cfg.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tfm_hal_platform.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers/Driver_Flash.c
    )
    target_include_directories(platform_bl2 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_drivers
        ${FSP_BL2_INCLUDES}
    )
    target_compile_definitions(platform_bl2 PUBLIC ${FSP_COMPILE_DEFS} _RA_TZ_SECURE=1)
    target_compile_options(platform_bl2 PRIVATE ${FSP_COMPILE_OPTIONS})

    # OFS goes in BL2 only, and ONLY with the discrete per-group MEMORY regions in
    # ra6e1_bl2.ld. Emitting it as one coalesced PT_LOAD zero-fills the FCU config
    # including FSPR and permanently bricks the part - it cost two EK-RA6M4 boards.
    # See DESIGN.md 8.4 and verify with: arm-none-eabi-readelf -l bin/bl2.axf
    target_sources(bl2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ra6e1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bl2_option_setting.c
    )

    if(RA6E1_BL2_HALT_AT_MAIN)
        message(WARNING "RA6E1_BL2_HALT_AT_MAIN=ON - BL2 spins at main(); not bootable")
        target_compile_definitions(bl2 PRIVATE BL2_HALT_AT_MAIN=1)
    endif()

    target_add_scatter_file(bl2
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/ra6e1_bl2.ld>
        $<$<C_COMPILER_ID:ARMClang>:${PLATFORM_DIR}/ext/common/armclang/tfm_common_bl2.sct>
        $<$<C_COMPILER_ID:IAR>:${PLATFORM_DIR}/ext/common/iar/tfm_common_bl2.icf>
    )
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
# Keyed off FSP_NS_APP_DIR, not NS: in the v2.2 SPE build NS is not set, but BL2 still
# references $<TARGET_FILE_DIR:tfm_ns> for signing. The ra6m4 port used the same
# condition.
if(FSP_NS_APP_DIR)
    fsp_validate_project("${FSP_NS_APP_DIR}" "non-secure")
    fsp_collect_sources(FSP_NS_SOURCES "${FSP_NS_APP_DIR}" "NS")
    fsp_collect_includes(FSP_NS_INCLUDES "${FSP_NS_APP_DIR}")

    add_executable(tfm_ns ${FSP_NS_SOURCES})

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
    target_link_libraries(tfm_ns PRIVATE tfm_api_ns)

    if(RA6E1_STDOUT_RTT)
        target_sources(tfm_ns PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rtt/SEGGER_RTT.c)
        target_include_directories(tfm_ns PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rtt)
    endif()

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
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
install(FILES       ${CMAKE_CURRENT_SOURCE_DIR}/flash_layout.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/region_defs.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/device_cfg.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis.h
        DESTINATION ${INSTALL_PLATFORM_NS_DIR})
