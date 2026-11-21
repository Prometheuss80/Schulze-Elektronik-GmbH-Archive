import serial
import time

# otevření portu
ser = serial.Serial(
    port="COM3",
    baudrate=9600,
    bytesize=8,
    parity='N',
    stopbits=1,
    timeout=1
)

time.sleep(1)  # krátká pauza po otevření

# HEX sekvence: %123\r5
data = bytes([0x25, 0x31, 0x32, 0x33, 0x0D, 0x35])

print("Sending:", data.hex())

ser.write(data)

ser.flush()
time.sleep(0.5)

ser.close()

print("Done.")
