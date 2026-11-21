#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from pathlib import Path

# ================== KONFIGURACE ==================
FIRMWARE_FILE    = Path("fw850en.bin")
DATA_BLOCKS_FILE = Path("INFO_data_blocks.txt")
DATA_TRACE_FILE  = Path("INFO_data_trace.txt")

FIRMWARE_SIZE    = 0x10000   # 64 kB
MIN_TEXT_LEN     = 4         # minimální délka textového řetězce (ASCII)
MIN_CONST_LEN    = 8         # minimální délka konstantního bloku (např. samé 0x00)
PRINT_SUMMARY    = True

# ================== FUNKCE ==================
def is_ascii(b):
    """Vrací True, pokud je byte tisknutelný ASCII znak."""
    return 32 <= b < 127

def ascii_preview(data, limit=40):
    """Vytvoří čitelný ASCII náhled z bajtů (max `limit` znaků)."""
    text = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data)
    if len(text) > limit:
        text = text[:limit] + "..."
    return f"\"{text}\""

def detect_blocks(fw):
    """
    Prohledá firmware a detekuje:
      - ASCII texty
      - Konstantní výplně (např. 00 00 00…)
      - Opakující se vzory (např. FF FF FF…)
    Vrací list (start, end, typ)
    """
    blocks = []
    i = 0
    while i < len(fw):
        b = fw[i]

        # --- ASCII text ---
        if is_ascii(b):
            start = i
            while i < len(fw) and is_ascii(fw[i]):
                i += 1
            if i - start >= MIN_TEXT_LEN:
                blocks.append((start, i - 1, "text"))
                continue

        # --- konstantní sekvence ---
        run_val = b
        run_len = 1
        j = i + 1
        while j < len(fw) and fw[j] == run_val:
            run_len += 1
            j += 1
        if run_len >= MIN_CONST_LEN:
            blocks.append((i, j - 1, f"const_{run_val:02X}"))
            i = j
            continue

        i += 1
    return blocks


def save_blocks(fw, blocks):
    """Uloží přehled bloků do data_blocks.txt a všechny adresy do data_trace.txt."""
    with DATA_BLOCKS_FILE.open("w", encoding="utf-8") as f_blocks, \
         DATA_TRACE_FILE.open("w", encoding="utf-8") as f_trace:

        f_blocks.write("=== DETEKOVANÉ DATOVÉ BLOKY ===\n")
        for start, end, btype in blocks:
            size = end - start + 1
            data_snippet = fw[start:end+1]
            ascii_str = ascii_preview(data_snippet)
            f_blocks.write(f"0x{start:04X} - 0x{end:04X} ({size} B) [{btype}]  {ascii_str}\n")

            # zapiš jednotlivé adresy
            for addr in range(start, end + 1):
                f_trace.write(f"0x{addr:04X}\n")

    print(f"[✓] data_blocks.txt vytvořen ({len(blocks)} bloků)")
    print(f"[✓] data_trace.txt vytvořen (adresy všech nalezených dat)")


# ================== HLAVNÍ PROGRAM ==================
def main():
    if not FIRMWARE_FILE.exists():
        print(f"[✗] Soubor {FIRMWARE_FILE} nebyl nalezen.")
        return

    fw = bytearray(FIRMWARE_FILE.read_bytes())
    if len(fw) < FIRMWARE_SIZE:
        fw.extend(b"\x00" * (FIRMWARE_SIZE - len(fw)))

    print(f"[i] Načten firmware ({len(fw)} bajtů).")
    print("[i] Hledám textové a konstantní bloky...")

    blocks = detect_blocks(fw)
    save_blocks(fw, blocks)

    if PRINT_SUMMARY:
        text_blocks = [b for b in blocks if b[2] == "text"]
        const_blocks = [b for b in blocks if "const" in b[2]]
        print("\n=== SOUHRN ===")
        print(f"Textové bloky : {len(text_blocks)}")
        print(f"Konstantní bloky : {len(const_blocks)}")
        total_bytes = sum(end - start + 1 for start, end, _ in blocks)
        print(f"Celkem označeno jako data: {total_bytes} B ({100*total_bytes/FIRMWARE_SIZE:.1f} %)")
        print("==============================")

if __name__ == "__main__":
    main()
