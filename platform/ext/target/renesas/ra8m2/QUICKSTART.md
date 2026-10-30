# RA8M2 — platform card

TF-M platform `renesas/ra8m2`. **Every procedure lives in the companion repository**: clone,
e2 build, TrustZone partitioning, build, flash, RTT and the test suites are all in
`ra_fsp_cmake_tfm/GETTING_STARTED.md`, written once for both parts and both toolchains. This page is
the part's own facts and the options that sit beside `config.cmake`.

| | |
|---|---|
| Device | R7KA8M2JFLCAC, Cortex-M85 |
| Board | EK-RA8M2 |
| Crypto engine | RSIP-E50D, not SCE9 |
| Secure slot | 294,912 B |
| Non-secure slot | 163,840 B |
| Generated projects | `ra_fsp_cmake_tfm/ra8m2_{gcc,iar}_mcuboot`, `..._{gcc,iar}_CPU0_secure`, `..._{gcc,iar}_CPU0_nonsecure` |
| Debug launches | `ra_fsp_cmake_tfm/ra8m2_gcc_mcuboot/`, all seventeen of them, under both toolchains |

**Every RA8M2 launch lives in the one bootloader project**, because a launch has to belong to
some project and that is where they were put. The second core is unused: the port is
single-core.

The two toolchain solutions target different board revisions — the GCC projects select
`ra8m2_ek` and the IAR projects `ra8m2_ek_v2`. Both have passed on hardware, but regenerating
either set against the other board changes the generated BSP.

## Platform options

Defaults, constraints and the flash each one costs are in `ra_fsp_cmake_tfm/CONFIGURATION.md` §6. This
is what each does on this part:

| Option | Effect |
|---|---|
| `CRYPTO_HW_ACCELERATOR` | RSIP-E50D ciphers, hashes and ECC. `OFF` reverts to software |
| `RA8M2_FSP_MBEDTLS` | builds the TF-M crypto from the FSP Mbed TLS tree, through [cmake/fsp_mbedtls.cmake](cmake/fsp_mbedtls.cmake) |
| `RA8M2_STDOUT_RTT` | console over SEGGER RTT rather than a UART |
| `RA8M2_RTT_BLOCKING` | RTT waits for the host instead of dropping output. Test builds only |
| `RA8M2_BL2_HALT_AT_MAIN` | BL2 spins at `main()` so registers can be read before MCUboot runs |
| `RA8M2_ORPHAN_CHECK_STRICT` | an orphan section becomes a link failure. Intended for CI |
| `RA8M2_NS_IN_SPE_BUILD` | builds the non-secure image inside the secure build |

## What is different about this part

**The memory is MRAM and there is no data flash.** The flash driver is `r_mram`, the storage
area is an emulated data-flash partition, and the MCUboot write alignment follows the MRAM
programming size rather than a code-flash write unit.

**Slot order is reversed for image 0.** The secure secondary sits below the secure primary, so
a `boot_rsp` offset of `0x68000` is the primary. Image 1 is the usual way round. Anything that
reads a `boot_rsp` or computes a flash row has to account for it.

**The signed image fills the slot to within about 800 B, and that is not the code budget.**
The flat binary spans the whole secure region, so it is region-sized whatever the image costs.
The code budget is the region, and this part has about 76 KB of it spare.
`ra_fsp_cmake_tfm/CONFIGURATION.md` §6 has both measurements.

**RSA-2048 key generation in plaintext format is unavailable, and there is no switch for it.**
Every other RSA-2048 operation works. This is the single PSA Arch crypto failure on this part.
`ra_fsp_cmake_tfm/KNOWN_LIMITATIONS.md` §2 has the mechanism.

Otherwise the engine accelerates the same set as SCE9 — AES in ECB, CBC and CTR at all three
key sizes, AES-GCM, CMAC, SHA-256, ECP, ECDSA and RSA, with BL2 hashing images on the engine
too. AES-CCM is software by choice; deterministic ECDSA, P-521 and Curve25519 are off because
the engine does not implement them.

The accelerator reaches the FSP `*_ALT` sources through
[platform/ext/accelerator/renesas/rsip_e50d](../../../accelerator/renesas/rsip_e50d), which is
a different directory from the RA6 parts. Three modules are deliberately never built from the
e2 projects — the FSP crypto stack, `rm_psa_crypto` and `rm_mcuboot_port` — and
`FSP_MODULES_NEVER_BUILT` in [CMakeLists.txt](CMakeLists.txt) enforces that.

## Files in this directory

| | |
|---|---|
| [config.cmake](config.cmake) | the platform defaults, commented throughout |
| [CMakeLists.txt](CMakeLists.txt) | the FSP module declarations and the option-memory brick guard, which carries this part's five address windows |
| [cmake/modules/](cmake/modules/) | one file per FSP module. [fsp_module_common.cmake](cmake/modules/fsp_module_common.cmake) documents the contract they work inside |
| [ra8m2_bl2.ld](ra8m2_bl2.ld), [ra8m2_bl2.icf](ra8m2_bl2.icf) | the bootloader link, including one `MEMORY` region per option word |
| [ra8m2_ddsc.c](ra8m2_ddsc.c) | the `gp_ddsc_*` definitions this part needs and the RA6 parts do not |
| [ra8m2_layout_checks.c](ra8m2_layout_checks.c) | the build-time assertions on the generated layout |
| [ns_app/](ns_app/) | the non-secure application build |
| [ns/tfm_service_tests.c](ns/tfm_service_tests.c) | the smoke test the application runs across the secure boundary |

## Where to read next

| Topic | Document |
|---|---|
| every procedure | `ra_fsp_cmake_tfm/GETTING_STARTED.md` |
| the layout, the TrustZone boundary values, the e2 project requirements | `ra_fsp_cmake_tfm/RA8M2_SOLUTION.md` |
| why the port is built this way | `ra_fsp_cmake_tfm/DESIGN.md` |
| what is untested or known broken | `ra_fsp_cmake_tfm/KNOWN_LIMITATIONS.md` |
| what a generated project must supply | `ra_fsp_cmake_tfm/E2_PROJECT_REQUIREMENTS.md` |
| adding an FSP module | `ra_fsp_cmake_tfm/ADDING_FSP_MODULES.md` |
