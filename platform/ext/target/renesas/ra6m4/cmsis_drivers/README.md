# FSP CMake Modules

This directory contains modular CMake build definitions for the Renesas FSP (Flexible Software Package).

## Module Structure

The FSP has been restructured into reusable CMake libraries:

### 1. **fsp_bsp** - Board Support Package (Core)
**File:** `fsp_bsp.cmake`
**Library:** `libfsp_bsp.a` (~355KB)

The foundation module required by all FSP components.

**Includes:**
- BSP core functionality (clocks, delays, interrupts)
- CMSIS startup and system initialization
- Board-specific initialization (RA6M4-EK)
- I/O Port driver (r_ioport)
- FSP-generated configuration (pin_data, vector_data, common_data)

**Dependencies:** None (base module)

---

### 2. **fsp_uart** - Serial Communications (SCI UART)
**File:** `fsp_uart.cmake`
**Library:** `libfsp_uart.a` (~113KB)

UART communication via SCI (Serial Communications Interface).

**Includes:**
- SCI UART driver (`r_sci_uart.c`)
- Supports asynchronous serial communication
- Configurable baud rates, parity, stop bits
- Interrupt-driven TX/RX

**Dependencies:** `fsp_bsp`

---

### 3. **fsp_flash** - Internal Flash Memory
**File:** `fsp_flash.cmake`
**Library:** `libfsp_flash.a` (~105KB)

High-performance flash driver for code flash and data flash.

**Includes:**
- Flash HP driver (`r_flash_hp.c`)
- Read/write/erase operations
- Code flash (1MB) and data flash (8KB) access
- Flash protection and status checking

**Dependencies:** `fsp_bsp`

---

## Build System

### Modular Build (Current - Default)

Uses separate libraries for each FSP module:

```cmake
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc_Modular.cmake)
```

**Benefits:**
- **Faster incremental builds** - Only changed modules rebuild
- **Better organization** - Clear module dependencies
- **Reusable** - Libraries can be used in multiple executables
- **TF-M compatible** - Modules can be integrated into TF-M platform

**Build artifacts:**
```
build/
├── libfsp_bsp.a       # BSP library
├── libfsp_uart.a      # UART module
├── libfsp_flash.a     # Flash module
└── FSP_Project_ra6m4.elf  # Final executable
```

### Monolithic Build (Original)

All sources compiled into single executable:

```cmake
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc.cmake)
```

To switch back, edit [CMakeLists.txt](../CMakeLists.txt):
```cmake
# include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc_Modular.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GeneratedSrc.cmake)
```

---

## Adding New Modules

To add a new FSP module (e.g., ADC, Timer):

### 1. Create Module Definition

Create `cmake/modules/fsp_<module>.cmake`:

```cmake
# FSP <Module> Library
add_library(fsp_<module> STATIC)

# Source files
target_sources(fsp_<module>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ra/fsp/src/r_<module>/r_<module>.c
)

# Dependencies
target_link_libraries(fsp_<module>
    PUBLIC
        fsp_bsp
)

# Compile options
target_compile_options(fsp_<module>
    PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${RASC_CMAKE_C_FLAGS}>
        $<$<CONFIG:Debug>:${RASC_DEBUG_FLAGS}>
        $<$<CONFIG:Release>:${RASC_RELEASE_FLAGS}>
)
```

### 2. Include in Build

Edit `cmake/GeneratedSrc_Modular.cmake`:

```cmake
include(${CMAKE_CURRENT_LIST_DIR}/modules/fsp_<module>.cmake)
```

### 3. Link to Application

```cmake
target_link_libraries(${PROJECT_NAME}.elf
    PRIVATE
        fsp_bsp
        fsp_uart
        fsp_flash
        fsp_<module>  # Add your module
)
```

---

## Module Dependencies

```
Application (FSP_Project_ra6m4.elf)
    │
    ├── fsp_uart
    │   └── fsp_bsp
    │
    ├── fsp_flash
    │   └── fsp_bsp
    │
    └── fsp_bsp (core)
```

All modules depend on `fsp_bsp` which provides:
- CMSIS headers
- FSP API headers
- Board configuration
- Compiler flags and definitions

---

## Usage in Applications

### Example: Using UART and Flash

```c
#include "hal_data.h"

void my_application(void)
{
    // UART instance defined in hal_data.c
    R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    R_SCI_UART_Write(&g_uart0_ctrl, "Hello\r\n", 7);

    // Flash instance defined in hal_data.c
    R_FLASH_HP_Open(&g_flash0_ctrl, &g_flash0_cfg);
    // ... flash operations
}
```

The module system automatically links the required libraries.

---

## Build Commands

```bash
# Configure
export ARM_TOOLCHAIN_PATH="C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin"
cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=cmake/gcc.cmake -DCMAKE_BUILD_TYPE=Debug -B build

# Build all modules and application
cmake --build build

# Build specific module
cmake --build build --target fsp_uart

# Clean
rm -rf build
```

---

## Advantages for TF-M Integration

The modular structure is ideal for Trusted Firmware-M (TF-M):

1. **Isolated modules** - Each driver is a separate library
2. **Clear dependencies** - Easy to see what TF-M needs
3. **Selective linking** - Include only required modules in Secure/Non-Secure worlds
4. **CMSIS wrappers** - Can wrap FSP modules as CMSIS drivers for TF-M

Example TF-M usage:
```cmake
# In TF-M platform CMakeLists.txt
target_link_libraries(platform_s
    PRIVATE
        fsp_bsp
        fsp_uart   # For secure logging
        fsp_flash  # For secure storage
)
```

---

## Maintenance

- **Auto-generated files**: BSP includes FSP-generated files from `ra_gen/`
- **RASC regeneration**: Modules automatically pick up changes when RASC regenerates code
- **Version compatibility**: Tested with FSP v6.1.0

---

## File Sizes

Typical library sizes (Debug build):

| Module | Size | Purpose |
|--------|------|---------|
| `libfsp_bsp.a` | ~355KB | Core BSP + I/O Port |
| `libfsp_uart.a` | ~113KB | SCI UART driver |
| `libfsp_flash.a` | ~105KB | Flash HP driver |
| **Total** | **~573KB** | All modules |

Final executable (with application): ~195KB (stripped: ~60KB)

---

## License

Same as FSP - BSD-3-Clause (Renesas)