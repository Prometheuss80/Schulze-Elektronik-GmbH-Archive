# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : main.py
#
# Sprint 7
# UI Version 2.0
#
# Main Program
# ==========================================================

from time import sleep_ms, ticks_ms, ticks_diff

import inithw
import led
import display
import buttons
import boot
import battery
import menu
import fram
import settings
import system
import uart
import adc
import balance


# ==========================================================
# RUNTIME BATTERY SAFETY
# ==========================================================

RUNTIME_CHECK_MS = 250
RUNTIME_FAIL_CONFIRM = 3

_runtimeCheckLast = ticks_ms()
_runtimeFailCount = 0


# ==========================================================
# INIT
# ==========================================================

inithw.Init()

system.Init()

uart.Init()

fram.Init()

settings.Init()

led.Init()

display.Init()

buttons.Init()

battery.Init()

adc.Init()

menu.Init()

balance.Init()


# ==========================================================
# BOOT
# ==========================================================

boot.Boot()


# ==========================================================
# RUNTIME BATTERY SAFETY
# ==========================================================

def RuntimeBatterySafety():

    global _runtimeCheckLast
    global _runtimeFailCount

    now = ticks_ms()

    if ticks_diff(
        now,
        _runtimeCheckLast
    ) < RUNTIME_CHECK_MS:
        return True

    _runtimeCheckLast = now

    result = battery.RuntimeSafetyCheck()

    if result == battery.CHECK_OK:

        _runtimeFailCount = 0
        return True

    _runtimeFailCount += 1

    print(
        "RUNTIME BATTERY FAIL {}/{}: result={} cell={} voltage={}mV".format(
            _runtimeFailCount,
            RUNTIME_FAIL_CONFIRM,
            result,
            battery.ErrorCell + 1
            if battery.ErrorCell >= 0
            else 0,
            battery.ErrorVoltage
        )
    )

    if _runtimeFailCount < RUNTIME_FAIL_CONFIRM:
        return True

    # Confirmed battery fault.
    _runtimeFailCount = 0

    system.EnterError()
    display.ShowBatteryError()

    return False


# ==========================================================
# MAIN LOOP
# ==========================================================

