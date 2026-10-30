# RA8M2 — platform card

TF-M platform `renesas/ra8m2`. Every procedure is in `ra_fsp_cmake_tfm/GETTING_STARTED.md`; this
card holds the part's facts and platform options.

| | |
|---|---|
| Device | R7KA8M2JFLCAC, Cortex-M85 |
| Board | EK-RA8M2 |
| Crypto engine | RSIP-E50D, not SCE9 |
| Secure slot | 294,912 B |
| Non-secure slot | 163,840 B |
| Generated projects | `ra_fsp_cmake_tfm/ra8m2_{gcc,iar}_mcuboot`, `..._{gcc,iar}_CPU0_secure`, `..._{gcc,iar}_CPU0_nonsecure` |
| Debug launches | `ra_fsp_cmake_tfm/ra8m2_gcc_mcuboot/`, all seventeen of them, under both toolchains |

All RA8M2 launches live in the bootloader project. The port is single-core. The GCC projects
target board `ra8m2_ek` and the IAR projects `ra8m2_ek_v2`; regenerating one set for the other
board changes the BSP.

## Platform options

Defaults and costs: `ra_fsp_cmake_tfm/CONFIGURATION.md`.

| Option | Effect |
|---|---|
| `CRYPTO_HW_ACCELERATOR` | RSIP-E50D ciphers, hashes and ECC. `OFF` reverts to software |
| `RA8M2_FSP_MBEDTLS` | builds the TF-M crypto from the FSP Mbed TLS tree, through [cmake/fsp_mbedtls.cmake](cmake/fsp_mbedtls.cmake) |
| `RA8M2_STDOUT_RTT` | console over SEGGER RTT. Leave `ON`: `OFF` does not build as shipped (CONFIGURATION §9) |
| `RA8M2_RTT_BLOCKING` | RTT waits for the host instead of dropping output. Test builds only |
| `RA8M2_BL2_HALT_AT_MAIN` | BL2 spins at `main()` so registers can be read before MCUboot runs |
| `RA8M2_ORPHAN_CHECK_STRICT` | an orphan section becomes a link failure. Intended for CI |
| `RA8M2_NS_IN_SPE_BUILD` | builds the non-secure image inside the secure build |

## What is different about this part

**MRAM, no data flash:** the driver is `r_mram`, storage is an emulated data-flash partition, and
MCUboot alignment follows the MRAM programming size. **Image-0 slot order is reversed** (secure
secondary below primary, so `boot_rsp` offset `0x68000` is the primary); account for it in
anything that reads `boot_rsp`.

**The signed image fills the slot; the code budget is the secure region**, about 76 KB spare
(`CONFIGURATION.md` §6).

**RSA-2048 key generation (plaintext format) is unavailable**, the single PSA Arch crypto failure
here. Engine coverage otherwise matches SCE9; details: `KNOWN_LIMITATIONS.md` §2.

The accelerator reaches the FSP `*_ALT` sources through
[platform/ext/accelerator/renesas/rsip_e50d](../../../accelerator/renesas/rsip_e50d);
`FSP_MODULES_NEVER_BUILT` in [CMakeLists.txt](CMakeLists.txt) keeps the FSP crypto stack,
`rm_psa_crypto` and `rm_mcuboot_port` out.

## Files in this directory

| | |
|---|---|
| [config.cmake](config.cmake) | platform defaults |
| [CMakeLists.txt](CMakeLists.txt) | FSP module declarations, brick guard |
| [cmake/modules/](cmake/modules/) | one file per FSP module; contract in [fsp_module_common.cmake](cmake/modules/fsp_module_common.cmake) |
| [ra8m2_bl2.ld](ra8m2_bl2.ld), [ra8m2_bl2.icf](ra8m2_bl2.icf) | bootloader link, one `MEMORY` region per option word |
| [ra8m2_ddsc.c](ra8m2_ddsc.c) | RA8M2-only `gp_ddsc_*` definitions |
| [ra8m2_layout_checks.c](ra8m2_layout_checks.c) | build-time layout assertions |
| [ns_app/](ns_app/), [ns/tfm_service_tests.c](ns/tfm_service_tests.c) | non-secure application and its smoke test |

## Where to read next

Layout and RDPM values: `ra_fsp_cmake_tfm/RA8M2_SOLUTION.md`. Limitations:
`KNOWN_LIMITATIONS.md`. Generated-project contract: `E2_PROJECT_REQUIREMENTS.md`. FSP modules:
`ADDING_FSP_MODULES.md`. Design: `DESIGN.md`.
