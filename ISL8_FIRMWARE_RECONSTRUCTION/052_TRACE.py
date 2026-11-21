#!/usr/bin/env python3
import re
import time
import sys
import os

# ==================== KONFIG =====================
LOG_FILE = "LOG_ISL8_ALL.txt"
TRACE_FILE = "TRACE_ISL8.txt"
BLOCK_DEBUG = "INFO_blocks_debug.txt"

# ---------------------------------------------
# ZASTAVIT ZPRACOVÁNÍ PO N ŘÁDCÍCH
# Pokud chceš celý log → MAX_LINES = None
# ---------------------------------------------
MAX_LINES = None       # např. 5_000_000 nebo 100_000 nebo None
#MAX_LINES = 2000000     # např. 5_000_000 nebo 100_000 nebo None

# Jak často tisknout průběžný stav
REPORT_INTERVAL = 2_000_000

# Regex: povolit whitespace + zachytit 4 hex + hvězdičku
pattern = re.compile(r"^\s*([0-9A-Fa-f]{4})\*")


# ==================== UTIL =======================
def format_time(seconds: float) -> str:
    h = int(seconds // 3600)
    m = int((seconds % 3600) // 60)
    s = int(seconds % 60)
    return f"{h:02}:{m:02}:{s:02}"


# ==================== TRACE V5.3 ==================
def main():
    # Smazat starý TRACE, pokud existuje
    if os.path.exists(TRACE_FILE):
        try:
            os.remove(TRACE_FILE)
        except Exception as e:
            print(f"[CHYBA] Nelze odstranit {TRACE_FILE}: {e}")
            sys.exit(1)

    total_input = 0
    start_time = time.time()

    # Mapa navštívených adres – 64 kB
    visited = [False] * 0x10000

    print("=======================================================")
    print(" TRACE V5.3 – Značení navštívených adres z logu")
    if MAX_LINES is None:
        print(" Režim: PLNÝ PRŮCHOD")
    else:
        print(f" Režim: TEST NA PRVNÍCH {MAX_LINES:,} ŘÁDCÍCH LOGU")
    print("=======================================================")

    # ========== 1. PRŮCHOD – POUZE ZNAČENÍ ADRES ==========
    with open(LOG_FILE, "r") as infile:
        for raw in infile:
            total_input += 1

            # STOP po MAX_LINES
            if MAX_LINES is not None and total_input > MAX_LINES:
                print(f"\n>>> Zastaveno po {MAX_LINES:,} řádcích (testovací režim).")
                break

            line = raw.rstrip("\n\r")

            m = pattern.match(line)
            if m:
                addr = int(m.group(1), 16)
                visited[addr] = True

            # průběžný report
            if total_input % REPORT_INTERVAL == 0:
                elapsed = time.time() - start_time
                used = sum(visited)
                percent = (used / 65536.0) * 100.0
                sys.stdout.write(
                    f"\rŘádků: {total_input:,} | "
                    f"Unikátních adres: {used:,} ({percent:5.2f} %) | "
                    f"Čas: {format_time(elapsed)}"
                )
                sys.stdout.flush()

    print("\nČtení logu dokončeno. Skládám bloky...")

    # ========== 2. Z BINARY MAPY UDĚLAT BLOKY ==========
    blocks = []
    in_block = False
    block_start = None

    for addr in range(0x0000, 0x10000):
        if visited[addr]:
            if not in_block:
                in_block = True
                block_start = addr
        else:
            if in_block:
                blocks.append((block_start, addr - 1))
                in_block = False

    if in_block:
        blocks.append((block_start, 0xFFFF))

    print(f"Nalezeno bloků: {len(blocks)}")
    print("Generuji výstupní TRACE...")

    # ========== 3. GENEROVAT TRACE ==========
    with open(TRACE_FILE, "w") as out:
        for i, (start_addr, end_addr) in enumerate(blocks):
            if i != 0:
                out.write("trace\n")
            for a in range(start_addr, end_addr + 1):
                out.write(f"{a:04X}\n")

    # ========== 4. DEBUG BLOKY ==========
    print(f"Generuji debug soubor: {BLOCK_DEBUG}")

    with open(BLOCK_DEBUG, "w") as dbg:
        dbg.write("start,end,length\n")
        for (start_addr, end_addr) in blocks:
            length = end_addr - start_addr + 1
            dbg.write(f"{start_addr:04X},{end_addr:04X},{length}\n")

    # ========== 5. SUMÁŘ ==========
    elapsed = time.time() - start_time
    used = sum(visited)
    percent = (used / 65536.0) * 100.0

    print("=======================================================")
    print(" HOTOVO – TRACE V5.3")
    print(f" Zpracováno řádků logu : {total_input:,}")
    print(f" Unikátních adres      : {used:,} / 65 536 ({percent:.2f} %)")
    print(f" Počet bloků           : {len(blocks):,}")
    print(f" TRACE soubor          : {TRACE_FILE}")
    print(f" DEBUG soubor          : {BLOCK_DEBUG}")
    print(f" Celkový čas           : {format_time(elapsed)}")
    print("=======================================================")


if __name__ == "__main__":
    main()
