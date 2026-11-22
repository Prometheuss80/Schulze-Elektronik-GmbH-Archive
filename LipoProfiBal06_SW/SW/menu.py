# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : menu.py
#
# Menu Engine
#
# Sprint 7
# UI Version 2.0
# ==========================================================

import display
import settings
import system
import led
import battery
from time import ticks_ms, ticks_diff


# ==========================================================
# MAIN MENU
# ==========================================================

MENU_MAIN  = 0
MENU_INFO  = 1
MENU_STATS = 2

MENU_COUNT = 3


# ==========================================================
# SETUP MENU
# ==========================================================

SET_CHEMISTRY = 0
SET_CELLCOUNT = 1
SET_UMAX      = 2
SET_UMIN      = 3
SET_DELTA     = 4
SET_UART      = 5
SET_FACTORY   = 6
SET_SAVE      = 7

SET_COUNT = 8

global EditLastActivity

EditLastActivity = ticks_ms()

# ==========================================================
# VARIABLES
# ==========================================================

Menu = MENU_MAIN

SetupItem = SET_CHEMISTRY

EditChanged = False
FactoryResetPending = False
EditLastActivity = ticks_ms()
MENU_TIMEOUT_MS = 10000


# ==========================================================
# INIT
# ==========================================================

def Init():

    global Menu
    global SetupItem
    global EditChanged
    global EditLastActivity
    global FactoryResetPending

    Menu = MENU_MAIN

    SetupItem = SET_CHEMISTRY
    EditChanged = False
    FactoryResetPending = False
    EditLastActivity = ticks_ms()


# ==========================================================
# TASK
# ==========================================================

def Task():

    global Menu
    global EditLastActivity

    if system.IsMenuView():

        if Menu != MENU_MAIN:

            if ticks_diff(
                ticks_ms(),
                EditLastActivity
            ) >= MENU_TIMEOUT_MS:

                Menu = MENU_MAIN

                display.ShowMain()
                
# ==========================================================
# MAIN MENU
# ==========================================================

def Next():

    global Menu

    global EditLastActivity

    Menu += 1
    EditLastActivity = ticks_ms()

    if Menu >= MENU_COUNT:

        Menu = MENU_MAIN

    Refresh()


# ==========================================================
# REFRESH MAIN MENU
# ==========================================================

def Refresh():

    global EditLastActivity

    EditLastActivity = ticks_ms()

    if Menu == MENU_MAIN:

        display.ShowMain()

    elif Menu == MENU_INFO:

        display.ShowMenu(

            2,

            "LipoProfiBal06",

            "Chemistry: {}".format(
                settings.CHEM_NAME[
                    settings.Chemistry
                ]
            ),

        (
            "Cells: AUTO"
            if settings.CellCount == settings.CELL_AUTO
            else "Cells: {}S".format(
                settings.CellCount
            )
        )

        )

    elif Menu == MENU_STATS:

        display.ShowMenu(

            3,

            "System Status",

            "Boots: {}".format(
                settings.BootCounter
            ),

            "FW 1.00"

        )


# ==========================================================
# SETUP MENU
# ==========================================================

def NextSetup():

    global SetupItem

    global EditLastActivity

    SetupItem += 1
    EditLastActivity = ticks_ms()

    if SetupItem >= SET_COUNT:

        SetupItem = SET_CHEMISTRY

    RefreshSetup()


# ==========================================================
# REFRESH SETUP VIEW
# ==========================================================

def RefreshSetup():

    if SetupItem == SET_CHEMISTRY:

        display.ShowSetup(

            "1/8",

            "Chemistry",

            settings.CHEM_NAME[
                settings.Chemistry
            ],

            False

        )


    elif SetupItem == SET_CELLCOUNT:

        display.ShowSetup(

            "2/8",

            "Cell Count",

            (
                "AUTO"
                if settings.CellCount == settings.CELL_AUTO
                else "{} S".format(
                settings.CellCount
                )
            ),

            False

        )


    elif SetupItem == SET_UMAX:

        display.ShowSetup(

            "3/8",

            "Over Voltage",

            "{} mV".format(
                settings.OverVoltage
            ),

            False

        )


    elif SetupItem == SET_UMIN:

        display.ShowSetup(

            "4/8",

            "Under Voltage",

            "{} mV".format(
                settings.UnderVoltage
            ),

            False

        )


    elif SetupItem == SET_DELTA:

        display.ShowSetup(

            "5/8",

            "Balance Delta",

            "{} mV".format(
                settings.BalanceDelta
            ),

            False

        )


    elif SetupItem == SET_UART:

        display.ShowSetup(

            "6/8",

            "UART",

            "ON"
            if settings.UartEnable
            else "OFF",

            False

        )


    elif SetupItem == SET_FACTORY:

        display.ShowSetup(

            "7/8",

            "Factory Reset",

            "YES"
            if FactoryResetPending
            else "No",

            False

        )


    elif SetupItem == SET_SAVE:

        display.ShowSetup(

            "8/8",

            "Save & Exit",

            "READY",

            False

        )


