# FSP BSP (Board Support Package) Library
# This library contains core BSP functionality required by all modules

# FSP base directory is relative to this platform directory
set(FSP_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

# Only create the library if it doesn't already exist (shared between S and NS)
if(NOT TARGET fsp_bsp)
    add_library(fsp_bsp STATIC)
endif()

# BSP source files
target_sources(fsp_bsp
    PRIVATE
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_common.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_clocks.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_delay.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_group_irq.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_guard.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_io.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_ipc.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_irq.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_macl.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_register_protection.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_sbrk.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_sdram.c
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/all/bsp_security.c
        # ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/ra6m4/bsp_linker.c  # Excluded - TF-M handles startup
        ${FSP_BASE_DIR}/bsp_init_stub.c  # Stub for g_init_info and g_main_stack
        # ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c  # Excluded - using TF-M startup
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c
        ${FSP_BASE_DIR}/fsp/ra/board/ra6m4_ek/board_init.c
        ${FSP_BASE_DIR}/fsp/ra/board/ra6m4_ek/board_leds.c
        # I/O Port driver (required by most peripherals)
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/r_ioport/r_ioport.c
        # Generated files
        ${FSP_BASE_DIR}/fsp/ra_gen/common_data.c
        ${FSP_BASE_DIR}/fsp/ra_gen/pin_data.c
        ${FSP_BASE_DIR}/fsp/ra_gen/vector_data.c
)

# BSP include directories (PUBLIC so dependent modules can use them)
target_include_directories(fsp_bsp
    PUBLIC
        ${FSP_BASE_DIR}/fsp/ra/arm/CMSIS_6/CMSIS/Core/Include
        ${FSP_BASE_DIR}/fsp/ra/fsp/inc
        ${FSP_BASE_DIR}/fsp/ra/fsp/inc/api
        ${FSP_BASE_DIR}/fsp/ra/fsp/inc/instances
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/bsp/mcu/ra6m4
        ${FSP_BASE_DIR}/fsp/ra_cfg/fsp_cfg
        ${FSP_BASE_DIR}/fsp/ra_cfg/fsp_cfg/bsp
        ${FSP_BASE_DIR}/fsp/ra_gen
        ${FSP_BASE_DIR}/fsp
)

# BSP compile definitions
target_compile_definitions(fsp_bsp
    PUBLIC
        _RA_CORE=CM33
        _RA_ORDINAL=1
        _RENESAS_RA_
)

# BSP compile options for GCC ARM (compatible with TF-M toolchain)
target_compile_options(fsp_bsp
    PRIVATE
        -mfloat-abi=soft  # Match TF-M's soft float ABI
        -mcpu=cortex-m33
        -mthumb
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
