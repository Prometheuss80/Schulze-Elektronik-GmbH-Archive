#!/usr/bin/env python3
import re
import time
import sys

LOG_FILE = "LOG_ISL8_ALL.txt"
TXT_OUT = "INFO_data.txt"
BIN_OUT = "INFO_data_mapa.bin"

pattern = re.compile(r"^([0-9A-F]{4})\*\d@$")
REPORT_INTERVAL = 1_000_000

total_lines = 0
lonely = set()        # osamocené adresy (unikátní)
all_addresses = set() # všechny adresy pro mapu
window = []           # tři poslední adresy

start_time = time.time()

print("📄 Analýza logu spuštěna...\n")

with open(LOG_FILE, "r") as f:
    for line in f:
        total_lines += 1
        m = pattern.match(line.strip())
        if not m:
            continue

        addr = int(m.group(1), 16)
        all_addresses.add(addr)
        window.append(addr)

        if len(window) > 3:
            window.pop(0)

        if len(window) == 3:
            a1, a2, a3 = window

            # Pokud a2 není sousední ani s a1, ani s a3, označíme ji
            if (a2 != a1 + 1) and (a3 != a2 + 1):
                lonely.add(a2)

            # Pokud se ukáže, že a2 a a3 jsou sousední, zrušíme a2 jako osamocenou
            if a2 + 1 == a3 and a2 in lonely:
                lonely.remove(a2)

        # průběžný výpis každého milionu řádků (na stejný řádek)
        if total_lines % REPORT_INTERVAL == 0:
            elapsed = time.time() - start_time
            h = int(elapsed // 3600)
            m_ = int((elapsed % 3600) // 60)
            s = int(elapsed % 60)
            sys.stdout.write(
                f"\rŘádků: {total_lines:,} | Osamocené: {len(lonely):,} | "
                f"Čas: {h:02d}:{m_:02d}:{s:02d}"
            )
            sys.stdout.flush()

# přejdeme na nový řádek po posledním výpisu
print()

# závěr – kontrola posledních adres
if len(window) >= 2:
    a1, a2 = window[-2:]
    if a2 != a1 + 1:
        lonely.add(a2)

# výstup do souboru TXT – setříděný
with open(TXT_OUT, "w") as out:
    for a in sorted(lonely):
        out.write(f"{a:04X}\n")
    out.write(f"\nCelkem osamocených adres: {len(lonely)}\n")

# výstup binární mapy
mapa = bytearray(0x00 for _ in range(65536))
for a in lonely:
    mapa[a] = 0x40  # '@'

with open(BIN_OUT, "wb") as f:
    f.write(mapa)

elapsed = time.time() - start_time
h = int(elapsed // 3600)
m_ = int((elapsed % 3600) // 60)
s = int(elapsed % 60)

print("\n✅ Hotovo!")
print(f"   Celkem řádků: {total_lines:,}")
print(f"   Osamocených adres: {len(lonely):,}")
print(f"   Výstupy: {TXT_OUT}, {BIN_OUT}")
print(f"⏱️  Doba běhu: {h:02d}:{m_:02d}:{s:02d}")
