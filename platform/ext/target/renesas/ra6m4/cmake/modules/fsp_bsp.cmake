# FSP BSP (Board Support Package) Library
# This library contains core BSP functionality required by all modules
#
# When FSP_S_APP_DIR is set, sources are pulled from the external RASC project.
# Otherwise, sources are pulled from the embedded fsp/ directory.

# Determine FSP source directory
if(FSP_S_APP_DIR)
    set(FSP_S_DIR ${FSP_S_APP_DIR})
    message(STATUS "FSP BSP: Using external RASC project: ${FSP_S_DIR}")
else()
    # Fall back to embedded FSP sources in platform directory
    set(FSP_S_DIR ${CMAKE_CURRENT_LIST_DIR}/../../fsp)
    message(STATUS "FSP BSP: Using embedded sources: ${FSP_S_DIR}")
endif()

# Only create the library if it doesn't already exist (shared between S and NS)
if(NOT TARGET fsp_bsp)
    add_library(fsp_bsp STATIC)
endif()

# BSP source files
target_sources(fsp_bsp
    PRIVATE
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_common.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_clocks.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_delay.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_group_irq.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_guard.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_io.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_irq.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_register_protection.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_sbrk.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_security.c
        ${FSP_S_DIR}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c
        # I/O Port driver (required by most peripherals)
        ${FSP_S_DIR}/ra/fsp/src/r_ioport/r_ioport.c
        # Generated files
        ${FSP_S_DIR}/ra_gen/common_data.c
        ${FSP_S_DIR}/ra_gen/pin_data.c
        ${FSP_S_DIR}/ra_gen/vector_data.c
)

# Conditionally include files that may not exist in all projects
if(EXISTS "${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_ipc.c")
    target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_ipc.c)
endif()
if(EXISTS "${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_macl.c")
    target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_macl.c)
endif()
if(EXISTS "${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_sdram.c")
    target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/all/bsp_sdram.c)
endif()
if(EXISTS "${FSP_S_DIR}/ra/board/ra6m4_ek/board_init.c")
    target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/ra/board/ra6m4_ek/board_init.c)
endif()
if(EXISTS "${FSP_S_DIR}/ra/board/ra6m4_ek/board_leds.c")
    target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/ra/board/ra6m4_ek/board_leds.c)
endif()

# Add stub file for g_init_info when not using FSP startup
# (TF-M handles startup, so we need stubs for FSP linker init symbols)
if(NOT FSP_S_APP_DIR)
    # Use stub from embedded sources
    target_sources(fsp_bsp PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../bsp_init_stub.c)
else()
    # When using external RASC project, check if stub exists there
    if(EXISTS "${FSP_S_DIR}/tfm_integration/bsp_init_stub.c")
        target_sources(fsp_bsp PRIVATE ${FSP_S_DIR}/tfm_integration/bsp_init_stub.c)
    else()
        # Fall back to platform stub
        target_sources(fsp_bsp PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../bsp_init_stub.c)
    endif()
endif()

# BSP include directories (PUBLIC so dependent modules can use them)
target_include_directories(fsp_bsp
    PUBLIC
        ${FSP_S_DIR}/ra/arm/CMSIS_6/CMSIS/Core/Include
        ${FSP_S_DIR}/ra/fsp/inc
        ${FSP_S_DIR}/ra/fsp/inc/api
        ${FSP_S_DIR}/ra/fsp/inc/instances
        ${FSP_S_DIR}/ra/fsp/src/bsp/mcu/ra6m4
        ${FSP_S_DIR}/ra_cfg/fsp_cfg
        ${FSP_S_DIR}/ra_cfg/fsp_cfg/bsp
        ${FSP_S_DIR}/ra_gen
        ${FSP_S_DIR}
)

# BSP compile definitions
# Note: _RA_ORDINAL is NOT defined here - it's target-specific:
#   - Secure side (S): _RA_ORDINAL=1 (defined by secure targets)
#   - Non-secure side (NS): _RA_ORDINAL=2 (defined by NS app's RASC_CMAKE_DEFINITIONS)
target_compile_definitions(fsp_bsp
    PUBLIC
        _RA_CORE=CM33
        _RENESAS_RA_
        _RA_TZ_SECURE=1
)

# BSP compile options for GCC ARM (compatible with TF-M toolchain)
target_compile_options(fsp_bsp
    PRIVATE
        -mfloat-abi=soft  # Match TF-M's soft float ABI
        -mcpu=cortex-m33
        -mthumb
        -mcmse             # Enable CMSE intrinsics for TrustZone
        -std=c99
        -fmessage-length=0
        -fsigned-char
        -ffunction-sections
        -fdata-sections
        $<$<CONFIG:Debug>:-g -O2>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:MinSizeRel>:-Os>
        $<$<CONFIG:RelWithDebInfo>:-g -O2>
)

# Add warnings (matching FSP defaults)
target_compile_options(fsp_bsp
    PRIVATE
        -Wunused
        -Wuninitialized
        -Wall
        -Wextra
        -Wmissing-declarations
        -Wconversion
        -Wpointer-arith
        -Wshadow
        -Wlogical-op
        -Waggregate-return
        -Wfloat-equal
)

# GCC 12.2+ specific flags
if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.2)
    target_compile_options(fsp_bsp
        PRIVATE
            --param=min-pagesize=0
            -Wno-format-truncation
            -Wno-stringop-overflow
    )
endif()
