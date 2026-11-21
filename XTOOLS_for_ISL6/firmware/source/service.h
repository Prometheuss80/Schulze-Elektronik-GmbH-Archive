#ifndef __SERVICE_H__
#define __SERVICE_H__

void Service_ReadEEPROM(void);
void Service_ReportUART(void);
void Service_LCDLoop(void);
unsigned int Service_GetEEPROM(unsigned char address);

#endif
