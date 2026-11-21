#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pathlib import Path
import re

# ============================================================
# KONFIGURACE
# ============================================================

HEX_FILE    = Path("Objects/Reconstruction.hex")
BIN_FILE    = Path("fw850en.bin")
REPORT_FILE = Path("INFO_diff_hex_report_FINAL.txt")

TIME_CODE_FILE       = Path("TIME_CODE_ADDRS.txt")
TIME_DATA_FILE       = Path("TIME_DATA_ADDRS.txt")
FINAL_USER_CODE_FILE = Path("FINAL_USER_CODE.txt")
FINAL_USER_DATA_FILE = Path("FINAL_USER_DATA.txt")

ROM_SIZE = 64 * 1024  # 64 kB


# ============================================================
# HELPERS
# ============================================================

def load_hex_to_bytes(path: Path, size: int) -> bytearray:
    mem = bytearray([0xFF] * size)
    with path.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line.startswith(":"):
                continue
            count = int(line[1:3], 16)
            addr  = int(line[3:7], 16)
            rectype = int(line[7:9], 16)
            if rectype == 0:
                data = bytes.fromhex(line[9:9 + count * 2])
                for i, b in enumerate(data):
                    if addr + i < size:
                        mem[addr + i] = b
            elif rectype == 1:
                break
    return mem


def load_simple_addr_file(path: Path) -> set[int]:
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        s = ln.strip()
        if not s or s.startswith(";"):
            continue

        # rozsahy: 0x1234-0x1250/xx
        if "-" in s:
            try:
                left, right = s.split("-", 1)
                right = right.split("/")[0]
                a1 = int(left.replace("0x", ""), 16)
                a2 = int(right.replace("0x", ""), 16)
                for a in range(min(a1, a2), max(a1, a2) + 1):
                    out.add(a & 0xFFFF)
                continue
            except:
                pass

        # jednotlivé adresy
        try:
            out.add(int(s.replace("0x", "")[:4], 16) & 0xFFFF)
        except:
            pass

    return out


# ============================================================
# MAIN
# ============================================================

def main():
    print("[+] DIFF + COVERAGE report")

    fw_bin = bytearray(BIN_FILE.read_bytes())
    if len(fw_bin) < ROM_SIZE:
        fw_bin.extend(b"\x00" * (ROM_SIZE - len(fw_bin)))
    fw_bin = fw_bin[:ROM_SIZE]

    fw_hex = load_hex_to_bytes(HEX_FILE, ROM_SIZE)

    # --------------------------------------------------------
    # DIFF
    # --------------------------------------------------------
    diffs = [
        (i, fw_bin[i], fw_hex[i])
        for i in range(ROM_SIZE)
        if fw_bin[i] != fw_hex[i]
    ]

    # --------------------------------------------------------
    # COVERAGE INPUTS
    # --------------------------------------------------------
    time_code = load_simple_addr_file(TIME_CODE_FILE)
    time_data = load_simple_addr_file(TIME_DATA_FILE)
    user_code = load_simple_addr_file(FINAL_USER_CODE_FILE)
    user_data = load_simple_addr_file(FINAL_USER_DATA_FILE)

    total_code = time_code | user_code
    total_data = time_data | user_data
    total_used = total_code | total_data

    # --------------------------------------------------------
    # STATISTIKY
    # --------------------------------------------------------
    def pct(n): return n / ROM_SIZE * 100.0

    stats = {
        "TIME CODE": len(time_code),
        "TIME DATA": len(time_data),
        "USER CODE": len(user_code),
        "USER DATA": len(user_data),
        "TOTAL CODE": len(total_code),
        "TOTAL DATA": len(total_data),
        "TOTAL USED": len(total_used),
    }

    # --------------------------------------------------------
    # VÝPIS NA TERMINÁL
    # --------------------------------------------------------
    print("\n=== DIFF ===")
    print(f"Rozdílů:        {len(diffs)}")
    print(f"Shodných bajtů: {ROM_SIZE - len(diffs)}")

    print("\n=== COVERAGE ===")
    for k in (
        "TIME CODE", "TIME DATA",
        "USER CODE", "USER DATA",
        "TOTAL CODE", "TOTAL DATA",
        "TOTAL USED"
    ):
        print(f"{k:12}: {stats[k]:6}  ({pct(stats[k]):6.2f} %)")

    # --------------------------------------------------------
    # REPORT DO SOUBORU
    # --------------------------------------------------------
    with REPORT_FILE.open("w", encoding="utf-8") as f:
        f.write("=== DIFF output.hex vs fw850en.bin ===\n\n")
        for a, b1, b2 in diffs:
            f.write(f"0x{a:04X}: BIN={b1:02X} HEX={b2:02X}\n")

        f.write("\n=== COVERAGE SUMMARY ===\n")
        for k in stats:
            f.write(f"{k:12}: {stats[k]:6}  ({pct(stats[k]):6.2f} %)\n")

    print(f"\n[✓] Report uložen do {REPORT_FILE}")


if __name__ == "__main__":
    main()
