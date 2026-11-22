# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : buttons.py
#
# Button Driver
#
# Sprint 7
# UI Version 2.0
# ==========================================================

from time import ticks_ms, ticks_diff

import inithw


# ==========================================================
# CONSTANTS
# ==========================================================

LONG_PRESS_MS = 2000


# ==========================================================
# VARIABLES
# ==========================================================

_oldMenu = 0
_oldSet = 0

_setStart = 0
_setLongHandled = False

MenuEvent = False
SetEvent = False
SetLongEvent = False


# ==========================================================
# INIT
# ==========================================================

def Init():

    global _oldMenu
    global _oldSet
    global _setStart
    global _setLongHandled

    global MenuEvent
    global SetEvent
    global SetLongEvent

    _oldMenu = inithw.ButtonMenu.value()
    _oldSet = inithw.ButtonSet.value()

    _setStart = 0
    _setLongHandled = False

    MenuEvent = False
    SetEvent = False
    SetLongEvent = False


# ==========================================================
# TASK
# ==========================================================

def Task():

    global _oldMenu
    global _oldSet
    global _setStart
    global _setLongHandled

    global MenuEvent
    global SetEvent
    global SetLongEvent

    menu = inithw.ButtonMenu.value()
    sett = inithw.ButtonSet.value()

    # ------------------------------------------------------
    # Clear events
    # ------------------------------------------------------

    MenuEvent = False
    SetEvent = False
    SetLongEvent = False


    # ======================================================
    # MENU BUTTON
    # ======================================================

    if menu and not _oldMenu:

        MenuEvent = True


    # ======================================================
    # SET BUTTON
    # ======================================================

    # ------------------------------------------------------
    # New SET press
    # ------------------------------------------------------

    if sett and not _oldSet:

        _setStart = ticks_ms()

        _setLongHandled = False


    # ------------------------------------------------------
    # SET held
    # ------------------------------------------------------

    if sett and _oldSet:

        if not _setLongHandled:

            pressTime = ticks_diff(
                ticks_ms(),
                _setStart
            )

            if pressTime >= LONG_PRESS_MS:

                SetLongEvent = True

                _setLongHandled = True


    # ------------------------------------------------------
    # SET release
    # ------------------------------------------------------

    if not sett and _oldSet:

        # If long press was already handled,
        # do NOT generate a short press.

        if not _setLongHandled:

            pressTime = ticks_diff(
                ticks_ms(),
                _setStart
            )

            if pressTime < LONG_PRESS_MS:

                SetEvent = True


        _setStart = 0


    # ======================================================
    # STORE CURRENT STATE
    # ======================================================

    _oldMenu = menu
    _oldSet = sett


# ==========================================================
# MENU PRESSED
# ==========================================================

def MenuPressed():

    global MenuEvent

    if MenuEvent:

        MenuEvent = False

        return True

    return False


# ==========================================================
# SET PRESSED
# ==========================================================

def SetPressed():

    global SetEvent

    if SetEvent:

        SetEvent = False

        return True

    return False


# ==========================================================
# SET LONG PRESSED
# ==========================================================

def SetLongPressed():

    global SetLongEvent

    if SetLongEvent:

        SetLongEvent = False

        return True

    return False