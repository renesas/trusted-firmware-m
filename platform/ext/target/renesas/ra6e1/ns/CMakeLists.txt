#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA6E1 non-secure platform library, for the SPLIT SPE/NSPE build.
#
# This file is INSTALLED as <api_ns>/platform/CMakeLists.txt and is add_subdirectory()'d
# by <api_ns>/CMakeLists.txt (TF-M's cmake/spe-CMakeLists.cmake). It runs in the NSPE
# build, not the SPE build, so:
#
#   * CMAKE_CURRENT_LIST_DIR is <api_ns>/platform - the INSTALLED tree, flat. There is no
#     ../cmake/modules, no ../cmsis_drivers, and CMAKE_SOURCE_DIR is the NS application,
#     not the TF-M source tree. Anything referenced here must be in the install rules in
#     the platform CMakeLists.
#   * the only contract that matters is the one spe-CMakeLists.cmake expects: a target
#     named platform_ns must exist by the time add_subdirectory(platform) returns. It then
#     appends uart_stdout.c (only if PLATFORM_DEFAULT_UART_STDOUT) and links
#     platform_ns_definitions into it.
#
# WHAT IS DELIBERATELY NOT HERE
#
# The FSP BSP. On this port the non-secure image is a whole FSP application - it keeps
# FSP's startup and FSP's linker script - so the BSP belongs to the APPLICATION, built by
# ns_app/CMakeLists.txt, not to the platform library. Pulling FSP in here as well would
# compile it twice and give the linker two startups to choose between.
#
# This replaces a copy of the RA6M4 file that had never been exercised: it included
# cmake/modules/fsp_uart.cmake, which does not exist in this port (the modules are bsp,
# flash and sce), and pulled cmsis_drivers/Driver_USART.c although RA6E1 logs over RTT.
# It would have failed on its first include().
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(RA6E1_NS_PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

# Port settings carried from the SPE build - console choice and the FSP project the secure
# side was built against. Generated during the SPE install; see ns/ra6e1_ns_config.cmake.in.
include(${RA6E1_NS_PLATFORM_DIR}/ra6e1_ns_config.cmake)

add_library(platform_ns STATIC)

#-------------------------------------------------------------------------------
# The FSP BSP
#
# This lives in platform_ns, NOT in the application, because a stock NSPE application -
# tf-m-tests' regression app being the one that matters - knows nothing about FSP and
# supplies only main(). Startup, vectors, clocks and drivers are the platform's job. That
# is also how every other vendor-SDK port draws the line; see nxp/lpcxpresso55s69, which
# puts its startup, board, clock and pin-mux files in platform_ns and defines no main().
#
# WHAT IS EXCLUDED, AND WHY IT MATTERS
#
#   ra_gen/main.c   - four generated lines: main() { hal_entry(); }. FSP bundles it with
#                     the generated project, but it is the APPLICATION's entry point, and
#                     tf-m-tests brings its own main(). Two definitions would collide.
#   src/hal_entry.c - the application. ns_app compiles it and wraps it.
#
# src/hal_warmstart.c IS included, despite living in FSP's src/ application area: it is
# R_BSP_WarmStart, a weak BSP hook that opens IOPORT at POST_C. Left with the application,
# a test app would silently link the BSP's weak default and never configure a pin - which
# costs nothing with RTT output and is a baffling failure the moment a test touches a
# peripheral.
#
# THE GLOB IS DELIBERATELY WIDE
#
# r_*/ and rm_*/ pick up every driver and middleware module, so a module ADDED IN e2 is
# compiled with no edit here. The narrow bsp+r_ioport list this replaces silently ignored
# anything else, which defeats the whole point of generating the project in e2.
#
# No opt-in registry, unlike FSP_MODULES_S on the secure side. That exists so nothing is
# dragged into a SIGNED SECURE image by accident; the NS image simply is the FSP
# application, so there is nothing to protect against and --gc-sections drops whatever is
# unreferenced.
#
# CONFIGURE_DEPENDS so adding a module in e2 does not also require remembering to
# reconfigure - CMake re-globs at build time. It costs a directory scan per build, which is
# the right trade for this workflow.
#-------------------------------------------------------------------------------
file(GLOB_RECURSE RA6E1_FSP_NS_SOURCES CONFIGURE_DEPENDS
     ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/src/bsp/*.c
     ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/src/r_*/*.c
     ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/src/rm_*/*.c
     ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_gen/*.c)

list(REMOVE_ITEM RA6E1_FSP_NS_SOURCES
     ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_gen/main.c)

# startup.c defines Reset_Handler and the FIXED vector table; vector_data.c defines
# g_vector_table, the APPLICATION vector table. Both must reach the image even though
# nothing calls them - and a static library only yields members that resolve an undefined
# symbol, so KEEP() in the linker script is not enough: KEEP stops --gc-sections discarding
# a section that is already there, it does not pull an archive member in.
#
# PUBLIC therefore, so the consuming executable compiles them directly. Same reason
# nxp/lpcxpresso55s69 marks its startup PUBLIC. The library's own copies simply go
# unextracted, because the executable has already defined the symbols.
set(RA6E1_FSP_NS_MUST_LINK
    ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c
    ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_gen/vector_data.c)
list(REMOVE_ITEM RA6E1_FSP_NS_SOURCES ${RA6E1_FSP_NS_MUST_LINK})

target_sources(platform_ns
    PUBLIC
        ${RA6E1_FSP_NS_MUST_LINK}
    PRIVATE
        ${RA6E1_FSP_NS_SOURCES}
        ${RA6E1_SPE_FSP_NS_APP_DIR}/src/hal_warmstart.c
)

target_include_directories(platform_ns
    PUBLIC
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/inc
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/inc/api
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/fsp/inc/instances
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra/arm/CMSIS_6/CMSIS/Core/Include
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_gen
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_cfg/fsp_cfg
        ${RA6E1_SPE_FSP_NS_APP_DIR}/ra_cfg/fsp_cfg/bsp
        ${RA6E1_SPE_FSP_NS_APP_DIR}/src
        # Debug/ holds the GENERATED layout - bsp_linker_info.h and the linker fragments.
        ${RA6E1_SPE_FSP_NS_APP_DIR}/Debug
)

# PUBLIC: the application compiles FSP headers too (hal_data.h from main.c), and it must
# see the same TrustZone flavour. _RA_TZ_NONSECURE / BSP_TZ_NONSECURE_BUILD tell FSP this
# image runs non-secure, so it skips the stack seal, the SAU setup and
# R_BSP_NonSecureEnter() - all of which belong to the secure side and have already run.
target_compile_definitions(platform_ns
    PUBLIC
        _RA_CORE=CM33
        _RA_TZ_NONSECURE=1
        BSP_TZ_NONSECURE_BUILD=1
)

# syscalls_ns.c is NOT here - it is compiled straight into the executable by
# ns_app/CMakeLists.txt. It supplies newlib's _close/_lseek/_read/_write, and in a static
# library those objects are never pulled in: the reference comes from libc, which the
# linker reaches only AFTER it has finished with platform_ns, so nosys.specs' stubs win
# and emit "_write is not implemented and will always fail". The NS image links with
# -Wl,-fatal-warnings, so that is a build failure, not a nuisance. The SPE build put the
# file in the executable for the same reason.
#
# platform_ns must still have at least one source, or CMake refuses to create the target.
if(NOT RA6E1_STDOUT_RTT)
    # Nothing else lands in platform_ns in a UART build yet. Keep the target legal rather
    # than letting the NS build fail with "no SOURCES given to target".
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/platform_ns_stub.c
         "/* Generated. platform_ns has no sources in this configuration - see the port's
"
         " * ns/CMakeLists.txt. A static library needs at least one. */
"
         "typedef int ra6e1_platform_ns_translation_unit_not_empty;
")
    target_sources(platform_ns PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/platform_ns_stub.c)
endif()

target_include_directories(platform_ns
    PUBLIC
        ${RA6E1_NS_PLATFORM_DIR}
        ${RA6E1_NS_PLATFORM_DIR}/include
        ${RA6E1_NS_PLATFORM_DIR}/ext/cmsis/Include
)

#-------------------------------------------------------------------------------
# Console
#
# RA6E1_STDOUT_RTT is carried over from the SPE build through spe_config.cmake, so the NS
# side cannot silently disagree with the secure side about the console.
#
# SEGGER_RTT.c is compiled into platform_ns, but rtt_ns_init.c is NOT - it defines
# __wrap_hal_entry(), which is only meaningful with -Wl,--wrap=hal_entry on the executable
# link line. That pairing lives with the application, in ns_app/CMakeLists.txt. Splitting
# them would give a library a symbol nothing calls and an application a flag with no
# definition; neither fails quietly, but both fail confusingly.
#-------------------------------------------------------------------------------
if(RA6E1_STDOUT_RTT)
    target_sources(platform_ns
        PRIVATE
            ${RA6E1_NS_PLATFORM_DIR}/rtt/SEGGER_RTT.c
            # The stdio_*() backend behind uart_stdout.h. tf-m-tests' NS log library calls
            # stdio_output_string() directly, so the regression tests need this in
            # platform_ns; our own ns_app does not, but carrying it costs nothing.
            #
            # RA6E1_STDOUT_NO_WRITE: ns/syscalls_ns.c owns _write() for the NS side, and
            # rtt_stdout.c would otherwise define a second one.
            ${RA6E1_NS_PLATFORM_DIR}/rtt/rtt_stdout.c
            ${RA6E1_NS_PLATFORM_DIR}/ns/syscalls_ns.c
    )
    target_compile_definitions(platform_ns PRIVATE RA6E1_STDOUT_NO_WRITE)
    target_include_directories(platform_ns
        PUBLIC
            ${RA6E1_NS_PLATFORM_DIR}/ext/common
    )
    target_include_directories(platform_ns
        PUBLIC
            ${RA6E1_NS_PLATFORM_DIR}/rtt
    )
    target_compile_definitions(platform_ns
        PUBLIC
            RA6E1_STDOUT_RTT
    )
endif()

#-------------------------------------------------------------------------------
# Linker script
#
# The NS image keeps FSP's own script, which INCLUDEs the solution's generated
# memory_regions.ld - so a repartitioned solution carries through with no edit anywhere.
#
# Published as INTERFACE_LINK_DEPENDS because that is how the NS-side
# target_add_scatter_file() finds it: unlike the secure build's macro of the same name, it
# takes no file argument, it reads this property off platform_ns (or globs *.ld from an
# install directory passed as an optional argument). See toolchain_ns_GNUARM.cmake.
#-------------------------------------------------------------------------------
set_target_properties(platform_ns PROPERTIES
    INTERFACE_LINK_DEPENDS ${RA6E1_SPE_FSP_NS_APP_DIR}/script/fsp.ld)

# fsp.ld is a stub: it INCLUDEs memory_regions.ld and fsp_gen.ld, which ld resolves through
# its library search path. Published as INTERFACE so ANY NSPE application inherits it -
# tf-m-tests' regression app links platform_ns but knows nothing about FSP, and without
# this it fails with "cannot open linker script file memory_regions.ld".
target_link_options(platform_ns INTERFACE
    -L${RA6E1_SPE_FSP_NS_APP_DIR}/Debug
    -L${RA6E1_SPE_FSP_NS_APP_DIR})

# DOMAIN_NS is also set by platform_ns_definitions, which spe-CMakeLists.cmake links in
# after this file returns. Set here as well so platform_ns' own sources see it - they are
# compiled with this target's definitions, not with a consumer's.
target_compile_definitions(platform_ns
    PUBLIC
        DOMAIN_NS=1
        __DOMAIN_NS=1
)
