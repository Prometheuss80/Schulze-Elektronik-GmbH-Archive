# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : settings.py
#
# User Settings + FRAM chemistry table
# ==========================================================

import fram


# ==========================================================
# FRAM
# ==========================================================

SIGNATURE = 0x55

ADR_SIGNATURE = 0x0000
ADR_BOOTCOUNT = 0x0001

ADR_CHEMISTRY = 0x0010
ADR_CELLCOUNT = 0x0011
ADR_UMAX      = 0x0012      # mirror of selected chemistry
ADR_UMIN      = 0x0014      # mirror of selected chemistry
ADR_DELTA     = 0x0016
ADR_UART      = 0x0018

# Five chemistry records, 4 bytes each:
#   +0 UMIN word
#   +2 UMAX word
ADR_CHEM_TABLE = 0x0020
CHEM_RECORD_SIZE = 4


# ==========================================================
# DEFAULTS
# ==========================================================

CELL_AUTO = 1

CHEM_NAME = (
    "LiPo",
    "LiIon",
    "LiFe",
    "LiHV",
    "NiMH"
)

# UMIN, UMAX [mV]
CHEM_DEFAULTS = (
    (3300, 4200),   # LiPo
    (3000, 4100),   # LiIon
    (2800, 3600),   # LiFe
    (3300, 4350),   # LiHV
    ( 900, 1500),   # NiMH
)

DEFAULT_CHEMISTRY = 0        # LiPo
DEFAULT_CELLCOUNT = CELL_AUTO
DEFAULT_DELTA = 10
DEFAULT_UART = True


# ==========================================================
# VARIABLES
# ==========================================================

BootCounter = 0

Chemistry = DEFAULT_CHEMISTRY
CellCount = DEFAULT_CELLCOUNT

UnderVoltage = CHEM_DEFAULTS[0][0]
OverVoltage = CHEM_DEFAULTS[0][1]

BalanceDelta = DEFAULT_DELTA
UartEnable = DEFAULT_UART


# ==========================================================
# CHEMISTRY TABLE HELPERS
# ==========================================================

def _ChemAddr(chem):
    return ADR_CHEM_TABLE + (chem * CHEM_RECORD_SIZE)


def _WriteChemLimits(chem, umin, umax):

    addr = _ChemAddr(chem)

    fram.WriteWord(
        addr,
        umin
    )

    fram.WriteWord(
        addr + 2,
        umax
    )


def _ReadChemLimits(chem):

    addr = _ChemAddr(chem)

    umin = fram.ReadWord(addr)
    umax = fram.ReadWord(addr + 2)

    return umin, umax


def _LimitsValid(chem, umin, umax):

    # Broad sanity check only. User is still allowed to customize limits.
    if umin <= 0:
        return False

    if umax <= umin:
        return False

    if umax > 6000:
        return False

    return True


def LoadChemistryLimits():

    global UnderVoltage
    global OverVoltage

    umin, umax = _ReadChemLimits(
        Chemistry
    )

    if not _LimitsValid(
        Chemistry,
        umin,
        umax
    ):

        umin, umax = CHEM_DEFAULTS[
            Chemistry
        ]

        _WriteChemLimits(
            Chemistry,
            umin,
            umax
        )

    UnderVoltage = umin
    OverVoltage = umax


def SaveChemistryLimits():

    _WriteChemLimits(
        Chemistry,
        UnderVoltage,
        OverVoltage
    )

    # Keep old mirror addresses valid for compatibility / diagnostics.
    fram.WriteWord(
        ADR_UMIN,
        UnderVoltage
    )

    fram.WriteWord(
        ADR_UMAX,
        OverVoltage
    )


# ==========================================================
# FACTORY DEFAULTS
# ==========================================================

