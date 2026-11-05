#!/usr/bin/env python3
"""
check_ofs.py - brick guard for Renesas RA option-setting (OFS) memory.

Having OFS in an image is expected. What bricks the part is a PT_LOAD segment spanning more than
one option word, because its gaps are zero-filled and programmed. A debugger flashes by program
header, so it writes 0x00000000 across every defined option register and reserved byte in the
span. Two EK-RA6M4 boards were destroyed this way, with no recovery known: GNU ld coalesced
their OFS words into one segment spanning 0x0100A100-0x0100A284, which covered eight defined
option registers, BPS and PBPS among them (DESIGN.md 8.4, DECISIONS D133). FSP's fsp_gen.ld
avoids this by giving each word its own MEMORY region, and so its own small PT_LOAD segment.
This guard enforces the same.

---------------------------------------------------------------------------------------------
The OFS region is per part and must be given. RA6E1, RA6M4 and RA6M5 share
0x0100A100-0x0100A2CF (--ra6-default). RA8M2's option memory is at 0x02c9f040+ with OTP blocks
from 0x02e07600+, in five windows (ra8m2/CMakeLists.txt, RA8M2_OFS_REGIONS).

This guard checks spanning, not whether the addresses belong to the device. A window that
misses the image's OFS segments reports CLEAN and PASS, a false negative that reads as
verification. A discrete OFS word placed at another part's address also passes. --region is
therefore required unless --ra6-default is given, and an image expected to carry OFS can
demand it with --require-segments.
---------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------
The flat .bin is a second route to the brick, and the ELF check does not cover it. An ELF with
discrete per-word OFS segments passes this guard, yet `objcopy -O binary` still bricks the
part. objcopy flattens from the lowest load address to the highest and zero-fills everything
between. RA8M2's bl2, whose last OFS word sits at 0x02c9f124, becomes a 13,234,472-byte bl2.bin
spanning 0x02000000-0x02c9f128; flashed at the MRAM base, its zero fill lands on the option
memory.

The .hex and .srec are record-based and skip the gaps, so they are safe; only the flat binary
coalesces. --emit-safe-bin writes a flat binary without the OFS segments. --check-flat-bin
looks for the sibling .bin and fails when the ELF carries OFS but the .bin is sized as if it
spans to it.
---------------------------------------------------------------------------------------------

Usage:
    check_ofs.py --region 0xSTART:0xEND [--region ...] [--require-segments] ELF [ELF ...]
    check_ofs.py --ra6-default [--require-segments] ELF [ELF ...]

    --region S:E        OFS address window, END exclusive. Repeatable; RA8M2 needs five.
    --ra6-default       use 0x0100A100:0x0100A2D0, the RA6E1/RA6M4/RA6M5 map.
    --require-segments  FAIL if an image has no OFS segment. Use for BL2, which carries the
                        option words; a CLEAN result there means the sections were dropped
                        or the window is wrong.
    --forbid-segments   FAIL if an image has any OFS segment. Use for tfm_s, which must never
                        carry option words. Without it, a secure image that picked one up
                        passes as "1 discrete OFS segment", because the default check asks
                        only for discrete segments.
    --max-word N        largest single option word in bytes (default 0x80: RA8M2's BPS is
                        0x80, RA6M5's BPS and PBPS are 0x10, RA6E1's and RA6M4's are 0xC).
                        Take it from the part's own region_defs.h. Too small is not safe
                        either: a correctly placed BPS group then reads as SPANNING and
                        fails the build.
    --check-flat-bin    also FAIL if the sibling <image>.bin spans from the image base into
                        an OFS region. The ELF can be correct and the .bin still brick the
                        part.
    --emit-safe-bin OUT before the checks, write a flat binary of the image's non-OFS
                        PT_LOAD segments to OUT. Takes exactly one image.
    --quarantine        on FAIL, rename the image and every flashable sibling (.axf .bin
                        .elf .hex .srec) to *.UNSAFE. A failing build still leaves those
                        files on disk, and the committed debug launches flash bl2.elf with
                        ATTR_BUILD_BEFORE_LAUNCH disabled. Without the rename, the next
                        launch programs the image the guard rejected.

Exit: 0 = safe; 1 = DO NOT FLASH: a spanning or forbidden OFS segment, a required segment
      missing, a missing image, or a flat .bin that reaches OFS or is missing; 2 = tool error.
"""
import os, re, shutil, subprocess, sys


