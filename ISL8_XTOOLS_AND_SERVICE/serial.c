#include <REG80C537_ISL8.H>
#include "serial.h"
#include "watchdog.h"

/*
 * ISL8 serial channel 0:
 *   RXD0 = P3.0
 *   TXD0 = P3.1
 *
 * 9600 baud setup follows the original ISL8 timer-1 configuration:
 *   PCON.SMOD = 1
 *   S0CON     = 0x52
 *   Timer 1   = mode 2, auto reload
 *   TH1/TL1   = 0xF7
 *
 * Original ROM routine A_FC74:
 *   TMOD high nibble -> 0x20
 *   TH1 = 0xF7
 *   TR1 = 1
 */

void Serial_Init(void)
{
    /* SMOD = 1, preserve lower PCON bits. */
    PCON |= 0x80u;

    /* Serial channel 0: mode 1, receiver enabled. */
    S0CON = 0x52u;

    /*
     * Preserve Timer 0 configuration.
     * Configure only Timer 1: mode 2 (8-bit auto reload).
     */
    TR1 = 0;
    TMOD = (unsigned char)((TMOD & 0x0Fu) | 0x20u);

    TH1 = 0xF7u;
    TL1 = 0xF7u;

    RI0 = 0;
    TI0 = 0;

    TR1 = 1;
}

void Serial_PutChar(char c)
{
    S0BUF = (unsigned char)c;

    while(!TI0)
    {
        WDT_Kick();
    }

    TI0 = 0;
}

void Serial_Print(const char *text)
{
    while(*text)
    {
        Serial_PutChar(*text++);
    }
}
