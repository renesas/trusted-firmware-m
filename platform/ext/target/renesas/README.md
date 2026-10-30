# TF-M on Renesas RA

**Start at `ra_fsp_cmake_tfm/GETTING_STARTED.md`** in the companion repository: every procedure,
both parts, both toolchains. The pages here are part cards.

| Platform | Part card | Status |
|---|---|---|
| `renesas/ra6m5` | [ra6m5/QUICKSTART.md](ra6m5/QUICKSTART.md) | supported, Arm GNU and IAR |
| `renesas/ra8m2` | [ra8m2/QUICKSTART.md](ra8m2/QUICKSTART.md) | supported, Arm GNU and IAR. The second core is unused; the port is single-core |
| `renesas/ra6e1` | — | reference only. It is the port the two supported parts derive from, and it is not maintained against the current build |

The crypto accelerators live outside this directory, one per engine:
[../../accelerator/renesas/sce9](../../accelerator/renesas/sce9) for RA6M5 and
[../../accelerator/renesas/rsip_e50d](../../accelerator/renesas/rsip_e50d) for RA8M2. Both
reach the FSP `*_ALT` sources rather than the FSP PSA crypto module.

[common/](common/) holds shared code, including `check_ofs.py`, the option-memory brick guard
every build runs over `bl2` and `tfm_s` (mechanism: `DESIGN.md` §8.4; manual check:
`GETTING_STARTED.md` §6).

**The shipped configuration is for development only**: `ra_fsp_cmake_tfm/README.md` lists why and
`DESIGN.md` §13 how to harden it.
