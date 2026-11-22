#ifndef INTERNALFAN_H
#define INTERNALFAN_H
void InternalFan_Init(void);
void InternalFan_Task(unsigned char manual_page,unsigned char valid,unsigned int raw,unsigned int ticks10ms);
unsigned char InternalFan_IsOn(void);
#endif
