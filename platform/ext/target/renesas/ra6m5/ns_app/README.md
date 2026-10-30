# RA6M5 non-secure application

The non-secure image is a separate CMake project, configured against the `api_ns/` tree the
secure build installs. The scripted build is `ra_fsp_cmake_tfm/GETTING_STARTED.md` §5.

By hand: configure and build the secure side with `-DFSP_*_APP_DIR` pointing at
`ra6m5_gcc_secure` and its siblings and `-DCMAKE_BUILD_TYPE=Debug`, run `cmake --install <spe>`,
then:

```sh
cmake -S platform/ext/target/renesas/ra6m5/ns_app -B <ns> -GNinja \
  -DCONFIG_SPE_PATH=<spe>/api_ns \
  -DTFM_TOOLCHAIN_FILE=<spe>/api_ns/cmake/toolchain_ns_GNUARM.cmake
cmake --build <ns>
```

The NS project defaults to the FSP project the SPE was built against; a different one is an
error.

Output: `tfm_ns.axf`, `tfm_ns_signed.bin` in `<ns>/bin/`, signed with the SPE's exported keys.
The app runs a PSA smoke test and prints to RTT channel 0; `main()` then returns into a
one-instruction spin, which is normal. `-DRA6M5_NS_SERVICE_TESTS=OFF` builds a plain FSP
application.
