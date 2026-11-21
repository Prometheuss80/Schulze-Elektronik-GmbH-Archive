import re

data = []

with open("DB_DATA_JOIN.txt") as f:
    for line in f:
        m = re.search(r'A_([0-9A-Fa-f]{4}).*0x([0-9A-Fa-f]{2})', line)
        if m:
            addr = int(m.group(1),16)
            val  = int(m.group(2),16)
            data.append((addr,val))

i = 0

while i < len(data):

    addr,val = data[i]

    if not (32 <= val <= 126 or val in (9,10,13)):
        print(f"A_{addr:04X}: DB 0x{val:02X}")
        i += 1
        continue

    start_addr = addr
    text_bytes = []
    j = i

    while j < len(data):

        b = data[j][1]

        if b == 0:
            break

        if not (32 <= b <= 126 or b in (9,10,13)):
            text_bytes = []
            break

        text_bytes.append(b)
        j += 1

    if text_bytes and len(text_bytes) >= 4:

        parts = []
        text = ""

        for b in text_bytes:

            if b == 10:
                if text:
                    parts.append(f'"{text}"')
                    text = ""
                parts.append("0x0A")
            elif b == 13:
                if text:
                    parts.append(f'"{text}"')
                    text = ""
                parts.append("0x0D")
            elif b == 9:
                if text:
                    parts.append(f'"{text}"')
                    text = ""
                parts.append("0x09")
            elif b == 34:
                text += '\\"'
            elif b == 92:
                text += "\\\\"
            else:
                text += chr(b)

        if text:
            parts.append(f'"{text}"')

        parts.append("0")

        db_line = ",".join(parts)

        print(f"\nA_{start_addr:04X}:")
        print(f"    DB {db_line}")

        i = j + 1

    else:
        print(f"A_{addr:04X}: DB 0x{val:02X}")
        i += 1