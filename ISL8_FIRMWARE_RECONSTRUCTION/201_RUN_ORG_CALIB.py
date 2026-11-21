import serial
import time

ser = serial.Serial("COM3", 9600, timeout=1)

time.sleep(1)

data = bytes([0x25, 0x31, 0x32, 0x33, 0x0D, 0x35])
ser.write(data)

time.sleep(0.2)

response = ser.read(64)
print("RX:", response)

ser.close()