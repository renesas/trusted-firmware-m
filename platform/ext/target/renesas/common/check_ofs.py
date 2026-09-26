#!/usr/bin/env python3
"""
check_ofs.py - BRICK GUARD for Renesas RA option-setting (OFS) memory.

Root cause of the two bricked EK-RA6M4 boards on 2026-07-21 (DESIGN.md 8.4): the OFS words
were placed so GNU ld coalesced them into ONE PT_LOAD segment spanning
0x0100A100-0x0100A284 with the gaps ZERO-FILLED. A debugger flashes by program header, so it
wrote 0x00000000 into the gap words - including PBPS (0x0100A1E0), the one-time Permanent
Block Protect - permanently locking the flash.

Having OFS in an image is FINE and expected. What is lethal is a LOAD segment that SPANS more
than one option word (i.e. covers gaps that get zero-filled and programmed). FSP's fsp_gen.ld
avoids this by giving each word its own MEMORY region -> a separate, tiny PT_LOAD segment.
This guard enforces the same.

---------------------------------------------------------------------------------------------
THE OFS REGION IS PER PART AND MUST BE GIVEN. Earlier revisions of this file hardcoded
RA6's 0x0100A100-0x0100A2CF and claimed "the RA6E1 option-setting map is byte-identical to the
RA6M4's ... so one copy serves both parts". That is true of RA6E1/RA6M4/RA6M5 and FALSE of
RA8M2, whose option memory is at 0x02c9f040+ with a second OTP block at 0x02e07600+.

The consequence was worse than no guard. Run against an RA8M2 bl2.axf carrying six OFS
segments, the hardcoded window matched none of them and it printed

    bl2.axf        CLEAN (no OFS segments)
    PASS: OFS (if any) is in discrete per-word segments - safe to flash.

- a false negative that reads as verification. It would also have passed the genuinely wrong
build that placed RA8M2's OFS0 at RA6M5's 0x0100A100, because that address IS inside the old
window and the segment was discrete: this guard checks SPANNING, not whether the addresses
belong to the device.

So --region is now required unless --ra6-default is given explicitly, and an image that is
expected to carry OFS can demand it with --require-segments. "I looked in the wrong place" can
no longer print as PASS.
---------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------
THE FLAT .bin IS A SECOND WAY IN, AND THE ELF CHECK DOES NOT COVER IT. An image whose ELF is
perfectly correct - discrete per-word OFS segments, this guard PASSing - still yields a lethal
`objcopy -O binary`. objcopy flattens from the lowest load address to the highest, ZERO-FILLING
everything between, so RA8M2's bl2, whose last OFS word sits at 0x02c9f124, produces a
13,234,472-byte bl2.bin spanning 0x02000000-0x02c9f128. Flash that at the MRAM base and the
zero fill lands on the option memory: the brick, reconstituted, from a build that passed.

The .hex and .srec are record-based and skip the gaps, so they are safe. Only the flat binary
coalesces. --check-flat-bin looks for the sibling .bin and fails when the ELF carries OFS but
the .bin is sized as if it spans to it.
---------------------------------------------------------------------------------------------

Usage:
    check_ofs.py --region 0xSTART:0xEND [--region ...] [--require-segments] ELF [ELF ...]
    check_ofs.py --ra6-default [--require-segments] ELF [ELF ...]

    --region S:E        OFS address window, END EXCLUSIVE. Repeatable - RA8M2 needs two.
    --ra6-default       use 0x0100A100:0x0100A2D0, the RA6E1/RA6M4/RA6M5 map.
    --require-segments  FAIL if an image has no OFS segment at all. Use for BL2, which is
                        the image that carries the option words; a CLEAN result there means
                        either the sections were dropped or the window is wrong.
    --max-word N        largest single option word in bytes (default 0x80: RA8M2's BPS is
                        0x80, RA6's largest is 0xC).
    --check-flat-bin    also FAIL if the sibling <image>.bin spans from the image base up
                        into an OFS region. See the note above - the ELF can be correct and
                        the .bin still brick the part.

Exit: 0 = safe, 1 = a spanning OFS segment (DO NOT FLASH) or a required segment missing,
      2 = tool error.
"""
import os, re, shutil, subprocess, sys

RA6_REGION = (0x0100A100, 0x0100A2D0)
READELF    = os.environ.get("READELF", "arm-none-eabi-readelf")


