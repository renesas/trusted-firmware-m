# TF-M on Renesas RA

**Start at `fsp_cmake/GETTING_STARTED.md`, in the companion repository.** It carries every
procedure — prerequisites, clone, the e2 studio build these ports read their layout from,
TrustZone partitioning, build, flash, RTT and the test suites — written once for both parts and
both toolchains. The pages here are part cards: the facts of one device and the options that
sit beside its `config.cmake`.

| Platform | Part card | Status |
|---|---|---|
| `renesas/ra6m5` | [ra6m5/QUICKSTART.md](ra6m5/QUICKSTART.md) | supported, Arm GNU and IAR |
| `renesas/ra8m2` | [ra8m2/QUICKSTART.md](ra8m2/QUICKSTART.md) | supported, Arm GNU and IAR. The second core is unused; the port is single-core |
| `renesas/ra6e1` | — | reference only. It is the port the two supported parts derive from, and it is not maintained against the current build |

The crypto accelerators live outside this directory, one per engine:
[../../accelerator/renesas/sce9](../../accelerator/renesas/sce9) for RA6M5 and
[../../accelerator/renesas/rsip_e50d](../../accelerator/renesas/rsip_e50d) for RA8M2. Both
reach the FSP `*_ALT` sources rather than the FSP PSA crypto module.

[common/](common/) holds what the parts share. `common/check_ofs.py` is the option-memory brick
guard: each option word must be linked into its own `MEMORY` region, because one coalesced load
segment zero-fills the memory between the words and writes values nobody chose. That failure
made two boards un-erasable, and it is invisible in the srec. Every part's `CMakeLists.txt`
runs the guard over `bl2` and `tfm_s` on every build; `fsp_cmake/GETTING_STARTED.md` §6 has the
form to run by hand before flashing.

**The shipped configuration is a development configuration.** Sample signing keys, dummy
provisioning, inactive anti-rollback, non-secure clock registers and an unenforced debug
lifecycle. `fsp_cmake/README.md` has the list and `fsp_cmake/DESIGN.md` §13 says how to address
each item.
