#!/usr/bin/env python3
"""Schulze ISL8 F2 factory-calibration patch for four EXACT SHA-256 originals.

Changes the 35-byte factory calibration entry and one ASCII digit of the
version shown on LCD: 8.10 -> 8.19 or 8.50 -> 8.59. Also changes
the original date and time shown in two locations. Never ships a ROM.
"""
# Copyright (c) 2026 Martin Stránský (Prometheuss80)
# SPDX-License-Identifier: MIT

from __future__ import annotations

import argparse
import hashlib
import sys
from dataclasses import dataclass
from pathlib import Path

ROM_SIZE = 65536
PATCH_SIZE = 35
OLD_DATE = b"15.06.04"
NEW_DATE = b"22.04.26"
OLD_TIME_EN = b"we 12:30"
NEW_TIME_EN = b"we 18:30"
OLD_TIME_DE = b"Mi 12:30"
NEW_TIME_DE = b"Mi 18:30"

@dataclass(frozen=True)
class Firmware:
    version: str
    language: str
    display_before: bytes
    display_after: bytes
    output_filename: str
    entry_offset: int
    lcall_target: int
    display_offset: int
    date_offsets: tuple[int, int]
    time_offsets: tuple[int, int]

# Original ROM SHA-256 only. The filename supplied by the user is never trusted.
KNOWN_FIRMWARE = {
    "d593901c113f3454459d136f2f9ec0e3815f0ba41f3cebe64d5608503361fbcf": Firmware("8.10", "DE", b"8.10\0", b"8.19\0", "fw819de.bin", 0xB9B5, 0x3BE1, 0x3131, (0x3057, 0x346F), (0x3061, 0x34A3)),
    "e22af897780acc77fcc9a33a3bdcff9091f5fa8c9985e5953013885acb63e6d5": Firmware("8.10", "EN", b"8.10\0", b"8.19\0", "fw819en.bin", 0xB9A2, 0x3BDE, 0x3131, (0x3057, 0x346C), (0x3061, 0x34A0)),
    "469b4f3621f412266676f3fcbcf209a752addaa56f6ce75edc62419a36d401bb": Firmware("8.50", "DE", b"8.50\0", b"8.59\0", "fw859de.bin", 0xBABA, 0x3BD8, 0x312D, (0x3053, 0x346B), (0x305D, 0x349F)),
    "f9d461cb2541e4584cf9811e5b68850b6dd13b36725339e09f433699b96aa5ed": Firmware("8.50", "EN", b"8.50\0", b"8.59\0", "fw859en.bin", 0xBAA7, 0x3BD5, 0x312D, (0x3053, 0x3468), (0x305D, 0x349C)),
}

# The version-specific LCALL target bytes are at offsets +29,+30 (original).
ORIGINAL_PREFIX = bytes.fromhex(
    "30 22 38 20 51 35 78 68 E6 60 30 08 E6 60 2C 08"
    " E6 60 28 30 50 25 D2 51 C2 37 78 5B 12"
)
ORIGINAL_SUFFIX = bytes.fromhex("00 00 00 00")
# After replacement the LCALL operands move to +28,+29.
PATCH_PREFIX = bytes.fromhex(
    "30 14 38 D2 50 D2 51 78 68 76 01 08 76 01 08 76 01 08"
    " 30 50 26 D2 51 C2 37 78 5B 12"
)
PATCH_SUFFIX = bytes.fromhex("00 00 00 00 00")
assert len(ORIGINAL_PREFIX) + 2 + len(ORIGINAL_SUFFIX) == PATCH_SIZE
assert len(PATCH_PREFIX) + 2 + len(PATCH_SUFFIX) == PATCH_SIZE


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def find_original_pattern(rom: bytes) -> tuple[int, int]:
    matches: list[tuple[int, int]] = []
    start = 0
    while True:
        pos = rom.find(ORIGINAL_PREFIX, start)
        if pos < 0:
            break
        if rom[pos + 31:pos + PATCH_SIZE] == ORIGINAL_SUFFIX and pos + PATCH_SIZE <= len(rom):
            target = int.from_bytes(rom[pos + 29:pos + 31], "big")
            matches.append((pos, target))
        start = pos + 1
    if len(matches) != 1:
        raise ValueError(f"Expected exactly one original calibration entry, found {len(matches)}")
    return matches[0]


