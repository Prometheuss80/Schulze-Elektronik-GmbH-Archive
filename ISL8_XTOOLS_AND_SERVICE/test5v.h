#ifndef ISL8_TEST5V_H
#define ISL8_TEST5V_H
#define TEST5V_OFF 0u
#define TEST5V_ON 1u
#define TEST5V_IDLE 2u
void Test5V_Init(void);
void Test5V_Task(void);
void Test5V_TickISR(void);
void Test5V_SetMode(unsigned char channel,unsigned char mode);
unsigned char Test5V_GetMode(unsigned char channel);
void Test5V_SetBusy(unsigned char channel,unsigned char busy);
#endif