while True:

    # ------------------------------------------------------
    # READ BUTTONS
    # ------------------------------------------------------

    buttons.Task()


    # ======================================================
    # BATTERY CHECK
    # ======================================================

    if system.IsCheck():

        if battery.CheckResult == battery.CHECK_PENDING:

            result = battery.CheckBattery()

            if result == battery.CHECK_OK:

                system.EnterReady()
                display.ShowMain()

                balance.Arm()

            elif result == battery.ERR_CELL_COUNT:

                display.ShowCellCountMismatch()

            else:

                system.EnterError()
                display.ShowBatteryError()

        elif battery.CheckResult == battery.ERR_CELL_COUNT:

            if buttons.SetPressed():

                if battery.AcceptDetectedAsAuto():

                    system.EnterCheck()

                    display.ShowInfo(
                        "Battery check",
                        "Please wait"
                    )

            elif buttons.MenuPressed():

                battery.RejectDetectedCount()
                system.EnterError()
                display.ShowBatteryError()


    # ======================================================
    # READY
    # ======================================================

    elif system.IsReady():

        # Continuous battery supervision.
        if not RuntimeBatterySafety():
            pass

        # MENU -> MENU VIEW

        elif buttons.MenuPressed():

            system.EnterMenuView()

            menu.Next()


        # SET LONG -> EDIT DIRECTLY

        elif buttons.SetLongPressed():

            menu.SetupItem = menu.SET_CHEMISTRY

            system.EnterSettingsEdit()

            menu.RefreshEdit()


        # SET SHORT -> SETTINGS VIEW

        elif buttons.SetPressed():

            menu.SetupItem = menu.SET_CHEMISTRY

            system.EnterSettingsView()

            menu.RefreshSetup()


    # ======================================================
    # MENU VIEW
    # ======================================================

    elif system.IsMenuView():

        # MENU -> NEXT SCREEN

        if buttons.MenuPressed():

            menu.Next()
            

        # --------------------------------------------------
        # SET LONG -> EDIT
        # --------------------------------------------------

        elif buttons.SetLongPressed():

            menu.SetupItem = menu.SET_CHEMISTRY

            system.EnterSettingsEdit()

            menu.RefreshEdit()


        # --------------------------------------------------
        # SET SHORT -> READY
        # --------------------------------------------------

        elif buttons.SetPressed():

            battery.ResetCheck()
            system.EnterCheck()

            display.ShowInfo(
                "Battery check",
                "Please wait"
            )


        # ======================================================
        # SETTINGS VIEW
        # ======================================================

    elif system.IsSettingsView():

    # --------------------------------------------------
    # SET SHORT -> NEXT SETTING
    # --------------------------------------------------

        if buttons.SetPressed():

            menu.NextSetup()


    # --------------------------------------------------
    # SET LONG -> EDIT
    # --------------------------------------------------

        elif buttons.SetLongPressed():

            menu.SetupItem = menu.SET_CHEMISTRY

            system.EnterSettingsEdit()

            menu.RefreshEdit()


    # --------------------------------------------------
    # MENU -> EXIT SET VIEW
    # --------------------------------------------------

        elif buttons.MenuPressed():

            # Leaving SET always runs the same final safety sequence
            # as startup: LED test, invalidate old battery validation,
            # then perform a fresh Battery Check.
            led.Test()

            battery.ResetCheck()
            system.EnterCheck()

            display.ShowInfo(
                "Battery check",
                "Please wait"
            )


    # ======================================================
    # SETTINGS EDIT
    # ======================================================

    elif system.IsSettingsEdit():

        # --------------------------------------------------
        # SAVE & EXIT
        # --------------------------------------------------

        if menu.SetupItem == menu.SET_SAVE:

            # SET -> SAVE

            if buttons.SetPressed():

                menu.SaveSetup()


            # MENU -> BACK TO FIRST ITEM

            elif buttons.MenuPressed():

                menu.SetupItem = menu.SET_CHEMISTRY

                menu.RefreshEdit()


        # --------------------------------------------------
        # NORMAL EDIT ITEM
        # --------------------------------------------------

        else:

            # SET -> CHANGE VALUE

            if buttons.SetPressed():

                menu.EditSetup()


            # MENU -> NEXT ITEM

            elif buttons.MenuPressed():

                menu.NextSetup()


            # LONG SET -> LEAVE EDIT

            elif buttons.SetLongPressed():

                system.EnterSettingsView()

                menu.RefreshSetup()


    # ======================================================
    # BALANCE
    # ======================================================

    elif system.IsBalance():

        # Balancing is allowed only while the already-validated pack
        # remains inside all safety limits.
        RuntimeBatterySafety()


    # ======================================================
    # ERROR
    # ======================================================

    elif system.IsError():

        # ERROR is safe but not a dead end.
        #
        # MENU -> normal information menu.
        # The system remains protected: leaving the menu can never
        # enter READY unless the latest Battery Check is CHECK_OK.
        if buttons.MenuPressed():

            system.EnterMenuView()
            menu.Menu = menu.MENU_MAIN
            menu.Next()

        # SET short -> normal settings view.
        elif buttons.SetPressed():

            menu.SetupItem = menu.SET_CHEMISTRY
            system.EnterSettingsView()
            menu.RefreshSetup()

        # SET long -> edit settings directly.
        elif buttons.SetLongPressed():

            menu.SetupItem = menu.SET_CHEMISTRY
            system.EnterSettingsEdit()
            menu.RefreshEdit()


    # ======================================================
    # TASKS
    # ======================================================

    menu.Task()

    adc.Task()

    battery.Task()

    system.Task()

    uart.Task()

    led.Task()

    balance.Task()

    display.Task()


    # ======================================================
    # LOOP DELAY
    # ======================================================

    sleep_ms(10)