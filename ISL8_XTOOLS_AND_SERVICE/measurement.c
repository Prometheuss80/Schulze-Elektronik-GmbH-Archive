#include <REG80C537_ISL8.H>
#include "measurement.h"
#include "system.h"
#include "adc.h"
/* Do not convert current RAW to mA until calibrated.
 * One channel per 100 ms to avoid a long blocking scan.
 * Measurements are informational: existing immediate A2/A3 guards
 * continue to measure directly in main.c. */
MEASUREMENT_DATA xdata measurement;
static unsigned char data scan_channel;
static unsigned int data next_scan;
void Measurement_Init(void)
{
    unsigned char i;
    for(i=0u;i<12u;i++) measurement.raw[i]=0u;
    measurement.valid_mask=0u;
    measurement.last_tick=0u;
    scan_channel=0u;
    next_scan=System_Ticks10ms();
}
void Measurement_Task(void)
{
    unsigned int now=System_Ticks10ms();
    if((signed int)(now-next_scan)<0) return;
    next_scan=(unsigned int)(now+10u);
    measurement.raw[scan_channel]=ADC_Measure(scan_channel);
    measurement.valid_mask|=(unsigned int)(1u<<scan_channel);
    measurement.last_tick=now;
    ++scan_channel;
    if(scan_channel>=12u) scan_channel=0u;
}