def die(msg):
    """Report a tool error and exit 2, never 1.

    Exit 1 is this guard's DO NOT FLASH verdict. A missing readelf or a bad argument must not
    read as a finding about the image.
    """
    print(msg, file=sys.stderr)
    raise SystemExit(2)

RA6_REGION = (0x0100A100, 0x0100A2D0)
READELF    = os.environ.get("READELF", "arm-none-eabi-readelf")


QUARANTINE_EXTS = (".axf", ".bin", ".elf", ".hex", ".srec")


def quarantine(elf):
    """Rename the image and its flashable siblings to *.UNSAFE. Returns what moved.

    Renamed, not deleted: a rejected image is evidence. bringup/bricking_evidence holds the
    one that destroyed two EK-RA6M4 boards (D133). The rename stops a rejected image staying
    in bin/ under the name a launch expects.
    """
    stem = os.path.splitext(elf)[0]
    moved = []
    for ext in QUARANTINE_EXTS:
        src = stem + ext
        if not os.path.isfile(src):
            continue
        dest = src + ".UNSAFE"
        try:
            if os.path.exists(dest):
                os.remove(dest)
            os.replace(src, dest)
            moved.append(os.path.basename(dest))
        except OSError as e:
            print(f"  *** QUARANTINE FAILED for {os.path.basename(src)}: {e} ***")
    return moved


def parse_args(argv):
    regions, images = [], []
    max_word = 0x80
    require = False
    forbid = False
    flat = False
    emit = None
    quar = False
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--region":
            i += 1
            if i >= len(argv):
                die("ERROR: --region needs START:END")
            try:
                s, e = argv[i].split(":")
                regions.append((int(s, 0), int(e, 0)))
            except ValueError:
                die(f"ERROR: cannot parse --region {argv[i]!r}, want 0xSTART:0xEND")
        elif a == "--ra6-default":
            regions.append(RA6_REGION)
        elif a == "--require-segments":
            require = True
        elif a == "--forbid-segments":
            forbid = True
        elif a == "--quarantine":
            quar = True
        elif a == "--check-flat-bin":
            flat = True
        elif a == "--emit-safe-bin":
            i += 1
            if i >= len(argv):
                die("ERROR: --emit-safe-bin needs an output path")
            emit = argv[i]
        elif a == "--max-word":
            i += 1
            max_word = int(argv[i], 0)
        elif a.startswith("--"):
            die(f"ERROR: unknown option {a}")
        else:
            images.append(a)
        i += 1

    if not regions:
        die("ERROR: no OFS region given. Pass --region 0xSTART:0xEND (repeatable), or\n"
                 "       --ra6-default for RA6E1/RA6M4/RA6M5. This is deliberately not\n"
                 "       optional: a guard pointed at the wrong window reports PASS on an\n"
                 "       unsafe image. See the note at the top of this file.")
    if not images:
        die("ERROR: no images given")
    for s, e in regions:
        if e <= s:
            die(f"ERROR: region 0x{s:X}:0x{e:X} is empty or reversed")
    if require and forbid:
        die("ERROR: --require-segments and --forbid-segments are opposites")
    return regions, images, max_word, require, forbid, flat, emit, quar


def all_load_segments(elf):
    """Every PT_LOAD with filesz > 0, as (file_offset, vaddr, paddr, filesz).

    objcopy -O binary lays out the flat image by physical address, so the flat-binary code
    below uses paddr. The OFS check uses vaddr, by which the option words are addressed; for
    OFS segments the two are equal (see the readelf output in DESIGN.md 8.4).
    """
    try:
        out = subprocess.run([READELF, "-l", elf], capture_output=True, text=True,
                             check=True).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        die(f"ERROR: readelf failed on {elf}: {e}")
    segs = []
    for m in re.finditer(
            r"LOAD\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)"
            r"\s+0x([0-9a-fA-F]+)",
            out):
        offset, vaddr, paddr, filesz = (int(m.group(1), 16), int(m.group(2), 16),
                                        int(m.group(3), 16), int(m.group(4), 16))
        if filesz:
            segs.append((offset, vaddr, paddr, filesz))
    return segs


def in_regions(addr, size, regions):
    return any(addr < e and (addr + size) > s for s, e in regions)


