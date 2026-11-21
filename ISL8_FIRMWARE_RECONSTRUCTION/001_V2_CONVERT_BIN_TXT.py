#!/usr/bin/env python3
import os
import time

# ================== Soubory ==================
script_dir = os.path.dirname(os.path.abspath(__file__))

input_file  = os.path.join(script_dir, "LOG_ISL8.BIN")
output_file = os.path.join(script_dir, "LOG_ISL8_ALL.txt")

# ================== Statistiky ==================
total_records = 0
error_sync = 0
error_seq = 0
error_crc = 0
error_addr = 0

last_seq = None

REPORT_INTERVAL = 5  # sekundy

print(f"📄 Konverze: {input_file}")
start_time = time.time()
last_report = start_time

# ================== Konverze ==================
with open(input_file, "rb") as fin, open(output_file, "w") as fout:

    while True:
        chunk = fin.read(5)
        if len(chunk) != 5:
            break

        lo, hi, marker, seq, crc = chunk

        # --- kontrola markeru ---
        if marker != 0xFF:
            error_sync += 1
            print("\n❌ SYNC ERROR")
            break

        # --- kontrola sekvence ---
        if not (0xA0 <= seq <= 0xA9):
            error_seq += 1
            print("\n❌ SEQ ERROR")
            break

        # --- CRC ---
        calc_crc = lo ^ hi ^ seq
        if crc != calc_crc:
            error_crc += 1
            print("\n❌ CRC ERROR")
            break

        # --- dekódování adresy ---
        addr = lo | (hi << 8)

        if addr > 0xFFFF:
            error_addr += 1
            print(f"\n❌ ADDR ERROR: {addr:04X}")
            break

        seq_val = seq - 0xA0

        # --- sekvenční kontrola ---
        if last_seq is not None:
            if seq_val != (last_seq + 1) % 10:
                error_seq += 1
                print("\n❌ SEQ BREAK")
                break

        last_seq = seq_val

        # --- zápis ---
        fout.write(f"{addr:04X}*{seq_val}@\n")

        total_records += 1

        # --- průběžný výpis ---
        now = time.time()
        if now - last_report >= REPORT_INTERVAL:
            elapsed = now - start_time
            h = int(elapsed // 3600)
            m = int((elapsed % 3600) // 60)
            s = int(elapsed % 60)

            print(
                f"\r⏱ {h:02d}:{m:02d}:{s:02d} | "
                f"📄 {total_records:,} záznamů",
                end=""
            )

            last_report = now

# ================== Výstup ==================
elapsed = time.time() - start_time
h = int(elapsed // 3600)
m = int((elapsed % 3600) // 60)
s = int(elapsed % 60)

print("\n======================================")
print(f"📄 Zpracováno: {total_records:,} záznamů")
print(f"❌ Sync: {error_sync}")
print(f"❌ Seq: {error_seq}")
print(f"❌ CRC: {error_crc}")
print(f"❌ Addr: {error_addr}")
print(f"⏱ Čas: {h:02d}:{m:02d}:{s:02d}")
print(f"💾 Výstup: {output_file}")
print("======================================")