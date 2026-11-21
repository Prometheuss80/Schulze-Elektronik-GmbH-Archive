#!/usr/bin/env python3
import os
import glob

# === Nastavení ===
INPUT_DIR = "SPLIT"
OUTPUT_FILE = "LOG_ISL8_ALL.txt"
BASE_NAME = "LOG_ISL8_"
EXT = ".txt"

def join_files():
    # Najdi všechny části ve složce split/
    files = sorted(glob.glob(os.path.join(INPUT_DIR, f"{BASE_NAME}[0-9][0-9][0-9]{EXT}")))
    
    if not files:
        print("⚠️ Ve složce 'split' nebyly nalezeny žádné části k spojení.")
        return

    print(f"🔍 Nalezeno {len(files)} souborů, spojuji...")

    # Smaž starý výstup, pokud existuje
    if os.path.exists(OUTPUT_FILE):
        os.remove(OUTPUT_FILE)
        print(f"🧹 Starý soubor {OUTPUT_FILE} odstraněn.")

    with open(OUTPUT_FILE, "wb") as outfile:
        for i, f in enumerate(files):
            print(f"📄 Přidávám {os.path.basename(f)} ({i+1}/{len(files)})")
            with open(f, "rb") as infile:
                while True:
                    chunk = infile.read(1024 * 1024)  # čtení po 1 MB
                    if not chunk:
                        break
                    outfile.write(chunk)

    print(f"✅ Hotovo — všechny části spojeny do {OUTPUT_FILE}")

if __name__ == "__main__":
    join_files()
