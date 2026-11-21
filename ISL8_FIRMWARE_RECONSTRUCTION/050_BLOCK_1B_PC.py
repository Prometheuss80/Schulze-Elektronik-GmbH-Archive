#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import time
from pathlib import Path

LOG_FILE   = Path("LOG_ISL8_ALL.txt")
TEMP_BLOCKS = Path("TEMP_BLOCKS.txt")
OUTPUT_TXT = Path("BLOCK.txt")
USER_1B    = Path("INFO_block_clean_data.txt")

HEX_RE = re.compile(rb"\b([0-9A-Fa-f]{4})\b")


def parse_addr(line_bytes):
    if b"trace" in line_bytes.lower():
        return "trace"
    m = HEX_RE.search(line_bytes)
    if m:
        return int(m.group(1), 16)
    return None


def fast_count_lines(path):
    c = 0
    with open(path, "rb") as f:
        for _ in f:
            c += 1
    return c


def main():
    t0 = time.time()

    print("[i] Počítám řádky (FAST)…")
    total_lines = fast_count_lines(LOG_FILE)
    print(f"[i] Celkem řádků: {total_lines:,}")

    print("[i] Předzpracování bloků → TEMP soubor…")

    one_byte_addrs = set()
    unique_addrs = set()

    last_print = t0
    current_block = []
    i = 0

    # --- FÁZE 1: STREAM ČTENÍ A BLOKY -> TEMP ---
    with open(LOG_FILE, "rb") as f, TEMP_BLOCKS.open("w") as out:
        for i, line in enumerate(f, 1):
            val = parse_addr(line)

            now = time.time()
            if now - last_print >= 1:
                elapsed = now - t0
                speed = i / elapsed
                pct = (i / total_lines) * 100
                eta = int((total_lines - i) / speed)

                print(
                    f"\r[RUN1] {pct:6.2f}% | {i}/{total_lines} | "
                    f"{int(speed):8d} ř/s | ETA {eta//60:02d}:{eta%60:02d} | "
                    f"adr {len(unique_addrs):6d}",
                    end=""
                )
                last_print = now

            # konec bloku
            if val == "trace":
                if current_block:
                    s = current_block[0]
                    e = current_block[-1]
                    out.write(f"{s:04X}-{e:04X}\n")
                    if len(current_block) == 1:
                        one_byte_addrs.add(s)
                current_block = []
                continue

            # adresa
            if isinstance(val, int):
                unique_addrs.add(val)

                if not current_block:
                    current_block = [val]
                else:
                    prev = current_block[-1]
                    if val == prev + 1:
                        current_block.append(val)
                    elif val in current_block:
                        pass
                    else:
                        s = current_block[0]
                        e = current_block[-1]
                        out.write(f"{s:04X}-{e:04X}\n")
                        if len(current_block) == 1:
                            one_byte_addrs.add(s)
                        current_block = [val]

    # poslední blok
    if current_block:
        s = current_block[0]
        e = current_block[-1]
        with TEMP_BLOCKS.open("a") as out:
            out.write(f"{s:04X}-{e:04X}\n")
        if len(current_block) == 1:
            one_byte_addrs.add(s)

    print("\n[i] FÁZE 1 hotova. Provádím MERGE…")

    # --- FÁZE 2: MERGE ---
    blocks = []

    with TEMP_BLOCKS.open("r") as f:
        for line in f:
            s_hex, e_hex = line.strip().split("-")
            s = int(s_hex, 16)
            e = int(e_hex, 16)
            blocks.append((s, e))

    blocks.sort(key=lambda x: x[0])

    merged = []
    last_s, last_e = blocks[0]

    for s, e in blocks[1:]:
        if s == last_s and e == last_e:
            continue
        merged.append((last_s, last_e))
        last_s, last_e = s, e

    merged.append((last_s, last_e))

    with OUTPUT_TXT.open("w") as f:
        for s, e in merged:
            f.write(f"0x{s:04X}-0x{e:04X}/{(e-s+1)}b\n")

    with USER_1B.open("w") as f:
        for a in sorted(one_byte_addrs):
            f.write(f"{a:04X}\n")

    t1 = time.time()
    print(f"[OK] Dokončeno za {t1 - t0:.2f} s")
    print(f"[OK] Bloků: {len(merged):,}")
    print(f"[OK] 1B dat: {len(one_byte_addrs):,}")
    print(f"[OK] TEMP: {TEMP_BLOCKS} (můžeš smazat)")


if __name__ == "__main__":
    main()
