# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : led.py
#
# LED Driver
# ==========================================================

from time import sleep_ms, ticks_ms

import inithw
import system
import battery

# ==========================================================
# INIT
# ==========================================================

def Init():

    AllOff()
    ReadyOff()
    DebugOff()

# ==========================================================
# TASK
# ==========================================================

def Task():

    if system.IsReady():
        ReadyOn()
    else:
        ReadyOff()

    if system.IsError():

        AllOff()

        cell = battery.ErrorCell

        if 0 <= cell < len(inithw.CellLED):

            if ((ticks_ms() // 500) & 1) == 0:
                CellOn(cell)

    elif not system.IsReady():

        AllOff()

    # In READY, balance.py owns the CELL LEDs.


# ==========================================================
# DEBUG LED
# ==========================================================

def DebugOn():
    inithw.DebugLED.on()

def DebugOff():
    inithw.DebugLED.off()

def DebugBlink(time=100):

    inithw.DebugLED.on()
    sleep_ms(time)
    inithw.DebugLED.off()

# ==========================================================
# READY LED
# ==========================================================

def ReadyOn():
    inithw.ReadyLED.on()

def ReadyOff():
    inithw.ReadyLED.off()

# ==========================================================
# CELL LED
# ==========================================================

def CellOn(cell):

    if 0 <= cell < len(inithw.CellLED):
        inithw.CellLED[cell].on()

def CellOff(cell):

    if 0 <= cell < len(inithw.CellLED):
        inithw.CellLED[cell].off()

# ==========================================================
# ALL CELL LED
# ==========================================================

def AllOn():

    for led in inithw.CellLED:
        led.on()

def AllOff():

    for led in inithw.CellLED:
        led.off()

# ==========================================================
# SHOW DETECTED CELLS
# ==========================================================

def ShowCells(count):

    AllOff()

    for i in range(min(count, len(inithw.CellLED))):
        inithw.CellLED[i].on()

# ==========================================================
# LED TEST
# ==========================================================

def Test():

    AllOn()
    sleep_ms(500)

    AllOff()
    sleep_ms(250)

    for led in inithw.CellLED:

        led.on()
        sleep_ms(200)
        led.off()

    sleep_ms(250)

    AllOn()
    sleep_ms(500)

    AllOff()