def WriteFactoryDefaults():
    """
    Restore user configuration to factory defaults.

    BootCounter is intentionally preserved as a lifetime statistic.
    """

    global Chemistry
    global CellCount
    global UnderVoltage
    global OverVoltage
    global BalanceDelta
    global UartEnable

    fram.WriteByte(
        ADR_SIGNATURE,
        SIGNATURE
    )

    for chem in range(
        len(CHEM_NAME)
    ):

        umin, umax = CHEM_DEFAULTS[
            chem
        ]

        _WriteChemLimits(
            chem,
            umin,
            umax
        )

    Chemistry = DEFAULT_CHEMISTRY
    CellCount = DEFAULT_CELLCOUNT
    BalanceDelta = DEFAULT_DELTA
    UartEnable = DEFAULT_UART

    UnderVoltage, OverVoltage = (
        CHEM_DEFAULTS[
            DEFAULT_CHEMISTRY
        ]
    )

    fram.WriteByte(
        ADR_CHEMISTRY,
        Chemistry
    )

    fram.WriteByte(
        ADR_CELLCOUNT,
        CellCount
    )

    fram.WriteWord(
        ADR_DELTA,
        BalanceDelta
    )

    fram.WriteByte(
        ADR_UART,
        1
    )

    SaveChemistryLimits()


# ==========================================================
# INIT
# ==========================================================

def Init():

    global BootCounter
    global Chemistry
    global CellCount
    global BalanceDelta
    global UartEnable

    # ------------------------------------------------------
    # First start
    # ------------------------------------------------------

    if fram.ReadByte(
        ADR_SIGNATURE
    ) != SIGNATURE:

        # Initialize boot counter once.
        fram.WriteWord(
            ADR_BOOTCOUNT,
            0
        )

        WriteFactoryDefaults()

    # ------------------------------------------------------
    # Ensure the whole chemistry table exists.
    # This also upgrades older FRAM layouts automatically.
    # ------------------------------------------------------

    for chem in range(
        len(CHEM_NAME)
    ):

        umin, umax = _ReadChemLimits(
            chem
        )

        if not _LimitsValid(
            chem,
            umin,
            umax
        ):

            du_min, du_max = (
                CHEM_DEFAULTS[chem]
            )

            _WriteChemLimits(
                chem,
                du_min,
                du_max
            )

    # ------------------------------------------------------
    # Boot counter
    # ------------------------------------------------------

    BootCounter = fram.ReadWord(
        ADR_BOOTCOUNT
    )

    BootCounter += 1

    fram.WriteWord(
        ADR_BOOTCOUNT,
        BootCounter
    )

    # ------------------------------------------------------
    # Load settings
    # ------------------------------------------------------

    Chemistry = fram.ReadByte(
        ADR_CHEMISTRY
    )

    if Chemistry >= len(CHEM_NAME):
        Chemistry = DEFAULT_CHEMISTRY

    CellCount = fram.ReadByte(
        ADR_CELLCOUNT
    )

    if CellCount < CELL_AUTO or CellCount > 6:
        CellCount = DEFAULT_CELLCOUNT

    BalanceDelta = fram.ReadWord(
        ADR_DELTA
    )

    if BalanceDelta < 1 or BalanceDelta > 100:
        BalanceDelta = DEFAULT_DELTA

    UartEnable = bool(
        fram.ReadByte(
            ADR_UART
        )
    )

    LoadChemistryLimits()


# ==========================================================
# SAVE
# ==========================================================

def Save():

    fram.WriteByte(
        ADR_CHEMISTRY,
        Chemistry
    )

    fram.WriteByte(
        ADR_CELLCOUNT,
        CellCount
    )

    fram.WriteWord(
        ADR_DELTA,
        BalanceDelta
    )

    fram.WriteByte(
        ADR_UART,
        1 if UartEnable else 0
    )

    SaveChemistryLimits()


# ==========================================================
# NEXT CHEMISTRY
# ==========================================================

def NextChemistry():

    global Chemistry

    # Save edited limits of the chemistry we are leaving.
    SaveChemistryLimits()

    Chemistry += 1

    if Chemistry >= len(
        CHEM_NAME
    ):
        Chemistry = 0

    # Load the stored UMIN/UMAX of the newly selected chemistry.
    LoadChemistryLimits()
