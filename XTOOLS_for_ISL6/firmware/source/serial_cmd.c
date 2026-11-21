#include "isl6_hw.h"
#include "watchdog.h"
#include "uart.h"
#include "lcd.h"
#include "delay.h"
#include "eeprom.h"
#include "service.h"
#include "serial_cmd.h"
#include "hw_profile.h"

#define CMD_BUFFER_SIZE  24

static char cmd[CMD_BUFFER_SIZE];

static unsigned char Upper(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        c = (unsigned char)(c - 'a' + 'A');

    return c;
}

static bit HexNibble(unsigned char c, unsigned char *value)
{
    c = Upper(c);

    if (c >= '0' && c <= '9')
    {
        *value = (unsigned char)(c - '0');
        return 1;
    }

    if (c >= 'A' && c <= 'F')
    {
        *value = (unsigned char)(c - 'A' + 10);
        return 1;
    }

    return 0;
}

static bit ParseHex16(char *s, unsigned int *value)
{
    unsigned char n;
    unsigned char i;
    unsigned int v;

    v = 0;

    for (i = 0; i < 4; i++)
    {
        if (!HexNibble((unsigned char)s[i], &n))
            return 0;

        v <<= 4;
        v |= n;
    }

    if (s[4] != 0)
        return 0;

    *value = v;
    return 1;
}

static bit ParseReadAddress(unsigned char *address)
{
    unsigned char hi;
    unsigned char lo;

    if (cmd[0] != 'R' || cmd[1] != 'E' || cmd[2] != 'A' ||
        cmd[3] != 'D' || cmd[4] != ' ')
        return 0;

    if (!HexNibble((unsigned char)cmd[5], &hi))
        return 0;

    if (!HexNibble((unsigned char)cmd[6], &lo))
        return 0;

    if (cmd[7] != 0)
        return 0;

    *address = (unsigned char)((hi << 4) | lo);

    return (*address <= 0x3F) ? 1 : 0;
}

/*
 * V0.6 SAFETEST intentionally allows WRITE to address 3F only.
 * Syntax: WRITE 3F A55A
 *
 * 3F is currently blank (FFFF) on this charger and lies outside the
 * active 00..0F block. Generic EEPROM writes are deliberately disabled.
 */
static bit ParseWrite(unsigned char *address, unsigned int *value)
{
    unsigned char hi;
    unsigned char lo;
    char hexbuf[5];

    /*
     * Syntax:
     *   WRITE aa xxxx
     *
     * aa   = EEPROM word address 00..3F
     * xxxx = 16-bit hexadecimal value
     */
    if (cmd[0] != 'W' || cmd[1] != 'R' || cmd[2] != 'I' ||
        cmd[3] != 'T' || cmd[4] != 'E' || cmd[5] != ' ')
        return 0;

    if (!HexNibble((unsigned char)cmd[6], &hi))
        return 0;

    if (!HexNibble((unsigned char)cmd[7], &lo))
        return 0;

    if (cmd[8] != ' ')
        return 0;

    *address = (unsigned char)((hi << 4) | lo);

    if (*address > 0x3F)
        return 0;

    hexbuf[0] = cmd[9];
    hexbuf[1] = cmd[10];
    hexbuf[2] = cmd[11];
    hexbuf[3] = cmd[12];
    hexbuf[4] = 0;

    if (cmd[13] != 0)
        return 0;

    return ParseHex16(hexbuf, value);
}

