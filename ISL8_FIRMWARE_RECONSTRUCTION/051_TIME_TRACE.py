#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
24_TIME_TRACE.py

Rozlišení KÓD vs DATA čistě podle časové posloupnosti čtení CPU.

PRAVIDLA:
- KÓD: adresa má alespoň jednou souseda (A-1 nebo A+1)
- DATA: adresa NIKDY nemá souseda (v celém logu)

VÝSTUPY:
- TIME_CODE_ADDRS.txt
- TIME_DATA_ADDRS.txt
- TIME_TRACE_STATS.txt
- FW_CODE_ISL8.bin   (CODE + DATA bajty z originálního FW, jinak 0x00)
- FULL_MAPA.bin      ('C' = code, 'D' = data, '?' = unknown)
"""

from pathlib import Path
import time

# ------------------------------------------------------------
# KONFIGURACE
# ------------------------------------------------------------
LOG_FILE = Path("LOG_ISL8_ALL.txt")
FW_FILE  = Path("fw850en.bin")

OUT_CODE      = Path("TIME_CODE_ADDRS.txt")
OUT_DATA      = Path("TIME_DATA_ADDRS.txt")
OUT_STATS     = Path("TIME_TRACE_STATS.txt")
OUT_FW_CODE   = Path("FW_CODE_ISL8.bin")
OUT_FULL_MAP  = Path("FULL_MAPA.bin")

FW_SIZE = 65536


# ------------------------------------------------------------
# PARSER ADRESY
# ------------------------------------------------------------
def parse_addr(line: str):
    """
    Vezme řádek 'FE80@3' -> 0xFE80
    """
    if len(line) < 4:
        return None
    try:
        return int(line[:4], 16)
    except ValueError:
        return None


# ------------------------------------------------------------
# MAIN
# ------------------------------------------------------------
def main():
    t0 = time.time()

    if not LOG_FILE.exists():
        print(f"[ERR] Nenalezen {LOG_FILE}")
        return

    if not FW_FILE.exists():
        print(f"[ERR] Nenalezen {FW_FILE}")
        return

    print("[i] Čtu LOG_ISL8.txt (time-trace režim)…")

    fw_orig = FW_FILE.read_bytes()
    if len(fw_orig) < FW_SIZE:
        fw_orig += b"\x00" * (FW_SIZE - len(fw_orig))
    fw_orig = fw_orig[:FW_SIZE]

    # ---- množiny adres ----
    seen = set()
    has_neighbor = set()

    prev_addr = None
    line_count = 0

    with LOG_FILE.open("r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line_count += 1
            addr = parse_addr(line.strip())
            if addr is None:
                prev_addr = None
                continue

            seen.add(addr)

            if prev_addr is not None:
                if addr == prev_addr + 1 or addr == prev_addr - 1:
                    has_neighbor.add(addr)
                    has_neighbor.add(prev_addr)

            prev_addr = addr

            if line_count % 5_000_000 == 0:
                elapsed = time.time() - t0
                print(f"  {line_count:,} řádků | {elapsed:6.1f}s")

    # --------------------------------------------------------
    # KLASIFIKACE
    # --------------------------------------------------------
    code_addrs = sorted(has_neighbor)
    data_addrs = sorted(a for a in seen if a not in has_neighbor)

    code_set = set(code_addrs)
    data_set = set(data_addrs)

    # --------------------------------------------------------
    # VÝSTUPNÍ BUFFERY (DŮLEŽITÁ OPRAVA)
    # --------------------------------------------------------
    fw_code = bytearray([0x00] * FW_SIZE)
    full_map = bytearray([ord(' ')] * FW_SIZE)

    # ---- CODE ----
    for a in code_set:
        fw_code[a] = fw_orig[a]
        full_map[a] = ord('@')

    # ---- DATA ----
    for a in data_set:
        fw_code[a] = fw_orig[a]
        full_map[a] = ord('*')

    # --------------------------------------------------------
    # ZÁPIS TEXTOVÝCH VÝSTUPŮ
    # --------------------------------------------------------
    with OUT_CODE.open("w") as f:
        for a in code_addrs:
            f.write(f"{a:04X}\n")

    with OUT_DATA.open("w") as f:
        for a in data_addrs:
            val = fw_orig[a]
            asc = chr(val) if 32 <= val < 127 else '.'
            f.write(f"{a:04X} / 0x{val:02X} / \"{asc}\"\n")

    elapsed = time.time() - t0

    with OUT_STATS.open("w") as f:
        f.write("=== TIME TRACE ANALÝZA ===\n\n")
        f.write(f"Řádků logu:        {line_count:,}\n")
        f.write(f"Unikátní adresy:   {len(seen)}\n\n")
        f.write(f"KÓD adresy:        {len(code_set)}\n")
        f.write(f"DATA adresy:       {len(data_set)}\n\n")
        f.write(f"KÓD pokrytí:       {len(code_set) / FW_SIZE * 100:6.2f} %\n")
        f.write(f"DATA pokrytí:      {len(data_set) / FW_SIZE * 100:6.2f} %\n")
        f.write(f"CELKEM pokryto:    {(len(code_set)+len(data_set)) / FW_SIZE * 100:6.2f} %\n\n")
        f.write(f"Čas běhu:          {elapsed:.2f} s\n")

    # --------------------------------------------------------
    # BINÁRNÍ VÝSTUPY
    # --------------------------------------------------------
    OUT_FW_CODE.write_bytes(fw_code)
    OUT_FULL_MAP.write_bytes(full_map)

    # --------------------------------------------------------
    # SUMMARY
    # --------------------------------------------------------
    print()
    print("[OK] HOTOVO")
    print(f"[OK] KÓD adres:        {len(code_set)}")
    print(f"[OK] DATA adres:       {len(data_set)}")
    print(f"[OK] CELKEM pokryto:   {len(code_set)+len(data_set)} / {FW_SIZE}")
    print(f"[OK] Čas:             {elapsed:.2f} s")
    print("[OK] Výstupy:")
    print(f"     {OUT_CODE}")
    print(f"     {OUT_DATA}")
    print(f"     {OUT_STATS}")
    print(f"     {OUT_FW_CODE}")
    print(f"     {OUT_FULL_MAP}")


# ------------------------------------------------------------
if __name__ == "__main__":
    main()