def parse_args(argv):
    regions, images = [], []
    max_word = 0x80
    require = False
    flat = False
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--region":
            i += 1
            if i >= len(argv):
                sys.exit("ERROR: --region needs START:END")
            try:
                s, e = argv[i].split(":")
                regions.append((int(s, 0), int(e, 0)))
            except ValueError:
                sys.exit(f"ERROR: cannot parse --region {argv[i]!r}, want 0xSTART:0xEND")
        elif a == "--ra6-default":
            regions.append(RA6_REGION)
        elif a == "--require-segments":
            require = True
        elif a == "--check-flat-bin":
            flat = True
        elif a == "--max-word":
            i += 1
            max_word = int(argv[i], 0)
        elif a.startswith("--"):
            sys.exit(f"ERROR: unknown option {a}")
        else:
            images.append(a)
        i += 1

    if not regions:
        sys.exit("ERROR: no OFS region given. Pass --region 0xSTART:0xEND (repeatable), or\n"
                 "       --ra6-default for RA6E1/RA6M4/RA6M5. This is deliberately not\n"
                 "       optional: a guard pointed at the wrong window reports PASS on an\n"
                 "       unsafe image. See the note at the top of this file.")
    if not images:
        sys.exit("ERROR: no images given")
    for s, e in regions:
        if e <= s:
            sys.exit(f"ERROR: region 0x{s:X}:0x{e:X} is empty or reversed")
    return regions, images, max_word, require, flat


def all_load_segments(elf):
    """Every PT_LOAD with filesz > 0, as (vaddr, paddr, filesz).

    objcopy -O binary lays the flat image out by PHYSICAL address, so the flat-binary
    check below must use paddr. The OFS check uses vaddr, which is what the option words
    are addressed by and what the RA6M4 post-mortem was written against; for OFS segments
    the two are equal anyway (see the readelf output in DESIGN.md 8.4).
    """
    try:
        out = subprocess.run([READELF, "-l", elf], capture_output=True, text=True,
                             check=True).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        sys.exit(f"ERROR: readelf failed on {elf}: {e}")
    segs = []
    for m in re.finditer(
            r"LOAD\s+0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)"
            r"\s+0x([0-9a-fA-F]+)",
            out):
        vaddr, paddr, filesz = (int(m.group(1), 16), int(m.group(2), 16),
                                int(m.group(3), 16))
        if filesz:
            segs.append((vaddr, paddr, filesz))
    return segs


def in_regions(addr, size, regions):
    return any(addr < e and (addr + size) > s for s, e in regions)


def load_segments(elf, regions):
    """PT_LOAD segments (vaddr, filesz) overlapping any region."""
    return [(v, f) for (v, _p, f) in all_load_segments(elf)
            if in_regions(v, f, regions)]


def flat_bin_verdict(elf, regions):
    """Check the FLAT BINARY actually on disk, not what the ELF implies.

    `objcopy -O binary` spans lowest paddr to highest and zero-fills the gaps, so an ELF
    whose OFS words sit far above the image produces a flat binary tens of MB long whose
    fill covers the option memory. The platform build defeats that by regenerating the
    .bin with --remove-section=.option_setting*, and this checks that it actually did:
    the safe binary is exactly the non-OFS span, anything longer still reaches the
    option words.

    A MISSING .bin is a FAILURE, never a pass. The guard target depends on the bin target
    (bl2/ is configured before platform/, so it exists), and "I could not find it" must
    not read as "it is fine" - that is the mistake this whole file was rewritten over.

    Returns (status, detail).
    """
    binpath = os.path.splitext(elf)[0] + ".bin"
    segs = all_load_segments(elf)
    ofs = [(v, p, f) for (v, p, f) in segs if in_regions(v, f, regions)]
    if not segs or not ofs:
        return "n/a", (binpath, None, None)

    lo = min(p for (_v, p, _f) in segs)
    hi_all = max(p + f for (_v, p, f) in segs)
    hi_other = max([p + f for (v, p, f) in segs if not in_regions(v, f, regions)] or [lo])
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
        sys.exit(f"ERROR: readelf '{READELF}' not on PATH (set $READELF)")
    regions, images, max_word, require, flat = parse_args(sys.argv[1:])

    where = ", ".join(f"{hex(s)}-{hex(e - 1)}" for s, e in regions)
    print(f"OFS brick guard: no PT_LOAD segment across {where} may exceed {max_word} bytes")
    print("(a span => zero-filled gaps => block-protect programmed to 0 => permanent brick, "
          "DESIGN.md 8.4)\n")

    bad = 0
    for elf in images:
        name = os.path.basename(elf)
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
        elif not segs:
            if require:
                bad += 1
                print(f"  {name:<14} *** NO OFS SEGMENTS, BUT --require-segments *** "
                      f"(sections dropped, or the region is wrong)")
            else:
                print(f"  {name:<14} CLEAN (no OFS segments, none required)")
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
                      f"fill past 0x{hi_other:08X}. Safe size is {safe} - regenerate with "
                      f"--remove-section=.option_setting*)")
            elif status == "missing":
                bad += 1
                print(f"  {bn:<14} *** FLAT BINARY MISSING, CANNOT VERIFY *** "
                      f"(expected {extra} bytes; a .bin this guard cannot see is a "
                      f"failure, not a pass)")
            elif status == "ok":
                print(f"  {bn:<14} flat .bin OK - {actual} bytes, OFS sections stripped")
            else:
                print(f"  {'':<14} flat .bin: n/a (no OFS above the image)")

    print()
    if bad:
        print(f"FAIL: {bad} image(s).")
        print("A spanning segment means the linker coalesced option words into one gap-filled")
        print("PT_LOAD; flashing it programs the block-protect words to 0 -> permanent brick.")
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
