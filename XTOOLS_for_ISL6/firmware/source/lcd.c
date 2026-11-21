#include "isl6_hw.h"
#include "watchdog.h"
#include "lcd.h"
#include "hw_profile.h"

static unsigned char lcd_mode = HW_MODE_430P;

void LCD_SetMode(unsigned char mode)
{
    lcd_mode = mode;
}

/*
 * ISL6 HD44780-compatible LCD, 4-bit mode:
 * P4.0 D4, P4.1 D5, P4.2 D6, P4.3 D7
 * P4.4 E, P4.5 R/W, P4.6 RS
 * P4.7 is preserved.
 *
 * Fixed delays are intentional.  The known-good V0.2C test proved
 * this write-only method on the real charger.
 */

static void SoftDelay(unsigned int count)
{
    volatile unsigned int i;

    for (i = 0; i < count; i++)
    {
        if ((i & 0x003F) == 0)
            WDT_Kick();
    }
}

static void PulseE(void)
{
    P4 |= 0x10;
    SoftDelay(20);
    P4 &= 0xEF;
    SoftDelay(20);
}

static void SetInitNibble(unsigned char n)
{
    unsigned char keep;

    keep = P4 & 0x80;
    P4 = keep | (n & 0x0F);
}

static void WriteNibble(unsigned char n, unsigned char rs)
{
    unsigned char keep;

    keep = P4 & 0x80;
    P4 = keep | (n & 0x0F);

    if (rs)
        P4 |= 0x40;
    else
        P4 &= 0xBF;

    P4 &= 0xDF;
    PulseE();
}

void LCD_Command(unsigned char command)
{
    WriteNibble((unsigned char)(command >> 4), 0);
    WriteNibble((unsigned char)(command & 0x0F), 0);
    SoftDelay(4000);
}

void LCD_PutChar(unsigned char c)
{
    WriteNibble((unsigned char)(c >> 4), 1);
    WriteNibble((unsigned char)(c & 0x0F), 1);
    SoftDelay(300);
}

void LCD_SetCursor(unsigned char row, unsigned char column)
{
    unsigned char address;

    address = (row == 0) ? column : (unsigned char)(0x40 + column);
    LCD_Command((unsigned char)(0x80 | address));
}

void LCD_WriteLine(unsigned char row, char *text)
{
    unsigned char i;
    unsigned char c;

    if (lcd_mode == HW_MODE_330)
    {
        /* 330D 1x16 is physically addressed as two 8-character halves. */
        LCD_SetCursor(0, 0);
        for (i = 0; i < 8; i++)
        {
            c = (*text != 0) ? (unsigned char)*text++ : (unsigned char)' ';
            LCD_PutChar(c);
        }

        LCD_SetCursor(1, 0);
        for (i = 0; i < 8; i++)
        {
            c = (*text != 0) ? (unsigned char)*text++ : (unsigned char)' ';
            LCD_PutChar(c);
        }
        return;
    }

    LCD_SetCursor(row, 0);
    for (i = 0; i < 16; i++)
    {
        if (*text != 0)
            LCD_PutChar((unsigned char)*text++);
        else
            LCD_PutChar(' ');
    }
}

void LCD_Init(void)
{
    SoftDelay(30000);

    SetInitNibble(0x03);
    PulseE();
    SoftDelay(10000);

    SetInitNibble(0x03);
    PulseE();
    SoftDelay(3000);

    SetInitNibble(0x03);
    PulseE();
    SoftDelay(3000);

    SetInitNibble(0x02);
    PulseE();
    SoftDelay(3000);

    LCD_Command(0x28);
    LCD_Command(0x08);
    LCD_Command(0x01);
    LCD_Command(0x06);
    LCD_Command(0x0C);
    LCD_Command(0x02);
}
