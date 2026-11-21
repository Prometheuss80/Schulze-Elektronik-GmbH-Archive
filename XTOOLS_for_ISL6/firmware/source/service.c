#include "watchdog.h"
#include "uart.h"
#include "lcd.h"
#include "delay.h"
#include "eeprom.h"
#include "service.h"

static unsigned int ee[16];

static bit Cal1OK(void)
{
    unsigned int inv;
    inv = (unsigned int)(~ee[0x01]);
    return (ee[0x09] == inv && ee[0x0E] == inv);
}

static bit Cal2OK(void)
{
    unsigned int inv;
    inv = (unsigned int)(~ee[0x03]);
    return (ee[0x06] == inv && ee[0x0B] == inv);
}

static bit Cal3OK(void)
{
    unsigned int inv;
    inv = (unsigned int)(~ee[0x04]);
    return (ee[0x07] == inv && ee[0x0C] == inv);
}

static bit Cal4OK(void)
{
    unsigned int inv;
    inv = (unsigned int)(~ee[0x05]);
    return (ee[0x08] == inv && ee[0x0D] == inv);
}

static void PutDec5(char *dst, unsigned int value)
{
    dst[0] = (char)('0' + (value / 10000) % 10);
    dst[1] = (char)('0' + (value / 1000) % 10);
    dst[2] = (char)('0' + (value / 100) % 10);
    dst[3] = (char)('0' + (value / 10) % 10);
    dst[4] = (char)('0' + value % 10);
}

static void MakeSNLine(char *line)
{
    unsigned char i;
    unsigned int value;
    unsigned int div;
    unsigned char digit;
    bit started;
    unsigned char pos;

    for (i = 0; i < 16; i++)
        line[i] = ' ';
    line[16] = 0;

    line[0] = 'S';
    line[1] = 'N';
    line[2] = ':';
    line[3] = ' ';

    value = ee[0x0A];
    div = 10000;
    started = 0;
    pos = 4;

    while (div != 0)
    {
        digit = (unsigned char)(value / div);
        value = value % div;

        if (digit != 0 || started || div == 1)
        {
            line[pos++] = (char)('0' + digit);
            started = 1;
        }
        div /= 10;
    }
}

static void MakeCalLine(char *line, unsigned char cal,
                        unsigned int value, bit ok)
{
    unsigned char i;

    for (i = 0; i < 16; i++)
        line[i] = ' ';
    line[16] = 0;

    line[0] = 'C';
    line[1] = 'A';
    line[2] = 'L';
    line[3] = (char)('0' + cal);
    line[4] = ':';

    PutDec5(&line[5], value);

    line[10] = ' ';

    if (ok)
    {
        line[11] = 'O';
        line[12] = 'K';
    }
    else
    {
        line[11] = 'F';
        line[12] = 'A';
        line[13] = 'U';
        line[14] = 'L';
        line[15] = 'T';
    }
}

static void MakeEEPLine(char *line, unsigned char address,
                        unsigned int value)
{
    unsigned char i;
    unsigned char h;

    for (i = 0; i < 16; i++)
        line[i] = ' ';
    line[16] = 0;

    line[0] = 'E';
    line[1] = 'E';
    line[2] = 'P';
    line[3] = ' ';

    h = (unsigned char)(address >> 4);
    line[4] = (char)(h < 10 ? '0' + h : 'A' + h - 10);

    h = (unsigned char)(address & 0x0F);
    line[5] = (char)(h < 10 ? '0' + h : 'A' + h - 10);

    line[6] = ':';
    PutDec5(&line[7], value);
}

void Service_ReadEEPROM(void)
{
    unsigned char i;

    for (i = 0; i < 16; i++)
    {
        ee[i] = EEPROM_ReadWord(i);
        WDT_Kick();
    }
}

static void ReportCal(unsigned char number, unsigned int value, bit ok)
{
    UART_PutString("CAL ");
    UART_PutChar((unsigned char)('0' + number));
    UART_PutString(" : ");
    UART_PutUInt16(value);

    if (ok)
        UART_PutString("  OK\r\n");
    else
        UART_PutString("  FAULT\r\n");
}

static void ReportUnknown(unsigned char address)
{
    UART_PutString("0x");
    UART_PutHex8(address);
    UART_PutString(" : 0x");
    UART_PutHex16(ee[address]);
    UART_PutString(" / ");
    UART_PutUInt16(ee[address]);
    UART_PutString("\r\n");
}

void Service_ReportUART(void)
{
    UART_PutString("\r\n================================\r\n");
    UART_PutString("XTOOL for ISL6 - EEPROM REPORT\r\n");
    UART_PutString("================================\r\n");

    UART_PutString("Serial Number : ");
    UART_PutUInt16(ee[0x0A]);
    UART_PutString("\r\n\r\n");

    ReportCal(1, ee[0x01], Cal1OK());
    ReportCal(2, ee[0x03], Cal2OK());
    ReportCal(3, ee[0x04], Cal3OK());
    ReportCal(4, ee[0x05], Cal4OK());

    UART_PutString("\r\nUnknown EEPROM words:\r\n");
    ReportUnknown(0x00);
    ReportUnknown(0x02);
    ReportUnknown(0x0F);

    UART_PutString("\r\nCalibration redundancy:\r\n");
    UART_PutString("CAL1 01 / 09 / 0E\r\n");
    UART_PutString("CAL2 03 / 06 / 0B\r\n");
    UART_PutString("CAL3 04 / 07 / 0C\r\n");
    UART_PutString("CAL4 05 / 08 / 0D\r\n");

    EEPROM_DumpAll();
}


unsigned int Service_GetEEPROM(unsigned char address)
{
    if (address < 16)
        return ee[address];

    return EEPROM_ReadWord(address);
}

void Service_LCDLoop(void)
{
    char line[17];

    while (1)
    {
        MakeSNLine(line);
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeCalLine(line, 1, ee[0x01], Cal1OK());
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeCalLine(line, 2, ee[0x03], Cal2OK());
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeCalLine(line, 3, ee[0x04], Cal3OK());
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeCalLine(line, 4, ee[0x05], Cal4OK());
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeEEPLine(line, 0x00, ee[0x00]);
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeEEPLine(line, 0x02, ee[0x02]);
        LCD_WriteLine(1, line);
        DelayMs(1000);

        MakeEEPLine(line, 0x0F, ee[0x0F]);
        LCD_WriteLine(1, line);
        DelayMs(1000);
    }
}
