# RA6M5 — platform card

TF-M platform `renesas/ra6m5`. Every procedure is in `ra_fsp_cmake_tfm/GETTING_STARTED.md`; this
card holds the part's facts and platform options.

| | |
|---|---|
| Device | R7FA6M5BH3CFC, Cortex-M33 |
| Board | CK-RA6M5 V2 |
| Crypto engine | SCE9 |
| Secure slot | 524,288 B |
| Non-secure slot | 458,752 B |
| Generated projects | `ra_fsp_cmake_tfm/ra6m5_{gcc,iar}_{mcuboot,secure,nonsecure}` |
| Debug launches | `ra_fsp_cmake_tfm/ra6m5_gcc_nonsecure/`, all nineteen of them |

The two toolchain solutions differ in pin configuration: the GCC solution selects `RA6M5_2 CK`
and the IAR solution `RA6M5 CK`, which changes the security attribution of 16 pins. Both have
passed on hardware.

## Platform options

Defaults and costs: `ra_fsp_cmake_tfm/CONFIGURATION.md`.

| Option | Effect |
|---|---|
| `CRYPTO_HW_ACCELERATOR` | SCE9 ciphers, hashes and ECC. `OFF` reverts to software |
| `RA6M5_FSP_MBEDTLS` | builds the TF-M crypto from the FSP Mbed TLS tree, through [cmake/fsp_mbedtls.cmake](cmake/fsp_mbedtls.cmake) |
| `RA6M5_STDOUT_RTT` | console over SEGGER RTT. Leave `ON`: `OFF` does not build as shipped (CONFIGURATION §9) |
| `RA6M5_RTT_BLOCKING` | RTT waits for the host instead of dropping output. Test builds only |
| `RA6M5_BL2_HALT_AT_MAIN` | BL2 spins at `main()` so registers can be read before MCUboot runs |
| `RA6M5_ORPHAN_CHECK_STRICT` | an orphan section becomes a link failure. Intended for CI |
| `RA6M5_NS_IN_SPE_BUILD` | builds the non-secure image inside the secure build |

## What the engine accelerates

AES (ECB, CBC, CTR, GCM), CMAC, SHA-256, ECC/ECDSA and RSA, including BL2's image hashing.
AES-CCM, deterministic ECDSA, P-521 and Curve25519 are software or off: `KNOWN_LIMITATIONS.md`
§2.

The accelerator reaches the FSP `*_ALT` sources through
[platform/ext/accelerator/renesas/sce9](../../../accelerator/renesas/sce9);
`FSP_MODULES_NEVER_BUILT` in [CMakeLists.txt](CMakeLists.txt) keeps the FSP crypto stack,
`rm_psa_crypto` and `rm_mcuboot_port` out.

## Files in this directory

| | |
|---|---|
| [config.cmake](config.cmake) | the platform defaults, commented throughout |
| [CMakeLists.txt](CMakeLists.txt) | the FSP module declarations and the option-memory brick guard |
| [cmake/modules/](cmake/modules/) | one file per FSP module. [fsp_module_common.cmake](cmake/modules/fsp_module_common.cmake) documents the contract they work inside |
| [ra6m5_bl2.ld](ra6m5_bl2.ld), [ra6m5_bl2.icf](ra6m5_bl2.icf) | the bootloader link, including one `MEMORY` region per option group |
| [ra6m5_layout_checks.c](ra6m5_layout_checks.c) | the build-time assertions on the generated layout |
| [ns_app/](ns_app/) | the non-secure application build |
| [ns/tfm_service_tests.c](ns/tfm_service_tests.c) | the smoke test the application runs across the secure boundary |

## Where to read next

Layout and RDPM values: `ra_fsp_cmake_tfm/RA6M5_SOLUTION.md`. Limitations:
`KNOWN_LIMITATIONS.md`. Generated-project contract: `E2_PROJECT_REQUIREMENTS.md`. FSP modules:
`ADDING_FSP_MODULES.md`. Design: `DESIGN.md`.
