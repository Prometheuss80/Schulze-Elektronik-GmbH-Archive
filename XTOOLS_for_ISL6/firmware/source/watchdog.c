#include "watchdog.h"

/*
 * WDT_Kick() itself is implemented in watchdog_asm.a51 so that the
 * two SAB80C535 watchdog instructions are GUARANTEED to be consecutive.
 */
void WDT_Init(void)
{
    WDT_Kick();
}
