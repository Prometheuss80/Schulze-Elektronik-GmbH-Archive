# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : display.py
#
# OLED Display Driver
#
# Sprint 7
# UI Version 2.0
# ==========================================================

import framebuf
from time import ticks_ms, ticks_diff

import inithw
import system
import settings

import battery


MAIN_REFRESH_MS = 250
_lastMainRefresh = ticks_ms()


# ==========================================================
# INIT
# ==========================================================

def Init():

    inithw.display.fill(0)
    inithw.display.show()


# ==========================================================
# TASK
# ==========================================================

def Task():

    global _lastMainRefresh

    if not system.IsReady():
        return

    now = ticks_ms()

    if ticks_diff(
        now,
        _lastMainRefresh
    ) < MAIN_REFRESH_MS:
        return

    _lastMainRefresh = now

    ShowMain()


# ==========================================================
# PRIVATE
# ==========================================================

def _ShowPBM(filename):

    try:

        with open(filename, "rb") as f:

            f.readline()
            f.readline()

            width, height = map(
                int,
                f.readline().decode().split()
            )

            data = bytearray(
                f.read()
            )

        fb = framebuf.FrameBuffer(
            data,
            width,
            height,
            framebuf.MONO_HLSB
        )

        x = (128 - width) // 2
        y = (64 - height) // 2

        inithw.display.fill(0)

        inithw.display.blit(
            fb,
            x,
            y
        )

        inithw.display.show()

    except:

        inithw.display.fill(0)

        inithw.display.text(
            "LOGO ERROR",
            0,
            28
        )

        inithw.display.show()


# ==========================================================
# BOOT / LOGO
# ==========================================================

def ShowLogo():

    _ShowPBM(
        "schulze.pbm"
    )


def ShowBoot():

    inithw.display.fill(0)

    inithw.display.text(
        "LipoProfiBal06",
        0,
        0
    )

    inithw.display.text(
        "FW 1.00",
        0,
        16
    )

    inithw.display.text(
        "HW Rev A",
        0,
        28
    )

    inithw.display.text(
        "Boot...",
        0,
        52
    )

    inithw.display.show()


# ==========================================================
# INFO
# ==========================================================

def ShowInfo(text1, text2):

    inithw.display.fill(0)

    inithw.display.text(
        text1,
        0,
        18
    )

    inithw.display.text(
        text2,
        0,
        34
    )

    inithw.display.show()


# ==========================================================
# MAIN BATTERY SCREEN
# ==========================================================

def ShowMain():

    inithw.display.fill(0)

    count = battery.CellCount

    if count < 1:
        count = 0
    elif count > 6:
        count = 6

    active = battery.CellVoltage[:count]
    umax = max(active) if active else 0

    # Always anchor active cells to the bottom of the six-line
    # battery area:
    #   6S -> rows 0..5
    #   4S -> rows 2..5
    #   2S -> rows 4..5
    first_row = 6 - count

    for i in range(count):

        cell = count - 1 - i
        row = first_row + i

        voltage = battery.CellVoltage[cell]
        diff = umax - voltage

        txt = "{:1d}:  {:4d}  {:3d}mV".format(
            cell + 1,
            voltage,
            diff
        )

        inithw.display.text(
            txt,
            0,
            row * 11
        )

    inithw.display.show()


def ShowCellCountMismatch():

    inithw.display.fill(0)

    inithw.display.text("WRONG CELL COUNT", 0, 0)

    inithw.display.text(
        "Set:{}S Det:{}S".format(
            settings.CellCount,
            battery.DetectedCellCount
        ),
        0,
        18
    )

    inithw.display.text("SET = AUTO", 0, 38)
    inithw.display.text("MENU = ERROR", 0, 52)

    inithw.display.show()


def ShowBatteryError():

    line1, line2 = battery.CheckText()

    inithw.display.fill(0)
    inithw.display.text("BATTERY ERROR", 0, 0)
    inithw.display.text(line1, 0, 22)
    inithw.display.text(line2, 0, 40)
    inithw.display.show()


# ==========================================================
# MENU SCREEN
# ==========================================================

def ShowMenu(page, title, line1="", line2=""):

    inithw.display.fill(0)

    inithw.display.text(
        "MENU",
        0,
        0
    )

    inithw.display.text(
        "{}/3".format(page),
        104,
        0
    )

    inithw.display.hline(
        0,
        10,
        128,
        1
    )

    inithw.display.text(
        title,
        0,
        20
    )

    if line1 != "":

        inithw.display.text(
            line1,
            0,
            36
        )

    if line2 != "":

        inithw.display.text(
            line2,
            0,
            50
        )

    inithw.display.show()


# ==========================================================
# SETUP VIEW / EDIT
# ==========================================================

def ShowSetup(
    item,
    title,
    value,
    edit=False,
    changed=False
):

    inithw.display.fill(0)

    # ------------------------------------------------------
    # HEADER
    # ------------------------------------------------------

    if edit:

        header = "EDIT"

    else:

        header = "SET"

    inithw.display.text(
        header,
        0,
        0
    )

    inithw.display.text(
        item,
        96,
        0
    )

    inithw.display.hline(
        0,
        10,
        128,
        1
    )

    # ------------------------------------------------------
    # TITLE
    # ------------------------------------------------------

    inithw.display.text(
        title,
        0,
        22
    )

    # ------------------------------------------------------
    # VALUE
    # ------------------------------------------------------

    value_text = str(
        value
    )

    if edit and changed:

        value_text = ">" + value_text + "<"

    inithw.display.text(
        value_text,
        0,
        44
    )

    inithw.display.show()


# ==========================================================
# SETUP CHEMISTRY
# ----------------------------------------------------------
# Compatibility with older code
# ==========================================================

def ShowChemistry(name):

    ShowSetup(
        "1/8",
        "Chemistry",
        name,
        False
    )