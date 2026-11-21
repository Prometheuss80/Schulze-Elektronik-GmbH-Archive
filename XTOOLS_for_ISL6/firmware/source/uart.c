#include "isl6_hw.h"
#include "uart.h"
#include "watchdog.h"

static unsigned char HexDigit(unsigned char v)
{
    v &= 0x0F;
    return v < 10 ? (unsigned char)('0' + v)
                  : (unsigned char)('A' + v - 10);
}

void UART_Init(void)
{
    RI = 0;
    TI = 0;
}

void UART_PutChar(unsigned char c)
{
    SBUF = c;
    while (TI == 0)
        WDT_Kick();
    TI = 0;
}

void UART_PutString(const char code *s)
{
    while (*s)
        UART_PutChar(*s++);
}

void UART_PutHex8(unsigned char v)
{
    UART_PutChar(HexDigit(v >> 4));
    UART_PutChar(HexDigit(v));
}

void UART_PutHex16(unsigned int v)
{
    UART_PutHex8((unsigned char)(v >> 8));
    UART_PutHex8((unsigned char)v);
}

void UART_PutUInt16(unsigned int value)
{
    unsigned int div;
    unsigned char digit;
    bit started;

    div = 10000;
    started = 0;

    while (div != 0)
    {
        digit = (unsigned char)(value / div);
        value = value % div;

        if (digit != 0 || started || div == 1)
        {
            UART_PutChar((unsigned char)('0' + digit));
            started = 1;
        }

        div /= 10;
    }
}


bit UART_CharAvailable(void)
{
    return RI ? 1 : 0;
}

unsigned char UART_GetChar(void)
{
    unsigned char c;

    c = SBUF;
    RI = 0;
    return c;
}
