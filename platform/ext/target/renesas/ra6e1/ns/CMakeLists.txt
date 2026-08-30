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

# DOMAIN_NS is also set by platform_ns_definitions, which spe-CMakeLists.cmake links in
# after this file returns. Set here as well so platform_ns' own sources see it - they are
# compiled with this target's definitions, not with a consumer's.
target_compile_definitions(platform_ns
    PUBLIC
        DOMAIN_NS=1
        __DOMAIN_NS=1
)
