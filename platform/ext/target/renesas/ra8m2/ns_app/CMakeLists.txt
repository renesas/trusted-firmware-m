#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA8M2 non-secure application - the NSPE half of the split SPE/NSPE build.
#
# This is a standalone CMake project, configured against an SPE build that has already been
# installed. The TF-M build does not add_subdirectory() it; the dependency runs the other way.
# See README.md for the two commands.
#
# How this port differs from a stock NSPE app
#
# The non-secure image is a whole FSP application. It keeps FSP's startup.c and FSP's own
# linker script (script/fsp.ld, which INCLUDEs the solution's generated memory_regions.ld), so
# its vectors, RAM and flash land in the partitions the e2 solution defined, and nothing here
# hardcodes an address.
#
# The BSP itself lives in platform_ns, not here, so that NSPE applications other than ours
# (tf-m-tests' regression app supplies only main() and knows nothing about FSP) still get a
# Reset_Handler and a vector table. This file is the application alone.
#
# FSP_NS_APP_DIR must be the non-secure project from the same solution the secure side was
# built against. The SPE records which one that was, and this file checks it rather than
# trusting the caller: a mismatched project links a valid image at the wrong address, and the
# first symptom is a board that resets in a loop.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

#-------------------------------------------------------------------------------
# Everything in this block must happen before project(), which fixes the toolchain; CMake
# does not allow it to change afterwards.
#-------------------------------------------------------------------------------
if(NOT DEFINED CONFIG_SPE_PATH OR NOT EXISTS ${CONFIG_SPE_PATH})
    message(FATAL_ERROR
        "CONFIG_SPE_PATH must point at an INSTALLED SPE tree (the api_ns directory), not "
        "at a TF-M build directory. Build the SPE, then 'cmake --install <spe-build-dir>'. "
        "See ns_app/README.md.")
endif()

# Read the port settings the SPE exported. It contains only set() calls, so it is safe this
# early, and it supplies the compiler prefix.
include(${CONFIG_SPE_PATH}/platform/ra8m2_ns_config.cmake)

# toolchain_ns_GNUARM.cmake finds the compiler with find_program(${CROSS_COMPILE}-gcc), which
# assumes it is on PATH. It usually is not (the toolchain ships inside e2 studio), so default
# to the exact prefix the SPE used. An explicit -DCROSS_COMPILE still wins.
if(NOT DEFINED CROSS_COMPILE AND RA8M2_SPE_CROSS_COMPILE)
    set(CROSS_COMPILE ${RA8M2_SPE_CROSS_COMPILE} CACHE STRING "Cross-compiler prefix")
endif()

# CPU architecture from the same cpuarch.cmake the secure side used. It must come before
# project(); cpuarch_ns.cmake explains why.
include(${CONFIG_SPE_PATH}/platform/cpuarch_ns.cmake)

# TFM_TOOLCHAIN_FILE is the name the TF-M build uses; CMake honours only CMAKE_TOOLCHAIN_FILE.
# The familiar spelling is accepted and mapped, so both halves are configured the same way.
if(DEFINED TFM_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${TFM_TOOLCHAIN_FILE})
endif()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CONFIG_SPE_PATH}/cmake/toolchain_ns_GNUARM.cmake)
endif()

project("RA8M2 TF-M non-secure application" LANGUAGES C ASM)

#-------------------------------------------------------------------------------
# The SPE half: interface sources, veneers, platform_ns, and the NS image signing rules.
#
# This one add_subdirectory() brings in tfm_api_ns, tfm_api_ns_tz, platform_ns,
# platform_region_defs, platform_ns_definitions and - because BL2 and
# PLATFORM_DEFAULT_IMAGE_SIGNING are on - the tfm_ns_signed_bin / tfm_s_ns_signed_bin
# targets. Signing happens on this side of the split, using the keys and layout files the
# SPE exported into <api_ns>/image_signing. The SPE does not sign the NS image.
#-------------------------------------------------------------------------------
# <api_ns>/CMakeLists.txt opens with include(spe_config) / include(spe_export), which are
# bare module names - they resolve only through CMAKE_MODULE_PATH, and both live in
# <api_ns>/cmake. Without this the SPE half fails on its first two lines.
list(APPEND CMAKE_MODULE_PATH ${CONFIG_SPE_PATH}/cmake)

add_subdirectory(${CONFIG_SPE_PATH} ${CMAKE_BINARY_DIR}/spe)

