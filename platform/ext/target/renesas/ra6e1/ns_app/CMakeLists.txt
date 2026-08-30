#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA6E1 non-secure application - the NSPE half of the split SPE/NSPE build.
#
# This is a STANDALONE CMake project, configured against an SPE build that has already been
# installed. It is not add_subdirectory()'d by the TF-M build; the dependency runs the other
# way. See README.md for the two commands.
#
# WHAT MAKES THIS PORT DIFFERENT FROM A STOCK NSPE APP
#
# The non-secure image is a whole FSP application. It keeps FSP's startup.c and FSP's own
# linker script (script/fsp.ld, which INCLUDEs the solution's generated memory_regions.ld),
# so its vectors, RAM and flash land in the partitions the e2 solution defined and nothing
# here hardcodes an address. That is why the FSP BSP is built HERE and not in platform_ns:
# it belongs to the application. Two copies would mean two startups and two vector tables.
#
# It also means FSP_NS_APP_DIR must be the non-secure project from the SAME solution the
# secure side was built against. The SPE records which one that was, and this file checks
# it rather than trusting the caller - a mismatched project links a perfectly valid image
# at the wrong address, and the first symptom is a board that resets in a loop.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

#-------------------------------------------------------------------------------
# Everything in this block MUST happen before project() - that is what fixes the
# toolchain, and CMake will not let it be changed afterwards.
#-------------------------------------------------------------------------------
if(NOT DEFINED CONFIG_SPE_PATH OR NOT EXISTS ${CONFIG_SPE_PATH})
    message(FATAL_ERROR
        "CONFIG_SPE_PATH must point at an INSTALLED SPE tree (the api_ns directory), not "
        "at a TF-M build directory. Build the SPE, then 'cmake --install <spe-build-dir>'. "
        "See ns_app/README.md.")
endif()

# Read the port settings the SPE exported. Only set() calls, safe this early, and it is
# where the compiler prefix comes from.
include(${CONFIG_SPE_PATH}/platform/ra6e1_ns_config.cmake)

# toolchain_ns_GNUARM.cmake finds the compiler with find_program(${CROSS_COMPILE}-gcc),
# which assumes it is on PATH. It generally is not - the toolchain ships inside e2 studio -
# so default to the exact prefix the SPE used. An explicit -DCROSS_COMPILE still wins.
if(NOT DEFINED CROSS_COMPILE AND RA6E1_SPE_CROSS_COMPILE)
    set(CROSS_COMPILE ${RA6E1_SPE_CROSS_COMPILE} CACHE STRING "Cross-compiler prefix")
endif()

# CPU architecture, from the SAME cpuarch.cmake the secure side used. Must be before
# project(): the toolchain file builds -mcpu/-march from TFM_SYSTEM_PROCESSOR and
# TFM_SYSTEM_ARCHITECTURE, and CMake fixes the toolchain during project(). Too late, and
# every compile gets a bare "-march=".
include(${CONFIG_SPE_PATH}/platform/cpuarch_ns.cmake)

# TFM_TOOLCHAIN_FILE is the name the TF-M build uses; CMake only honours
# CMAKE_TOOLCHAIN_FILE. Accept the familiar spelling and map it, so the two halves are
# configured the same way.
if(DEFINED TFM_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${TFM_TOOLCHAIN_FILE})
endif()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CONFIG_SPE_PATH}/cmake/toolchain_ns_GNUARM.cmake)
endif()

project("RA6E1 TF-M non-secure application" LANGUAGES C ASM)

