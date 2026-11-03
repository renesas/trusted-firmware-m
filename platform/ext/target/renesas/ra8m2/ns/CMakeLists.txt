#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA8M2 non-secure platform library, for the split SPE/NSPE build.
#
# This file is installed as <api_ns>/platform/CMakeLists.txt and is add_subdirectory()'d by
# <api_ns>/CMakeLists.txt (TF-M's cmake/spe-CMakeLists.cmake). It runs in the NSPE build, not
# the SPE build, so:
#
#   * CMAKE_CURRENT_LIST_DIR is <api_ns>/platform, the installed tree, flat. There is no
#     ../cmake/modules or ../cmsis_drivers, and CMAKE_SOURCE_DIR is the NS application, not
#     the TF-M source tree. Anything referenced here must be in the install rules in the
#     platform CMakeLists.
#   * the contract is the one spe-CMakeLists.cmake expects: a target named platform_ns must
#     exist when add_subdirectory(platform) returns. It then appends uart_stdout.c (only if
#     PLATFORM_DEFAULT_UART_STDOUT) and links platform_ns_definitions into it.
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(RA8M2_NS_PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

# Port settings carried from the SPE build - console choice and the FSP project the secure
# side was built against. Generated during the SPE install; see ns/ra8m2_ns_config.cmake.in.
include(${RA8M2_NS_PLATFORM_DIR}/ra8m2_ns_config.cmake)

add_library(platform_ns STATIC)

#-------------------------------------------------------------------------------
# The FSP BSP
#
# The BSP lives in platform_ns, not in the application, because a stock NSPE application
# (tf-m-tests' regression app) knows nothing about FSP and supplies only main(). Startup,
# vectors, clocks and drivers are the platform's job. nxp/lpcxpresso55s69 draws the same
# line: its startup, board, clock and pin-mux files are in platform_ns, and it defines no
# main().
#
# Excluded:
#
#   ra_gen/main.c   - main() { hal_entry(); }. FSP generates it with the project, but it is
#                     the application's entry point, and tf-m-tests brings its own main().
#                     Two definitions would collide.
#   src/hal_entry.c - the application. ns_app compiles it and wraps it.
#
# src/hal_warmstart.c is included, although it lives in FSP's src/ application area: it is
# R_BSP_WarmStart, a weak BSP hook that opens IOPORT at POST_C. Left with the application, a
# test app would link the BSP's weak default and never configure a pin, which goes unnoticed
# with RTT output and fails confusingly once a test touches a peripheral.
#
# The glob is deliberately wide: r_*/ and rm_*/ pick up every driver and middleware module,
# so a module added in e2 is compiled with no edit here. There is no opt-in registry like
# FSP_MODULES_S on the secure side; that registry keeps code out of the signed secure image,
# while the NS image is the FSP application and --gc-sections drops whatever is unreferenced.
#
# CONFIGURE_DEPENDS re-globs at build time, so adding a module in e2 needs no manual
# reconfigure, at the cost of a directory scan per build.
#-------------------------------------------------------------------------------
file(GLOB_RECURSE RA8M2_FSP_NS_SOURCES CONFIGURE_DEPENDS
     ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/src/bsp/*.c
     ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/src/r_*/*.c
     ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/src/rm_*/*.c
     ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_gen/*.c)

list(REMOVE_ITEM RA8M2_FSP_NS_SOURCES
     ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_gen/main.c)

# startup.c defines Reset_Handler and the fixed vector table; vector_data.c defines
# g_vector_table, the application vector table. Both must reach the image although nothing
# calls them. A static library yields only members that resolve an undefined symbol, and
# KEEP() in the linker script does not help: it stops --gc-sections discarding a section
# already in the link, but does not pull an archive member in.
#
# They are therefore PUBLIC, so the consuming executable compiles them directly, as
# nxp/lpcxpresso55s69 does with its startup. The library's own copies stay unextracted
# because the executable already defines the symbols.
set(RA8M2_FSP_NS_MUST_LINK
    ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c
    ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_gen/vector_data.c)
list(REMOVE_ITEM RA8M2_FSP_NS_SOURCES ${RA8M2_FSP_NS_MUST_LINK})

target_sources(platform_ns
    PUBLIC
        ${RA8M2_FSP_NS_MUST_LINK}
    PRIVATE
        ${RA8M2_FSP_NS_SOURCES}
        ${RA8M2_SPE_FSP_NS_APP_DIR}/src/hal_warmstart.c
)

target_include_directories(platform_ns
    PUBLIC
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/inc
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/inc/api
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/fsp/inc/instances
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra/arm/CMSIS_6/CMSIS/Core/Include
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_gen
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_cfg/fsp_cfg
        ${RA8M2_SPE_FSP_NS_APP_DIR}/ra_cfg/fsp_cfg/bsp
        ${RA8M2_SPE_FSP_NS_APP_DIR}/src
        # Debug/ holds the generated layout: bsp_linker_info.h and the linker fragments.
        ${RA8M2_SPE_FSP_NS_APP_DIR}/Debug
)

# PUBLIC: the application compiles FSP headers too (hal_data.h from main.c) and must see the
# same TrustZone flavour. _RA_TZ_NONSECURE tells FSP this image runs non-secure, so it skips
# the stack seal, the SAU setup and R_BSP_NonSecureEnter(), which belong to the secure side
# and have already run.
#
# BSP_TZ_NONSECURE_BUILD is deliberately not defined here. The generated bsp_mcu_family_cfg.h
# derives it from _RA_TZ_NONSECURE and defines it as `(1)`, so `BSP_TZ_NONSECURE_BUILD=1` on
# the command line is redundant and a token mismatch, which fails under -Werror:
#     bsp_mcu_family_cfg.h:37: error: "BSP_TZ_NONSECURE_BUILD" redefined [-Werror]
# The PSA Arch tests compile NS code with -Wall -Werror -Wextra.
target_compile_definitions(platform_ns
    PUBLIC
        # CPU0, not CM33: on a multi-core part RASC names the rzone PROCESSOR rather
        # than the core type, and the e2 projects define _RA_CORE=CPU0. Must match what
        # the secure side passes (CMakeLists.txt, FSP_COMPILE_DEFS).
        _RA_CORE=CPU0
        _RA_TZ_NONSECURE=1
)

# syscalls_ns.c is compiled into the executable by ns_app/CMakeLists.txt, and also into
# platform_ns in an RTT build (Console, below). The executable's copy is the one linked; the
# reason is at the syscalls_ns.c target_sources() in ns_app/CMakeLists.txt.
#
# platform_ns must still have at least one source, or CMake refuses to create the target.
if(NOT RA8M2_STDOUT_RTT)
    # A UART build adds no console sources. The stub keeps the target legal even if the FSP
    # glob finds nothing, rather than failing with "no SOURCES given to target".
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/platform_ns_stub.c
         "/* Generated. platform_ns has no sources in this configuration - see the port's
"
         " * ns/CMakeLists.txt. A static library needs at least one. */
"
         "typedef int ra8m2_platform_ns_translation_unit_not_empty;
")
    target_sources(platform_ns PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/platform_ns_stub.c)
endif()

target_include_directories(platform_ns
    PUBLIC
        ${RA8M2_NS_PLATFORM_DIR}
        ${RA8M2_NS_PLATFORM_DIR}/include
        ${RA8M2_NS_PLATFORM_DIR}/ext/cmsis/Include
)

#-------------------------------------------------------------------------------
# Console
#
# RA8M2_STDOUT_RTT is carried over from the SPE build through ra8m2_ns_config.cmake, so the NS
# side cannot disagree with the secure side about the console.
#
# SEGGER_RTT.c is compiled into platform_ns, but rtt_ns_init.c is not: it defines
# __wrap_hal_entry(), which works only with -Wl,--wrap=hal_entry on the executable link line.
# That pairing lives with the application, in ns_app/CMakeLists.txt.
#-------------------------------------------------------------------------------
if(RA8M2_STDOUT_RTT)
    target_sources(platform_ns
        PRIVATE
            ${RA8M2_NS_PLATFORM_DIR}/rtt/SEGGER_RTT.c
            # The stdio_*() backend behind uart_stdout.h. tf-m-tests' NS log library calls
            # stdio_output_string() directly, so the regression tests need it in
            # platform_ns; our own ns_app does not.
            #
            # RA8M2_STDOUT_NO_WRITE: ns/syscalls_ns.c owns _write() for the NS side, and
            # rtt_stdout.c would otherwise define a second one.
            ${RA8M2_NS_PLATFORM_DIR}/rtt/rtt_stdout.c
            ${RA8M2_NS_PLATFORM_DIR}/ns/syscalls_ns.c
    )
    target_compile_definitions(platform_ns PRIVATE RA8M2_STDOUT_NO_WRITE)
    # 2 is SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL. Carried over from the SPE build like
    # RA8M2_STDOUT_RTT, because the fast output (the PSA Arch suites) comes from the NS side.
    # See RA8M2_RTT_BLOCKING in config.cmake.
    if(RA8M2_RTT_BLOCKING)
        target_compile_definitions(platform_ns PRIVATE SEGGER_RTT_MODE_DEFAULT=2)
    endif()
    target_include_directories(platform_ns
        PUBLIC
            ${RA8M2_NS_PLATFORM_DIR}/ext/common
    )
    target_include_directories(platform_ns
        PUBLIC
            ${RA8M2_NS_PLATFORM_DIR}/rtt
    )
    target_compile_definitions(platform_ns
        PUBLIC
            RA8M2_STDOUT_RTT
    )
endif()

#-------------------------------------------------------------------------------
# What the secure image's crypto can do, for NS code that probes it
#
# With the RSIP-E50D accelerator the SPE does not provide PSA_ALG_DETERMINISTIC_ECDSA: the engine
# draws its own ECDSA nonce, FSP does not support deterministic ECDSA on the SCE, and the
# accelerator removes it from the secure PSA configuration (accelerator/renesas/rsip_e50d,
# crypto_accelerator_config.h). NS builds compile against the client configuration and cannot
# see that, so it is exported explicitly. The psa-arch-tests Renesas target keys its
# deterministic-ECDSA tests on this.
#-------------------------------------------------------------------------------
if(RA8M2_SPE_CRYPTO_HW_ACCELERATOR)
    target_compile_definitions(platform_ns
        PUBLIC
            RENESAS_RA_NO_DETERMINISTIC_ECDSA
    )
endif()

#-------------------------------------------------------------------------------
# Linker script
#
# The NS image keeps FSP's own script, which INCLUDEs the solution's generated
# memory_regions.ld, so a repartitioned solution carries through with no edit.
#
# Published as INTERFACE_LINK_DEPENDS because the NS-side target_add_scatter_file() reads it
# from that property of platform_ns (or globs *.ld from an install directory passed as an
# optional argument); unlike the secure build's macro of the same name, it takes no file
# argument. See toolchain_ns_GNUARM.cmake.
#-------------------------------------------------------------------------------
# fsp.ld / fsp.icf: same stub, one per toolchain. This is read with get_target_property()
# and tested with EXISTS at configure time, so it cannot be a generator expression.
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    set(RA8M2_NS_FSP_SCATTER script/fsp.icf)
else()
    set(RA8M2_NS_FSP_SCATTER script/fsp.ld)
endif()

set_target_properties(platform_ns PROPERTIES
    INTERFACE_LINK_DEPENDS ${RA8M2_SPE_FSP_NS_APP_DIR}/${RA8M2_NS_FSP_SCATTER})

# fsp.ld is a stub that INCLUDEs memory_regions.ld and fsp_gen.ld, which ld resolves through
# its library search path. Published as INTERFACE so any NSPE application inherits it:
# tf-m-tests' regression app links platform_ns but knows nothing about FSP, and without this
# fails with "cannot open linker script file memory_regions.ld". ILINK resolves an ICF
# "include" through --config_search, its counterpart of ld's -L here: fsp.icf includes
# memory_regions.icf and fsp_gen.icf by bare name, both in Debug/.
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    target_link_options(platform_ns INTERFACE
        "SHELL:--config_search ${RA8M2_SPE_FSP_NS_APP_DIR}/Debug"
        "SHELL:--config_search ${RA8M2_SPE_FSP_NS_APP_DIR}")
else()
    target_link_options(platform_ns INTERFACE
        -L${RA8M2_SPE_FSP_NS_APP_DIR}/Debug
        -L${RA8M2_SPE_FSP_NS_APP_DIR})
endif()

# DOMAIN_NS is also set by platform_ns_definitions, which spe-CMakeLists.cmake links in
# after this file returns. Set here as well so platform_ns' own sources see it - they are
# compiled with this target's definitions, not with a consumer's.
target_compile_definitions(platform_ns
    PUBLIC
        DOMAIN_NS=1
        __DOMAIN_NS=1
)
