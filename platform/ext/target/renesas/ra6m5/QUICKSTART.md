# RA6M5 — platform card

TF-M platform `renesas/ra6m5`. **Every procedure lives in the companion repository**: clone,
e2 build, TrustZone partitioning, build, flash, RTT and the test suites are all in
`fsp_cmake/GETTING_STARTED.md`, written once for both parts and both toolchains. This page is
the part's own facts and the options that sit beside `config.cmake`.

| | |
|---|---|
| Device | R7FA6M5BH3CFC, Cortex-M33 |
| Board | CK-RA6M5 V2 |
| Crypto engine | SCE9 |
| Secure slot | 524,288 B |
| Non-secure slot | 458,752 B |
| Generated projects | `fsp_cmake/ra6m5_{gcc,iar}_{mcuboot,secure,nonsecure}` |
| Debug launches | `fsp_cmake/ra6m5_gcc_nonsecure/`, all nineteen of them |

The two toolchain solutions differ in pin configuration: the GCC solution selects `RA6M5_2 CK`
and the IAR solution `RA6M5 CK`, which changes the security attribution of 16 pins. Both have
passed on hardware.

## Platform options

Defaults, constraints and the flash each one costs are in `fsp_cmake/CONFIGURATION.md` §6. This
is what each does on this part:

| Option | Effect |
|---|---|
| `CRYPTO_HW_ACCELERATOR` | SCE9 ciphers, hashes and ECC. `OFF` reverts to software |
| `RA6M5_FSP_MBEDTLS` | builds the TF-M crypto from the FSP Mbed TLS tree, through [cmake/fsp_mbedtls.cmake](cmake/fsp_mbedtls.cmake) |
| `RA6M5_STDOUT_RTT` | console over SEGGER RTT rather than a UART |
| `RA6M5_RTT_BLOCKING` | RTT waits for the host instead of dropping output. Test builds only |
| `RA6M5_BL2_HALT_AT_MAIN` | BL2 spins at `main()` so registers can be read before MCUboot runs |
| `RA6M5_ORPHAN_CHECK_STRICT` | an orphan section becomes a link failure. Intended for CI |
| `RA6M5_NS_IN_SPE_BUILD` | builds the non-secure image inside the secure build |

## What the engine accelerates

AES in ECB, CBC and CTR at 128, 192 and 256 bits, AES-GCM, CMAC, SHA-256, ECP and ECDSA, and
RSA. BL2 hashes images on the engine as well. AES-CCM is built from software deliberately,
because the FSP SCE9 implementation caps associated data below what Protected Storage needs.
Deterministic ECDSA, P-521 and Curve25519 are off, because the engine does not implement them.
`fsp_cmake/KNOWN_LIMITATIONS.md` §2 is the full statement, including what is untested.

The accelerator reaches the FSP `*_ALT` sources through
[platform/ext/accelerator/renesas/sce9](../../../accelerator/renesas/sce9), not through the FSP
PSA crypto module. Three modules are deliberately never built from the e2 projects — the FSP
crypto stack, `rm_psa_crypto` and `rm_mcuboot_port` — and `FSP_MODULES_NEVER_BUILT` in
[CMakeLists.txt](CMakeLists.txt) enforces that.

## Files in this directory

| | |
|---|---|
| [config.cmake](config.cmake) | the platform defaults, commented throughout |
| [CMakeLists.txt](CMakeLists.txt) | the FSP module declarations and the option-memory brick guard |
| [cmake/modules/](cmake/modules/) | one file per FSP module. [fsp_module_common.cmake](cmake/modules/fsp_module_common.cmake) documents the contract they work inside |
| [ra6m5_bl2.ld](ra6m5_bl2.ld), [ra6m5_bl2.icf](ra6m5_bl2.icf) | the bootloader link, including one `MEMORY` region per option word |
| [ra6m5_layout_checks.c](ra6m5_layout_checks.c) | the build-time assertions on the generated layout |
| [ns_app/](ns_app/) | the non-secure application build |
| [ns/tfm_service_tests.c](ns/tfm_service_tests.c) | the smoke test the application runs across the secure boundary |

## Where to read next

| Topic | Document |
|---|---|
| every procedure | `fsp_cmake/GETTING_STARTED.md` |
| the layout, the TrustZone boundary values, the e2 project requirements | `fsp_cmake/RA6M5_SOLUTION.md` |
| why the port is built this way | `fsp_cmake/DESIGN.md` |
| what is untested or known broken | `fsp_cmake/KNOWN_LIMITATIONS.md` |
| what a generated project must supply | `fsp_cmake/E2_PROJECT_REQUIREMENTS.md` |
| adding an FSP module | `fsp_cmake/ADDING_FSP_MODULES.md` |
