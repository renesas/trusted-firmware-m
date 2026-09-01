# RA6E1 non-secure application (NSPE build)

The split SPE/NSPE build. Two CMake projects, configured one after the other: the secure
build produces an installed `api_ns/` tree, and this project is configured against it.

## Why the split exists

The single-build arrangement — `tfm_ns` defined inside the SPE build — was only ever viable
while the non-secure image made *no PSA calls*. `tfm_api_ns` is created by the NSPE build
from the installed tree and does not exist in an SPE build, so linking it there yields
`cannot find -ltfm_api_ns`. The moment NS code calls a PSA API, the split is required.

It is also the prerequisite for running the official `tf-m-tests` regression suite, which
is built as an NSPE application against exactly this interface.

## Building

```sh
# 1. Secure side, as before
cmake -S . -B build_ra6e1 -GNinja \
      -DTFM_PLATFORM=renesas/ra6e1 \
      -DFSP_S_APP_DIR=<...>/ra6e1_secure \
      -DFSP_BL2_APP_DIR=<...>/ra6e1_mcuboot \
      -DFSP_NS_APP_DIR=<...>/ra6e1_nonsecure
cmake --build build_ra6e1

# 2. Install it. This is the step that produces api_ns/ - the SPE build directory itself
#    is not a valid CONFIG_SPE_PATH.
cmake --install build_ra6e1

# 3. Non-secure side
cmake -S platform/ext/target/renesas/ra6e1/ns_app -B build_ra6e1_ns -GNinja \
      -DCONFIG_SPE_PATH=<abs>/build_ra6e1/api_ns \
      -DTFM_TOOLCHAIN_FILE=<abs>/build_ra6e1/api_ns/cmake/toolchain_ns_GNUARM.cmake
cmake --build build_ra6e1_ns
```

`FSP_NS_APP_DIR` is not needed in step 3: the SPE records which project it was built
against and this build defaults to it. Passing a *different* one is a hard error, not a
warning — the NS image is placed by that project's generated `memory_regions.ld`, so a
mismatched project produces an image that links cleanly and boots nowhere.

## Output

| File | Where |
|---|---|
| `tfm_ns.axf` / `.bin` | `build_ra6e1_ns/bin/` |
| `tfm_ns_signed.bin` | `build_ra6e1_ns/bin/` |

The combined `tfm_s_ns_signed.bin` that TF-M's NSPE rules would otherwise build is
deliberately excluded — see the comment in `CMakeLists.txt`. It is byte-for-byte the two
signed images end to end, nothing consumes it at runtime, and as an extra row in the debug
session's program list it invites an address mistake that silently overwrites the secure
slot and the NSC veneers.

Signing moves to this side of the split. The keys, `imgtool` and the layout files come from
`api_ns/image_signing/`, exported by the SPE, so both halves are signed with the same keys
BL2 verifies against. The SPE no longer signs the NS image; `bl2.srec` and
`tfm_s_signed.bin` still come from the secure build.

## What runs

`main()` → `hal_entry()`, intercepted by `-Wl,--wrap=hal_entry` into `__wrap_hal_entry()`,
which initialises RTT, prints a banner, runs the service smoke test, then calls the real
`hal_entry()`. For a non-secure build FSP's `hal_entry()` is empty, so `main()` returns and
`Reset_Handler` parks in a one-instruction spin — that is the normal end of the program,
not a fault.

Results are on RTT channel 0 and in `g_ns_test_results`, readable from a debugger. Set
`-DRA6E1_NS_SERVICE_TESTS=OFF` for a plain FSP application with no PSA calls.
