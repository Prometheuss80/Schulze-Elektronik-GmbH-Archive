#include "system.h"
#include "test5v.h"
#include "measurement.h"
#include "internalfan.h"
#include "battery_protection.h"
#include "charge.h"
#include "hardware.h"
#include "watchdog.h"
#include "lcd.h"
#include "keyboard.h"
#include "sound.h"
#include "serial.h"
#include "timekeeper.h"
#include "gui.h"
#include "service.h"
#include "adc.h"
#include "eeprom_fixed_test.h"

/* Shared CC charge ramp for A2/A3. Percentages use 32-bit arithmetic. */
static unsigned int CC_GetRampStep(unsigned int actual_ma,unsigned int target_ma)
{
    unsigned long scaled;
    if(target_ma==0u || actual_ma>=target_ma) return 0u;
    scaled=(unsigned long)actual_ma*100UL;
    if(scaled<(unsigned long)target_ma*90UL) return 32u;
    if(scaled<(unsigned long)target_ma*95UL) return 16u;
    if(scaled<(unsigned long)target_ma*98UL) return 8u;
    if(scaled<(unsigned long)target_ma*99UL) return 4u;
    return 1u;
}

static signed char StepSigned(signed char v,signed char delta)
{
    signed int n=(signed int)v+(signed int)delta;
    if(n>127) n=127;
    if(n<-128) n=-128;
    return (signed char)n;
}

static void CalibAdjust(TIMEKEEPER_INFO *t,unsigned char channel,unsigned char edit,signed char delta)
{
    signed char tol,ofs;

    if(channel==0u)
    {
        tol=(signed char)t->copy[0].tol_a1;
        ofs=(signed char)t->copy[0].ofs_a1;
        if(edit==0u) tol=StepSigned(tol,delta); else ofs=StepSigned(ofs,delta);
        Timekeeper_WriteCalibration(TMK_CAL_A1,tol,ofs);
    }
    else if(channel==1u)
    {
        tol=(signed char)t->copy[0].tol_a2;
        ofs=(signed char)t->copy[0].ofs_a2;
        if(edit==0u) tol=StepSigned(tol,delta); else ofs=StepSigned(ofs,delta);
        Timekeeper_WriteCalibration(TMK_CAL_A2,tol,ofs);
    }
    else if(channel==2u)
    {
        tol=(signed char)t->copy[0].tol_a3;
        ofs=(signed char)t->copy[0].ofs_a3;
        if(edit==0u) tol=StepSigned(tol,delta); else ofs=StepSigned(ofs,delta);
        Timekeeper_WriteCalibration(TMK_CAL_A3,tol,ofs);
    }
    else
    {
        tol=(signed char)t->batt_tol[0];
        tol=StepSigned(tol,delta);
        Timekeeper_WriteBattTol(tol);
    }

    Timekeeper_ReadInfo(t);
}

static void CalibClear(TIMEKEEPER_INFO *t,unsigned char channel)
{
    if(channel==0u) Timekeeper_WriteCalibration(TMK_CAL_A1,0,0);
    else if(channel==1u) Timekeeper_WriteCalibration(TMK_CAL_A2,0,0);
    else if(channel==2u) Timekeeper_WriteCalibration(TMK_CAL_A3,0,0);
    else Timekeeper_WriteBattTol(0);
    Timekeeper_ReadInfo(t);
}

static unsigned char RtcMaxDay(unsigned char month,unsigned char year)
{
    if(month==2u) return ((year&3u)==0u)?29u:28u;
    if(month==4u || month==6u || month==9u || month==11u) return 30u;
    return 31u;
}

static void RtcAdjust(TIMEKEEPER_INFO *t,unsigned char field,signed char delta)
{
    signed int v,minv,maxv;
    unsigned char month,year;
    if(field==TMK_RTC_HOUR) { v=Timekeeper_BcdToBin(t->hour_bcd); minv=0; maxv=23; }
    else if(field==TMK_RTC_MINUTE) { v=Timekeeper_BcdToBin(t->minute_bcd); minv=0; maxv=59; }
    else if(field==TMK_RTC_SECOND) { v=Timekeeper_BcdToBin(t->second_bcd); minv=0; maxv=59; }
    else if(field==TMK_RTC_DAY)
    {
        v=Timekeeper_BcdToBin(t->day_bcd); minv=1;
        month=Timekeeper_BcdToBin(t->month_bcd); year=Timekeeper_BcdToBin(t->year_bcd);
        maxv=RtcMaxDay(month,year);
    }
    else if(field==TMK_RTC_MONTH) { v=Timekeeper_BcdToBin(t->month_bcd); minv=1; maxv=12; }
    else if(field==TMK_RTC_YEAR) { v=Timekeeper_BcdToBin(t->year_bcd); minv=0; maxv=99; }
    else { v=Timekeeper_BcdToBin(t->weekday_bcd); minv=1; maxv=7; }

    v+=(signed int)delta;
    if(v>maxv) v=minv;
    if(v<minv) v=maxv;
    Timekeeper_RtcWriteValue(field,(unsigned char)v);
    Timekeeper_ReadInfo(t);
}

unsigned int g_eeprom_word06;
unsigned int xdata g_eeprom_words[64];
unsigned char g_eeprom_full_valid;
unsigned char data g_eeprom_address _at_ 0x30;
unsigned char data g_eeprom_write_hi _at_ 0x31;
unsigned char data g_eeprom_write_lo _at_ 0x32;
unsigned char g_eeprom_word0_valid;
unsigned int g_eeprom_sn_edit;
unsigned char g_eeprom_sn_digit;
unsigned char g_eeprom_sn_write_status;
unsigned char g_eeprom_sync_status;
unsigned char g_eeprom_erase_status;
unsigned char g_timekeeper_erase_status;

static unsigned int EepromLogical(unsigned int raw)
{
    return (unsigned int)((raw<<8)|(raw>>8));
}

static void EepromReadAll(void)
{
    unsigned char i;
    g_eeprom_full_valid=0u;
    for(i=0u;i<64u;i++)
    {
        g_eeprom_address=i;
        g_eeprom_words[i]=EEPROM_Read_DataAddr();
    }
    g_eeprom_word06=g_eeprom_words[0x06u];
    g_eeprom_full_valid=1u;
}

static void SnAdjustDigit(signed char delta)
{
    static const unsigned int code divs[5]={10000u,1000u,100u,10u,1u};
    unsigned int d=divs[g_eeprom_sn_digit];
    unsigned int digit=(g_eeprom_sn_edit/d)%10u;
    unsigned int rest=g_eeprom_sn_edit-(digit*d);
    if(delta>0) digit=(digit==9u)?0u:(digit+1u);
    else digit=(digit==0u)?9u:(digit-1u);
    g_eeprom_sn_edit=rest+(digit*d);
}

static unsigned char EepromSyncSerialToTimekeeper(TIMEKEEPER_INFO *t)
{
    unsigned int s0,s1,s2;
    s0=EepromLogical(g_eeprom_words[0x06u]);
    s1=EepromLogical(g_eeprom_words[0x1Bu]);
    s2=EepromLogical(g_eeprom_words[0x30u]);

    /* Never propagate an erased/invalid EEPROM serial number into TK. */
    if(s0!=s1 || s0!=s2 || s0==0xFFFFu) return 0u;

    Timekeeper_WriteSerial(s0);
    Timekeeper_ReadInfo(t);
    if(t->copy[0].serial_number!=s0 || t->copy[1].serial_number!=s0 ||
       t->copy[2].serial_number!=s0) return 0u;
    return 1u;
}

static unsigned char EepromEraseAll(void)
{
    unsigned char i,ok=1u;
    g_eeprom_write_hi=0xFFu;
    g_eeprom_write_lo=0xFFu;

    /* Use only the already hardware-proven per-word WRITE sequence.
     * No unverified 93C46 ERAL command is introduced here. */
    for(i=0u;i<64u;i++)
    {
        g_eeprom_address=i;
        EEPROM_Write_DataAddr();
        WDT_Kick();
    }

    EepromReadAll();
    for(i=0u;i<64u;i++)
        if(EepromLogical(g_eeprom_words[i])!=0xFFFFu) ok=0u;
    return ok;
}

static unsigned char EepromWriteSerial(TIMEKEEPER_INFO *t,unsigned int sn)
{
    static const unsigned char code a[3]={0x06u,0x1Bu,0x30u};
    unsigned char i,ok=1u;

    g_eeprom_write_hi=(unsigned char)(sn>>8);
    g_eeprom_write_lo=(unsigned char)(sn&0xFFu);
    for(i=0u;i<3u;i++)
    {
        g_eeprom_address=a[i];
        EEPROM_Write_DataAddr();
        WDT_Kick();
    }

    Timekeeper_WriteSerial(sn);
    EepromReadAll();
    Timekeeper_ReadInfo(t);

    for(i=0u;i<3u;i++)
        if(EepromLogical(g_eeprom_words[a[i]])!=sn) ok=0u;
    if(t->copy[0].serial_number!=sn || t->copy[1].serial_number!=sn ||
       t->copy[2].serial_number!=sn) ok=0u;
    return ok;
}


#define A3_CHARGE_HARD_RAW 2300u /* bench trip, provisional: approx 1.6A at 0.7mA/RAW */
#define A3_CC_IDLE 0xFBFFu
#define A3_CC_MAX  0xFF60u /* experimental range: verify A3 duty and current */
#define A2_CHARGE_HARD_RAW 3600u /* ~5.30 A using current bench calibration */
#define A2_OVERVOLT_MARGIN_MV 150u /* emergency cutoff above selected CV voltage */
#define A3_DISCH_HARD_MA   150u

/* A2 service-test CC/CV controller: bench verification required before battery use. */
#define A2_CC_IDLE 0xFBFFu
#define A2_CC_MAX  0xFFFFu
static void A2PowerStop(unsigned char *running,unsigned char status)
{
    BatteryProtection_Stop();
    HW_A2Enable(0u);
    HW_A2SetCompare(A2_CC_IDLE);
    *running=0u;
    GUI_A2SetRunning(0u);
    GUI_A2SetCompare(A2_CC_IDLE);
    GUI_A2SetStatus(status);
}