#-------------------------------------------------------------------------------
# The combined S+NS image is not built.
#
# spe-CMakeLists.cmake creates tfm_s_ns_signed_bin as an ALL target: assemble.py
# concatenates the two signed images into one tfm_s_ns_signed.bin, so one file can be flashed
# instead of two. Nothing consumes it at runtime: BL2 has MCUBOOT_IMAGE_NUMBER 2 and verifies
# two separate images in two separate slots.
#
# It is dropped because it is a hazard. It is another entry in the debug session's program
# list with no address of its own, and a wrong address is silent and destructive: entered at
# its size rather than its start, it overwrites part of the secure slot, the NSC veneers, the
# whole non-secure slot and part of the non-secure secondary slot. Being last in the list, it
# does so after the correct images have been written.
#
# To restore it: remove this, and restore SECURE_IMAGE_OFFSET / SECURE_IMAGE_MAX_SIZE /
# NON_SECURE_IMAGE_OFFSET / NON_SECURE_IMAGE_MAX_SIZE in flash_layout.h with the adjacency
# assert in ra8m2_layout_checks.c. Without those, building this target explicitly fails in
# assemble.py with "NameError: name 'SECURE_IMAGE_OFFSET' is not defined".
#-------------------------------------------------------------------------------
if(TARGET tfm_s_ns_signed_bin)
    set_target_properties(tfm_s_ns_signed_bin PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# The NS image must still be signed by default.
#
# Upstream, tfm_ns_signed_bin is not an ALL target; only tfm_s_ns_signed_bin is, and the NS
# image is signed only because the combined image depends on it. Excluding the combined target
# would therefore stop producing tfm_ns_signed.bin, the file BL2 verifies, so the signing
# target joins ALL on its own.
#
# add_dependencies() on the target, not DEPENDS on the output file: the custom command that
# writes tfm_ns_signed.bin is declared in the SPE subdirectory (${CMAKE_BINARY_DIR}/spe), and
# a file-level dependency does not cross directory scopes. It would resolve to nothing here,
# and the build would produce no signed image without an error.
if(TARGET tfm_ns_signed_bin)
    add_custom_target(ra8m2_ns_signed ALL)
    add_dependencies(ra8m2_ns_signed tfm_ns_signed_bin)
endif()

#-------------------------------------------------------------------------------
# FSP non-secure project
#-------------------------------------------------------------------------------
if(NOT DEFINED FSP_NS_APP_DIR)
    set(FSP_NS_APP_DIR ${RA8M2_SPE_FSP_NS_APP_DIR})
endif()

# The solution emits one or the other depending on the toolchain it was generated for.
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set(RA8M2_NS_REGIONS Debug/memory_regions.icf)
else()
    set(RA8M2_NS_REGIONS Debug/memory_regions.ld)
endif()

if(NOT EXISTS ${FSP_NS_APP_DIR}/${RA8M2_NS_REGIONS})
    message(FATAL_ERROR
        "RA8M2: ${FSP_NS_APP_DIR}/${RA8M2_NS_REGIONS} not found. The solution emits the "
        "layout during the e2 build - build the non-secure project in e2 studio once, then "
        "re-run cmake. Note the toolchain: a GCC solution emits .ld, an IAR one .icf.")
endif()

# An error, not a warning. The NS image is placed by the FSP project's own generated linker
# script; if that project came from a different solution than the secure image, the two
# disagree on slot addresses, the NSC window and the RAM split, and the image still links.
get_filename_component(_ns_dir_real  "${FSP_NS_APP_DIR}"          REALPATH)
get_filename_component(_spe_dir_real "${RA8M2_SPE_FSP_NS_APP_DIR}" REALPATH)
if(NOT _ns_dir_real STREQUAL _spe_dir_real)
    message(FATAL_ERROR
        "RA8M2: FSP_NS_APP_DIR (${_ns_dir_real}) is not the project the SPE was built "
        "against (${_spe_dir_real}). The layout comes from the solution, so these must be "
        "the same project. Override deliberately by setting RA8M2_SPE_FSP_NS_APP_DIR too.")
endif()

# The application only.
#
# The FSP BSP (startup, vectors, clocks, drivers, and every module the e2 project enables)
# lives in platform_ns, installed with the SPE. See <api_ns>/platform/CMakeLists.txt, "The FSP
# BSP", for the reasons and the exclusions. Adding an FSP module in e2 needs no edit here.
#
#   ra_gen/main.c     FSP's generated entry point: main() { hal_entry(); }. Compiled here, not
#                     in platform_ns, because a stock NSPE application brings its own main()
#                     (tf-m-tests does), and two definitions would collide.
#   src/hal_entry.c   the application body, and the target of the --wrap below.
#
# src/hal_warmstart.c is not here: it is R_BSP_WarmStart, board init, and belongs to the
# platform.
add_executable(tfm_ns
    ${FSP_NS_APP_DIR}/ra_gen/main.c
    ${FSP_NS_APP_DIR}/src/hal_entry.c
)

# syscalls_ns.c goes in the executable, as well as in platform_ns. A static library yields its
# objects only to resolve an undefined symbol, and libc, which references _write, is reached
# after the linker has finished with platform_ns. nosys.specs' stubs would be used instead and
# warn, which -Wl,-fatal-warnings turns into a failure. Compiled here, the symbols are present,
# and platform_ns' copy stays unextracted rather than colliding.
target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns/syscalls_ns.c)

