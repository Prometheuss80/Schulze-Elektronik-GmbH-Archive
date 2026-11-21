#!/usr/bin/env python3
import os
import glob

# === Nastavení ===
INPUT_FILE = "LOG_ISL8.txt"
OUTPUT_DIR = "SPLIT"
CHUNK_SIZE_MB = 100
CHUNK_SIZE = CHUNK_SIZE_MB * 1024 * 1024  # bajtů
BASE_NAME = "LOG_ISL8_"
EXT = ".txt"

def prepare_output_dir():
    # Vytvoř složku, pokud neexistuje
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Odstraň staré části
    old_files = glob.glob(os.path.join(OUTPUT_DIR, f"{BASE_NAME}*{EXT}"))
    if old_files:
        print(f"🧹 Mažu {len(old_files)} starých souborů...")
        for f in old_files:
            try:
                os.remove(f)
            except OSError as e:
                print(f"⚠️ Nelze smazat {f}: {e}")
    else:
        print("✅ Složka je prázdná, nic k mazání.")

def split_file():
    prepare_output_dir()
    part = 0
    bytes_written = 0
    output = None

    with open(INPUT_FILE, "rb") as infile:
        while True:
            chunk = infile.read(1024 * 1024)  # čti po 1 MB
            if not chunk:
                break

            # Když není otevřený výstup, otevři nový
            if output is None:
                out_name = os.path.join(OUTPUT_DIR, f"{BASE_NAME}{part:03d}{EXT}")
                print(f"📄 Vytvářím {out_name}...")
                output = open(out_name, "wb")
                bytes_written = 0

            output.write(chunk)
            bytes_written += len(chunk)

            # Pokud jsme dosáhli limitu, zavři a přejdi na další část
            if bytes_written >= CHUNK_SIZE:
                output.close()
                output = None
                part += 1

        # Zavři poslední otevřený soubor
        if output:
            output.close()

    print("✅ Hotovo — soubor rozdělen na části.")

if __name__ == "__main__":
    split_file()