@dataclass(frozen=True)
class PatchReport:
    firmware: Firmware
    original_sha256: str
    patched_sha256: str
    entry_offset: int
    lcall_target: int
    display_offset: int
    date_offsets: tuple[int, int]
    time_offsets: tuple[int, int]
    changed_offsets: tuple[int, ...]


def patch_rom_with_report(rom: bytes) -> tuple[bytes, PatchReport]:
    if len(rom) != ROM_SIZE:
        raise ValueError(f"Expected a {ROM_SIZE}-byte ROM, got {len(rom)} bytes")
    digest = sha256(rom)
    firmware = KNOWN_FIRMWARE.get(digest)
    if firmware is None:
        raise ValueError("Unsupported or already modified firmware (unknown SHA-256): " + digest)

    entry, target = find_original_pattern(rom)
    if (entry, target) != (firmware.entry_offset, firmware.lcall_target):
        raise ValueError("Calibration location/LCALL target differs from SHA-256 profile")
    # Following LCALL and SETB 5Dh must remain outside the patched range.
    if rom[entry + PATCH_SIZE:entry + PATCH_SIZE + 1] != b"\x12":
        raise ValueError("Expected following LCALL is missing")
    if rom[entry + PATCH_SIZE + 3:entry + PATCH_SIZE + 5] != b"\xD2\x5D":
        raise ValueError("Expected following SETB 5Dh is missing")

    # Require exactly one null-terminated ASCII version string at its known offset.
    text = firmware.display_before
    matches = [i for i in range(len(rom)) if rom.startswith(text, i)]
    if matches != [firmware.display_offset]:
        raise ValueError(f"Expected one exact {text!r} version string at 0x{firmware.display_offset:04X}, found {matches}")
    display_digit_offset = firmware.display_offset + 3
    if rom[display_digit_offset] != ord("0"):
        raise ValueError("Unexpected LCD version digit")
    if not (display_digit_offset < entry or display_digit_offset >= entry + PATCH_SIZE):
        raise ValueError("LCD version digit overlaps the calibration patch")

    # The original version date and time each occur twice: the edit field and
    # its explanatory LCD text. Preserve German/English weekday abbreviations.
    time_before = OLD_TIME_DE if firmware.language == "DE" else OLD_TIME_EN
    time_after = NEW_TIME_DE if firmware.language == "DE" else NEW_TIME_EN
    if len(firmware.date_offsets) != 2 or len(firmware.time_offsets) != 2:
        raise ValueError("Invalid date/time profile offsets")

    def check_exact_strings(label: str, before: bytes, positions: tuple[int, int]) -> None:
        found = [i for i in range(len(rom)) if rom.startswith(before, i)]
        if found != sorted(positions):
            raise ValueError(
                f"Expected exactly two {label} occurrences at "
                f"{[hex(p) for p in positions]}, found {[hex(p) for p in found]}"
            )
        for pos in positions:
            if rom[pos + len(before):pos + len(before) + 2] != b"\x00\x00":
                raise ValueError(f"Unexpected terminator following {label} at 0x{pos:04X}")

    check_exact_strings("date", OLD_DATE, firmware.date_offsets)
    check_exact_strings("time", time_before, firmware.time_offsets)

    payload = PATCH_PREFIX + target.to_bytes(2, "big") + PATCH_SUFFIX
    if len(payload) != PATCH_SIZE or payload[27:30] != b"\x12" + target.to_bytes(2, "big"):
        raise AssertionError("Invalid LCALL encoding")
    # Absolute branch target must still be the same after the patch.
    if entry + 21 + 0x26 != entry + 3 + 0x38:
        raise AssertionError("Bad relative branch target")

    output = bytearray(rom)
    output[entry:entry + PATCH_SIZE] = payload
    output[display_digit_offset] = ord("9")
    for pos in firmware.date_offsets:
        output[pos:pos + len(OLD_DATE)] = NEW_DATE
    for pos in firmware.time_offsets:
        output[pos:pos + len(time_before)] = time_after
    patched = bytes(output)
    if patched[firmware.display_offset:firmware.display_offset + len(text)] != firmware.display_after:
        raise AssertionError("LCD text did not match desired replacement")
    for pos in firmware.date_offsets:
        if patched[pos:pos + len(NEW_DATE) + 2] != NEW_DATE + b"\x00\x00":
            raise AssertionError("Updated date or terminator corrupted")
    for pos in firmware.time_offsets:
        if patched[pos:pos + len(time_after) + 2] != time_after + b"\x00\x00":
            raise AssertionError("Updated time or terminator corrupted")
    changed = tuple(i for i, (old, new) in enumerate(zip(rom, patched)) if old != new)
    allowed = set(range(entry, entry + PATCH_SIZE)) | {display_digit_offset}
    for pos in firmware.date_offsets:
        allowed.update(range(pos, pos + len(OLD_DATE)))
    for pos in firmware.time_offsets:
        allowed.update(range(pos, pos + len(time_before)))
    if not changed or not set(changed).issubset(allowed):
        raise AssertionError("Unexpected byte changed outside authorized areas")
    return patched, PatchReport(firmware, digest, sha256(patched), entry, target, firmware.display_offset, firmware.date_offsets, firmware.time_offsets, changed)


