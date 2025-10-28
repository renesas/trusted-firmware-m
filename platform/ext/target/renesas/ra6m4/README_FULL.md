# Trusted Firmware-M (TF-M) Port for Renesas RA6M4

Complete platform port of ARM Trusted Firmware-M for the Renesas RA6M4 (R7FA6M4AF) microcontroller.

## Quick Start

See [BUILD.md](BUILD.md) for detailed build instructions.

## Hardware

- **MCU**: Renesas R7FA6M4AF3CFB
- **Core**: ARM Cortex-M33 with TrustZone, FPU, DSP
- **Flash**: 1 MB code + 8 KB data flash
- **RAM**: 256 KB SRAM
- **Board**: EK-RA6M4 Evaluation Kit

## Enabled Services

✅ **Crypto** - PSA Crypto API with MbedTLS 3.6.3  
✅ **ITS** - Internal Trusted Storage (2 KB in data flash)  
✅ **PS** - Protected Storage with AES-GCM encryption (3 KB)  
✅ **Attestation** - Device identity with 256-bit key  
✅ **Platform** - System reset and IOCTL services  
✅ **MCUboot** - Secure bootloader with OTA support (Renesas fork v2.1.0+renesas.3)

## Build Results

| Component | Flash | RAM |
|-----------|-------|-----|
| BL2 (MCUboot) | 26 KB (19.95%) | 20 KB |
| TF-M Secure | 115 KB (90.08%) | 47 KB |

## Memory Map

**Code Flash (1 MB)**
```
0x00000000: BL2 Bootloader          (128 KB, ~26 KB used)
0x00020000: Secure Primary Slot     (128 KB, ~115 KB used)
0x00040000: Non-Secure Primary Slot (128 KB)
0x00060000: Secure Secondary Slot   (128 KB) - OTA
0x00080000: NS Secondary Slot       (128 KB) - OTA
0x000A0000: Scratch Area            (384 KB) - MCUboot swap
```

**Data Flash (8 KB at 0x08000000)**
```
0x08000000: OTP/NV Counters (2 KB)
0x08000800: Protected Storage (3 KB, encrypted)
0x08001400: Internal Trusted Storage (2 KB)
```

## Key Features

### Security
- TrustZone-M hardware isolation
- RSA-3072 image signing
- AES-GCM encrypted storage
- Rollback protection
- Boot measurements for attestation

### Storage
- Persistent secure storage in data flash
- Separate drivers for code/data flash
- Wear leveling and power-fail safe

### Bootloader
- MCUboot with swap-based OTA updates
- Authenticated firmware images
- Revert on boot failure

## Architecture

### Platform Files
- [config.cmake](config.cmake) - TF-M configuration
- [flash_layout.h](flash_layout.h) - Memory layout
- [region_defs.h](region_defs.h) - Region boundaries
- [target_cfg.c](target_cfg.c) - TrustZone configuration
- [tfm_hal_platform.c](tfm_hal_platform.c) - Platform HAL
- [startup_ra6m4.c](startup_ra6m4.c) - Vector table

### CMSIS Drivers
- [Driver_Flash.c](cmsis_drivers/Driver_Flash.c) - Flash (code + data)
- [Driver_USART.c](cmsis_drivers/Driver_USART.c) - UART logging

### FSP Integration
Modular Renesas FSP libraries:
- **fsp_bsp** - Board support, clocks, GPIO
- **fsp_uart** - SCI UART HAL
- **fsp_flash** - Flash HP driver

## Key Implementations

### 1. CMSIS 6 Compatibility
FSP uses CMSIS 6, TF-M expects CMSIS 5. Compatibility layer in `platform/include/cmsis_override.h` detects version and provides shims.

### 2. MbedTLS Bare-Metal
Threading disabled for embedded use:
```cmake
set(THREADS_PREFER_PTHREAD_FLAG OFF)
set(CMAKE_THREAD_LIBS_INIT "")
set(CMAKE_HAVE_THREADS_LIBRARY FALSE)
```

### 3. Renesas MCUboot
Uses Renesas fork with RA6M4 optimizations:
```cmake
set(MCUBOOT_GIT_REPOSITORY "https://github.com/renesas/mcuboot.git")
set(MCUBOOT_VERSION "2.1.0+renesas.3")
```

### 4. Dual Flash Support
- Driver_FLASH0: Code flash for firmware
- Driver_FLASH1: Data flash for ITS/PS/OTP
- Different sector sizes handled per driver

## Requirements

**Tools:**
- ARM GNU Toolchain 13.2.1+
- CMake 3.21+
- Ninja
- Python 3.8+

**Python packages:**
```bash
pip install jinja2 pyyaml click cbor2 cryptography intelhex
```

## Known Limitations

- Soft-float only (no FPU optimization yet)
- Software crypto (HW acceleration stubbed)
- Flash-based NV counters (should use HW for production)
- Development keys (dummy provisioning enabled)

## Security Notes

⚠️ **Development Configuration - NOT for production:**

For production:
1. Disable `TFM_DUMMY_PROVISIONING`
2. Provision real keys
3. Enable HW crypto acceleration
4. Use HW NV counters
5. Set debug authentication policies
6. Enable fault injection hardening
7. Review TrustZone configuration

## Testing Status

✅ Build verified with all services  
✅ MCUboot integration  
✅ Memory layout validated  
⏳ Hardware testing pending  

## References

- [TF-M Docs](https://tf-m-user-guide.trustedfirmware.org/)
- [PSA Certified](https://www.psacertified.org/)
- [RA6M4](https://www.renesas.com/ra6m4)
- [Renesas FSP](https://github.com/renesas/fsp)
- [MCUboot](https://www.mcuboot.com/)

## License

BSD-3-Clause (consistent with TF-M and Renesas FSP)

---

**Status**: ✅ Build Verified | ⏳ Hardware Testing Pending  
**Updated**: October 2025
