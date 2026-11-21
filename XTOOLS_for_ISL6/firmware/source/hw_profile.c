#include "isl6_hw.h"
#include "watchdog.h"
#include "lcd.h"
#include "hw_profile.h"

static unsigned char hw_mode = HW_MODE_430P;

unsigned char HWProfile_GetMode(void)
{
    return hw_mode;
}

char *HWProfile_GetModelText(void)
{
    if (hw_mode == HW_MODE_330)
        return "ISL6-330D";
    return "ISL6-430/530/636 family";
}

static void WaitRelease(void)
{
    unsigned int n;
    n = 0;
    while (n < 3000)
    {
        WDT_Kick();
        if ((P1 & 0xC0) == 0xC0)
            n++;
        else
            n = 0;
    }
}

void HWProfile_Select(void)
{
    /*
     * IMPORTANT: no EEPROM access is allowed before this selection.
     * Both LCD types understand the same controller addressing.  Only the
     * first 8 characters of each controller row are used for this screen,
     * therefore it is readable on both 1x16 (330D) and 2x16 displays.
     * The universal instruction is: PRESS / LEFT KEY.
     *
     * KEY1 = P1.6 -> MODE 2 / ISL6-430/530/636 family
     * KEY2 = P1.7 -> MODE 1 / ISL6-330D
     *
     * The physical left/right key wiring is reversed between the 330D and
     * 430/530/636 family.  With this mapping the LEFT physical key selects
     * the correct hardware profile on both families.
     */
    LCD_SetCursor(0,0);
    LCD_PutChar('P'); LCD_PutChar('R'); LCD_PutChar('E'); LCD_PutChar('S');
    LCD_PutChar('S'); LCD_PutChar(' '); LCD_PutChar(' '); LCD_PutChar(' ');
    LCD_SetCursor(1,0);
    LCD_PutChar('L'); LCD_PutChar('E'); LCD_PutChar('F'); LCD_PutChar('T');
    LCD_PutChar(' '); LCD_PutChar('K'); LCD_PutChar('E'); LCD_PutChar('Y');

    while (1)
    {
        WDT_Kick();

        if ((P1 & 0x40) == 0)
        {
            hw_mode = HW_MODE_430P;
            break;
        }

        if ((P1 & 0x80) == 0)
        {
            hw_mode = HW_MODE_330;
            break;
        }
    }

    LCD_SetMode(hw_mode);
    WaitRelease();
}
