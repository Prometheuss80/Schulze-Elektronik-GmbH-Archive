#include <REG80C537_ISL8.H>
#include "charge.h"
/* Data model only: no new power enabling until protection is validated. */
CHARGE_CHANNEL xdata charge[3];
void Charge_Init(void)
{
    unsigned char i;
    for(i=0u;i<3u;i++)
    {
        charge[i].u_set_mv=0UL;
        charge[i].i_set_ma=0u;
        charge[i].time_set_min=0u;
        charge[i].cap_set_mah=0u;
        charge[i].mode=CHARGE_CHARGE;
        charge[i].state=CHARGE_STOPPED;
        charge[i].elapsed_sec=0UL;
        charge[i].capacity_mas=0UL;
    }
}