static void A3Stop(unsigned char *running)
{
    /* Original zero-current runtime stop: remove the power path immediately,
     * then return CC3 to FBFF while CCEN/Timer2 remains armed. */
    BatteryProtection_Stop();
    HW_A3PowerOffKeepPWM();
    *running=0u;
    GUI_A3SetRunning(0u);
}

/* V1.30: pre-arm CC3 before connecting either A3 power direction.
 * HW_Delay() is a short CPU-loop delay; eight calls give a deliberately
 * conservative settling interval of roughly tens of milliseconds on the
 * present 6 MHz service build.  Exact timing is not used for protection. */
static void A3ArmDelay(void)
{
    unsigned char n;
    for(n=0u;n<8u;n++)
    {
        HW_Delay();
        WDT_Kick();
    }
}

/* F1 START must be held across an RTC second transition.  This makes an
 * accidental short press unable to energize A3 and does not depend on a
 * compiler-loop timing constant. */
static unsigned char A3ConfirmStartHold(void)
{
    unsigned char sec=(unsigned char)(Timekeeper_ReadByte(0x9FF9u)&0x7Fu);
    unsigned char edges=0u;
    while(Keyboard_GetKey()==KEY_F1)
    {
        unsigned char now=(unsigned char)(Timekeeper_ReadByte(0x9FF9u)&0x7Fu);
        WDT_Kick();
        if(now!=sec)
        {
            sec=now;
            ++edges;
            /* Two RTC transitions guarantee more than one full second hold. */
            if(edges>=2u) return 1u;
        }
    }
    return 0u;
}

/* Approximate key-repeat delay.  HW_Delay is already used throughout XTOOLS
 * as the short hardware delay. Exact milliseconds are not safety-critical. */
static void A3RepeatDelay(unsigned int ticks)
{
    while(ticks--)
    {
        HW_Delay();
        WDT_Kick();
    }
}

static void A3AutoRepeat(KEY_CODE key,const TIMEKEEPER_INFO *t,unsigned char hw_cursor,
                         unsigned char calib_channel,unsigned char calib_edit,unsigned char time_field,
                         unsigned char a3_running)
{
    int delta=(key==KEY_PLUS)?1:-1;
    unsigned int n;

    GUI_A3Adjust(delta);
    if(a3_running && GUI_A3IsPwmEdit())
    {
        if(GUI_A3GetModeDischarge()) HW_A3SetCompare(GUI_A3GetDischargeCompare());
        else HW_A3SetCompare(GUI_A3GetChargeCompare());
    }
    Sound_BeepShort();
    GUI_DrawPage(PAGE_MEASUREMENTS,t,hw_cursor,calib_channel,calib_edit,time_field);

    /* About half a second before repeat begins. */
    for(n=0u;n<250u;n++)
    {
        if(Keyboard_GetKey()!=key) return;
        HW_Delay();
        WDT_Kick();
    }

    while(Keyboard_GetKey()==key)
    {
        GUI_A3Adjust(delta);
        if(a3_running && GUI_A3IsPwmEdit())
        {
            if(GUI_A3GetModeDischarge()) HW_A3SetCompare(GUI_A3GetDischargeCompare());
            else HW_A3SetCompare(GUI_A3GetChargeCompare());
        }
        GUI_DrawPage(PAGE_MEASUREMENTS,t,hw_cursor,calib_channel,calib_edit,time_field);
        A3RepeatDelay(70u);
    }
}

/* RS232 diagnostics: 9600 baud, decimal values, no printf dependency. */
static void A2SerialUL(unsigned long v)
{
    char b[11]; unsigned char i=0u;
    do { b[i++]=(char)('0'+(char)(v%10UL)); v/=10UL; } while(v && i<10u);
    while(i) Serial_PutChar(b[--i]);
}
static void A2SerialU16(unsigned int v) { A2SerialUL((unsigned long)v); }
#if 0 /* UNUSED L16: A2SerialSL; retained for future work */
static void A2SerialSL(signed long v)
{
    if(v<0L) { Serial_PutChar('-'); A2SerialUL((unsigned long)(-(v+1L))+1UL); }
    else A2SerialUL((unsigned long)v);
}
#endif /* UNUSED: A2SerialSL */
/* RECUP ADC4 calibration: 16 bench points RAW 25..239, 152..897 mA.
 * Extrapolation beyond measured range is NOT verified. */
static unsigned long A1RecupRawMa(unsigned int raw)
{
    if(raw<=7u) return 0UL;
    return ((unsigned long)raw*3482UL+500UL)/1000UL+65UL;
}

