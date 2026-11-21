#include "isl6_hw.h"
#include "watchdog.h"
#include "delay.h"

/*
 * Timer0 mode 1, assuming the ISL6 16 MHz CPU clock.
 * 16 MHz / 12 = 1.333333 MHz timer rate.
 * 1333 counts ~= 1 ms -> reload 0xFACB.
 */
void DelayMs(unsigned int ms)
{
    while (ms != 0)
    {
        TR0 = 0;
        TMOD = (TMOD & 0xF0) | 0x01;
        TF0 = 0;
        TH0 = 0xFA;
        TL0 = 0xCB;
        TR0 = 1;

        while (TF0 == 0)
            ;

        TR0 = 0;
        TF0 = 0;

        WDT_Kick();
        ms--;
    }
}
