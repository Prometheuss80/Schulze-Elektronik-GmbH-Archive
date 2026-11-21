#!/usr/bin/env python3
import time
import os

filename = "LOG_ISL8.txt"

bad_lines = 0
total_lines = 0
prev_counter = None
in_error_seq = False

PROGRESS_INTERVAL = 1_000_000
start_time = time.time()

# ------------------------------------------------------------
# ZÍSKÁNÍ POČTU ŘÁDKŮ – BEZ NAČÍTÁNÍ DO RAM
# ------------------------------------------------------------
print("[i] Počítám řádky (stream)...")

total_lines_est = 0
with open(filename, "rb") as f:
    for _ in f:
        total_lines_est += 1

print(f"[i] Odhad celkových řádků: {total_lines_est:,}\n")

# ------------------------------------------------------------
# BEZPEČNÁ KONTROLA POSLEDNÍHO ŘÁDKU
# ------------------------------------------------------------
print("[i] Kontroluji poslední řádek...")

with open(filename, "rb") as f:
    try:
        f.seek(-2, os.SEEK_END)
        while True:
            if f.read(1) == b"\n":
                break
            f.seek(-2, os.SEEK_CUR)
    except OSError:
        pass

    last_line = f.readline().decode("ascii", errors="replace").rstrip("\r\n")

if len(last_line) != 7:
    print("⚠️  Poslední řádek je nekompletní — bude odstraněn.")

    with open(filename, "rb") as f, open(filename + ".tmp", "wb") as w:
        for raw in f:
            if len(raw.strip()) == 7:
                w.write(raw)

    os.replace(filename + ".tmp", filename)
    print("✔️  Opraveno.\n")
else:
    print("✔️  Poslední řádek je kompletní.\n")


# ------------------------------------------------------------
# VALIDACE ŘÁDKU (je extrémně rychlá)
# ------------------------------------------------------------
def is_valid_line(line, prev_counter):
    if len(line) != 7:
        return False, prev_counter

    # hex
    if not all(c in "0123456789ABCDEF" for c in line[0:4]):
        return False, prev_counter

    if line[4] != "*":
        return False, prev_counter

    if not line[5].isdigit():
        return False, prev_counter

    cur = int(line[5])

    if prev_counter is not None:
        expected = (prev_counter + 1) % 10
        if cur != expected:
            return False, cur

    if line[6] != "@":
        return False, cur

    return True, cur


# ------------------------------------------------------------
# HLAVNÍ STREAM ČTENÍ + LIVE PROGRESS
# ------------------------------------------------------------
print("📄 Kontrola logu spuštěna...\n")

with open(filename, "r", encoding="ascii", errors="replace") as f:
    for line_num, raw_line in enumerate(f, start=1):
        
        current_line = raw_line.rstrip("\r\n")

        # ---- PRŮBĚŽNÝ PROGRESS ----
        if line_num % PROGRESS_INTERVAL == 0:
            elapsed = time.time() - start_time
            speed = line_num / elapsed if elapsed > 0 else 0
            eta = (total_lines_est - line_num) / speed if speed > 0 else 0

            h = int(elapsed // 3600); m = int((elapsed % 3600) // 60); s = int(elapsed % 60)
            eh = int(eta // 3600); em = int((eta % 3600) // 60); es = int(eta % 60)
            pct = (line_num / total_lines_est) * 100 if total_lines_est else 0

            print(
                f"\r⏳ {pct:6.2f}% | "
                f"Řádků: {line_num:,}/{total_lines_est:,} | "
                f"⚡ {int(speed):8d} ř/s | "
                f"ETA {eh:02d}:{em:02d}:{es:02d} | "
                f"⏱ {h:02d}:{m:02d}:{s:02d}",
                end='', flush=True
            )

        # ---- VALIDACE ----
        ok, prev_counter = is_valid_line(current_line, prev_counter)

        if not ok:
            bad_lines += 1
            if not in_error_seq:
                print(f"\n❌ Chybný řádek {line_num}: {current_line}")
                in_error_seq = True
        else:
            in_error_seq = False

elapsed = time.time() - start_time
h = int(elapsed // 3600); m = int((elapsed % 3600) // 60); s = int(elapsed % 60)

print("\n\n✅ Kontrola dokončena")
print(f"Celkem řádků: {line_num:,}")
print(f"Chybných řádků: {bad_lines:,}")
print(f"⏱ Celkový čas: {h:02d}:{m:02d}:{s:02d}")