#-------------------------------------------------------------------------------
# The SPE half: interface sources, veneers, platform_ns, and the NS image signing rules.
#
# This one add_subdirectory() brings in tfm_api_ns, tfm_api_ns_tz, platform_ns,
# platform_region_defs, platform_ns_definitions and - because BL2 and
# PLATFORM_DEFAULT_IMAGE_SIGNING are on - the tfm_ns_signed_bin / tfm_s_ns_signed_bin
# targets. Signing happens on THIS side of the split, using the keys and layout files the
# SPE exported into <api_ns>/image_signing. The SPE no longer signs the NS image.
#-------------------------------------------------------------------------------
# <api_ns>/CMakeLists.txt opens with include(spe_config) / include(spe_export), which are
# bare module names - they resolve only through CMAKE_MODULE_PATH, and both live in
# <api_ns>/cmake. Without this the SPE half fails on its first two lines.
list(APPEND CMAKE_MODULE_PATH ${CONFIG_SPE_PATH}/cmake)

add_subdirectory(${CONFIG_SPE_PATH} ${CMAKE_BINARY_DIR}/spe)

#-------------------------------------------------------------------------------
# FSP non-secure project
#-------------------------------------------------------------------------------
if(NOT DEFINED FSP_NS_APP_DIR)
    set(FSP_NS_APP_DIR ${RA6E1_SPE_FSP_NS_APP_DIR})
endif()

if(NOT EXISTS ${FSP_NS_APP_DIR}/Debug/memory_regions.ld)
    message(FATAL_ERROR
        "RA6E1: ${FSP_NS_APP_DIR}/Debug/memory_regions.ld not found. The solution emits the "
        "layout during the e2 build - build the non-secure project in e2 studio once, then "
        "re-run cmake.")
endif()

# Not merely a warning. The NS image is placed by the FSP project's own generated linker
# script; if it came from a different solution than the secure image, the two agree about
# nothing - slot addresses, NSC window, RAM split - and the image still links cleanly.
get_filename_component(_ns_dir_real  "${FSP_NS_APP_DIR}"          REALPATH)
get_filename_component(_spe_dir_real "${RA6E1_SPE_FSP_NS_APP_DIR}" REALPATH)
if(NOT _ns_dir_real STREQUAL _spe_dir_real)
    message(FATAL_ERROR
        "RA6E1: FSP_NS_APP_DIR (${_ns_dir_real}) is not the project the SPE was built "
        "against (${_spe_dir_real}). The layout comes from the solution, so these must be "
        "the same project. Override deliberately by setting RA6E1_SPE_FSP_NS_APP_DIR too.")
endif()

