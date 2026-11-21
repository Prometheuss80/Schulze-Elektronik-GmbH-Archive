#include "isl6_hw.h"
#include "watchdog.h"
#include "delay.h"
#include "beeper.h"

/*
 * P1.1 -> 40106 -> piezo.
 *
 * About 1.5 kHz, 50/50:
 * 16 MHz / 12 = 1.333 MHz
 * half period about 333 us = 444 timer counts
 * reload = 65536 - 444 = 0xFE44.
 */

static void Beeper_HalfPeriod(void)
{
    TR0 = 0;
    TMOD = (TMOD & 0xF0) | 0x01;
    TF0 = 0;
    TH0 = 0xFE;
    TL0 = 0x44;
    TR0 = 1;

    while (TF0 == 0)
        ;

    TR0 = 0;
    TF0 = 0;
}

static void Beeper_ToneCycles(unsigned int cycles)
{
    while (cycles != 0)
    {
        BEEPER = 1;
        Beeper_HalfPeriod();

        BEEPER = 0;
        Beeper_HalfPeriod();

        WDT_Kick();
        cycles--;
    }

    BEEPER = 0;
}

void Beeper_Init(void)
{
    BEEPER = 0;
}

void Beeper_OK(void)
{
    /* Short tone ~90 ms, pause 100 ms, long tone ~280 ms. */
    Beeper_ToneCycles(135);
    DelayMs(100);
    Beeper_ToneCycles(420);
}
