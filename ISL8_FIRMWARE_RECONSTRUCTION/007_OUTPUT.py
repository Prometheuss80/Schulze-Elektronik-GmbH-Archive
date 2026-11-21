#!/usr/bin/env python3
import os
import sys

MAP_FILE = "INFO_code_mapa.bin"
FW_FILE = "fw850en.bin"
OUT_FILE = "FW_ISL8.bin"
MEM_SIZE = 64 * 1024  # 64 KB

def main():
    # --- načtení mapy adres ---
    if not os.path.exists(MAP_FILE):
        print(f"❌ Soubor {MAP_FILE} nenalezen.")
        sys.exit(1)
    with open(MAP_FILE, "rb") as f:
        map_data = f.read()
    if len(map_data) != MEM_SIZE:
        print(f"⚠️  Velikost mapy je {len(map_data)} B, očekáváno {MEM_SIZE} B.")
        if len(map_data) > MEM_SIZE:
            map_data = map_data[:MEM_SIZE]
        else:
            map_data = map_data.ljust(MEM_SIZE, b'\x00')

    # --- načtení firmware dat ---
    if not os.path.exists(FW_FILE):
        print(f"❌ Soubor {FW_FILE} nenalezen.")
        sys.exit(1)
    with open(FW_FILE, "rb") as f:
        fw_data = f.read()

    # --- tvorba výstupu ---
    out = bytearray(MEM_SIZE)
    fw_index = 0
    inserted = 0

    for i in range(MEM_SIZE):
        if map_data[i] == 0x40:  # '@'
            if fw_index < len(fw_data):
                out[i] = fw_data[fw_index]
                fw_index += 1
                inserted += 1
            else:
                print(f"⚠️  Nedostatek dat ve {FW_FILE}, adresa 0x{i:04X}")
                break
        else:
            out[i] = 0x00

    # --- zápis výstupu ---
    with open(OUT_FILE, "wb") as f:
        f.write(out)

    print(f"✅ Hotovo.")
    print(f"   Výstupní soubor: {OUT_FILE}")
    print(f"   Velikost: {len(out):,} B")
    print(f"   Vložených hodnot z {FW_FILE}: {inserted:,}")
    print(f"   Zbývá nevyužito: {len(fw_data) - fw_index:,} B\n")

if __name__ == "__main__":
    main()