def load_segments(elf, regions):
    """PT_LOAD segments (vaddr, filesz) overlapping any region."""
    return [(v, f) for (_o, v, _p, f) in all_load_segments(elf)
            if in_regions(v, f, regions)]


def emit_safe_bin(elf, regions, outpath):
    """Write a flat binary from the ELF's PT_LOAD segments, excluding the OFS ones.

    Deliberately address-driven. `objcopy -O binary --remove-section=.option_setting*` works
    only on GNU output. IAR's ILINK renames sections to opaque positional names taken from the
    .icf block order: the RA6M5 bl2 puts its three option words in sections P1, P7 and P11.
    A name pattern then matches nothing, objcopy copies everything, and the result is the full
    multi-megabyte brick with no error reported. The names also move whenever the .icf block
    order changes, so hardcoding them does not help.

    Addresses do not move: they come from the device's option memory map, which --region
    already describes for the check. This function therefore emits the safe binary from the
    program headers directly, with no objcopy on either toolchain.
    """
    segs = all_load_segments(elf)
    keep = [(o, p, f) for (o, v, p, f) in segs if not in_regions(v, f, regions)]
    if not keep:
        die(f"ERROR: {elf} has no loadable content outside the OFS regions")

    lo = min(p for (_o, p, _f) in keep)
    hi = max(p + f for (_o, p, f) in keep)
    image = bytearray(hi - lo)
    with open(elf, "rb") as fh:
        for (off, paddr, filesz) in keep:
            fh.seek(off)
            chunk = fh.read(filesz)
            if len(chunk) != filesz:
                die(f"ERROR: {elf} truncated reading segment at offset 0x{off:X}")
            image[paddr - lo:paddr - lo + filesz] = chunk

    with open(outpath, "wb") as fh:
        fh.write(image)

    dropped = len(segs) - len(keep)
    print(f"  {os.path.basename(outpath):<14} {len(image)} bytes, "
          f"0x{lo:08X}-0x{hi:08X}, {dropped} OFS segment(s) excluded")


def flat_bin_verdict(elf, regions):
    """Check the flat binary on disk, not what the ELF implies.

    `objcopy -O binary` spans lowest paddr to highest and zero-fills the gaps, so an ELF whose
    OFS words sit far above the image yields a flat binary tens of MB long whose fill covers
    the option memory. The platform build regenerates the .bin with --emit-safe-bin
    (emit_safe_bin()), and this checks that it did: the safe binary is the non-OFS span, and
    anything longer still reaches the option words.

    A missing .bin is a failure, never a pass: "I could not find it" must not read as "it is
    fine". The guard target depends on the bin target (bl2/ is configured before platform/,
    so it exists).

    Returns (status, detail).
    """
    binpath = os.path.splitext(elf)[0] + ".bin"
    segs = all_load_segments(elf)
    ofs = [(v, p, f) for (_o, v, p, f) in segs if in_regions(v, f, regions)]
    if not segs or not ofs:
        return "n/a", (binpath, None, None)

    lo = min(p for (_o, _v, p, _f) in segs)
    hi_all = max(p + f for (_o, _v, p, f) in segs)
    hi_other = max([p + f for (_o, v, p, f) in segs
                    if not in_regions(v, f, regions)] or [lo])
    safe_size = hi_other - lo
    if hi_all <= hi_other:
        return "n/a", (binpath, None, None)

    if not os.path.isfile(binpath):
        return "missing", (binpath, None, safe_size)
    actual = os.path.getsize(binpath)
    if actual > safe_size:
        return "spanning", (binpath, actual,
                            (lo, hi_all, hi_other, actual - safe_size, safe_size))
    return "ok", (binpath, actual, safe_size)


