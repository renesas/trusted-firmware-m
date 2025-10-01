# Trusted Firmware-M Platform Support for Renesas RA6M4

This directory contains the TF-M platform implementation for the Renesas RA6M4 microcontroller using modular FSP libraries.

## Hardware Specifications

- **MCU**: Renesas R7FA6M4AF
- **Core**: ARM Cortex-M33 with TrustZone, FPU (FPv5-SP-D16), DSP
- **Flash**: 1MB (0x00000000 - 0x000FFFFF)
- **RAM**: 256KB (0x20000000 - 0x2003FFFF)
- **Data Flash**: 8KB (0x08000000 - 0x08001FFF)
- **System Clock**: 200MHz

## Memory Layout

### Flash Partitioning

```
0x0000_0000 - 0x0001_FFFF : BL2 Bootloader (128KB)
0x0002_0000 - 0x0007_FFFF : Secure Image (384KB)
0x0008_0000 - 0x000F_FFFF : Non-Secure Image (512KB)
```

### RAM Partitioning

```
0x2000_0000 - 0x2001_FFFF : Secure RAM (128KB)
0x2002_0000 - 0x2003_FFFF : Non-Secure RAM (128KB)
```

### Data Flash Usage

```
0x0800_0000 - 0x0800_07FF : NV Counters (2KB)
0x0800_0800 - 0x0800_13FF : Protected Storage (3KB)
0x0800_1400 - 0x0800_1BFF : Internal Trusted Storage (2KB)
```

## FSP Integration

This platform uses the modular FSP library structure from the `fsp_cmake` project:

- **fsp_bsp** - Board Support Package (BSP)
- **fsp_uart** - UART driver (SCI0)
- **fsp_flash** - Flash driver (Flash HP)

### Prerequisites

1. **FSP Project** with UART and Flash drivers generated
2. **ARM GCC Toolchain** (`arm-none-eabi-gcc`)
3. **CMake** 3.21+
4. **Ninja** build system

## Building TF-M for RA6M4

### Step 1: Set FSP Path

Set the path to your FSP project:

```bash
export FSP_ROOT_DIR="C:/Users/Michael/Documents/GitHub/fsp_cmake/FSP_Project_ra6m4"
```

### Step 2: Configure TF-M Build

```bash
cd "C:/Users/Michael/Documents/GitHub/trusted-firmware-m"

cmake -S . -B build_ra6m4 \
  -DTFM_PLATFORM=renesas/ra6m4 \
  -DTFM_TOOLCHAIN_FILE=toolchain_GNUARM.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DFSP_ROOT_DIR="${FSP_ROOT_DIR}" \
  -DARM_TOOLCHAIN_PATH="C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin" \
  -G Ninja
```

### Step 3: Build

```bash
cmake --build build_ra6m4
```

### Output Files

```
build_ra6m4/bin/
├── bl2.bin          # Bootloader
├── tfm_s.bin        # Secure firmware
├── tfm_ns.bin       # Non-secure firmware
└── *.hex/*.elf      # Debug files
```

## Directory Structure

```
renesas/ra6m4/
├── cpuarch.cmake              # Cortex-M33 architecture
├── config.cmake               # Platform configuration
├── CMakeLists.txt             # Build system (integrates FSP modules)
├── device_cfg.h               # Device configuration
├── target_cfg.c/h             # SAU/System initialization
├── flash_layout.h             # Flash memory layout
├── region_defs.h              # Memory regions
├── tfm_hal_platform.c         # Platform HAL
├── tfm_hal_isolation.c        # MPU/SAU configuration
├── tfm_interrupts.c           # Interrupt handling
└── cmsis_drivers/
    ├── Driver_USART.c         # UART wrapper (FSP SCI)
    └── Driver_Flash.c         # Flash wrapper (FSP Flash HP)
```

## Key Features

✅ **Modular FSP Integration**
- Links FSP libraries directly into TF-M
- BSP, UART, and Flash as separate modules
- Easy to add new FSP modules (ADC, Timer, etc.)

✅ **TrustZone Configuration**
- SAU configured for Secure/Non-Secure regions
- MPU enforces memory protection
- Secure Gateway (CMSE Veneer) region

✅ **CMSIS Driver Wrappers**
- FSP drivers wrapped as CMSIS drivers
- Compatible with TF-M driver model

## Configuration Options

Edit `config.cmake` to customize:

```cmake
# Flash layout
set(FLASH_AREA_BL2_SIZE     0x20000)  # BL2 size
set(FLASH_S_PARTITION_SIZE  0x60000)  # Secure size
set(FLASH_NS_PARTITION_SIZE 0x80000)  # Non-secure size

# FSP modules to include
set(FSP_MODULES "bsp;uart;flash")  # Add modules here
```

## Adding New FSP Modules

To add a new module (e.g., ADC):

1. **Generate in RASC** (in FSP project)
2. **Create module CMake** (in FSP project):
   ```bash
   # See FSP_Project_ra6m4/cmake/modules/ADD_NEW_MODULE.md
   ```
3. **Add to TF-M platform**:
   Edit `CMakeLists.txt`:
   ```cmake
   include(${FSP_ROOT_DIR}/cmake/modules/fsp_adc.cmake)
   target_link_libraries(platform_s PUBLIC fsp_adc)
   ```

## TrustZone Security

### SAU Configuration
- **Region 0**: Non-Secure Flash
- **Region 1**: Non-Secure RAM
- **Region 2**: Non-Secure Peripherals

### MPU Configuration
- **Region 0**: Secure Code (RO, Executable)
- **Region 1**: Secure RAM (RW, Non-Executable)

## Flashing

### Using J-Link

```bash
JLinkExe -device R7FA6M4AF -if SWD -speed 4000
> loadfile build_ra6m4/bin/bl2.hex
> loadfile build_ra6m4/bin/tfm_s.hex
> loadfile build_ra6m4/bin/tfm_ns.hex
> r
> go
> exit
```

### Using Renesas Flash Programmer

Flash the combined image or individual hex files starting at 0x00000000.

## Debugging

```bash
# Terminal 1: Start GDB Server
JLinkGDBServer -device R7FA6M4AF -if SWD

# Terminal 2: Connect GDB
arm-none-eabi-gdb build_ra6m4/bin/tfm_s.elf
(gdb) target remote localhost:2331
(gdb) load
(gdb) break main
(gdb) continue
```

## Known Limitations

1. **No Hardware Crypto** - Uses mbedTLS software crypto only
2. **Minimal Startup** - Uses basic startup code
3. **No Lifecycle** - Lifecycle management not implemented
4. **No OTA** - Firmware update support incomplete

## Testing

### Build with Tests

```bash
cmake -S . -B build_ra6m4 \
  -DTFM_PLATFORM=renesas/ra6m4 \
  -DTEST_S=ON \
  -DTEST_NS=ON \
  -DFSP_ROOT_DIR="${FSP_ROOT_DIR}"

cmake --build build_ra6m4
```

### UART Console

Connect UART0 (SCI0) at 115200 baud to see TF-M boot messages.

## References

- [TF-M Documentation](https://trustedfirmware-m.readthedocs.io/)
- [FSP Modular Build Guide](../../../../../fsp_cmake/FSP_Project_ra6m4/cmake/modules/README.md)
- [Renesas RA6M4](https://www.renesas.com/ra6m4)

## License

BSD-3-Clause

Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.