static bit Same(char *a, char *b)
{
    while (*a != 0 && *b != 0)
    {
        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return (*a == 0 && *b == 0);
}

static void SendInfo(void)
{
    UART_PutString("\r\nXTOOLS for Schulze ISL6 V0.8 UNIVERSAL\r\n");
    if (HWProfile_GetMode() == HW_MODE_330)
    {
        UART_PutString("MODE: 1\r\n");
        UART_PutString("MODEL: ISL6-330D\r\n");
        UART_PutString("EEPROM: 93C46B 4-WIRE\r\n");
        UART_PutString("LCD: 1x16\r\n");
    }
    else
    {
        UART_PutString("MODE: 2\r\n");
        UART_PutString("MODEL: ISL6-430/530/636 family\r\n");
        UART_PutString("EEPROM: 93C46B 3-WIRE\r\n");
        UART_PutString("LCD: 2x16\r\n");
    }
    UART_PutString("ACCESS: RAW EEPROM READ / WRITE 00-3F\r\n");
    UART_PutString("Serial Number: ");
    UART_PutUInt16(Service_GetEEPROM(0x0A));
    UART_PutString("\r\n");
    UART_PutString("Commands: PING INFO READ aa DUMP WRITE aa xxxx RESET\r\n");
}

static void SendRead(unsigned char address)
{
    unsigned int value;

    /* Live EEPROM read, not the startup snapshot. */
    value = EEPROM_ReadWord(address);

    UART_PutString("READ 0x");
    UART_PutHex8(address);
    UART_PutString(" = 0x");
    UART_PutHex16(value);
    UART_PutString(" / ");
    UART_PutUInt16(value);
    UART_PutString("\r\n");
}

static void RestartByWatchdog(void)
{
    /*
     * Intentional reset:
     * - stop the service timer
     * - tell PC/LCD what is happening
     * - stop servicing BOTH watchdog paths
     *
     * The external ADM706 on P1.4 (and, if active, the 80C535 internal
     * watchdog) will then reset the charger exactly as a hardware restart.
     */
    TR0 = 0;
    TF0 = 0;

    UART_PutString("RESET REQUESTED - WAIT FOR WATCHDOG\r\n");
    LCD_WriteLine(1, "RESTARTING...");

    EA = 0;

    while (1)
    {
        /* Deliberately NO WDT_Kick(). */
    }
}

static bit ExecuteCommand(void)
{
    unsigned char address;
    unsigned int value;
    unsigned int oldvalue;
    unsigned int readback;
    bit ok;

    if (Same(cmd, "PING"))
    {
        UART_PutString("PONG\r\n");
        return 1;
    }

    if (Same(cmd, "INFO"))
    {
        SendInfo();
        return 1;
    }

    if (Same(cmd, "DUMP"))
    {
        EEPROM_DumpAll();
        return 1;
    }

    if (ParseReadAddress(&address))
    {
        SendRead(address);
        return 1;
    }

    if (ParseWrite(&address, &value))
    {
        oldvalue = EEPROM_ReadWord(address);

        UART_PutString("WRITE 0x");
        UART_PutHex8(address);
        UART_PutString(" OLD=0x");
        UART_PutHex16(oldvalue);
        UART_PutString(" NEW=0x");
        UART_PutHex16(value);
        UART_PutString("\r\n");

        ok = EEPROM_WriteWord(address, value);
        readback = EEPROM_ReadWord(address);

        UART_PutString("READBACK 0x");
        UART_PutHex8(address);
        UART_PutString(" = 0x");
        UART_PutHex16(readback);
        UART_PutString("\r\n");

        if (ok && readback == value)
            UART_PutString("RESULT OK\r\n");
        else
            UART_PutString("RESULT FAIL\r\n");

        return 1;
    }

    if (Same(cmd, "RESET"))
    {
        RestartByWatchdog();
        return 1; /* not reached */
    }

    UART_PutString("ERROR UNKNOWN COMMAND\r\n");
    return 0;
}

static void Tick1ms_Start(void)
{
    TR0 = 0;
    TMOD = (TMOD & 0xF0) | 0x01;
    TF0 = 0;
    TH0 = 0xFA;
    TL0 = 0xCB;
    TR0 = 1;
}

static bit Tick1ms_Poll(void)
{
    if (TF0 == 0)
        return 0;

    TR0 = 0;
    TF0 = 0;
    TH0 = 0xFA;
    TL0 = 0xCB;
    TR0 = 1;

    return 1;
}

void SerialCmd_Window(void)
{
    unsigned char seconds;
    unsigned char index;
    unsigned int milliseconds;
    unsigned char c;
    bit valid;
    bit link_active;

    seconds = 30;
    index = 0;
    milliseconds = 0;
    link_active = 0;

    UART_PutString("\r\nSERVICE COMMAND WINDOW 30s\r\n");
    UART_PutString("RAW EEPROM WRITE 00-3F / READBACK ENABLED\r\n");

    /*
     * V0.6B:
     * Write LCD once before the prompt. During the active PC link,
     * no periodic LCD refresh is performed, keeping UART RX available.
     */
    LCD_WriteLine(1, "PC WAITING");

    Tick1ms_Start();
    UART_PutString("CMD> ");

    while (seconds != 0)
    {
        WDT_Kick();

        if (UART_CharAvailable())
        {
            c = UART_GetChar();

            if (c == '\r' || c == '\n')
            {
                if (index != 0)
                {
                    cmd[index] = 0;
                    UART_PutString("\r\n");

                    valid = ExecuteCommand();
                    index = 0;

                    if (valid)
                    {
                        seconds = 30;
                        milliseconds = 0;

                        /*
                         * Only a successfully received command proves that
                         * a PC is really connected. Show LINK ACTIVE once.
                         * No further LCD refresh is done during the session.
                         */
                        if (!link_active)
                        {
                            link_active = 1;
                            LCD_WriteLine(1, "LINK ACTIVE");
                        }

                        Tick1ms_Start();
                    }

                    UART_PutString("CMD> ");
                }
            }
            else if (c == 8 || c == 127)
            {
                if (index != 0)
                    index--;
            }
            else if (index < (CMD_BUFFER_SIZE - 1))
            {
                c = Upper(c);
                cmd[index++] = (char)c;
            }
        }

        if (Tick1ms_Poll())
        {
            milliseconds++;

            if (milliseconds >= 1000)
            {
                milliseconds = 0;
                seconds--;
            }
        }
    }

    TR0 = 0;
    TF0 = 0;

    LCD_WriteLine(1, "PC LINK CLOSED");
    UART_PutString("\r\nSERVICE WINDOW CLOSED\r\n");
    DelayMs(500);
}
