import serial
import time

# ==== NASTAVENÍ PORTU ====
PORT = "COM3"          # Windows: COM5 apod.
#PORT = "/dev/ttyUSB0" # Linux/RPi

BAUD = 9600

ser = serial.Serial(
    PORT,
    BAUD,
    timeout=0.2,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE
)

print("Otevřen port:", PORT)

def send_hex(hex_list, pause=0.15):
    data = bytes(hex_list)
    ser.write(data)
    ser.flush()
    print("TX:", data.hex(" "))
    time.sleep(pause)

#print("Start sekvence...")
time.sleep(1)


# ---- 2 SERVICE ENABLE ----
send_hex([0x13, 0x11, 0x2F], 0.5)

# ---- 3 SERVICE GROUP ----
send_hex([0x13, 0x11, 0x1E], 0.5)

# ---- 4 CALIB ----
send_hex([0x13, 0x11, 0xE1], 0.5)

print("READY")
ser.close()