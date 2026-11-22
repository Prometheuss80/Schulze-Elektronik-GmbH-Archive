#include "hardware.h"
#include "watchdog.h"

/*
 * Same principle as the proven XTOOLS SOUND_SHORT routine:
 * V1.46: higher piezo drive frequency (about 2x), while the number of
 * transitions is doubled so the audible beep duration stays approximately unchanged.
 *
 * P4.0..P4.3 are keyboard inputs, therefore only the PIEZO bit
 * is changed here -- never the complete P4 register.
 */
static void Sound_Delay1ms(void)
{
    volatile unsigned int i;

    /*
     * TEST delay. Exact pitch is not important for this test.
     * The shorter delay raises pitch; actual loudness depends on piezo resonance.
     */
    for(i=0;i<20u;i++) { ; }
}

void Sound_BeepShort(void)
{
    unsigned char i;

    for(i=0;i<240u;i++)
    {
        PIEZO = !PIEZO;
        Sound_Delay1ms();
        WDT_Kick();
    }

    PIEZO = 0;
}


static void Sound_Gap(void)
{
    unsigned char n;
    volatile unsigned int i;
    for(n=0u;n<35u;n++)
    {
        for(i=0u;i<90u;i++) { ; }
        WDT_Kick();
    }
}

void Sound_BeepTriple(void)
{
    Sound_BeepShort();
    Sound_Gap();
    Sound_BeepShort();
    Sound_Gap();
    Sound_BeepShort();
}