def main():
    if not shutil.which(READELF):
        die(f"ERROR: readelf '{READELF}' not on PATH (set $READELF)")
    regions, images, max_word, require, forbid, flat, emit, quar = parse_args(sys.argv[1:])

    # Emit first, then check what was emitted, when one invocation is asked to do both.
    # The builds do not ask: all three parts run --emit-safe-bin and --check-flat-bin as
    # separate targets, sequenced by add_dependencies. That is not the weaker arrangement -
    # --check-flat-bin re-reads the file from disk, which is the artifact that gets flashed,
    # and the dependency makes it the file the emit target just wrote.
    if emit:
        if len(images) != 1:
            die("ERROR: --emit-safe-bin takes exactly one image")
        emit_safe_bin(images[0], regions, emit)

    where = ", ".join(f"{hex(s)}-{hex(e - 1)}" for s, e in regions)
    print(f"OFS brick guard: no PT_LOAD segment across {where} may exceed {max_word} bytes")
    print("(a span => zero-filled gaps => eight defined option registers written to 0 "
          "=> two boards bricked, no recovery known. DESIGN.md 8.4)\n")

    bad = 0
    for elf in images:
        name = os.path.basename(elf)
        was = bad
        if not os.path.isfile(elf):
            print(f"  {name:<14} MISSING")
            bad += 1
            continue
        segs = load_segments(elf, regions)
        spanning = [(v, s) for (v, s) in segs if s > max_word]
        if spanning:
            bad += 1
            det = ", ".join(f"0x{v:08X}+{s}B" for v, s in spanning)
            print(f"  {name:<14} *** SPANNING OFS SEGMENT - DO NOT FLASH *** ({det})")
        elif segs and forbid:
            bad += 1
            det = ", ".join(f"0x{v:08X}({s}B)" for v, s in segs)
            print(f"  {name:<14} *** OFS SEGMENT IN AN IMAGE THAT MUST CARRY NONE *** "
                  f"({det}; --forbid-segments)")
        elif not segs:
            if require:
                bad += 1
                print(f"  {name:<14} *** NO OFS SEGMENTS, BUT --require-segments *** "
                      f"(sections dropped, or the region is wrong)")
            else:
                why = "and none allowed" if forbid else "none required"
                print(f"  {name:<14} CLEAN (no OFS segments, {why})")
        else:
            det = ", ".join(f"0x{v:08X}({s}B)" for v, s in segs)
            print(f"  {name:<14} OK - {len(segs)} discrete OFS segment(s): {det}")

        if flat:
            status, (binpath, actual, extra) = flat_bin_verdict(elf, regions)
            bn = os.path.basename(binpath)
            if status == "spanning":
                lo, hi_all, hi_other, fill, safe = extra
                bad += 1
                print(f"  {bn:<14} *** FLAT BINARY REACHES OFS - DO NOT FLASH *** "
                      f"({actual} bytes, 0x{lo:08X}-0x{hi_all:08X}; {fill} bytes of zero "
                      f"fill past 0x{hi_other:08X}. Safe size is {safe} - re-emit with "
                      f"--emit-safe-bin, which is what the build uses, or by hand with "
                      f"objcopy --remove-section=.option_setting*)")
            elif status == "missing":
                bad += 1
                print(f"  {bn:<14} *** FLAT BINARY MISSING, CANNOT VERIFY *** "
                      f"(expected {extra} bytes; a .bin this guard cannot see is a "
                      f"failure, not a pass)")
            elif status == "ok":
                print(f"  {bn:<14} flat .bin OK - {actual} bytes, OFS sections stripped")
            else:
                print(f"  {'':<14} flat .bin: n/a (no OFS above the image)")

        if quar and bad > was:
            moved = quarantine(elf)
            if moved:
                print(f"  {'':<14} quarantined: {', '.join(moved)}")

    print()
    if bad:
        print(f"FAIL: {bad} image(s).")
        print("An image that must carry no option words has some: the sections were placed")
        print("in an image other than BL2. Check which object pulled them in.")
        print("A spanning segment means the linker coalesced option words into one gap-filled")
        print("PT_LOAD; flashing it writes 0 to eight defined option registers. That is what")
        print("destroyed two EK-RA6M4 boards on 2026-07-21, and no recovery is known (D133).")
        print("Give each OFS word its OWN region (FSP's fsp_gen.ld does; so do the ra*_bl2.ld")
        print("and ra*_bl2.icf in each platform). A missing-but-required segment means the")
        print("option words are not in the image, or this guard was given the wrong window.")
        print("A spanning FLAT BINARY is the same brick by another route: the ELF can be")
        print("correct and `objcopy -O binary` still zero-fills from the image base up to")
        print("the option words. Flash the .elf or the .hex, which skip the gaps, and do")
        print("not ship the .bin for an image that carries OFS.")
        print("DESIGN.md 8.4.")
        return 1
    print("PASS: OFS is in discrete per-word segments within the given region(s).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