# Same collection the SPE build uses for the NS image: the whole FSP project tree, because
# the NS side really is a complete FSP application rather than a set of chosen modules.
file(GLOB_RECURSE FSP_NS_SOURCES
     ${FSP_NS_APP_DIR}/ra/fsp/src/bsp/*.c
     ${FSP_NS_APP_DIR}/ra/fsp/src/r_ioport/*.c
     ${FSP_NS_APP_DIR}/ra_gen/*.c
     ${FSP_NS_APP_DIR}/src/*.c)

file(GLOB FSP_NS_INCLUDE_DIRS LIST_DIRECTORIES true
     ${FSP_NS_APP_DIR}/ra/fsp/inc
     ${FSP_NS_APP_DIR}/ra/fsp/inc/api
     ${FSP_NS_APP_DIR}/ra/fsp/inc/instances
     ${FSP_NS_APP_DIR}/ra/arm/CMSIS_6/CMSIS/Core/Include
     ${FSP_NS_APP_DIR}/ra_gen
     ${FSP_NS_APP_DIR}/ra_cfg/fsp_cfg
     ${FSP_NS_APP_DIR}/ra_cfg/fsp_cfg/bsp
     ${FSP_NS_APP_DIR}/src
     # Debug/ holds the GENERATED layout - bsp_linker_info.h, memory_regions.ld,
     # fsp_gen.ld. It is emitted by the e2 build and deliberately not committed, which is
     # why an e2 build is a prerequisite for this one.
     ${FSP_NS_APP_DIR}/Debug)

add_executable(tfm_ns ${FSP_NS_SOURCES})

target_include_directories(tfm_ns PRIVATE ${FSP_NS_INCLUDE_DIRS})

# _RA_TZ_NONSECURE / BSP_TZ_NONSECURE_BUILD tell FSP this image runs non-secure: it then
# skips the stack seal, the SAU setup and R_BSP_NonSecureEnter(), all of which belong to
# the secure side and are already done by the time this image runs.
target_compile_definitions(tfm_ns PRIVATE
    _RA_CORE=CM33
    _RA_TZ_NONSECURE=1
    BSP_TZ_NONSECURE_BUILD=1
)

# syscalls_ns.c goes in the EXECUTABLE, not in platform_ns. In a static library its
# objects are never pulled in - libc is what references _write, and the linker reaches
# libc after it has finished with platform_ns, so nosys.specs' stubs are used instead and
# warn. NS links with -Wl,-fatal-warnings, so those warnings fail the build.
target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns/syscalls_ns.c)

# tfm_ns_interface_dispatch() - the seam between the PSA client shims and the veneers.
# TF-M ships two: bare metal, and one that takes an RTOS mutex. tfm_api_ns_tz supplies
# neither, because which one is right is the application's decision, not the platform's.
# Bare metal is correct here and ONLY here: it calls the veneer with no locking, which is
# a data race the moment a second thread makes a PSA call. Switch to
# tfm_ns_interface_rtos.c when this application gains a scheduler.
target_sources(tfm_ns PRIVATE
    ${CONFIG_SPE_PATH}/interface/src/os_wrapper/tfm_ns_interface_bare_metal.c)

target_link_libraries(tfm_ns PRIVATE
    tfm_api_ns
    $<$<BOOL:${CONFIG_TFM_USE_TRUSTZONE}>:tfm_api_ns_tz>
)

#-------------------------------------------------------------------------------
# Console and the service smoke test
#-------------------------------------------------------------------------------
set(RA6E1_NS_SERVICE_TESTS ON CACHE BOOL
    "Exercise the PSA services from the NS image on startup")

if(RA6E1_STDOUT_RTT)
    # rtt_ns_init.c lives with the APPLICATION, not with platform_ns, because it only works
    # together with the --wrap below: it defines __wrap_hal_entry(), and the flag is what
    # redirects main()'s call into it. A constructor cannot be used instead - this image has
    # no .init_array, since FSP's Reset_Handler reaches main() without newlib's crt0, so a
    # constructor would link and never run. See the file.
    target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/rtt/rtt_ns_init.c)
    target_link_options(tfm_ns PRIVATE -Wl,--wrap=hal_entry)

    if(RA6E1_NS_SERVICE_TESTS)
        target_sources(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns/tfm_service_tests.c)
        target_include_directories(tfm_ns PRIVATE ${CONFIG_SPE_PATH}/platform/ns)
        target_compile_definitions(tfm_ns PRIVATE RA6E1_NS_SERVICE_TESTS)
    endif()
elseif(RA6E1_NS_SERVICE_TESTS)
    # The test reports over RTT and records to g_ns_test_results. Without RTT it still
    # records, but nothing announces which step is running, so a hang is unattributable.
    message(WARNING
        "RA6E1: RA6E1_NS_SERVICE_TESTS is on but RA6E1_STDOUT_RTT is off. The test is not "
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
# INTERFACE_LINK_DEPENDS, set in the installed platform/CMakeLists.txt. This is the one
# place where the NS and secure macros of the same name differ in signature.
target_add_scatter_file(tfm_ns)
target_link_options(tfm_ns PRIVATE
    -L${FSP_NS_APP_DIR}/Debug
    -L${FSP_NS_APP_DIR})

# spe-CMakeLists.cmake's signing rules depend on the target tfm_ns_bin and on
# $<TARGET_FILE_DIR:tfm_ns>/tfm_ns.bin, so both the .axf name and the output directory
# matter. SUFFIX/RUNTIME_OUTPUT_DIRECTORY are what make those two agree.
set_target_properties(tfm_ns PROPERTIES
    SUFFIX ".axf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_convert_to_bin_target(tfm_ns)
