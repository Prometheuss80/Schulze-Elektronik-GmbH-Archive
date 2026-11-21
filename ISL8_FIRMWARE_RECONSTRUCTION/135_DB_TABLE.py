import re

data = []

with open("DB_TABLE_JOIN.txt") as f:
    for line in f:

        m = re.search(r'A_([0-9A-Fa-f]{4}).*0x([0-9A-Fa-f]{2})', line)

        if m:
            addr = int(m.group(1),16)
            val  = int(m.group(2),16)
            data.append((addr,val))

i = 0

while i < len(data):

    addr,val = data[i]

    # konec tabulky
    if i+1 < len(data) and data[i][1] == 0 and data[i+1][1] == 0:

        print(f"A_{addr:04X}: DB 0x00,0x00")

        i += 2
        print("")
        continue

    # standardní 3-byte záznam
    if i+2 < len(data):

        a1,v1 = data[i]
        a2,v2 = data[i+1]
        a3,v3 = data[i+2]

        print(f"A_{a1:04X}: DB 0x{v1:02X},0x{v2:02X},0x{v3:02X}")

        i += 3
    else:
        break