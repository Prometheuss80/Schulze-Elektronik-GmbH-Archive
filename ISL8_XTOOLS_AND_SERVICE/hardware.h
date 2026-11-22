#ifndef HARDWARE_H
#define HARDWARE_H

#include <REG80C537_ISL8.H>

#define LCD_E       0x01u
#define LCD_RW      0x02u
#define LCD_RS      0x04u
#define LCD_CS      0x08u
#define LCD_RESET   0x10u
#define LATCH_B     0x20u
#define LATCH_C     0x40u

sbit PIEZO    = P4^4;
sbit OE_373BC = P4^5;

void HW_Init(void);
void HW_Delay(void);


/* TEST HW controlled functions. Only these six latch outputs are touched. */
#define HWTEST_FAN_INT   0u
#define HWTEST_FAN_OUT   1u
#define HWTEST_BLINK     2u
#define HWTEST_DISCH_RES 3u
#define HWTEST_5V_A1     4u
#define HWTEST_5V_A2     5u
#define HWTEST_COUNT     6u

void HW_TestSet(unsigned char function_id,unsigned char on);
void HW_Test5V_ApplyISR(unsigned char active_mask);
unsigned char HW_TestGet(unsigned char function_id);
void HW_TestAllOff(void);

void HW_A2AllOff(void);
void HW_A2PWMInit(void);
void HW_A2SetCompare(unsigned int compare_value);
void HW_A2Enable(unsigned char on);

/* A3 service power test. Latch C bits are active LOW; safe state is HIGH. */
void HW_A3AllOff(void);
void HW_A3PowerOffKeepPWM(void);
void HW_A3SetDirection(unsigned char discharge);
void HW_A3PWMInit(void);
void HW_A3SetCompare(unsigned int compare_value);

void HW_A1AllOff(void);
void HW_A1PWMInit(void);
void HW_A1SetCompare(unsigned int value);
void HW_A1ChargeEnable(unsigned char on);
void HW_A1DischResEnable(unsigned char on);
void HW_A1RecupEnable(unsigned char on);

#endif
