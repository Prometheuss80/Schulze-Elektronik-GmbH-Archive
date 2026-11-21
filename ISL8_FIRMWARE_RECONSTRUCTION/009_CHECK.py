#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
from pathlib import Path

DATA_FILE  = Path("user_clean_data.txt")   # 100% data z BLOCK/FINAL_DATA
BLOCK_FILE = Path("BLOCK.txt")             # bloky z 14_BLOCK.py
OUTPUT_FILE = Path("DATA_CODE_CONFLICTS.txt")

# user_clean_data.txt:
#  - formát typu:
#      0x090F - 0x0912 (4 B) [data]  "... "
#      0x17D4 - 0x17D4 (1 B) [text]  "$"
RANGE_RE  = re.compile(r'0x([0-9A-Fa-f]{4})\s*-\s*0x([0-9A-Fa-f]{4})')
SINGLE_RE = re.compile(r'0x([0-9A-Fa-f]{4})')

# BLOCK.txt:
#  - formát typu:
#      0x0000-0x0002/3b
BLOCK_RE  = re.compile(r'0x([0-9A-Fa-f]{4})-0x([0-9A-Fa-f]{4})')

def load_data_addrs(path: Path):
    """
    Načte user_clean_data.txt a vrátí set všech adres označených jako data/text/const_00.
    (Každá řádka může být rozsah nebo 1b.)
    """
    if not path.exists():
        raise FileNotFoundError(path)

    addrs = set()
    lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()

    for ln in lines:
        s = ln.strip()
        if not s:
            continue

        m_range = RANGE_RE.search(s)
        if m_range:
            a1 = int(m_range.group(1), 16)
            a2 = int(m_range.group(2), 16)
            lo, hi = min(a1, a2), max(a1, a2)
            for a in range(lo, hi + 1):
                addrs.add(a)
            continue

        # pokud není rozsah, zkus single
        m_single = SINGLE_RE.search(s)
        if m_single:
            a = int(m_single.group(1), 16)
            addrs.add(a)

    return addrs


def load_blocks(path: Path):
    """
    Načte BLOCK.txt a vrátí seznam bloků [(start,end)].
    Očekávaný formát řádku: 0xAAAA-0xBBBB/xxb
    """
    if not path.exists():
        raise FileNotFoundError(path)

    blocks = []
    lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()

    for ln in lines:
        s = ln.strip()
        if not s:
            continue

        m = BLOCK_RE.search(s)
        if not m:
            continue

        start = int(m.group(1), 16)
        end   = int(m.group(2), 16)
        if end < start:
            start, end = end, start

        blocks.append((start, end))

    # seřadíme podle začátku – usnadní vyhledávání
    blocks.sort(key=lambda x: x[0])
    return blocks


def find_conflicts(data_addrs, blocks):
    """
    data_addrs: set[int]
    blocks: list[(start,end)] seřazené podle startu

    Vrací seznam záznamů:
      (block_start, block_end, [adresy_data_v_bloku])
    """
    conflicts = []

    if not data_addrs or not blocks:
        return conflicts

    data_sorted = sorted(data_addrs)
    bi = 0
    n_blocks = len(blocks)

    for addr in data_sorted:
        # posuň blokový index tak, aby block_end >= addr
        while bi < n_blocks and blocks[bi][1] < addr:
            bi += 1
        if bi >= n_blocks:
            break

        # teď může addr ležet v blocks[bi] nebo následujících
        for j in range(bi, n_blocks):
            start, end = blocks[j]
            if start > addr:
                break
            if start <= addr <= end:
                # našli jsme konflikt – ulož
                conflicts.append((start, end, addr))
                # NEbreak – teoreticky může být v overlappujících blocích,
                # ale BLOCK by je mít neměl; necháme tak pro jistotu.
        # dál pokračujeme s dalším addr

    return conflicts


def main():
    print(f"[i] Načítám data z: {DATA_FILE}")
    data_addrs = load_data_addrs(DATA_FILE)
    print(f"[i] Načteno data-adres: {len(data_addrs)}")

    print(f"[i] Načítám bloky z: {BLOCK_FILE}")
    blocks = load_blocks(BLOCK_FILE)
    print(f"[i] Načteno bloků: {len(blocks)}")

    print("[i] Hledám konflikty DATA vs BLOCK...")
    conflicts = find_conflicts(data_addrs, blocks)
    print(f"[i] Nalezeno konfliktních výskytů: {len(conflicts)}")

    if not conflicts:
        with OUTPUT_FILE.open("w", encoding="utf-8") as f:
            f.write("; ŽÁDNÉ konflikty data/kód nenalezeny.\n")
        print(f"[OK] Výstup zapsán do: {OUTPUT_FILE}")
        return

    # seskupíme konflikty podle bloku, aby to bylo čitelnější
    # (start,end) -> set(adres)
    grouped = {}
    for start, end, addr in conflicts:
        key = (start, end)
        grouped.setdefault(key, set()).add(addr)

    with OUTPUT_FILE.open("w", encoding="utf-8") as f:
        f.write("; DATA adresy, které leží uvnitř BLOCK (pravděpodobně kód)\n")
        f.write("; Formát: 0xAAAA-0xBBBB  | data: 0xXXXX, ...\n\n")

        for (start, end) in sorted(grouped.keys()):
            addrs = sorted(grouped[(start, end)])
            addr_list = ", ".join(f"0x{a:04X}" for a in addrs)
            f.write(f"0x{start:04X}-0x{end:04X}  | data: {addr_list}\n")

    print(f"[OK] Výstup zapsán do: {OUTPUT_FILE}")
    print("[i] Teď můžeš ručně rozhodnout, co z toho jsou skutečná data a co má být kód.")


if __name__ == "__main__":
    main()

