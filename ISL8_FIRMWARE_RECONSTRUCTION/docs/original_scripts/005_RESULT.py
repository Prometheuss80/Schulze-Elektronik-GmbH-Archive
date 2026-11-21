#!/usr/bin/env python3
import re
import time

LOG_FILE = "LOG_ISL8_ALL.txt"
OUT_FILE = "INFO_code_mapa.bin"

pattern = re.compile(r'^([0-9A-F]{4})\*\d@$')
visited = bytearray(b'\x00' * 65536)

total_lines = 0
found_addresses = 0
total_possible = 65536
REPORT_INTERVAL_LINES = 1_000_000

print(f"📄 Analýza logu: {LOG_FILE}")
start_time = time.time()

with open(LOG_FILE, "r", encoding="ascii", errors="replace") as f:
    for line in f:
        total_lines += 1
        line = line.strip()

        match = pattern.match(line)
        if match:
            addr = int(match.group(1), 16)
            if visited[addr] == 0:
                visited[addr] = ord('@')  # uloží ASCII znak '@'
                found_addresses += 1

        # průběžný výpis každého milionu řádků
        if total_lines % REPORT_INTERVAL_LINES == 0:
            coverage = (found_addresses / total_possible) * 100
            elapsed = time.time() - start_time
            h = int(elapsed // 3600)
            m = int((elapsed % 3600) // 60)
            s = int(elapsed % 60)
            print(f"\r🔹 Zpracováno {total_lines:,} řádků | nalezeno {found_addresses:,} adres | "
                  f"pokrytí: {coverage:.4f}% | ⏱ {h:02d}:{m:02d}:{s:02d}", end='')

# konečný výpis (přejdeme na nový řádek)
print()

# uložíme mapu jako ASCII znaky
with open(OUT_FILE, "wb") as out:
    out.write(visited)

elapsed = time.time() - start_time
h = int(elapsed // 3600)
m = int((elapsed % 3600) // 60)
s = int(elapsed % 60)
coverage = (found_addresses / total_possible) * 100

print("\n✅ Analýza dokončena")
print(f"⏱ Doba zpracování: {h:02d}:{m:02d}:{s:02d}")
print(f"📈 Nalezeno adres: {found_addresses:,} / 65,536")
print(f"📊 Pokrytí: {coverage:.4f}%")
print(f"💾 Výsledek uložen do: {OUT_FILE}")
