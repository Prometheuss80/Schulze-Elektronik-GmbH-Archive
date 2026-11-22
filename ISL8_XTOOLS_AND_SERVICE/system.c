#include <REG80C537_ISL8.H>
#include "system.h"
#include "test5v.h"
/* 16MHz / 12 = 13333 timer counts per 10ms. Timer1=UART, Timer2=PWM. */
static volatile unsigned int data ticks10;
void System_Timer0_ISR(void) interrupt 1
{
    TH0=0xCBu; TL0=0xEBu;
    ++ticks10;
    Test5V_TickISR();
}
void System_Init(void)
{
    TR0=0; ET0=0;
    TMOD=(unsigned char)((TMOD&0xF0u)|0x01u);
    TH0=0xCBu; TL0=0xEBu; TF0=0;
    ticks10=0u;
    ET0=1; IEN0 |= 0x80u; TR0=1;
}
unsigned int System_Ticks10ms(void)
{
    unsigned int t;
    unsigned char old=ET0;
    ET0=0; t=ticks10; ET0=old;
    return t;
}