# tfm_ns_interface_dispatch() connects the PSA client shims to the veneers. TF-M ships two
# implementations and tfm_api_ns_tz supplies neither, because the choice belongs to the
# application. Bare metal is correct for this application only: it calls the veneer without
# locking, which is a data race as soon as a second thread makes a PSA call. tf-m-tests brings
# its own RTOS-aware os_wrapper, which is why this belongs here and not in platform_ns.
target_sources(tfm_ns PRIVATE
    ${CONFIG_SPE_PATH}/interface/src/os_wrapper/tfm_ns_interface_bare_metal.c)

# platform_ns explicitly, not only transitively through tfm_api_ns: this application needs its
# FSP include directories and its PUBLIC startup and vector sources.
target_link_libraries(tfm_ns PRIVATE
    platform_ns
    tfm_api_ns
    $<$<BOOL:${CONFIG_TFM_USE_TRUSTZONE}>:tfm_api_ns_tz>
)

#-------------------------------------------------------------------------------
# Console and the service smoke test
#-------------------------------------------------------------------------------
set(RA8M2_NS_SERVICE_TESTS ON CACHE BOOL
    "Exercise the PSA services from the NS image on startup")

if(RA8M2_STDOUT_RTT)
    # rtt_ns_init.c lives with the application, not with platform_ns, because it works only
    # together with the --wrap below: it defines __wrap_hal_entry(), and the flag redirects
    # main()'s call into it. A constructor cannot replace it: FSP's Reset_Handler reaches
    # main() without newlib's crt0, so this image has no .init_array and a constructor would
    # link and never run. See the file.
    target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/rtt/rtt_ns_init.c)
    # ILINK has --wrap too, with the same __wrap_/__real_ convention, so rtt_ns_init.c is the
    # same for both toolchains; only the spelling of the option differs.
    if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
        target_link_options(tfm_ns PRIVATE "SHELL:--wrap hal_entry")
    else()
        target_link_options(tfm_ns PRIVATE -Wl,--wrap=hal_entry)
    endif()

    if(RA8M2_NS_SERVICE_TESTS)
        target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns/tfm_service_tests.c)
        target_include_directories(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns)
        target_compile_definitions(tfm_ns PRIVATE RA8M2_NS_SERVICE_TESTS)
    endif()
elseif(RA8M2_NS_SERVICE_TESTS)
    # The test reports over RTT and records to g_ns_test_results. Without RTT it still
    # records, but nothing announces which step is running, so a hang is unattributable.
    message(WARNING
        "RA8M2: RA8M2_NS_SERVICE_TESTS is on but RA8M2_STDOUT_RTT is off. The test is not "
        "built. Turn RTT on, or read g_ns_test_results from a debugger with the test "
        "wired in manually.")
endif()

#-------------------------------------------------------------------------------
# Link
#
# FSP's own linker script, which INCLUDEs Debug/memory_regions.ld, so a repartitioned
# solution carries through with no edit here.
#-------------------------------------------------------------------------------
# No file argument: the NS-side macro takes the script from platform_ns'
# INTERFACE_LINK_DEPENDS, set in the installed platform/CMakeLists.txt. The NS and secure
# macros of the same name differ in signature here only.
target_add_scatter_file(tfm_ns)
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    target_link_options(tfm_ns PRIVATE
        "SHELL:--config_search ${FSP_NS_APP_DIR}/Debug"
        "SHELL:--config_search ${FSP_NS_APP_DIR}")
else()
    target_link_options(tfm_ns PRIVATE
        -L${FSP_NS_APP_DIR}/Debug
        -L${FSP_NS_APP_DIR})
endif()

# spe-CMakeLists.cmake's signing rules depend on the target tfm_ns_bin and on
# $<TARGET_FILE_DIR:tfm_ns>/tfm_ns.bin, so both the .axf name and the output directory matter.
# SUFFIX and RUNTIME_OUTPUT_DIRECTORY make the two agree.
set_target_properties(tfm_ns PROPERTIES
    SUFFIX ".axf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_convert_to_bin_target(tfm_ns)
