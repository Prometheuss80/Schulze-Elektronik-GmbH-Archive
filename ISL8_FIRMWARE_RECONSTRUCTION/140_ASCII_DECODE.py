import re

data = []

with open("Reconstruction.a51") as f:
    for line in f:
        m = re.search(r'0x([0-9A-Fa-f]{2})', line)
        if m:
            data.append(int(m.group(1),16))

CHAR_W = 5
CHAR_H = 7

start_ascii = 0x20

for c in range(len(data)//CHAR_W):

    char = data[c*CHAR_W:(c+1)*CHAR_W]

    print()
    print("ASCII",hex(start_ascii+c), chr(start_ascii+c))

    for y in range(CHAR_H):

        line=""

        for x in range(CHAR_W):

            bit = (char[x] >> y) & 1

            if bit:
                line+="#"
            else:
                line+="."

        print(line)