# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : fram.py
#
# MB85RC256 FRAM Driver
# ==========================================================

import inithw

FRAM_ADDR = 0x50


# ==========================================================
# INIT
# ==========================================================

def Init():
    pass


# ==========================================================
# READ BYTE
# ==========================================================

def ReadByte(addr):

    adr = bytes([
        (addr >> 8) & 0xFF,
        addr & 0xFF
    ])

    inithw.i2c.writeto(FRAM_ADDR, adr)

    return inithw.i2c.readfrom(FRAM_ADDR, 1)[0]


# ==========================================================
# WRITE BYTE
# ==========================================================

def WriteByte(addr, value):

    data = bytes([
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        value & 0xFF
    ])

    inithw.i2c.writeto(FRAM_ADDR, data)


# ==========================================================
# READ WORD (16 bit)
# ==========================================================

def ReadWord(addr):

    lo = ReadByte(addr)
    hi = ReadByte(addr + 1)

    return lo | (hi << 8)


# ==========================================================
# WRITE WORD (16 bit)
# ==========================================================

def WriteWord(addr, value):

    WriteByte(addr, value & 0xFF)
    WriteByte(addr + 1, (value >> 8) & 0xFF)


# ==========================================================
# READ BLOCK
# ==========================================================

def ReadBlock(addr, length):

    adr = bytes([
        (addr >> 8) & 0xFF,
        addr & 0xFF
    ])

    inithw.i2c.writeto(FRAM_ADDR, adr)

    return inithw.i2c.readfrom(FRAM_ADDR, length)


# ==========================================================
# WRITE BLOCK
# ==========================================================

def WriteBlock(addr, data):

    buf = bytearray()

    buf.append((addr >> 8) & 0xFF)
    buf.append(addr & 0xFF)

    buf.extend(data)

    inithw.i2c.writeto(FRAM_ADDR, buf)