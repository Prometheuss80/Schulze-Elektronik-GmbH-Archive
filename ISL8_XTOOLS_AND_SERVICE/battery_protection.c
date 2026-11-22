#include "battery_protection.h"
#include "adc.h"
/* A3 discharge is voltage-only: ADC11 does not sense discharge current.
 * All thresholds are provisional and require validation on real hardware. */
#define BP_INTERVAL_TICKS  2u /* target 20 ms, actual latency depends on ADC/main loop */
#define BP_IDLE_MARGIN     5u
#define BP_A3_DISCONNECT_MV 9000L
#define BP_CONFIRM_COUNT   2u
static unsigned char bp_mode,bp_armed,bp_rise,bp_fall;
static unsigned int bp_threshold,bp_next;
void BatteryProtection_Init(void)
{
    bp_mode=0u;bp_armed=0u;bp_rise=0u;bp_fall=0u;
    bp_threshold=0u;bp_next=0u;
}
void BatteryProtection_Stop(void) { bp_mode=0u;bp_armed=0u;bp_rise=0u;bp_fall=0u; }
#if 0 /* UNUSED L16: BatteryProtection_IsArmed; retained for future work */
unsigned char BatteryProtection_IsArmed(void) { return bp_armed; }
#endif /* UNUSED: BatteryProtection_IsArmed */
void BatteryProtection_Start(unsigned char mode,unsigned int idle_raw,unsigned int now)
{
    bp_mode=mode;bp_armed=0u;bp_rise=0u;bp_fall=0u;
    bp_threshold=(idle_raw>65530u)?65535u:(unsigned int)(idle_raw+BP_IDLE_MARGIN);
    bp_next=now;
}
unsigned char BatteryProtection_Task(unsigned int now,signed char a3_tol,signed char a3_ofs)
{
    unsigned int raw;
    if(bp_mode==0u) return 0u;
    if((signed int)(now-bp_next)<0) return 0u;
    bp_next=(unsigned int)(now+BP_INTERVAL_TICKS);
    if(bp_mode==BP_A3_DISCH)
    {
        ADC_VOLTAGE_RESULT v;
        ADC_MeasureA3(a3_tol,a3_ofs,&v);
        /* Ignore invalid negative conversion as fault, never as 'connected'. */
        if(v.mv_cal<0L || v.mv_cal>=BP_A3_DISCONNECT_MV)
        {
            if(++bp_fall>=BP_CONFIRM_COUNT) return 1u;
        }
        else bp_fall=0u;
        return 0u;
    }
    raw=(bp_mode==BP_A2_CHARGE)?ADC_Measure(ADC_CH_A2_CURRENT):ADC_MeasureA3Current();
    if(!bp_armed)
    {
        if(raw>bp_threshold)
        {
            if(++bp_rise>=BP_CONFIRM_COUNT) { bp_armed=1u;bp_fall=0u; }
        }
        else bp_rise=0u;
    }
    else if(raw<=bp_threshold)
    {
        if(++bp_fall>=BP_CONFIRM_COUNT) return 1u;
    }
    else bp_fall=0u;
    return 0u;
}
