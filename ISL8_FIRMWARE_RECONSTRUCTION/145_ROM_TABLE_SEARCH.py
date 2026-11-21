import re
import struct

rom = {}

pattern = re.compile(r"A_([0-9A-Fa-f]{4}):\s+DB\s+0x([0-9A-Fa-f]{2})")

with open("Reconstruction.a51") as f:
    for line in f:
        m = pattern.search(line)
        if m:
            addr = int(m.group(1), 16)
            val = int(m.group(2), 16)
            rom[addr] = val

# vytvoření lineární ROM
start = min(rom.keys())
end = max(rom.keys())

data = bytearray()

for a in range(start, end + 1):
    data.append(rom.get(a, 0))

print("ROM range:", hex(start), "-", hex(end))
print()

# -----------------------------
# ASCII STRINGS
# -----------------------------

print("ASCII STRINGS:")
for i in range(len(data)-4):
    s = []
    j = i
    while j < len(data) and 32 <= data[j] <= 126:
        s.append(chr(data[j]))
        j += 1

    if len(s) >= 6:
        print(hex(start+i), "".join(s))
        i = j


print("\n16-bit increasing tables:")

# -----------------------------
# 16bit TABLE DETECTION
# -----------------------------

for i in range(0, len(data)-12, 2):

    v1 = data[i]<<8 | data[i+1]
    v2 = data[i+2]<<8 | data[i+3]
    v3 = data[i+4]<<8 | data[i+5]
    v4 = data[i+6]<<8 | data[i+7]

    if v1 < v2 < v3 < v4:
        if 500 < v1 < 10000:
            print(hex(start+i),
                  [v1,v2,v3,v4])


print("\nPossible voltage tables:")

# -----------------------------
# VOLTAGE TABLES
# -----------------------------

for i in range(0,len(data)-6,2):

    v1 = data[i]<<8 | data[i+1]
    v2 = data[i+2]<<8 | data[i+3]
    v3 = data[i+4]<<8 | data[i+5]

    if 3000 < v1 < 4500 and abs(v2-v1) < 500:
        print(hex(start+i), v1,v2,v3)


print("\n32-bit constants:")

# -----------------------------
# 32bit CONSTANTS
# -----------------------------

for i in range(len(data)-8):

    v = struct.unpack(">I", data[i:i+4])[0]

    if 1000000 < v < 100000000:
        print(hex(start+i), hex(v))