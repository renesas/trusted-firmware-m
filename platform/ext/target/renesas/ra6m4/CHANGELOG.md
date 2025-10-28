# TF-M RA6M4 Port - Change Log

## Initial Port - October 2025

### Summary

Complete platform port of ARM Trusted Firmware-M (TF-M) to Renesas RA6M4 (R7FA6M4AF) microcontroller with full PSA services and MCUboot integration.

**TF-M Configuration:**
- **SPM Backend**: SFN (Secure Function Model) - lightweight, no IPC overhead
- **Isolation Level**: 1 - Basic TrustZone isolation, no MPU between partitions
- **Profile**: Custom (Medium + Attestation) - Crypto, ITS, PS, Attestation, Platform
- **Optimized for**: Resource-constrained devices with ~20% less RAM than IPC model

### Features Implemented

#### Core Platform
- [x] ARM Cortex-M33 TrustZone configuration (SAU/MPC/PPC)
- [x] Memory layout for 1 MB flash + 256 KB RAM
- [x] Vector table with 496 RA6M4 interrupt handlers
- [x] TF-M compatible startup code
- [x] Platform HAL implementation

#### PSA Services
- [x] **Crypto Partition** - Full PSA Crypto API with MbedTLS 3.6.3
- [x] **Internal Trusted Storage (ITS)** - 2 KB in data flash
- [x] **Protected Storage (PS)** - 3 KB with AES-GCM encryption
- [x] **Initial Attestation** - Device identity with 256-bit key
- [x] **Platform Services** - System reset and IOCTL

#### Bootloader
- [x] MCUboot (BL2) integration
- [x] Renesas MCUboot fork (v2.1.0+renesas.3) with RA6M4 optimizations
- [x] RSA-3072 image signing
- [x] Swap-based OTA firmware updates
- [x] 384 KB scratch area for upgrades

#### Storage
- [x] Dual flash driver (code flash + data flash)
- [x] CMSIS Flash driver for Driver_FLASH0 (code) and Driver_FLASH1 (data)
- [x] Separate device IDs for code/data flash
- [x] 64-byte sector support for data flash

#### Renesas FSP Integration
- [x] Modular FSP library integration
- [x] BSP module (clocks, GPIO, system initialization)
- [x] UART module (SCI channel for debug output)
- [x] Flash HP module (code and data flash operations)
- [x] CMSIS 6 compatibility layer

### Key Files Added/Modified

#### Platform Configuration
- `config.cmake` - TF-M and service configuration
- `flash_layout.h` - Memory layout with MCUboot slots
- `region_defs.h` - Memory region boundaries  
- `CMakeLists.txt` - Build system integration

#### Hardware Abstraction
- `target_cfg.c` - TrustZone SAU/MPC/PPC configuration
- `target_cfg.h` - Target configuration header
- `tfm_hal_platform.c` - Platform HAL with system reset and IOCTL
- `tfm_peripherals_def.c` - Peripheral definitions
- `startup_ra6m4.c` - TF-M compatible vector table and startup

#### CMSIS Drivers
- `cmsis_drivers/Driver_Flash.c` - Unified flash driver (code + data)
- `cmsis_drivers/Driver_USART.c` - UART driver for logging

#### FSP Integration
- `cmake/modules/RenesasFSP.cmake` - FSP module integration
- `bsp_init_stub.c` - FSP BSP initialization stubs

#### Documentation
- `README.md` - Complete port documentation
- `BUILD.md` - Detailed build instructions  
- `CHANGELOG.md` - This file

### Build Results

With all services enabled and INFO level logging:

| Component | Flash Used | Flash Allocated | Utilization | RAM Used |
|-----------|------------|-----------------|-------------|----------|
| BL2 (MCUboot) | 25 KB | 128 KB | 19.1% | 19 KB |
| TF-M Secure | 121 KB | 128 KB | 92.1% | 47 KB |

**Note**: Logging adds ~2.6 KB to TF-M Secure for UART driver and stdio infrastructure. To disable logging and reduce size, set `TFM_PARTITION_LOG_LEVEL=TFM_PARTITION_LOG_LEVEL_SILENCE` and `TFM_SPM_LOG_LEVEL=TFM_SPM_LOG_LEVEL_SILENCE` in config.cmake.

### Memory Map

#### Code Flash (1 MB)
```
0x00000000 - 0x0001FFFF: BL2 Bootloader (128 KB, 25 KB used)
0x00020000 - 0x0003FFFF: Secure Primary Slot (128 KB, 121 KB used)
0x00040000 - 0x0005FFFF: Non-Secure Primary Slot (128 KB)
0x00060000 - 0x0007FFFF: Secure Secondary Slot (128 KB) - OTA
0x00080000 - 0x0009FFFF: NS Secondary Slot (128 KB) - OTA
0x000A0000 - 0x000FFFFF: Scratch Area (384 KB) - MCUboot swap
```