# ==========================================================
# REFRESH EDIT
# ==========================================================

def RefreshEdit():

    if SetupItem == SET_CHEMISTRY:

        display.ShowSetup(

            "1/8",

            "Chemistry",

            settings.CHEM_NAME[
                settings.Chemistry
            ],

            True,
            EditChanged

        )


    elif SetupItem == SET_CELLCOUNT:

        display.ShowSetup(

            "2/8",

            "Cell Count",

        (
            "AUTO"
            if settings.CellCount == settings.CELL_AUTO
            else "{} S".format(
                settings.CellCount
            )
        ),

            True,
            EditChanged

        )


    elif SetupItem == SET_UMAX:

        display.ShowSetup(

            "3/8",

            "Over Voltage",

            "{} mV".format(
                settings.OverVoltage
            ),

            True,
            EditChanged

        )


    elif SetupItem == SET_UMIN:

        display.ShowSetup(

            "4/8",

            "Under Voltage",

            "{} mV".format(
                settings.UnderVoltage
            ),

            True,
            EditChanged

        )


    elif SetupItem == SET_DELTA:

        display.ShowSetup(

            "5/8",

            "Balance Delta",

            "{} mV".format(
                settings.BalanceDelta
            ),

            True,
            EditChanged

        )


    elif SetupItem == SET_UART:

        display.ShowSetup(

            "6/8",

            "UART",

            "ON"
            if settings.UartEnable
            else "OFF",

            True,
            EditChanged

        )


    elif SetupItem == SET_FACTORY:

        display.ShowSetup(

            "7/8",

            "Factory Reset",

            "YES"
            if FactoryResetPending
            else "No",

            True,
            EditChanged

        )


    elif SetupItem == SET_SAVE:

        display.ShowSetup(

            "8/8",

            "Save & Exit",

            "READY",

            True,
            EditChanged

        )


# ==========================================================
# EDIT SETUP ITEM
# ==========================================================

def EditSetup():

    global EditChanged
    global EditLastActivity
    global FactoryResetPending

    EditChanged = True
    EditLastActivity = ticks_ms()

    # ------------------------------------------------------
    # CHEMISTRY
    # ------------------------------------------------------

    if SetupItem == SET_CHEMISTRY:

        settings.NextChemistry()


    # ------------------------------------------------------
    # CELL COUNT
    # ------------------------------------------------------

    elif SetupItem == SET_CELLCOUNT:

        settings.CellCount += 1

        if settings.CellCount > 6:

            settings.CellCount = settings.CELL_AUTO


    # ------------------------------------------------------
    # OVER VOLTAGE
    # ------------------------------------------------------

    elif SetupItem == SET_UMAX:

        settings.OverVoltage += 10

        if settings.OverVoltage > 5000:

            settings.OverVoltage = 3000


    # ------------------------------------------------------
    # UNDER VOLTAGE
    # ------------------------------------------------------

    elif SetupItem == SET_UMIN:

        settings.UnderVoltage += 10

        if settings.UnderVoltage > 4000:

            settings.UnderVoltage = 2500


    # ------------------------------------------------------
    # BALANCE DELTA
    # ------------------------------------------------------

    elif SetupItem == SET_DELTA:

        settings.BalanceDelta += 5

        if settings.BalanceDelta > 50:

            settings.BalanceDelta = 5


    # ------------------------------------------------------
    # UART
    # ------------------------------------------------------

    elif SetupItem == SET_UART:

        settings.UartEnable = not settings.UartEnable


    # ------------------------------------------------------
    # FACTORY RESET
    # ------------------------------------------------------

    elif SetupItem == SET_FACTORY:

        FactoryResetPending = not FactoryResetPending


    # ------------------------------------------------------
    # SAVE
    # ------------------------------------------------------

    elif SetupItem == SET_SAVE:

        SaveSetup()

        return


    RefreshEdit()


# ==========================================================
# SAVE SETUP
# ==========================================================

def SaveSetup():

    global Menu
    global SetupItem
    global EditChanged
    global EditLastActivity
    global FactoryResetPending

    if FactoryResetPending:

        settings.WriteFactoryDefaults()
        FactoryResetPending = False

    else:

        settings.Save()

    Menu = MENU_MAIN

    SetupItem = SET_CHEMISTRY

    EditChanged = False
    FactoryResetPending = False
    EditLastActivity = ticks_ms()

    led.Test()

    battery.ResetCheck()

    display.ShowInfo(
        "Battery check",
        "Please wait"
    )

    system.EnterCheck()