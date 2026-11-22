# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : system.py
#
# System State Machine
#
# Sprint 7
# UI Version 2.0
# ==========================================================


# ==========================================================
# STATES
# ==========================================================

STATE_BOOT          = 0
STATE_CHECK         = 1
STATE_READY         = 2
STATE_MENU_VIEW     = 3
STATE_SETTINGS_VIEW = 4
STATE_SETTINGS_EDIT = 5
STATE_BALANCE       = 6
STATE_ERROR         = 7


# ==========================================================
# VARIABLES
# ==========================================================

State = STATE_BOOT


# ==========================================================
# INIT
# ==========================================================

def Init():

    global State

    State = STATE_BOOT


# ==========================================================
# TASK
# ==========================================================

def Task():

    global State

    if State == STATE_BOOT:

        return

    elif State == STATE_CHECK:

        # main.py coordinates Battery Check because the check can
        # require user confirmation.
        return

    elif State == STATE_READY:

        return

    elif State == STATE_MENU_VIEW:

        return

    elif State == STATE_SETTINGS_VIEW:

        return

    elif State == STATE_SETTINGS_EDIT:

        return

    elif State == STATE_BALANCE:

        return

    elif State == STATE_ERROR:

        return


# ==========================================================
# PUBLIC
# ==========================================================

def SetState(newState):

    global State

    State = newState


def GetState():

    return State


# ==========================================================
# STATE TESTS
# ==========================================================

def IsBoot():

    return State == STATE_BOOT


def IsCheck():

    return State == STATE_CHECK


def IsReady():

    return State == STATE_READY


def IsMenuView():

    return State == STATE_MENU_VIEW


def IsSettingsView():

    return State == STATE_SETTINGS_VIEW


def IsSettingsEdit():

    return State == STATE_SETTINGS_EDIT


def IsBalance():

    return State == STATE_BALANCE


def IsError():

    return State == STATE_ERROR


# ==========================================================
# STATE TRANSITIONS
# ==========================================================

def EnterBoot():

    SetState(
        STATE_BOOT
    )


def EnterCheck():

    SetState(
        STATE_CHECK
    )


def EnterReady():

    SetState(
        STATE_READY
    )


def EnterMenuView():

    SetState(
        STATE_MENU_VIEW
    )


def EnterSettingsView():

    SetState(
        STATE_SETTINGS_VIEW
    )


def EnterSettingsEdit():

    SetState(
        STATE_SETTINGS_EDIT
    )


def EnterBalance():

    SetState(
        STATE_BALANCE
    )


def EnterError():

    SetState(
        STATE_ERROR
    )


# ==========================================================
# COMPATIBILITY
# ----------------------------------------------------------
# Starší kód používá EnterSetup() / IsSetup().
# Dočasně je zachováme, aby při přechodu na UI v2.0
# nevznikaly zbytečné chyby.
# ==========================================================

def EnterSetup():

    EnterSettingsEdit()


def IsSetup():

    return IsSettingsEdit()