#ifndef GUI_H
#define GUI_H
#include "timekeeper.h"

typedef enum
{
    PAGE_STATUS=0,
    PAGE_CALIB,
    PAGE_USER,
    PAGE_USER_ERASE,
    PAGE_TIME,
    PAGE_MEASUREMENTS,
    PAGE_A2_TEST,
    PAGE_A1_TEST,
    PAGE_HARDWARE,
    PAGE_HW_TEST,
    PAGE_TIMEKEEPER,
    PAGE_EEPROM,
    PAGE_EEPROM_DUMP,
    PAGE_EEPROM_SN,
    PAGE_EEPROM_ERASE,
    PAGE_CHARGE,
    PAGE_COUNT
} GUI_PAGE;

void GUI_ShowSplash(void);
void GUI_ChargeSelect(void);
void GUI_ChargeEdit(void);
void GUI_ChargeAdjust(signed char delta);
void GUI_ChargeMode(void);
void GUI_ChargeStartDemo(void);
void GUI_ChargeStop(void);
void GUI_DrawPage(GUI_PAGE page,const TIMEKEEPER_INFO *t,unsigned char hw_cursor,
                  unsigned char calib_channel,unsigned char calib_edit,unsigned char time_field);
void GUI_A1EditNext(void);
void GUI_A1BenchLive(unsigned int cc,unsigned int raw,unsigned char running,unsigned char fault);
void GUI_A1BenchMode(unsigned char discharge);
void GUI_A1BenchTotals(unsigned long elapsed_ticks,unsigned long capacity_x100);
void GUI_A1ModeNext(void);
void GUI_A1SelectDischargePath(unsigned char path);
void GUI_A1Adjust(int delta);
unsigned char GUI_A1IsCurrentEdit(void);
unsigned int GUI_A1CurrentSet(void);
unsigned int GUI_A1VoltageSet(void);
unsigned int GUI_A1CapacitySet(void);
unsigned int GUI_A1TimeSet(void);
unsigned char GUI_A1IsAuto(void);
void GUI_A1ToggleAuto(void);
unsigned char GUI_A1RecupEnabled(void);
unsigned int GUI_A1RecupBattLimit(void);
unsigned char GUI_A2IsAuto(void);
void GUI_A2ToggleAuto(void);
unsigned char GUI_A3IsAuto(void);
void GUI_A3ToggleAuto(void);
void GUI_A2EditNext(void);
void GUI_A2ResetPWM(void);
void GUI_A2StartPWM(void);
void GUI_A2SetRunning(unsigned char running);
unsigned char GUI_A2IsPwmEdit(void);
void GUI_A2Adjust(int delta);
unsigned int GUI_A2GetCompare(void);
unsigned int GUI_A2GetPwmPct(void);
unsigned int GUI_A2TargetCompare(void);
unsigned int GUI_A2TargetCurrent(void);
unsigned int GUI_A2VoltageSet(void);
unsigned int GUI_A2CapacitySet(void);
unsigned int GUI_A2TimeSet(void);
void GUI_A2SetStatus(unsigned char status);
void GUI_A2SetTotals(unsigned long mah,unsigned long sec);
void GUI_A2SetCompare(unsigned int compare);
unsigned int GUI_A3TargetCompare(void);
void GUI_A3EditNext(void);
void GUI_A3SetNext(void);
void GUI_A3Adjust(int delta);
void GUI_A3ToggleMode(void);
unsigned char GUI_A3GetModeDischarge(void);
unsigned int GUI_A3GetTargetCurrent(void);
unsigned int GUI_A3DischargeEstimatedMa(unsigned int cc);
unsigned int GUI_A3GetVoltageLimit(void);
unsigned char GUI_A3IsPwmEdit(void);
unsigned int GUI_A3GetDischargeCompare(void);
unsigned int GUI_A3GetChargePwmPercent(void);
unsigned int GUI_A3GetChargeCompare(void);
void GUI_A3SetRunning(unsigned char running);
void GUI_A3SetChargeZeroRaw(unsigned int raw);
void GUI_A3SetLive(unsigned int cc,unsigned long mah,unsigned long sec,unsigned char status);
unsigned int GUI_A3CapacitySet(void);
unsigned int GUI_A3TimeSet(void);

void GUI_EepromDumpFirst(void);
void GUI_EepromDumpNext(void);
void GUI_EepromDumpPrev(void);

#endif

unsigned char GUI_A1IsToggleEdit(void);
