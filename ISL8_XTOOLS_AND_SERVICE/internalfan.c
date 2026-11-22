#include "internalfan.h"
#include "hardware.h"
/* ADC8 raw decreases as the board warms. Hysteresis from ROM study.
   Fan state is independent of charger page, running, STOP and END. 2650/2800 */
#define FAN_ON_RAW  2300u
#define FAN_OFF_RAW 2500u
static unsigned char fan_on;
static unsigned int next_tick;
void InternalFan_Init(void)
{
    fan_on=0u;
    next_tick=0u;
    HW_TestSet(HWTEST_FAN_INT,0u);
}
#if 0 /* UNUSED L16: InternalFan_IsOn; retained for future work */
unsigned char InternalFan_IsOn(void) { return fan_on; }
#endif /* UNUSED: InternalFan_IsOn */
void InternalFan_Task(unsigned char manual_page,unsigned char valid,unsigned int raw,unsigned int ticks10ms)
{
    if(manual_page) return; /* Preserve manual control in HW TEST. */
    if((signed int)(ticks10ms-next_tick)<0) return;
    next_tick=(unsigned int)(ticks10ms+50u); /* 500 ms */
    if(!valid) return; /* Keep startup OFF until first valid ADC8 sample. */
    else if(raw<FAN_ON_RAW) fan_on=1u;
    else if(raw>=FAN_OFF_RAW) fan_on=0u;
    HW_TestSet(HWTEST_FAN_INT,fan_on);
}