void main(void)
{
    TIMEKEEPER_INFO t;
    GUI_PAGE page;
    KEY_CODE key,last_key;
    unsigned char last_second;
    unsigned char hw_cursor;
    unsigned char calib_channel;
    unsigned char calib_edit;
    unsigned int repeat_deadline;
    unsigned char repeat_armed;
    unsigned char time_field;
    unsigned char a3_running;
    unsigned char a3_start_rearmed;
    unsigned int a3_cc;
    unsigned int a3_idle_raw=0u;
    unsigned int a3_tick=0u,a3_subsecond=0u,a3_cv_ticks=0u;
    unsigned long a3_mah=0UL,a3_rem=0UL,a3_seconds=0UL;
    unsigned char a3_cv=0u,a3_status=0u;
    unsigned char a1_running=0u,a1_fault=0u,a1_rearm=1u,a1_disch=0u;
    unsigned int a1_recup_cv_cap=5000u,a1_recup_low_ticks=0u;
    unsigned char a1_recup_cv_seen=0u;
    unsigned char a1_up_confirm=0u;
    unsigned char a1_res_up_confirm=0u;
    unsigned char a1_cv_active=0u;
    unsigned int a1_charge_cv_ticks=0u;

    unsigned int a1_start_tick=0u;
    unsigned int a1_cc=0xFBFFu,a1_raw=0u,a1_sample_tick=0u,a1_control_tick=0u;
    unsigned long a1_elapsed_ticks=0UL,a1_capacity_x100=0UL;
    unsigned char a2_running=0u;
    unsigned int a2_start_tick=0u;
    unsigned int a2_last_serial_tick=0u;
    unsigned char a2_rearmed=1u;
    unsigned int a2_control_tick=0u,a2_lcd_tick=0u;
    unsigned int a2_compare=A2_CC_IDLE;
    unsigned int a2_manual_cc=A2_CC_IDLE;
    unsigned int a2_cv_low_ticks=0u;
    unsigned int a2_subsecond=0u;
    unsigned long a2_elapsed=0UL,a2_mah=0UL,a2_mah_remainder=0UL;
    unsigned char a2_cv=0u;
    unsigned int a12_repeat_start=0u,a12_repeat_due=0u;
    KEY_CODE a12_repeat_key=KEY_NONE;
    unsigned char a12_repeat_fire=0u;
    KEY_CODE a1_repeat_key=KEY_NONE;
    unsigned int a1_repeat_start=0u,a1_repeat_due=0u;
    /* A3 MAN repeat: same tick-based approach as working A2 MAN. */
    KEY_CODE a3_repeat_key=KEY_NONE;
    unsigned int a3_repeat_start=0u,a3_repeat_due=0u;
    unsigned char a3_repeat_fire=0u;


    HW_Init();
    WDT_Init();
    LCD_InitGraphic();
    Serial_Init();
    ADC_Init();
    Charge_Init();
    System_Init();
    Test5V_Init();
    Measurement_Init();
    InternalFan_Init();
    BatteryProtection_Init();

    Timekeeper_ReadInfo(&t);
    Service_SendStartupReport(&t);

    page=PAGE_STATUS;
    hw_cursor=0u;
    calib_channel=0u;   /* A1 */
    calib_edit=0u;      /* TOL */
    last_key=KEY_NONE;
    last_second=t.second_bcd;
    repeat_deadline=0u;
    repeat_armed=0u;
    time_field=TMK_RTC_HOUR;
    a3_running=0u;
    a3_start_rearmed=1u;
    a3_cc=GUI_A3GetChargeCompare();
    GUI_A3SetRunning(0u);
    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
    Sound_BeepTriple();

    while(1)
    {
        WDT_Kick();
        /* A1 manual bench emergency key polling before LCD/ADC work. */
        if(page==PAGE_A1_TEST && a1_running)
        {
            KEY_CODE stop_key=Keyboard_GetKey();
            if(stop_key==KEY_F2 || stop_key==KEY_ESC)
            {
                HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_fault=0u;
                a1_cc=0xFBFFu;a1_up_confirm=0u;a1_res_up_confirm=0u;
                a1_recup_cv_cap=5000u;a1_recup_low_ticks=0u;
                a1_recup_cv_seen=0u;a1_cv_active=0u;a1_charge_cv_ticks=0u;
                if(stop_key==KEY_ESC) page=PAGE_HARDWARE;
            GUI_A1BenchLive(a1_cc,a1_raw,a1_running,a1_fault);
            GUI_A1BenchTotals(a1_elapsed_ticks,a1_capacity_x100);
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                last_key=stop_key;continue;
            }
        }
        /* Physical F2/ESC polling BEFORE ADC, serial and LCD work. */
        if(page==PAGE_A2_TEST && a2_running)
        {
            KEY_CODE stop_key=Keyboard_GetKey();
            if(stop_key==KEY_F2 || stop_key==KEY_ESC)
            {
                A2PowerStop(&a2_running,0u);
                a2_rearmed=0u;
                if(stop_key==KEY_ESC) { HW_A2AllOff(); page=PAGE_HARDWARE; }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                last_key=stop_key;
                continue;
            }
        }
        Test5V_SetBusy(0u,0u);
        Test5V_SetBusy(1u,a2_running);
        Test5V_Task();
        Measurement_Task();
        if(page==PAGE_A1_TEST &&
           (unsigned int)(System_Ticks10ms()-a1_sample_tick)>=5u)
        {
            {
                unsigned int now=System_Ticks10ms();
                unsigned int dt=(unsigned int)(now-a1_sample_tick);
                a1_sample_tick=now;
                if(a1_running)
                {
                    unsigned long ma;
                    a1_elapsed_ticks+=(unsigned long)dt;
                    if(a1_disch==2u) ma=A1RecupRawMa(a1_raw);
                    else if(a1_disch) ma=((unsigned long)a1_raw*1140UL+500UL)/1000UL;
                    else ma=(a1_raw<=7u)?0UL:((unsigned long)a1_raw*3UL+15UL);
                    if(a1_capacity_x100<4000000000UL-ma*(unsigned long)dt)
                        a1_capacity_x100+=ma*(unsigned long)dt;
                }
            }
            a1_raw=ADC_Measure(ADC_CH_A1_CURRENT);
            /* Discharge: extended RAW limit, 10min timeout and 2S voltage cutoff.
             * RAW->mA equivalence in discharge remains to be verified. */
            if(a1_running)
            {
                unsigned char trip=0u;
                /* Independently limit A1 output power in either direction.
                 * Current conversion is bench-calibrated only up to 1 A CH
                 * and 290 mA RES.  Keep the original RAW limits as well. */
                {
                    ADC_VOLTAGE_RESULT a1_vr;
                    unsigned long a1_ma, a1_mw;
                    ADC_MeasureA1((signed char)t.copy[0].tol_a1,
                                  (signed char)t.copy[0].ofs_a1,&a1_vr);
                    if(!a1_disch && a1_raw>=700u) trip=1u; /* ~2A A1 charge */
                    if(a1_disch)
                    {
                        a1_ma=(a1_disch==2u)?A1RecupRawMa(a1_raw):(((unsigned long)a1_raw*1140UL+500UL)/1000UL);
                        if(a1_disch==2u)
                        {
                            ADC_BATT_RESULT batt;
                            ADC_MeasureBattPlus((signed char)t.batt_tol[0],&batt);
                            if(a1_vr.mv_cal>24000L || a1_vr.mv_cal<=0L) trip=1u;
                            /* Below 18V RECUP hands over to RES, not FAULT. */
                            if(batt.mv<11500u || batt.mv>=14700u) trip=1u;
                            /* Removed experimental 5.2A RECUP ceiling. */
                            /* RECUP bench: no automatic 20 s timeout.
                             * Voltage, RAW and power protections remain active. */
                        }
                        else
                        {
                            /* RES does not depend on BATT+ voltage or RECUP availability.
                             * UMIN is editable; the 20W hardware power cap stays. */
                            if(a1_vr.mv_cal<=0L || a1_vr.mv_cal>60000L) trip=1u;
                        }
                    }
                    else a1_ma=(a1_raw<=7u)?0UL:((unsigned long)a1_raw*3UL+15UL);
                    if(a1_vr.mv_cal<=0L) trip=1u;
                    if(!a1_disch && a1_vr.mv_cal>15000L) trip=1u; /* charge 15V cap, MAN/AUTO */
                    /* Stop normally at UMIN / capacity / time, independent of power trip. */
                    if(a1_disch && a1_vr.mv_cal>0L &&
                       a1_vr.mv_cal<=(signed long)GUI_A1VoltageSet())
                    { HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_cc=0xFBFFu; }
                    if(a1_running && a1_elapsed_ticks>=((unsigned long)GUI_A1TimeSet()*100UL))
                    { HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_cc=0xFBFFu; }
                    if(a1_running && a1_capacity_x100>=((unsigned long)GUI_A1CapacitySet()*360000UL))
                    { HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_cc=0xFBFFu; }
                    else
                    {
                        a1_mw=((unsigned long)a1_vr.mv_cal*a1_ma)/1000UL;
                        if(!a1_disch && a1_mw>30000UL) trip=1u; /* 15V x 2A charge envelope */
                    }
                }
                if(trip && a1_running)
                {
                    HW_A1AllOff();a1_running=0u;
                    a1_fault=1u;a1_rearm=0u;
                    Sound_BeepTriple();
                }
            }
            /* A1 CHARGE AUTO: A2-matched 100ms ramp, CC feedback, CV taper.
             * Charge ceiling 2A/15V applies to both MAN and AUTO.
             * A1 current calibration is provisional: mA ~= RAW*3+15. */
            if(a1_running && a1_disch==0u && GUI_A1IsAuto() &&
               (unsigned int)(System_Ticks10ms()-a1_control_tick)>=10u)
            {
                ADC_VOLTAGE_RESULT cv;
                unsigned int target=GUI_A1CurrentSet(),ma,step,limit;
                a1_control_tick=System_Ticks10ms();
                if(target>2000u) target=2000u;
                limit=GUI_A1VoltageSet();
                if(limit>15000u) limit=15000u;
                ADC_MeasureA1((signed char)t.copy[0].tol_a1,
                              (signed char)t.copy[0].ofs_a1,&cv);
                ma=(a1_raw<=7u)?0u:(unsigned int)((unsigned long)a1_raw*3UL+15UL);
                if(cv.mv_cal<=0L || cv.mv_cal>(signed long)limit+150L || a1_raw>=700u)
                {
                    HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_fault=1u;
                    Sound_BeepTriple();
                }
                else
                {
                    if(cv.mv_cal>=(signed long)limit) a1_cv_active=1u;
                    if(a1_cv_active)
                    {
                        if(cv.mv_cal>=(signed long)limit && a1_cc>0xFBFFu)
                        {
                            step=(cv.mv_cal>(signed long)limit+20L)?24u:8u;
                            a1_cc=(a1_cc>0xFBFFu+step)?(unsigned int)(a1_cc-step):0xFBFFu;
                        }
                        else if(cv.mv_cal+30L<(signed long)limit && ma+5u<target && a1_cc<0xFF4Fu)
                            ++a1_cc;
                        if(ma<=target/20u && cv.mv_cal+30L>=(signed long)limit)
                            ++a1_charge_cv_ticks;
                        else a1_charge_cv_ticks=0u;
                        if(a1_charge_cv_ticks>=100u)
                        {HW_A1AllOff();a1_running=0u;a1_rearm=0u;}
                    }
                    else if(ma<target && a1_cc<0xFF4Fu)
                    {
                        step=CC_GetRampStep(ma,target);
                        a1_cc=(a1_cc>0xFF4Fu-step)?0xFF4Fu:(unsigned int)(a1_cc+step);
                    }
                    else if(ma>target+10u && a1_cc>0xFBFFu)
                    {
                        step=(ma>target+100u)?24u:4u;
                        a1_cc=(a1_cc<0xFBFFu+step)?0xFBFFu:(unsigned int)(a1_cc-step);
                    }
                    if(a1_running) HW_A1SetCompare(a1_cc);
                }
            }
            /* AUTO RECUP: CC current regulation with BATT+ CV cap.
             * CV is a feedback limiter, not linear taper to zero.
             * R->D is one-way on A1 <18V or when CV cap <1A AND
             * measured input current <1A continuously for 2 seconds.
             * The 1A criterion is only relevant when user requests >=1A.
             * Invalid BATT+, >14.7V, and overcurrent still FAULT. */
            if(a1_running && a1_disch==2u && GUI_A1IsAuto() &&
               (unsigned int)(System_Ticks10ms()-a1_control_tick)>=10u)
            {
                ADC_BATT_RESULT batt_auto;
                ADC_VOLTAGE_RESULT v_auto;
                unsigned int limit=GUI_A1RecupBattLimit();
                unsigned int requested=GUI_A1CurrentSet();
                unsigned int target,raw_target,step=1u,diff;
                unsigned char transfer=0u;
                a1_control_tick=System_Ticks10ms();
                ADC_MeasureBattPlus((signed char)t.batt_tol[0],&batt_auto);
                ADC_MeasureA1((signed char)t.copy[0].tol_a1,
                              (signed char)t.copy[0].ofs_a1,&v_auto);
                if(v_auto.mv_cal>0L && v_auto.mv_cal<18000L) transfer=1u;
                if(requested>10000u) requested=10000u;
                if(batt_auto.mv<11500u || batt_auto.mv>=14700u ||
                   v_auto.mv_cal<=0L || v_auto.mv_cal>24000L)
                {
                    HW_A1AllOff();a1_running=0u;a1_rearm=0u;
                    a1_fault=1u;Sound_BeepTriple();
                }
                else
                {
                    if(a1_recup_cv_cap>requested) a1_recup_cv_cap=requested;
                    /* Enter CV 50mV below B LIM; release only below that
                     * threshold minus another 50mV. Hard 14.7V fault stays.
                     * The pre-limit prevents late action at 14.7V. */
                    if((unsigned long)batt_auto.mv+50UL>=(unsigned long)limit)
                        a1_cv_active=1u;
                    else if((unsigned long)batt_auto.mv+100UL<(unsigned long)limit)
                        a1_cv_active=0u;
                    if(a1_cv_active)
                    {
                        unsigned int reduce;
                        a1_recup_cv_seen=1u;
                        /* Faster taper as voltage approaches the limit. */
                        reduce=((unsigned long)batt_auto.mv+20UL>=(unsigned long)limit)?200u:100u;
                        if(a1_recup_cv_cap>reduce) a1_recup_cv_cap-=reduce;
                        else a1_recup_cv_cap=0u;
                    }
                    else if(a1_recup_cv_cap<requested)
                    {
                        if(a1_recup_cv_cap+50u>requested) a1_recup_cv_cap=requested;
                        else a1_recup_cv_cap+=50u;
                    }
                    target=(a1_recup_cv_cap<requested)?a1_recup_cv_cap:requested;
                    if(a1_recup_cv_seen && requested>=1000u &&
                       a1_recup_cv_cap<1000u && A1RecupRawMa(a1_raw)<1000UL)
                    {
                        if(a1_recup_low_ticks<20u) ++a1_recup_low_ticks;
                        if(a1_recup_low_ticks>=20u) transfer=1u;
                    }
                    else a1_recup_low_ticks=0u;
                    if(transfer)
                    {
                        /* Never energise both output paths together.
                         * Switch from idle CC1; RES regulator starts anew. */
                        HW_A1AllOff();
                        a1_cc=0xFBFFu;
                        /* AllOff clears CCEN. Re-enable CC1 before RES power! */
                        HW_A1PWMInit();
                        HW_A1SetCompare(a1_cc);
                        a1_up_confirm=0u;a1_res_up_confirm=0u;
                        a1_cv_active=0u;
                        a1_disch=1u;
                        GUI_A1SelectDischargePath(1u);
                        GUI_A1BenchMode(1u);
                        a1_control_tick=System_Ticks10ms();
                        HW_Delay();
                        HW_A1DischResEnable(1u);
                    }
                    else
                    {
                        if(target<=65u) raw_target=7u;
                        else raw_target=(unsigned int)
                           (((unsigned long)(target-65u)*1000UL+3481UL)/3482UL);
                        if(raw_target<1499u) ++raw_target;
                        if(!a1_cv_active && a1_raw+2u<raw_target && a1_cc<0xFF4Fu)
                        {
                            if(a1_up_confirm<3u) ++a1_up_confirm;
                            if(a1_up_confirm<3u) { /* wait for stable undershoot */ }
                            else
                            {
                            diff=raw_target-a1_raw;
                            if(diff>100u) step=8u;
                            else if(diff>40u) step=4u;
                            else if(diff>12u) step=2u;
                            if(a1_cc>0xFF4Fu-step) a1_cc=0xFF4Fu;
                            else a1_cc=(unsigned int)(a1_cc+step);
                            HW_A1SetCompare(a1_cc);
                            }
                        }
                        else if(a1_raw>raw_target+2u && a1_cc>0xFBFFu)
                        {
                            a1_up_confirm=0u;
                            diff=a1_raw-raw_target;
                            if(diff>100u) step=8u;
                            else if(diff>40u) step=4u;
                            else if(diff>12u) step=2u;
                            if(a1_cc<0xFBFFu+step) a1_cc=0xFBFFu;
                            else a1_cc=(unsigned int)(a1_cc-step);
                            HW_A1SetCompare(a1_cc);
                        }
                        else a1_up_confirm=0u;
                    }
                }
            }

            /* AUTO RES: A2-inspired 100ms multi-step ramp, 1 RAW
             * above setpoint. RAW RES calibration: mA = RAW*1140/1000.
             * Only RES mode; other modes remain unchanged.
             * Max 8 CC1 counts / 100ms when far from target; near
             * target single steps. Existing RAW=470 hard trip retained. */
            if(a1_running && a1_disch==1u && GUI_A1IsAuto() &&
               (unsigned int)(System_Ticks10ms()-a1_control_tick)>=10u)
            {
                unsigned int target=GUI_A1CurrentSet();
                unsigned int raw_target=(unsigned int)
                    (((unsigned long)target*1000UL+1139UL)/1140UL);
                unsigned int step=1u;
                a1_control_tick=System_Ticks10ms();
                ++raw_target;
                if(a1_raw+2u<raw_target && a1_cc<0xFF4Fu)
                {
                    if(a1_res_up_confirm<3u) ++a1_res_up_confirm;
                    if(a1_res_up_confirm>=3u)
                    {
                    unsigned int diff=raw_target-a1_raw;
                    if(diff>100u) step=8u;
                    else if(diff>40u) step=4u;
                    else if(diff>12u) step=2u;
                    if(a1_cc>0xFF4Fu-step) a1_cc=0xFF4Fu;
                    else a1_cc=(unsigned int)(a1_cc+step);
                    HW_A1SetCompare(a1_cc);
                    }
                }
                else if(a1_raw>raw_target+2u && a1_cc>0xFBFFu)
                {
                    unsigned int diff=a1_raw-raw_target;
                    a1_res_up_confirm=0u;
                    if(diff>100u) step=8u;
                    else if(diff>40u) step=4u;
                    else if(diff>12u) step=2u;
                    else step=1u;
                    if(a1_cc<0xFBFFu+step) a1_cc=0xFBFFu;
                    else a1_cc=(unsigned int)(a1_cc-step);
                    HW_A1SetCompare(a1_cc);
                }
                else a1_res_up_confirm=0u;
            }
            GUI_A1BenchLive(a1_cc,a1_raw,a1_running,a1_fault);
            GUI_A1BenchTotals(a1_elapsed_ticks,a1_capacity_x100);
        }
        InternalFan_Task((unsigned char)(page==PAGE_HW_TEST),
                         (unsigned char)((measurement.valid_mask & (1u<<8))!=0u),
                         measurement.raw[8],System_Ticks10ms());

        /* Independent disconnect guard, ahead of CC/CV and LCD work.
         * Two confirming samples; true response time must be measured. */
        if((a2_running || a3_running) &&
           BatteryProtection_Task(System_Ticks10ms(),
             (signed char)t.copy[0].tol_a3,(signed char)t.copy[0].ofs_a3))
        {
            if(a2_running)
            {
                A2PowerStop(&a2_running,6u);
                a2_rearmed=0u;
            }
            if(a3_running)
            {
                A3Stop(&a3_running);
                a3_status=6u;
                GUI_A3SetLive(a3_cc,a3_mah,a3_seconds,a3_status);
                a3_start_rearmed=0u;
            }
            Sound_BeepTriple();
        }
        /* Nonblocking 100 ms A2 controller. Timer subtraction tolerates wrap. */
        if(page==PAGE_A2_TEST && a2_running &&
           (unsigned int)(System_Ticks10ms()-a2_control_tick)>=10u)
        {
            unsigned int raw,now,dt,ma,target,cutoff,voltage;
            unsigned long power_mw;
            ADC_VOLTAGE_RESULT vr;
            now=System_Ticks10ms();
            dt=(unsigned int)(now-a2_control_tick);
            a2_control_tick=now;
            /* Do not discard elapsed ticks: doing so made TIME run too slowly. */
            raw=ADC_Measure(ADC_CH_A2_CURRENT);
            ADC_MeasureA2((signed char)t.copy[0].tol_a2,
                          (signed char)t.copy[0].ofs_a2,&vr);
            ma=(raw<=10u)?0u:(unsigned int)(((unsigned long)raw*1463UL)/1000UL+30UL);
            voltage=(vr.mv_cal>0L && vr.mv_cal<65535L)?(unsigned int)vr.mv_cal:0u;
            target=GUI_A2TargetCurrent();
            cutoff=target/20u;
            power_mw=((unsigned long)voltage*(unsigned long)ma)/1000UL;
            if(raw>A2_CHARGE_HARD_RAW || power_mw>95000UL ||
               (unsigned long)voltage >= (unsigned long)GUI_A2VoltageSet() + A2_OVERVOLT_MARGIN_MV || voltage==0u)
            {
                A2PowerStop(&a2_running,6u);
            }
            else
            {
                /* Capacity integration: mA * 10 ms, carry 360000 units/mAh. */
                a2_mah_remainder+=(unsigned long)ma*(unsigned long)dt;
                while(a2_mah_remainder>=360000UL)
                {
                    ++a2_mah;
                    a2_mah_remainder-=360000UL;
                }
                a2_subsecond=(unsigned int)(a2_subsecond+dt);
                while(a2_subsecond>=100u) {++a2_elapsed;a2_subsecond-=100u;}
                GUI_A2SetTotals(a2_mah,a2_elapsed);
                if(a2_elapsed>=(unsigned long)GUI_A2TimeSet()) A2PowerStop(&a2_running,4u);
                else if(a2_mah>=(unsigned long)GUI_A2CapacitySet()) A2PowerStop(&a2_running,5u);
                else
                {
                    if(!GUI_A2IsAuto())
                    {
                        /* MAN: user-controlled compare, still enforce voltage ceiling. */
                        if(voltage>=GUI_A2VoltageSet()) a2_cv=1u;
                        if(voltage+50u>=GUI_A2VoltageSet() &&
                           a2_compare>A2_CC_IDLE)
                        {
                            unsigned int reduction=
                                voltage>=GUI_A2VoltageSet()?24u:4u;
                            a2_compare=(a2_compare>A2_CC_IDLE+reduction)?
                                (unsigned int)(a2_compare-reduction):A2_CC_IDLE;
                            GUI_A2SetStatus(2u);
                        }
                        else
                        {
                            /* Do not automatically restore current after CV reduction. */
                            GUI_A2SetStatus(1u);
                        }
                        if(a2_compare<a2_manual_cc) a2_manual_cc=a2_compare;
                    }
                    else
                    {
                    if(voltage>=GUI_A2VoltageSet()) a2_cv=1u;
                    if(a2_cv)
                    {
                        GUI_A2SetStatus(2u);
                        if(voltage>=GUI_A2VoltageSet() && a2_compare>A2_CC_IDLE)
                        {
                            unsigned int reduction=(voltage>GUI_A2VoltageSet()+20u)?24u:8u;
                            if(a2_compare>A2_CC_IDLE+reduction) a2_compare-=reduction;
                            else a2_compare=A2_CC_IDLE;
                        }
                        else if(voltage+30u<GUI_A2VoltageSet() &&
                                ma+5u<target && a2_compare<A2_CC_MAX)
                            ++a2_compare;
                        if(ma<=cutoff && voltage+30u>=GUI_A2VoltageSet())
                            a2_cv_low_ticks=(unsigned int)(a2_cv_low_ticks+dt);
                        else a2_cv_low_ticks=0u;
                        if(a2_cv_low_ticks>=1000u) A2PowerStop(&a2_running,3u);
                    }
                    else
                    {
                        GUI_A2SetStatus(1u);
                        if(ma<target && a2_compare<A2_CC_MAX)
                        {
                            unsigned int step=CC_GetRampStep(ma,target);
                            if(a2_compare>A2_CC_MAX-step) a2_compare=A2_CC_MAX;
                            else a2_compare=(unsigned int)(a2_compare+step);
                        }
                        else if(ma>target+10u && a2_compare>A2_CC_IDLE)
                        {
                            unsigned int step=(ma>target+100u)?24u:4u;
                            if(a2_compare<A2_CC_IDLE+step) a2_compare=A2_CC_IDLE;
                            else a2_compare=(unsigned int)(a2_compare-step);
                        }
                    }
                    } /* AUTO branch */
                    if(a2_running)
                    {
                        HW_A2SetCompare(a2_compare);
                        GUI_A2SetCompare(a2_compare);
                    }
                }
            }
            if(!a2_running) { a2_rearmed=0u; Sound_BeepTriple(); }
        }
        /* A3 charge feedback; ADC11 scale 0.700mA/RAW remains provisional. */
        if(page==PAGE_MEASUREMENTS && a3_running &&
           !GUI_A3GetModeDischarge() &&
           (unsigned int)(System_Ticks10ms()-a3_tick)>=10u)
        {
            unsigned int now=System_Ticks10ms(),dt=(unsigned int)(now-a3_tick);
            unsigned int raw=ADC_MeasureA3Current(),ma,voltage,target,limit;
            ADC_VOLTAGE_RESULT vr;
            a3_tick=now;
            ADC_MeasureA3((signed char)t.copy[0].tol_a3,
                          (signed char)t.copy[0].ofs_a3,&vr);
            ma=(raw<=a3_idle_raw)?0u:
                (unsigned int)(((unsigned long)(raw-a3_idle_raw)*700UL)/1000UL);
            voltage=(vr.mv_cal>0L && vr.mv_cal<65535L)?(unsigned int)vr.mv_cal:0u;
            target=GUI_A3GetTargetCurrent();if(target>1500u)target=1500u;
            limit=GUI_A3GetVoltageLimit();
            if(raw>=A3_CHARGE_HARD_RAW || voltage>8450u || voltage<1000u)
                a3_status=6u;
            else
            {
                a3_rem+=(unsigned long)ma*dt;
                while(a3_rem>=360000UL) {++a3_mah;a3_rem-=360000UL;}
                a3_subsecond=(unsigned int)(a3_subsecond+dt);
                while(a3_subsecond>=100u) {++a3_seconds;a3_subsecond-=100u;}
                if(a3_seconds>=GUI_A3TimeSet()) a3_status=4u;
                else if(a3_mah>=GUI_A3CapacitySet()) a3_status=5u;
                else
                {
                    if(!GUI_A3IsAuto())
                    {
                        /* MAN: +/- writes the actual CC3 register directly.
                         * No second target variable and no automatic ramp.
                         * Preserve voltage/current/time/capacity protection. */
                        a3_status=1u;
                        if(voltage>=limit)
                        {
                            a3_cc=A3_CC_IDLE;
                            a3_status=2u;
                        }
                    }
                    else
                    {
                    if(voltage>=limit) a3_cv=1u;
                    if(a3_cv)
                    {
                        a3_status=2u;
                        if(voltage>=limit && a3_cc>A3_CC_IDLE)
                        {
                            unsigned int step=voltage>limit+20u?8u:2u;
                            a3_cc=a3_cc>A3_CC_IDLE+step?a3_cc-step:A3_CC_IDLE;
                        }
                        else if(voltage+15u<limit && ma+5u<target && a3_cc<A3_CC_MAX) ++a3_cc;
                        if(ma<=target/20u && voltage+15u>=limit) a3_cv_ticks=(unsigned int)(a3_cv_ticks+dt);
                        else a3_cv_ticks=0u;
                        if(a3_cv_ticks>=1000u) a3_status=3u;
                    }
                    else
                    {
                        a3_status=1u;
                        if(ma<target && a3_cc<A3_CC_MAX)
                        {
                            unsigned int step=CC_GetRampStep(ma,target);
                            a3_cc=a3_cc>A3_CC_MAX-step?A3_CC_MAX:a3_cc+step;
                        }
                        else if(ma>target+10u && a3_cc>A3_CC_IDLE)
                        {
                            unsigned int step=ma>target+50u?16u:4u;
                            a3_cc=a3_cc<A3_CC_IDLE+step?A3_CC_IDLE:a3_cc-step;
                        }
                    }
                    } /* AUTO branch */
                    if(a3_status==1u || a3_status==2u) HW_A3SetCompare(a3_cc);
                }
            }
            if(a3_status>=3u) {A3Stop(&a3_running);Sound_BeepTriple();}
            GUI_A3SetLive(a3_cc,a3_mah,a3_seconds,a3_status);
        }
        /* V1.60 A3 discharge: continuous 2.5W target limiting, not FAULT.
         * Current is NOT measured; the displayed current and QUANT use the
         * estimate from the currently applied CC3 (including ramp). */
        if(page==PAGE_MEASUREMENTS && a3_running &&
           GUI_A3GetModeDischarge() &&
           (unsigned int)(System_Ticks10ms()-a3_tick)>=10u)
        {
            unsigned int now=System_Ticks10ms();
            unsigned int dt=(unsigned int)(now-a3_tick);
            unsigned int target_ma=GUI_A3GetTargetCurrent();
            unsigned int effective_ma,target_cc,voltage,estimate_ma;
            unsigned long y;
            ADC_VOLTAGE_RESULT vr;
            a3_tick=now;
            ADC_MeasureA3((signed char)t.copy[0].tol_a3,
                          (signed char)t.copy[0].ofs_a3,&vr);
            voltage=(vr.mv_cal>0L && vr.mv_cal<65535L)?(unsigned int)vr.mv_cal:0u;
            if(voltage==0u || voltage>=9000u ||
               voltage<=GUI_A3GetVoltageLimit())
                a3_status=6u;
            else
            {
                unsigned int power_ma=(unsigned int)(2500000UL/(unsigned long)voltage);
                if(target_ma>400u) target_ma=400u;
                effective_ma=target_ma<power_ma?target_ma:power_ma;
                /* Conservative downward rounding for the power limit:
                 * Y=floor((I+83)*10420/65536). */
                y=((unsigned long)(effective_ma+83u)*10420UL)>>16;
                target_cc=(unsigned int)(A3_CC_IDLE+(unsigned int)y);
                if(target_cc>A3_CC_MAX) target_cc=A3_CC_MAX;
                /* MAN: +/- controls CC3 directly.  The periodic loop may
                 * only reduce it to enforce the 2.5W/current limit. */
                if(!GUI_A3IsAuto())
                {
                    if(a3_cc>target_cc)
                    {
                        a3_cc=target_cc;
                        HW_A3SetCompare(a3_cc);
                    }
                    target_cc=a3_cc;
                }
                if(a3_cc<target_cc)
                {
                    unsigned int step=(target_cc-a3_cc)>8u?8u:1u;
                    a3_cc=(a3_cc+step>target_cc)?target_cc:(unsigned int)(a3_cc+step);
                    HW_A3SetCompare(a3_cc);
                }
                else if(a3_cc>target_cc)
                {
                    /* Reduce immediately when voltage rises / power limit falls. */
                    a3_cc=target_cc;
                    HW_A3SetCompare(a3_cc);
                }
                estimate_ma=GUI_A3DischargeEstimatedMa(a3_cc);
                /* CC3 is quantised: don't report more than the power-limited
                 * command, especially near the 2.5W boundary. */
                if(estimate_ma>effective_ma) estimate_ma=effective_ma;
                a3_rem+=(unsigned long)estimate_ma*dt;
                while(a3_rem>=360000UL) {++a3_mah;a3_rem-=360000UL;}
                a3_subsecond=(unsigned int)(a3_subsecond+dt);
                while(a3_subsecond>=100u) {++a3_seconds;a3_subsecond-=100u;}
                if(a3_seconds>=GUI_A3TimeSet()) a3_status=4u;
                else if(a3_mah>=GUI_A3CapacitySet()) a3_status=5u;
                else a3_status=1u;
            }
            if(a3_status>=3u) {A3Stop(&a3_running);Sound_BeepTriple();}
            GUI_A3SetLive(a3_cc,a3_mah,a3_seconds,a3_status);
        }
        /* A2 RS232 logging is intentionally separate from control cadence. */
        if(page==PAGE_A2_TEST &&
           (unsigned int)(System_Ticks10ms()-a2_last_serial_tick)>=100u)
        {
            a2_last_serial_tick=System_Ticks10ms();
            Serial_Print("A2 CC2=");A2SerialU16(GUI_A2GetCompare());
            Serial_Print(" I_SET=");A2SerialU16(GUI_A2TargetCurrent());
            Serial_Print(" Q_mAh=");A2SerialUL(a2_mah);
            Serial_Print(" T_s=");A2SerialUL(a2_elapsed);
            Serial_Print("\r\n");
        }
        /* STOP/ESC has priority over ADC work while A3 is energized. */
        key=Keyboard_GetKey();
        if(page==PAGE_MEASUREMENTS && a3_running && (key==KEY_F2 || key==KEY_ESC))
        {
            A3Stop(&a3_running);
            Sound_BeepShort();
            if(key==KEY_ESC) page=PAGE_HARDWARE;
            GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            last_key=key;
            continue;
        }

        /* Continuous current guard while active.  ADC11 is already filtered
         * by the reconstructed original 9C23/77D3 measurement path. */
        if(page==PAGE_MEASUREMENTS && a3_running && !GUI_A3GetModeDischarge())
        {
            unsigned int guard_i=ADC_MeasureA3Current();
            if(guard_i>=A3_CHARGE_HARD_RAW)
            {
                A3Stop(&a3_running);
                Sound_BeepTriple();
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
        }

        /* Live RTC + live measurement refresh once per second. */
        if((Timekeeper_ReadByte(0x9FF9u)&0x7Fu)!=last_second)
        {
            Timekeeper_ReadInfo(&t);
            last_second=t.second_bcd;

            if(page==PAGE_MEASUREMENTS && a3_running)
            {
                ADC_VOLTAGE_RESULT vr;
                unsigned int im=ADC_MeasureA3Current();
                unsigned int vlim=GUI_A3GetVoltageLimit();
                unsigned char stop=0u;

                ADC_MeasureA3((signed char)t.copy[0].tol_a3,(signed char)t.copy[0].ofs_a3,&vr);
                /* ADC11/P8.3 measures A3 charge current only. */
                if(!GUI_A3GetModeDischarge() && im>=A3_CHARGE_HARD_RAW) stop=1u;
                else if(!GUI_A3GetModeDischarge() && vr.mv_cal>8450L) stop=1u;
                else if(GUI_A3GetModeDischarge() && vr.mv_cal>0L && vr.mv_cal<=(signed long)vlim) stop=1u;
                else if(vr.mv_cal>8450L) stop=1u;
                /* A3 DIS power is continuously limited by CC3, not FAULT. */

                if(stop)
                {
                    A3Stop(&a3_running);
                    Sound_BeepTriple();
                }

            }
            if(page!=PAGE_A2_TEST) GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
        }

        if(page==PAGE_A2_TEST &&
           (unsigned int)(System_Ticks10ms()-a2_lcd_tick)>=50u)
        {
            a2_lcd_tick=System_Ticks10ms();
            GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
        }
                if(page!=PAGE_A2_TEST) key=Keyboard_GetKey();
        if(page==PAGE_A1_TEST && !a1_running && !a1_fault && key==KEY_NONE) a1_rearm=1u;
        if(page==PAGE_A2_TEST && !a2_running && key==KEY_NONE) a2_rearmed=1u;
        /* A new A3 START is accepted only after all keys have been physically
           released since the previous START/STOP. This removes the old one-shot
           behaviour without allowing an unintended restart from a held key. */
        if(page==PAGE_MEASUREMENTS && !a3_running && key==KEY_NONE) a3_start_rearmed=1u;

        /* Timer0-based key repeat, independent of RTC seconds.
         * First press remains handled by the original key dispatch below.
         * Only TIME/CALIB PLUS/MINUS repeat; START/STOP never repeat. */
        if((page==PAGE_CALIB || page==PAGE_TIME) &&
           (key==KEY_PLUS || key==KEY_MINUS) && key==last_key && repeat_armed)
        {
            unsigned int now=System_Ticks10ms();
            if((signed int)(now-repeat_deadline)>=0)
            {
                repeat_deadline=(unsigned int)(now+15u);
                if(page==PAGE_CALIB)
                    CalibAdjust(&t,calib_channel,calib_edit,(key==KEY_PLUS)?1:-1);
                else
                    RtcAdjust(&t,time_field,(key==KEY_PLUS)?1:-1);
                Sound_BeepShort();
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
        }
        if(key!=last_key)
        {
            repeat_armed=(unsigned char)((key==KEY_PLUS || key==KEY_MINUS) &&
                (page==PAGE_CALIB || page==PAGE_TIME));
            repeat_deadline=(unsigned int)(System_Ticks10ms()+50u);
        }

        if(page==PAGE_A1_TEST && !GUI_A1IsToggleEdit() && (key==KEY_PLUS || key==KEY_MINUS) &&
           (!a1_running || (GUI_A1IsAuto() && GUI_A1IsCurrentEdit())))
        {
            unsigned int rn=System_Ticks10ms();
            if(key!=a1_repeat_key)
            {
                a1_repeat_key=key;a1_repeat_start=rn;
                a1_repeat_due=(unsigned int)(rn+50u);
            }
            else if((unsigned int)(rn-a1_repeat_start)>=50u &&
                    (signed int)(rn-a1_repeat_due)>=0)
            {
                GUI_A1Adjust((key==KEY_PLUS)?1:-1);
                a1_repeat_due=(unsigned int)(rn+10u);
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
        }
        else a1_repeat_key=KEY_NONE;
        /* A1/A2: nonblocking PLUS/MINUS repeat after 500 ms, every 100 ms.
         * Only the first press beeps. Repeated actions pass through the
         * same key-dispatch paths as ordinary presses (no duplicate logic). */
        a12_repeat_fire=0u;
        if((page==PAGE_A1_TEST || page==PAGE_A2_TEST) &&
           (key==KEY_PLUS || key==KEY_MINUS))
        {
            unsigned int rn=System_Ticks10ms();
            if(key!=a12_repeat_key)
            {
                a12_repeat_key=key;
                a12_repeat_start=rn;
                a12_repeat_due=(unsigned int)(rn+50u);
            }
            else if((unsigned int)(rn-a12_repeat_start)>=50u &&
                    (signed int)(rn-a12_repeat_due)>=0)
            {
                a12_repeat_fire=1u;
                a12_repeat_due=(unsigned int)(rn+10u);
            }
        }
        else a12_repeat_key=KEY_NONE;
        /* The original A3 handler ran only on key transitions. Holding +
         * therefore changed CC3 once, unlike A2 MAN. Generate repeat events
         * without blocking the 100ms protection/control loop. */
        a3_repeat_fire=0u;
        if(page==PAGE_MEASUREMENTS && a3_running && !GUI_A3IsAuto() &&
           (key==KEY_PLUS || key==KEY_MINUS))
        {
            unsigned int rn=System_Ticks10ms();
            if(key!=a3_repeat_key)
            {
                a3_repeat_key=key;
                a3_repeat_start=rn;
                a3_repeat_due=(unsigned int)(rn+30u);
            }
            else if((unsigned int)(rn-a3_repeat_start)>=30u &&
                    (signed int)(rn-a3_repeat_due)>=0)
            {
                a3_repeat_fire=1u;
                a3_repeat_due=(unsigned int)(rn+10u);
            }
        }
        else a3_repeat_key=KEY_NONE;
        if(key!=KEY_NONE && (key!=last_key || a3_repeat_fire || a12_repeat_fire))
        {
            if(page==PAGE_STATUS)
            {
                if(key==KEY_F1)
                {
                    page=PAGE_CALIB;
                    calib_channel=0u;
                    calib_edit=0u;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F2)
                {
                    page=PAGE_USER;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F3)
                {
                    page=PAGE_TIME;
                    time_field=TMK_RTC_HOUR;
                    Timekeeper_ReadInfo(&t);
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F4)
                {
                    /* EEPROM MENU STEP 1:
                     * keep the hardware-proven single WORD06 read unchanged. */
                    {
                        /* Immediate progress screen before the complete EEPROM read. */
                        LCD_ClearBuffer();
                        LCD_Print(54u,22u,"WAITING....");
                        LCD_Print(42u,34u,"READ EEPROM");
                        LCD_Refresh();

                        EepromReadAll();
                    }
                    g_eeprom_word0_valid = 1u;
                    g_eeprom_sync_status=0u;
                    page=PAGE_EEPROM;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F5)
                {
                    page=PAGE_HARDWARE;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
            }
            else if(page==PAGE_EEPROM)
            {
                if(key==KEY_ESC || key==KEY_F5)
                {
                    page=PAGE_STATUS;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F1)
                {
                    GUI_EepromDumpFirst();
                    page=PAGE_EEPROM_DUMP;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F2)
                {
                    g_eeprom_sn_edit=EepromLogical(g_eeprom_words[0x06u]);
                    g_eeprom_sn_digit=0u;
                    g_eeprom_sn_write_status=0u;
                    page=PAGE_EEPROM_SN;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F3)
                {
                    g_eeprom_sync_status=EepromSyncSerialToTimekeeper(&t)?1u:2u;
                    if(g_eeprom_sync_status==1u) Sound_BeepTriple(); else Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_F4)
                {
                    g_eeprom_erase_status=0u;
                    page=PAGE_EEPROM_ERASE;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
            }
            else if(page==PAGE_EEPROM_ERASE)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_EEPROM;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
                else if(key==KEY_ENTER && g_eeprom_erase_status==0u)
                {
                    g_eeprom_erase_status=EepromEraseAll()?1u:2u;
                    if(g_eeprom_erase_status==1u) Sound_BeepTriple(); else Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
            }
            else if(page==PAGE_EEPROM_SN)
            {
                if(key==KEY_ESC)
                {
                    if(g_eeprom_sn_write_status==1u)
                    {
                        g_eeprom_sn_write_status=0u;
                        Sound_BeepShort();
                    }
                    else
                    {
                        page=PAGE_EEPROM;
                        Sound_BeepShort();
                    }
                }
                else if(key==KEY_F2)
                {
                    g_eeprom_sn_write_status=0u;
                    g_eeprom_sn_digit++;
                    if(g_eeprom_sn_digit>=5u) g_eeprom_sn_digit=0u;
                    Sound_BeepShort();
                }
                else if(key==KEY_PLUS) { g_eeprom_sn_write_status=0u; SnAdjustDigit(1); Sound_BeepShort(); }
                else if(key==KEY_MINUS) { g_eeprom_sn_write_status=0u; SnAdjustDigit(-1); Sound_BeepShort(); }
                else if(key==KEY_ENTER)
                {
                    if(g_eeprom_sn_write_status!=1u)
                    {
                        g_eeprom_sn_write_status=1u;
                        Sound_BeepShort();
                    }
                    else
                    {
                        g_eeprom_sn_write_status=EepromWriteSerial(&t,g_eeprom_sn_edit)?2u:3u;
                        if(g_eeprom_sn_write_status==2u) Sound_BeepTriple(); else Sound_BeepShort();
                    }
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_EEPROM_DUMP)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_EEPROM;
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
else if(key==KEY_MINUS)
                {
                    GUI_EepromDumpPrev();
                    Sound_BeepShort();
                    GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
                }
            }
            else if(page==PAGE_CALIB)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_STATUS;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1) { calib_channel=0u; calib_edit=0u; Sound_BeepShort(); }
                else if(key==KEY_F2) { calib_channel=1u; calib_edit=0u; Sound_BeepShort(); }
                else if(key==KEY_F3) { calib_channel=2u; calib_edit=0u; Sound_BeepShort(); }
                else if(key==KEY_F4) { calib_channel=3u; calib_edit=0u; Sound_BeepShort(); }
                else if(key==KEY_ENTER && calib_channel<3u)
                {
                    calib_edit=(calib_edit==0u)?1u:0u;
                    Sound_BeepShort();
                }
                else if(key==KEY_PLUS)
                {
                    CalibAdjust(&t,calib_channel,calib_edit,1);
                    repeat_deadline=(unsigned int)(System_Ticks10ms()+50u);
                    Sound_BeepShort();
                }
                else if(key==KEY_MINUS)
                {
                    CalibAdjust(&t,calib_channel,calib_edit,-1);
                    repeat_deadline=(unsigned int)(System_Ticks10ms()+50u);
                    Sound_BeepShort();
                }
                else if(key==KEY_F5)
                {
                    CalibClear(&t,calib_channel);
                    Sound_BeepShort();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_TIME)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_STATUS;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1)
                {
                    if(time_field==TMK_RTC_HOUR) time_field=TMK_RTC_MINUTE;
                    else if(time_field==TMK_RTC_MINUTE) time_field=TMK_RTC_SECOND;
                    else time_field=TMK_RTC_HOUR;
                    Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    time_field=(time_field==TMK_RTC_DAY)?TMK_RTC_MONTH:TMK_RTC_DAY;
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    time_field=(time_field==TMK_RTC_YEAR)?TMK_RTC_WEEKDAY:TMK_RTC_YEAR;
                    Sound_BeepShort();
                }
                else if(key==KEY_F5)
                {
                    if(t.rtc_running) Timekeeper_RtcStop(); else Timekeeper_RtcStart();
                    Timekeeper_ReadInfo(&t);
                    last_second=t.second_bcd;
                    Sound_BeepShort();
                }
                else if(key==KEY_PLUS) { RtcAdjust(&t,time_field,1); repeat_deadline=(unsigned int)(System_Ticks10ms()+50u); Sound_BeepShort(); }
                else if(key==KEY_MINUS) { RtcAdjust(&t,time_field,-1); repeat_deadline=(unsigned int)(System_Ticks10ms()+50u); Sound_BeepShort(); }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_USER)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_STATUS;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1)
                {
                    Timekeeper_ResetOwner();
                    Timekeeper_ReadInfo(&t);
                    Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    Timekeeper_ResetPassword();
                    Timekeeper_ReadInfo(&t);
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    g_timekeeper_erase_status=0u;
                    page=PAGE_USER_ERASE;
                    Sound_BeepShort();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_USER_ERASE)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_USER;
                    Sound_BeepShort();
                }
                else if(key==KEY_ENTER && g_timekeeper_erase_status==0u)
                {
                    g_timekeeper_erase_status=Timekeeper_Erase55()?1u:2u;
                    Sound_BeepTriple();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_HARDWARE)
            {
                if(key==KEY_ESC)
                {
                    page=PAGE_STATUS;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1)
                {
                    /* Enter TEST HW in a defined safe state. */
                    HW_TestAllOff();
                    hw_cursor=HWTEST_FAN_INT;
                    page=PAGE_HW_TEST;
                    Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    HW_TestAllOff();
                    HW_A1AllOff();HW_A1PWMInit();
                    a1_cc=0xFBFFu;a1_running=0u;a1_fault=0u;a1_rearm=1u;a1_disch=0u;
                    a1_elapsed_ticks=0UL;a1_capacity_x100=0UL;
                            a1_recup_cv_cap=5000u;a1_recup_low_ticks=0u;
                            a1_recup_cv_seen=0u;
                    GUI_A1BenchMode(a1_disch);
                    a1_raw=ADC_Measure(ADC_CH_A1_CURRENT);
                    GUI_A1BenchLive(a1_cc,a1_raw,0u,0u);
                    page=PAGE_A1_TEST;
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    HW_TestAllOff();
                    HW_A2AllOff();
                    HW_A2PWMInit();
                    HW_A2SetCompare(GUI_A2GetCompare());
                    a2_running=0u;
                    a2_rearmed=1u;
                    GUI_A2SetStatus(0u);
                    GUI_A2SetTotals(0UL,0UL);
                    GUI_A2ResetPWM();
                    GUI_A2SetRunning(0u);
                    HW_A2SetCompare(0xFBFFu);
                    page=PAGE_A2_TEST;
                    Sound_BeepShort();
                }
                else if(key==KEY_F4)
                {
                    /* Enter A3 test only from a defined safe power state. */
                    HW_TestAllOff();
                    HW_A3AllOff();
                    /* V1.34: initialize Timer2/CC3 as soon as the A3 page is
                     * entered, while both A3 power paths are safely HIGH/OFF.
                     * This gives the compare output time to reach a defined
                     * state before the first START after reset. */
                    HW_A3PWMInit();
                    A3ArmDelay();
                    a3_running=0u;
                    GUI_A3SetRunning(0u);
                    page=PAGE_MEASUREMENTS;
                    Sound_BeepShort();
                }
                else if(key==KEY_F5)
                {
                    /* Informational CHARGE page; never enable outputs. */
                    page=PAGE_CHARGE;
                    Sound_BeepShort();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_CHARGE)
            {
                if(key==KEY_ESC) {page=PAGE_HARDWARE;Sound_BeepShort();}
                else if(key==KEY_F1) {GUI_ChargeSelect();Sound_BeepShort();}
                else if(key==KEY_F2) {GUI_ChargeEdit();Sound_BeepShort();}
                else if(key==KEY_F3) {GUI_ChargeStartDemo();Sound_BeepShort();}
                else if(key==KEY_F4) {GUI_ChargeStop();Sound_BeepShort();}
                else if(key==KEY_F5) {GUI_ChargeMode();Sound_BeepShort();}
                else if(key==KEY_PLUS) {GUI_ChargeAdjust(1);Sound_BeepShort();}
                else if(key==KEY_MINUS) {GUI_ChargeAdjust(-1);Sound_BeepShort();}
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_A1_TEST)
            {
                if(key==KEY_ESC)
                {
                    HW_A1AllOff();
                    a1_running=0u;page=PAGE_HARDWARE;Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    HW_A1AllOff();a1_running=0u;a1_rearm=0u;a1_fault=0u;
                    a1_cc=0xFBFFu;a1_up_confirm=0u;a1_res_up_confirm=0u;
                    a1_recup_cv_cap=5000u;a1_recup_low_ticks=0u;
                    a1_recup_cv_seen=0u;a1_cv_active=0u;a1_charge_cv_ticks=0u;
                    Sound_BeepShort();
                }
                else if(key==KEY_F4 && !a1_running)
                {
                    HW_A1AllOff();
                    a1_disch=(unsigned char)(a1_disch==0u?1u:0u);
                    a1_cc=0xFBFFu;a1_fault=0u;a1_rearm=0u;
                    a1_up_confirm=0u;a1_res_up_confirm=0u;a1_cv_active=0u;
                    GUI_A1ModeNext();GUI_A1BenchMode(a1_disch);
                    /* Preserve AUTO/MAN selection when changing direction. */
                    Sound_BeepShort();
                }
                else if(key==KEY_F1 && !a1_running && a1_rearm && !a1_fault)
                {
                    /* Refuse start if idle ADC current is already near limit. */
                    a1_raw=ADC_Measure(ADC_CH_A1_CURRENT);
                    if(a1_raw>=80u) {a1_fault=1u;a1_rearm=0u;Sound_BeepTriple();}
                    else
                    {
                        unsigned char safe=1u;
                        /* AUTO selection for DISCHARGE at each START:
                         * RECUP only if enabled AND A1/BATT voltage valid.
                         * Otherwise use RES. No live branch switching. */
                        if(a1_disch)
                        {
                            ADC_VOLTAGE_RESULT vr;
                            ADC_MeasureA1((signed char)t.copy[0].tol_a1,
                                          (signed char)t.copy[0].ofs_a1,&vr);
                            {
                                ADC_BATT_RESULT batt;
                                unsigned char eligible=0u;
                                ADC_MeasureBattPlus((signed char)t.batt_tol[0],&batt);
                                if(GUI_A1RecupEnabled() &&
                                   vr.mv_cal>=18000L && vr.mv_cal<=24000L &&
                                   batt.mv>=11500u &&
                                   batt.mv<GUI_A1RecupBattLimit() && batt.mv<14700u)
                                    eligible=1u;
                                a1_disch=eligible?2u:1u;
                                GUI_A1SelectDischargePath(a1_disch);
                                GUI_A1BenchMode(a1_disch);
                            }
                            if(vr.mv_cal<=0L || vr.mv_cal>60000L ||
                               vr.mv_cal<=(signed long)GUI_A1VoltageSet()) safe=0u;
                        }
                        if(!safe) {a1_fault=1u;a1_rearm=0u;Sound_BeepTriple();}
                        else
                        {
                            HW_A1AllOff();
                            a1_cc=0xFBFFu;
                            a1_recup_cv_cap=5000u;a1_recup_low_ticks=0u;
                            a1_recup_cv_seen=0u;a1_cv_active=0u;a1_charge_cv_ticks=0u;
                            a1_up_confirm=0u;a1_res_up_confirm=0u;
                            a1_elapsed_ticks=0UL;a1_capacity_x100=0UL;
                            HW_A1PWMInit();HW_A1SetCompare(a1_cc);
                            if(a1_disch==2u) HW_A1RecupEnable(1u);
                            else if(a1_disch) HW_A1DischResEnable(1u);
                            else HW_A1ChargeEnable(1u);
                            a1_start_tick=System_Ticks10ms();a1_control_tick=a1_start_tick;
                            a1_running=1u;Sound_BeepTriple();
                        }
                    }
                }
                else if(key==KEY_F3)
                {
                    GUI_A1EditNext();Sound_BeepShort();
                }
                else if(key==KEY_F5 && !a1_running)
                {
                    /* First AUTO release: RES only; other modes stay MAN. */
                    GUI_A1ToggleAuto();Sound_BeepShort();
                }
                else if(a1_running && !GUI_A1IsAuto() && key==KEY_MINUS && a1_cc>0xFBFFu)
                {
                    --a1_cc;HW_A1SetCompare(a1_cc);if(!a12_repeat_fire) Sound_BeepShort();
                }
                else if(a1_running && !GUI_A1IsAuto() && key==KEY_PLUS && a1_cc<0xFF4Fu)
                {
                    ++a1_cc;HW_A1SetCompare(a1_cc);if(!a12_repeat_fire) Sound_BeepShort();
                }
                else if((key==KEY_PLUS || key==KEY_MINUS) &&
                        (!a1_running ||
                         (GUI_A1IsAuto() && GUI_A1IsCurrentEdit())))
                {
                    /* While AUTO RES runs, only CURRENT may be edited.
                     * MAN keeps direct CC1 keys; all other live settings
                     * are locked to avoid changing protection limits. */
                    GUI_A1Adjust((key==KEY_PLUS)?1:-1);
                    if(!a12_repeat_fire) Sound_BeepShort();
                }
                GUI_A1BenchLive(a1_cc,a1_raw,a1_running,a1_fault);
            GUI_A1BenchTotals(a1_elapsed_ticks,a1_capacity_x100);
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_A2_TEST)
            {
                if(key==KEY_ESC)
                {
                    A2PowerStop(&a2_running,0u);
                    HW_A2AllOff();
                    page=PAGE_HARDWARE;
                    Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    A2PowerStop(&a2_running,0u);
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    GUI_A2EditNext();
                    Sound_BeepShort();
                }
                else if(key==KEY_F5 && !a2_running)
                {
                    GUI_A2ToggleAuto();Sound_BeepShort();
                }
                else if(key==KEY_PLUS || key==KEY_MINUS)
                {
                    if(a2_running && !GUI_A2IsAuto())
                    {
                        unsigned int ceiling=GUI_A2TargetCompare();
                        if(ceiling>A2_CC_MAX) ceiling=A2_CC_MAX;
                        if(key==KEY_PLUS && a2_compare<ceiling) ++a2_compare;
                        else if(key==KEY_MINUS && a2_compare>A2_CC_IDLE) --a2_compare;
                        a2_manual_cc=a2_compare;
                        HW_A2SetCompare(a2_compare);GUI_A2SetCompare(a2_compare);
                    }
                    else if(!a2_running || GUI_A2IsAuto())
                        GUI_A2Adjust((key==KEY_PLUS)?1:-1);
                    if(!a12_repeat_fire) Sound_BeepShort();
                }
                else if(key==KEY_F1 && !a2_running && a2_rearmed)
                {
                    ADC_VOLTAGE_RESULT v;
                    unsigned int raw;
                    a2_rearmed=0u;
                    /* Same pre-arm principle as A3: power OFF, initialize PWM,
                     * preload requested CC2, settle, then enable latch C3.
                     * F1 is a short press, guarded by key release/re-arm. */
                    ADC_MeasureA2((signed char)t.copy[0].tol_a2,
                                  (signed char)t.copy[0].ofs_a2,&v);
                    raw=ADC_Measure(ADC_CH_A2_CURRENT);
                    if(v.mv_cal>0L && v.mv_cal<GUI_A2VoltageSet() && raw<=10u)
                    {
                        HW_A2Enable(0u);
                        HW_A2PWMInit();
                        a2_compare=A2_CC_IDLE;
                        a2_manual_cc=A2_CC_IDLE;
                        a2_cv=0u;a2_cv_low_ticks=0u;
                        a2_elapsed=0UL;a2_mah=0UL;a2_mah_remainder=0UL;
                        a2_subsecond=0u;
                        GUI_A2SetTotals(0UL,0UL);
                        GUI_A2SetStatus(1u);
                        GUI_A2SetCompare(a2_compare);
                        HW_A2SetCompare(a2_compare);
                        A3ArmDelay();
                        a2_start_tick=System_Ticks10ms();
                        a2_control_tick=a2_start_tick;
                        BatteryProtection_Start(BP_A2_CHARGE,raw,System_Ticks10ms());
                        HW_A2Enable(1u);
                        a2_running=1u;
                        GUI_A2SetRunning(1u);
                        Sound_BeepTriple();
                    }
                    else Sound_BeepTriple();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_MEASUREMENTS)
            {
                if(key==KEY_ESC)
                {
                    A3Stop(&a3_running);
                    page=PAGE_HARDWARE;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1 && !a3_running && a3_start_rearmed)
                {
                    a3_start_rearmed=0u;
                    if(A3ConfirmStartHold())
                    {
                        /* Audible arming indication happens before power enable. */
                        Sound_BeepTriple();
                        /* Runtime START intentionally does NOT reset Timer2,
                         * CCEN or CC3 to FFFF.  First disconnect both paths,
                         * then preload the requested compare and only after a
                         * settling delay clear the selected active-low path. */
                        HW_A3PowerOffKeepPWM();
                        /* Both MAN and AUT use the same Timer2/CC3 startup.
                         * Keep both A3 power paths OFF until PWM is settled. */
                        HW_A3PWMInit();
                        HW_A3SetCompare(A3_CC_IDLE);
                        a3_status=1u;
                        if(!GUI_A3GetModeDischarge())
                        {
                            a3_idle_raw=ADC_MeasureA3Current();
                            GUI_A3SetChargeZeroRaw(a3_idle_raw);
                        }

                        if(GUI_A3GetModeDischarge())
                        {
                            /* V1.27 diagnostic DISCHARGE test.  FC00h is not a
                             * guessed current conversion: this is the exact compare
                             * value used in the first bench test where the physical
                             * discharge current settled near 120 mA.  ADC11 does not
                             * measure discharge current, so no current feedback is
                             * attempted here. */
                            a3_cc=A3_CC_IDLE;
                            HW_A3SetCompare(a3_cc);
                            /* ARM: CC3/Timer2 runs first while both A3 enables
                             * remain OFF.  Only after settling connect DISCHARGE. */
                            A3ArmDelay();
                            {
                                ADC_VOLTAGE_RESULT pre;
                                ADC_MeasureA3((signed char)t.copy[0].tol_a3,
                                  (signed char)t.copy[0].ofs_a3,&pre);
                                if(pre.mv_cal>=9000L || pre.mv_cal<1000L)
                                {
                                    a3_status=6u;
                                    HW_A3PowerOffKeepPWM();
                                }
                                else HW_A3SetDirection(1u);
                            }
                        }
                        else
                        {
                            /* Diagnostic CHARGE: use the manually selected compare while
                             * Timer2/CC3 is already running from the confirmed original
                             * zero-current idle state FBFFh. */
                            a3_cc=A3_CC_IDLE;
                            HW_A3SetCompare(a3_cc);
                            /* Same pre-arm sequence for CHARGE.  The diagnostic hard guard
                             * is RAW>=1400 (about 1 A from the present bench data)
                             * and is active immediately
                             * after the power direction is connected. */
                            A3ArmDelay();
                            HW_A3SetDirection(0u);
                        }
                        if(a3_status!=6u)
                        {
                            unsigned int idle=0u;
                            unsigned char mode=BP_A3_DISCH;
                            if(!GUI_A3GetModeDischarge())
                            {
                                /* Idle sample must be captured BEFORE A3 enable.
                                 * Capture above at startup for accurate baseline. */
                                idle=a3_idle_raw;
                                mode=BP_A3_CHARGE;
                            }
                            BatteryProtection_Start(mode,idle,System_Ticks10ms());
                        }
                        a3_running=(a3_status==6u)?0u:1u;
                        a3_tick=System_Ticks10ms();a3_subsecond=0u;
                        a3_mah=0UL;a3_rem=0UL;a3_seconds=0UL;
                        a3_cv=0u;a3_cv_ticks=0u;
                        GUI_A3SetLive(a3_cc,0UL,0UL,a3_status);
                        GUI_A3SetRunning(a3_running);
                    }
                }
                else if(key==KEY_F2)
                {
                    /* STOP: immediate break-before-make OFF, no hold required. */
                    A3Stop(&a3_running);
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    GUI_A3EditNext();
                    Sound_BeepShort();
                }
                else if(key==KEY_F4 && !a3_running)
                {
                    GUI_A3ToggleMode();
                    Sound_BeepShort();
                }
                else if(key==KEY_F5 && !a3_running)
                {
                    GUI_A3ToggleAuto();Sound_BeepShort();
                }
                else if((key==KEY_PLUS || key==KEY_MINUS) &&
                        1u)
                {
                    if(a3_running && !GUI_A3IsAuto())
                    {
                        /* Direct manual CC3 control, as in A2 MAN.
                         * One compare count per key event; hard protection
                         * remains active in the 100ms loop. */
                        unsigned int ceiling=A3_CC_MAX;
                        if(GUI_A3GetModeDischarge())
                        {
                            ADC_VOLTAGE_RESULT dis_v;
                            unsigned int voltage,power_ma,limit_cc;
                            unsigned long y;
                            ADC_MeasureA3((signed char)t.copy[0].tol_a3,
                                (signed char)t.copy[0].ofs_a3,&dis_v);
                            voltage=(dis_v.mv_cal>0L && dis_v.mv_cal<65535L)?
                                (unsigned int)dis_v.mv_cal:0u;
                            if(voltage>GUI_A3GetVoltageLimit() && voltage<9000u)
                            {
                                power_ma=(unsigned int)(2500000UL/(unsigned long)voltage);
                                if(power_ma>400u) power_ma=400u;
                                if(power_ma>GUI_A3GetTargetCurrent())
                                    power_ma=GUI_A3GetTargetCurrent();
                                y=((unsigned long)(power_ma+83u)*10420UL)>>16;
                                limit_cc=(unsigned int)(A3_CC_IDLE+(unsigned int)y);
                                if(limit_cc<ceiling) ceiling=limit_cc;
                            }
                            else ceiling=A3_CC_IDLE;
                        }
                        if(a3_cc>ceiling) a3_cc=ceiling;
                        if(key==KEY_PLUS && a3_cc<ceiling) ++a3_cc;
                        else if(key==KEY_MINUS && a3_cc>A3_CC_IDLE) --a3_cc;
                        HW_A3SetCompare(a3_cc);
                        GUI_A3SetLive(a3_cc,a3_mah,a3_seconds,a3_status);
                        Sound_BeepShort();
                    }
                    else if(!a3_running)
                        A3AutoRepeat(key,&t,hw_cursor,calib_channel,calib_edit,time_field,a3_running);
                    /* AUTO RUN keeps parameters locked. */
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(page==PAGE_HW_TEST)
            {
                if(key==KEY_ESC)
                {
                    HW_TestAllOff();
                    page=PAGE_HARDWARE;
                    Sound_BeepShort();
                }
                else if(key==KEY_F1)
                {
                    if(hw_cursor==0u) hw_cursor=(HWTEST_COUNT-1u);
                    else --hw_cursor;
                    Sound_BeepShort();
                }
                else if(key==KEY_F2)
                {
                    ++hw_cursor;
                    if(hw_cursor>=HWTEST_COUNT) hw_cursor=0u;
                    Sound_BeepShort();
                }
                else if(key==KEY_F3)
                {
                    if(hw_cursor==HWTEST_5V_A1 || hw_cursor==HWTEST_5V_A2)
                    Test5V_SetMode((unsigned char)(hw_cursor-HWTEST_5V_A1),TEST5V_ON);
                else HW_TestSet(hw_cursor,1u);
                    Sound_BeepShort();
                }
                else if(key==KEY_F4)
                {
                    if(hw_cursor==HWTEST_5V_A1 || hw_cursor==HWTEST_5V_A2)
                    Test5V_SetMode((unsigned char)(hw_cursor-HWTEST_5V_A1),TEST5V_OFF);
                else HW_TestSet(hw_cursor,0u);
                    Sound_BeepShort();
                }
                else if(key==KEY_F5)
                {
                    if(hw_cursor==HWTEST_5V_A1 || hw_cursor==HWTEST_5V_A2)
                        Test5V_SetMode((unsigned char)(hw_cursor-HWTEST_5V_A1),TEST5V_IDLE);
                    else HW_TestAllOff();
                    Sound_BeepShort();
                }
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
            else if(key==KEY_ESC)
            {
                page=PAGE_STATUS;
                Sound_BeepShort();
                GUI_DrawPage(page,&t,hw_cursor,calib_channel,calib_edit,time_field);
            }
        }

        /* Key-change event handling.
         * Do not re-sample here: a GUI/ADC redraw can be long enough for the
         * user to release one key and press another before the redraw ends.
         * Keeping the key sampled at the START of this loop means the next
         * different key is accepted immediately, even if KEY_NONE was missed. */
        last_key=key;
    }
}