def patch_rom(rom: bytes) -> tuple[bytes, str, int, int]:
    """Backwards-compatible V1 function used by existing integrations."""
    patched, report = patch_rom_with_report(rom)
    return patched, f"{report.firmware.version} {report.firmware.language}", report.entry_offset, report.lcall_target


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Patch four exact ISL8 ROMs: F2 calibration, LCD version, date and time.")
    parser.add_argument("input", nargs="?", default=Path("original.bin"), type=Path,
                        help="Original, unmodified 64 KiB ROM (default: original.bin)")
    parser.add_argument("-o", "--output", type=Path, default=None,
                        help="Optional output filename; default is auto-selected fw819de.bin etc.")
    parser.add_argument("--dry-run", action="store_true", help="Print intended changes without writing a BIN")
    args = parser.parse_args(argv)
    try:
        rom = args.input.read_bytes()
        patched, report = patch_rom_with_report(rom)
        fw = report.firmware
        output_path = args.output if args.output is not None else args.input.parent / fw.output_filename
        source = args.input.resolve()
        dest = output_path.resolve()
        if source == dest and not args.dry_run:
            raise ValueError("Refusing to overwrite input ROM")
        if dest.exists() and not args.dry_run:
            raise ValueError(f"Output already exists (refusing to overwrite): {dest}")
        print(f"Identified firmware:  {fw.version} {fw.language} (exact SHA-256)")
        print(f"Original SHA-256:     {report.original_sha256}")
        print(f"Calibration entry:   0x{report.entry_offset:04X}, 35 bytes")
        print(f"Preserved LCALL:     0x{report.lcall_target:04X}")
        print(f"LCD version at:      0x{report.display_offset:04X}")
        print(f"LCD text:            {fw.display_before[:-1].decode('ascii')} -> {fw.display_after[:-1].decode('ascii')} (ASCII 0x30 -> 0x39)")
        weekday = "Mi" if fw.language == "DE" else "we"
        print(f"Date:                {OLD_DATE.decode()} -> {NEW_DATE.decode()}")
        print(f"Date at:             {', '.join(f'0x{p:04X}' for p in report.date_offsets)}")
        print(f"Time:                {weekday} 12:30 -> {weekday} 18:30")
        print(f"Time at:             {', '.join(f'0x{p:04X}' for p in report.time_offsets)}")
        print(f"Modified byte count: {len(report.changed_offsets)}")
        print(f"New SHA-256:         {report.patched_sha256}")
        print(f"Output file:         {output_path}")
        if args.dry_run:
            print("Dry run: no file written")
        else:
            with output_path.open("xb") as f:
                f.write(patched)
            print("Saved successfully")
        return 0
    except (ValueError, OSError, AssertionError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
