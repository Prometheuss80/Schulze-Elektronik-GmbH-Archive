#include <REG80C537_ISL8.H>
#include "test5v.h"
#include "hardware.h"
/* After 40106: 10ms H, 10ms L, 10ms H, ~140ms L. */
static volatile unsigned char data modes[2], busy[2], phase;
static unsigned char data applied=0xFFu;
static void ApplyNow(void)
{
    unsigned char pulse,desired;
    pulse=(phase==0u || phase==2u)?0u:1u;
    desired=(unsigned char)(
       ((busy[0]==0u && (modes[0]==TEST5V_ON ||
          (modes[0]==TEST5V_IDLE && pulse)))?1u:0u) |
       ((busy[1]==0u && (modes[1]==TEST5V_ON ||
          (modes[1]==TEST5V_IDLE && pulse)))?2u:0u));
    if(desired!=applied) { applied=desired; HW_Test5V_ApplyISR(desired); }
}
void Test5V_Init(void)
{
    unsigned char et=ET0; ET0=0;
    modes[0]=TEST5V_IDLE; modes[1]=TEST5V_IDLE;
    busy[0]=0u; busy[1]=0u; phase=16u; applied=0xFFu;
    ApplyNow(); ET0=et;
}
void Test5V_TickISR(void)
{
    ++phase; if(phase>=17u) phase=0u;
    ApplyNow();
}
void Test5V_Task(void) { /* ISR driven. */ }
void Test5V_SetMode(unsigned char ch,unsigned char mode)
{
    unsigned char et;
    if(ch>1u || mode>TEST5V_IDLE) return;
    et=ET0; ET0=0; modes[ch]=mode; ApplyNow(); ET0=et;
}
unsigned char Test5V_GetMode(unsigned char ch)
{ return ch<2u?modes[ch]:TEST5V_OFF; }
void Test5V_SetBusy(unsigned char ch,unsigned char is_busy)
{
    unsigned char et;
    if(ch>1u) return;
    et=ET0; ET0=0; busy[ch]=(is_busy!=0u); ApplyNow(); ET0=et;
}
