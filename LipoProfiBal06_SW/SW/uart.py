# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : uart.py
#
# WinSoft Compatible RS232 Interface
#
# Sprint 7
# UI Version 2.0
# ==========================================================

from time import ticks_ms, ticks_diff, sleep_ms

import inithw
import battery
import settings
import system
import balance


# ==========================================================
# VARIABLES
# ==========================================================

Enabled = False

_lastTx = ticks_ms()
_seconds = 0
_errorSent = False


# ==========================================================
# ENABLE
# ==========================================================

def Enable():

    global Enabled
    global _lastTx
    global _seconds

    Enabled = True

    _lastTx = ticks_ms()

    _seconds = 0


# ==========================================================
# INIT
# ==========================================================

def Init():

    global Enabled
    global _lastTx
    global _seconds

    Enabled = False

    _lastTx = ticks_ms()

    _seconds = 0


# ==========================================================
# TASK
# ==========================================================

def Task():

    global _lastTx
    global _seconds
    global _errorSent

    if not Enabled:
        return

    if not settings.UartEnable:
        return

    # ------------------------------------------------------
    # ERROR
    # ------------------------------------------------------
    # Send exactly one error line when entering ERROR.
    # No live data are transmitted while the system is not READY.
    if system.IsError():

        if not _errorSent:
            SendError()
            _errorSent = True

        return

    # Leaving ERROR arms the one-shot error sender for the next fault.
    _errorSent = False

    # ------------------------------------------------------
    # LIVE DATA
    # ------------------------------------------------------
    # Live telemetry is permitted ONLY in the real READY state.
    if not system.IsReady():
        return

    if ticks_diff(
        ticks_ms(),
        _lastTx
    ) >= 1000:

        _lastTx = ticks_ms()
        _seconds += 1

        SendWinSoft()

        SendBalancer()

        if (_seconds % 60) == 0:
            SendBalTime()


# ==========================================================
# SEND WINSOFT
# ==========================================================

def SendWinSoft():

    if battery.Temperature is None:

        temp = "--"

    else:

        temp = "{:2d}".format(
            battery.Temperature
        )


    # ------------------------------------------------------
    # CELL VALUES FOR WINSOFT
    # ------------------------------------------------------
    # WinSoft treats even a few millivolts as a real cell and creates
    # a graph for it. Therefore every channel above the detected
    # runtime CellCount must be transmitted as an exact zero.
    uart_cells = [0, 0, 0, 0, 0, 0]

    count = battery.CellCount

    if count < 0:
        count = 0
    elif count > 6:
        count = 6

    for i in range(count):
        uart_cells[i] = battery.CellVoltage[i]


    line = (

        "{}:{:5d}:{:5d}:{:5d}:{} {:2};"
        "{:5d};{:5d};{:5d};{:5d};{:5d};{:5d};{:5d};{:5d};\r\n"

    ).format(

        battery.SerialPort,

        _seconds,

        battery.TotalVoltage,

        battery.Current,

        battery.State,

        temp,

        uart_cells[0],

        uart_cells[1],

        uart_cells[2],

        uart_cells[3],

        uart_cells[4],

        uart_cells[5],

        0,

        battery.TotalVoltage

    )


    inithw.uart.write(
        line.encode()
    )



# ==========================================================
# SEND ERROR
# ==========================================================

def SendError():
    """
    Send one human-readable error line when Battery Check or
    Runtime Safety enters ERROR.
    """

    try:
        line1, line2 = battery.CheckText()

        if line2:
            SendLine(
                "* ERROR: {} - {}".format(
                    line1,
                    line2
                )
            )
        else:
            SendLine(
                "* ERROR: {}".format(
                    line1
                )
            )

    except:
        SendLine("* ERROR: BATTERY CHECK")


# ==========================================================
# SEND ONE LINE
# ==========================================================

def SendLine(text):

    if not settings.UartEnable:
        return

    inithw.uart.write(
        (text + "\r\n").encode()
    )

    # WinSoft potřebuje krátkou prodlevu
    # mezi jednotlivými řádky hlavičky.

    sleep_ms(20)


# ==========================================================
# HEADER
# ==========================================================

def Header():

    if not settings.UartEnable:
        return

    SendLine("* HHHHHHHHHH")

    SendLine("*")

    SendLine("* by Prometheuss80")

    SendLine("* ProfiBal 06")

    SendLine("*")

    SendLine(
        "* Hardware Revision ......... Rev A"
    )

    SendLine(
        "* Serial Number ............. 02026080001"
    )

    SendLine(
        "* Prog Date ................. 02.08.26"
    )

    SendLine(
        "* Software Version .......... 01.00"
    )

    SendLine(
        "* Build ..................... 2026-08-02"
    )

    SendLine("*")


    # ------------------------------------------------------
    # CELL COUNT
    # ------------------------------------------------------

    SendLine(
        "* Cell Count ................ {}".format(
            battery.CellCount
        )
    )


    # ------------------------------------------------------
    # CHEMISTRY
    # ------------------------------------------------------

    SendLine(
        "* Chemistry ................. {}".format(
            settings.CHEM_NAME[
                settings.Chemistry
            ]
        )
    )


    # ------------------------------------------------------
    # VOLTAGE SETTINGS
    # ------------------------------------------------------

    SendLine(
        "* Overvoltage ............... {} mV".format(
            settings.OverVoltage
        )
    )

    SendLine(
        "* Undervoltage .............. {} mV".format(
            settings.UnderVoltage
        )
    )

    SendLine(
        "* Balancing Difference ...... {} mV".format(
            settings.BalanceDelta
        )
    )

    SendLine("*")


    # ------------------------------------------------------
    # STATUS
    # ------------------------------------------------------

    SendLine(
        "* Boot Counter .............. {}".format(
            settings.BootCounter
        )
    )

    SendLine(
        "* FRAM ...................... OK"
    )

    SendLine(
        "* OLED ...................... OK"
    )

    SendLine(
        "* UART ...................... 9600 Baud"
    )

    SendLine(
        "* ADC ....................... Waiting"
    )

    SendLine(
        "* Balance Engine ............ Ready"
    )

    SendLine("*")

    SendLine(
        "* Active Balancing System"
    )

    SendLine("*")


# ==========================================================
# BALANCER
# ==========================================================

def SendBalancer():

    active = balance.ActiveCells()

    if not active:
        return

    SendLine(
        "* Balancer: {}".format(
            " ".join(
                str(cell)
                for cell in active
            )
        )
    )


# ==========================================================
# BALANCE TIME
# ==========================================================

def SendBalTime():

    count = battery.CellCount

    if count < 1:
        return

    parts = []

    for cell in range(count):

        parts.append(
            "{}={:05d}".format(
                cell + 1,
                battery.BalanceTime[cell]
            )
        )

    SendLine(
        "* Bal-Time {} sec".format(
            "  ".join(parts)
        )
    )
