#!/usr/bin/env python3
import serial
import time
import os
import shutil

# ================== Konfigurace ==================
USB_PORT = '/dev/ttyACM0'
BAUDRATE = 2000000
BUFFER_SIZE_FILE = 4*1024*1024
BUFFER_SIZE_SERIAL = 8*1024*1024
LOG_INTERVAL = 5
TIMEOUT = 2
INIT_TIMEOUT = 10
MAX_DURATION = 7200 # 1 hodina

# ================== Příprava souboru ==================
script_dir = os.path.dirname(os.path.abspath(__file__))
log_file_path = os.path.join(script_dir, "LOG_ISL8.txt")
old_log_path = os.path.join(script_dir, "LOG_ISL8_old.txt")

if os.path.exists(log_file_path):
    if os.path.exists(old_log_path):
        os.remove(old_log_path)
    shutil.move(log_file_path, old_log_path)
    print(f"♻️  Starý log přejmenován na {os.path.basename(old_log_path)}")

print(f"📡 Logovací soubor: {log_file_path}")

f = open(log_file_path, "w", buffering=BUFFER_SIZE_FILE, encoding="ascii", errors="replace")

# ================== Inicializace sériového portu ==================
ser = serial.Serial(
    port=USB_PORT,
    baudrate=BAUDRATE,
    timeout=0,
    write_timeout=0
)

try:
    ser.set_buffer_size(rx_size=BUFFER_SIZE_SERIAL, tx_size=BUFFER_SIZE_SERIAL)
except AttributeError:
    pass

# ================== Čekání na první data ==================
print(f"⌛ Čekám na příchozí data (max {INIT_TIMEOUT} s)...")
start_wait = time.time()
start_time = None
while True:
    if ser.in_waiting > 0:
        start_time = time.time()
        last_data_time = start_time
        print("▶️ Zahájeno logování dat...")
        break
    if time.time() - start_wait > INIT_TIMEOUT:
        print("⚠️ Žádná data nepřišla, kontrola spojení selhala.")
        ser.close()
        f.close()
        exit(1)
    time.sleep(0.1)

# ================== Logger ==================
last_report_time = 0
bytes_written = 0
line_count = 0

while True:
    data = ser.read(ser.in_waiting or 1024)
    if data:
        decoded = data.decode('ascii', errors='replace')
        f.write(decoded)
        bytes_written += len(decoded)
        line_count += decoded.count('\n')
        last_data_time = time.time()

    now = time.time()

    # výpis každých LOG_INTERVAL sekund
    if now - last_report_time >= LOG_INTERVAL:
        elapsed = now - start_time
        mb_written = bytes_written / (1024 * 1024)
        print(
            f"\r⏱ {int(elapsed // 60):02d}:{int(elapsed % 60):02d} | "
            f"📄 {mb_written:.1f} MB | 🧾 {line_count:,} řádků",
            end=''
        )
        last_report_time = now

    # ukončení po neaktivitě
    if now - last_data_time > TIMEOUT:
        elapsed = now - start_time
        print(f"\n📁 Konec souboru — přenos ukončen (bez dat).  ⏱ {int(elapsed // 60):02d}:{int(elapsed % 60):02d}")
        break

    # ukončení po dosažení limitu
    if now - start_time >= MAX_DURATION:
        elapsed = now - start_time
        print(f"\n🕒 Maximální doba logování dosažena — ukončuji záznam.  ⏱ {int(elapsed // 60):02d}:{int(elapsed % 60):02d}")
        break

# ================== Uzavření ==================
f.close()
ser.close()

elapsed = time.time() - start_time
print(f"✅ Logování dokončeno. Celkem {line_count:,} řádků, {bytes_written/1024:.1f} kB dat.  ⏱ {int(elapsed // 60):02d}:{int(elapsed % 60):02d}")