#### Data Flash (8 KB at 0x08000000)
```
0x08000000 - 0x080007FF: OTP/NV Counters (2 KB)
0x08000800 - 0x080013FF: Protected Storage (3 KB, encrypted)
0x08001400 - 0x08001BFF: Internal Trusted Storage (2 KB)
```

### Issues Resolved

1. **MbedTLS Threading Error**
   - **Issue**: MbedTLS CMake tried to find pthreads on bare-metal build
   - **Solution**: Disabled pthread search in config.cmake
   - **Files**: `config.cmake:129-135`

2. **CMSIS 6 Compatibility**
   - **Issue**: FSP uses CMSIS 6, TF-M expects CMSIS 5
   - **Solution**: Version detection and compatibility shims
   - **Files**: `platform/include/cmsis_override.h`

3. **Vector Table Size**
   - **Issue**: RA6M4 has 496 interrupts requiring 2048 bytes
   - **Solution**: Increased S_CODE_VECTOR_TABLE_SIZE to 0x800
   - **Files**: `region_defs.h:33`

4. **Storage HAL Configuration**
   - **Issue**: Missing ITS/PS flash area definitions
   - **Solution**: Added TFM_HAL_ITS_* and TFM_HAL_PS_* defines
   - **Files**: `flash_layout.h:119-130`

5. **Platform Service Functions**
   - **Issue**: Undefined tfm_platform_hal_system_reset and tfm_platform_hal_ioctl
   - **Solution**: Implemented in tfm_hal_platform.c
   - **Files**: `tfm_hal_platform.c:60-78`

6. **BL2 Memory Regions**
   - **Issue**: Missing BL2-specific memory region definitions
   - **Solution**: Added BL2_CODE_START, BL2_DATA_SIZE, etc.
   - **Files**: `region_defs.h:75-87`

7. **MCUboot Integration**
   - **Issue**: Generic MCUboot lacks RA6M4 optimizations
   - **Solution**: Auto-download Renesas MCUboot fork
   - **Files**: `config.cmake:97-99`

8. **UART Driver Not Linked**
   - **Issue**: UART driver compiled but not linked (logging was silenced, causing linker garbage collection)
   - **Root Cause**: TFM_PARTITION_LOG_LEVEL and TFM_SPM_LOG_LEVEL were set to SILENCE
   - **Solution**: Enabled INFO level logging to activate UART driver chain
   - **Files**: `config.cmake:25-28`
   - **Result**: UART driver now functional (+2.6 KB for logging infrastructure)

### Testing Status

- [x] Clean build with all services
- [x] MCUboot integration and signing
- [x] Memory layout validation
- [x] Binary size verification
- [ ] Hardware testing on EK-RA6M4
- [ ] PSA API functional tests
- [ ] OTA firmware update testing

### Known Limitations

1. Software floating-point only (no FPU optimization)
2. Crypto acceleration stubbed (software only)
3. Flash-based NV counters (should use hardware for production)
4. Development provisioning (dummy keys)
5. Debug authentication set to CHIP_DEFAULT

### Dependencies

- **ARM GNU Toolchain**: 13.2.1 or later
- **CMake**: 3.21 or later
- **Python**: 3.8 or later with jinja2, pyyaml, click, cbor2, cryptography, intelhex
- **Renesas FSP**: Generated from e² studio or FSP Configurator
- **MbedTLS**: 3.6.3 (auto-downloaded)
- **MCUboot**: 2.1.0+renesas.3 from Renesas fork (auto-downloaded)

### Future Work

- [ ] Enable hardware crypto acceleration
- [ ] Hardware NV counter implementation
- [ ] Secure debug authentication configuration
- [ ] FPU optimization (hard-float build)
- [ ] Power management integration
- [ ] Additional FSP peripheral drivers
- [ ] PSA Crypto API test suite execution
- [ ] OTA update testing and validation
- [ ] Hardware bring-up and validation
- [ ] PSA Certified evaluation

### References

- [TF-M Documentation](https://tf-m-user-guide.trustedfirmware.org/)
- [PSA Certified](https://www.psacertified.org/)
- [Renesas RA6M4](https://www.renesas.com/ra6m4)
- [Renesas FSP](https://github.com/renesas/fsp)
- [Renesas MCUboot](https://github.com/renesas/mcuboot)

---

**Port Status**: ✅ Build Verified | ⏳ Hardware Testing Pending  
**Last Updated**: October 27, 2025
