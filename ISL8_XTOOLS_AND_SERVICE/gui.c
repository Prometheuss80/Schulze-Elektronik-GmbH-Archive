#include "charge.h"
#include "test5v.h"
#include "measurement.h"
#include "internalfan.h"
#include <REG80C537_ISL8.H>
#include "lcd.h"
#include "lcd_bitmap.h"
#include "gui.h"
extern unsigned int g_eeprom_word06;
extern unsigned int xdata g_eeprom_words[64];
extern unsigned char g_eeprom_full_valid;
extern unsigned char g_eeprom_word0_valid;
extern unsigned int g_eeprom_sn_edit;
extern unsigned char g_eeprom_sn_digit;
extern unsigned char g_eeprom_sn_write_status;
extern unsigned char g_eeprom_sync_status;
extern unsigned char g_eeprom_erase_status;
extern unsigned char g_timekeeper_erase_status;

static unsigned char eeprom_dump_page=0u;
/* A3 service-test setup.  Power control is intentionally still disabled;
   these values prepare the user interface and the later safe ramp test. */
static unsigned char a3_mode_discharge=0u; /* 0=CHARGE, 1=DISCHARGE */
static unsigned char a3_edit_field=0u;     /* C/D:0..4 where 4=PWM */
static unsigned char a3_running=0u;
static unsigned char a3_auto=1u; /* AUTO default: preserve proven A3 discharge operation; F5 selects MAN */

/* Separate setup for each direction.  These are service-test values only. */
static unsigned int a3_charge_voltage_mv=8400u;
static unsigned int a3_charge_current_ma=1000u;
static unsigned int a3_charge_capacity_mah=2000u;
static unsigned int a3_charge_time_s=3600u;
static unsigned int a3_discharge_voltage_mv=6600u;
static unsigned int a3_discharge_current_ma=100u;
static unsigned int a3_discharge_capacity_mah=2000u;
static unsigned int a3_discharge_time_s=3600u;
static unsigned int a3_discharge_cc=0xFC00u; /* existing discharge bench control */
static unsigned int a3_charge_pwm_pct=5u;   /* V1.35 manual CHARGE LOW duty: 5..50 % */

/* A2: separate charge-only scope setup. No power enable until verified. */
static unsigned char a2_edit_field=0u; /* 0 TIME,1 QUANT,2 CURRENT,3 VOLT */
static unsigned int a2_voltage_mv=8400u;
static unsigned int a2_current_ma=2000u;
static unsigned int a2_capacity_mah=2000u;
static unsigned int a2_time_s=3600u;
static unsigned char a2_running_ui=0u;
static unsigned char a2_auto=0u; /* MAN default, AUTO selectable with F5 at STOP */
static unsigned int a2_pwm_pct=0u; /* 0 = A3-matched FBFF idle compare, not 0% duty */
/* Schulze current steps. Selection and compare preview are separate from
   the deliberately conservative, manually controlled power-test PWM. */
static const unsigned int code a2_steps_ma[]={150u,200u,250u,300u,400u,500u,600u,800u,1000u,1200u,1500u,2000u,2500u,3000u,3500u,4000u,4500u,5000u};
static const unsigned int code a3_charge_steps_ma[]={80u,100u,150u,200u,250u,300u,400u,500u,600u,800u,1000u,1200u,1500u};
static const unsigned int code a3_discharge_steps_ma[]={80u,100u,150u,200u,250u,300u,400u};
#define A2_MAX_MV 46475u
#define A3_MAX_MV 8450u
#define MIN_MV 750u
/* mW / mV => mA, with 32-bit intermediates */
#if 0 /* UNUSED L16: PowerCurrentLimit; retained for future work */
static unsigned int PowerCurrentLimit(unsigned long max_mw,unsigned int voltage_mv,unsigned int hard_ma)
{
    unsigned long v;
    if(voltage_mv==0u) return 0u;
    v=(max_mw*1000UL)/(unsigned long)voltage_mv;
    if(v>(unsigned long)hard_ma) v=hard_ma;
    return (unsigned int)v;
}
#endif /* UNUSED: PowerCurrentLimit */
static unsigned int SelectStep(unsigned int old,int delta,const unsigned int code *steps,unsigned char count,unsigned int limit)
{
    unsigned char i,idx=0u;
    for(i=0u;i<count;i++) if(steps[i]<=old) idx=i;
    if(delta>0 && idx+1u<count) ++idx;
    if(delta<0 && idx>0u) --idx;
    while(idx>0u && steps[idx]>limit) --idx;
    return steps[idx];
}
static unsigned int SchulzeCompare(unsigned int ma,unsigned int offset,unsigned int factor,unsigned int maximum)
{
    unsigned long x;
    if(ma<=offset) return 0xFBFFu;
    x=((unsigned long)(ma-offset)*(unsigned long)factor)/65536UL;
    if(x>1024UL) x=1024UL;
    if((unsigned int)(0xFBFFu+(unsigned int)x)>maximum) return maximum;
    return (unsigned int)(0xFBFFu+(unsigned int)x);
}
static unsigned char a2_status=0u; /* 0 STOP,1 CC,2 CV,3 END,4 TIME,5 QUANT,6 FAULT */
static unsigned long a2_delivered_mah=0UL;
static unsigned long a2_elapsed_s=0UL;
static unsigned int a2_live_compare=0xFBFFu;
void GUI_A2SetStatus(unsigned char st) { a2_status=st; }
void GUI_A2SetTotals(unsigned long mah,unsigned long sec) { a2_delivered_mah=mah; a2_elapsed_s=sec; }
void GUI_A2SetCompare(unsigned int cc) { a2_live_compare=cc; }
unsigned int GUI_A2TargetCurrent(void) { return a2_current_ma; }
unsigned int GUI_A2VoltageSet(void) { return a2_voltage_mv; }
unsigned int GUI_A2CapacitySet(void) { return a2_capacity_mah; }
unsigned int GUI_A2TimeSet(void) { return a2_time_s; }
unsigned int GUI_A2TargetCompare(void) { return SchulzeCompare(a2_current_ma,10u,11261u,0xFF60u); }
#if 0 /* UNUSED L16: GUI_A3TargetCompare; retained for future work */
unsigned int GUI_A3TargetCompare(void)
{
    if(a3_mode_discharge) {
        unsigned long x=((unsigned long)(a3_discharge_current_ma+83u)*10420UL)/65536UL;
        return (unsigned int)(0xFBFFu+(unsigned int)x);
    }
    return SchulzeCompare(a3_charge_current_ma,6u,38375u,0xFF69u);
}
#endif /* UNUSED: GUI_A3TargetCompare */

void GUI_A2SetRunning(unsigned char running) { a2_running_ui=running?1u:0u; }
unsigned char GUI_A2IsAuto(void) {return a2_auto;}
void GUI_A2ToggleAuto(void) {if(!a2_running_ui) a2_auto=(unsigned char)!a2_auto;}
unsigned char GUI_A3IsAuto(void) {return a3_auto;}
void GUI_A3ToggleAuto(void) {if(!a3_running) a3_auto=(unsigned char)!a3_auto;}
/* Display-only inverse of the quantized original compare formulas.
 * This is a CC reference, NOT an ADC measurement or calibrated current. */
static unsigned int CurrentFromCC(unsigned int cc,unsigned int offset,
                                  unsigned int factor,unsigned char subtract)
{
    unsigned long ma;
    if(cc<=0xFBFFu) return 0u;
    ma=(((unsigned long)(cc-0xFBFFu)*65536UL)+32768UL)/(unsigned long)factor;
    if(subtract) {if(ma<=offset) return 0u;ma-=offset;}
    else ma+=offset;
    return ma>65535UL?65535u:(unsigned int)ma;
}
static void Dec16(unsigned int v,char *s);
static void DrawModeOrCurrent(unsigned char running,unsigned char is_auto,
                              unsigned int cc,unsigned int offset,
                              unsigned int factor,unsigned char subtract)
{
    char b[8];
    unsigned int ma;
    if(is_auto) {LCD_PrintInverse(203u,55u,"AUT");return;}
    if(!running) {LCD_PrintInverse(203u,55u,"MAN");return;}
    ma=CurrentFromCC(cc,offset,factor,subtract);
    /* 5 digits * 6 pixels = 30 pixels; F5 window is 47 pixels. */
    Dec16(ma,b);
    LCD_PrintInverse(198u,55u,b);
}

#if 0 /* UNUSED L16: GUI_A2IsPwmEdit; retained for future work */
unsigned char GUI_A2IsPwmEdit(void) { return 0u; }
#endif /* UNUSED: GUI_A2IsPwmEdit */
#if 0 /* UNUSED L16: GUI_A2StartPWM; retained for future work */
void GUI_A2StartPWM(void) { a2_live_compare=0xFBFFu; }
#endif /* UNUSED: GUI_A2StartPWM */
void GUI_A2ResetPWM(void) { a2_live_compare=0xFBFFu; }
void GUI_A2EditNext(void) { a2_edit_field=(unsigned char)((a2_edit_field+1u)%4u); }
unsigned int GUI_A2GetCompare(void) { return a2_live_compare; }
#if 0 /* UNUSED L16: GUI_A2GetPwmPct; retained for future work */
unsigned int GUI_A2GetPwmPct(void) { return (unsigned int)(((unsigned long)(a2_live_compare-0xFBFFu)*100UL)/1024UL); }
#endif /* UNUSED: GUI_A2GetPwmPct */
void GUI_A2Adjust(int delta)
{
    unsigned int *v;
    unsigned int step,minv,maxv;
    if(a2_edit_field==0u) {v=&a2_time_s;step=60u;minv=60u;maxv=61200u;}
    else if(a2_edit_field==1u) {v=&a2_capacity_mah;step=100u;minv=100u;maxv=25000u;}
    else if(a2_edit_field==2u) {a2_current_ma=SelectStep(a2_current_ma,delta,a2_steps_ma,16u,5000u);return;}
    else {v=&a2_voltage_mv;step=100u;minv=7500u;maxv=A2_MAX_MV;}
    if(delta>0) {if(*v<=maxv-step)*v+=step;else *v=maxv;}
    else {if(*v>=minv+step)*v-=step;else *v=minv;}
}

void GUI_A3EditNext(void)
{
    a3_edit_field=(unsigned char)((a3_edit_field+1u)%4u);
}

static void A3AdjustUnsigned(unsigned int *v,int delta,unsigned int step,
                             unsigned int minv,unsigned int maxv)
{
    if(delta>0)
    {
        if(*v <= (unsigned int)(maxv-step)) *v=(unsigned int)(*v+step);
        else *v=maxv;
    }
    else
    {
        if(*v >= (unsigned int)(minv+step)) *v=(unsigned int)(*v-step);
        else *v=minv;
    }
}

void GUI_A3Adjust(int delta)
{
    unsigned int *v;

    if(a3_mode_discharge)
    {
        if(a3_edit_field==0u) { v=&a3_discharge_voltage_mv; A3AdjustUnsigned(v,delta,100u,MIN_MV,A3_MAX_MV); }
        else if(a3_edit_field==1u) { v=&a3_discharge_current_ma; a3_discharge_current_ma=SelectStep(a3_discharge_current_ma,delta,
            a3_discharge_steps_ma,6u,400u); }
        else if(a3_edit_field==2u) { v=&a3_discharge_capacity_mah; A3AdjustUnsigned(v,delta,25u,25u,25000u); }
        else if(a3_edit_field==3u) { v=&a3_discharge_time_s; A3AdjustUnsigned(v,delta,60u,60u,61200u); }
        else { A3AdjustUnsigned(&a3_discharge_cc,delta,0x10u,0xFC00u,0xFD00u); }

    }
    else
    {
        if(a3_edit_field==0u) { v=&a3_charge_voltage_mv; A3AdjustUnsigned(v,delta,100u,MIN_MV,A3_MAX_MV); }
        else if(a3_edit_field==1u) { v=&a3_charge_current_ma; a3_charge_current_ma=SelectStep(a3_charge_current_ma,delta,
            a3_charge_steps_ma,12u,1500u); }
        else if(a3_edit_field==2u) { v=&a3_charge_capacity_mah; A3AdjustUnsigned(v,delta,25u,25u,25000u); }
        else if(a3_edit_field==3u) { v=&a3_charge_time_s; A3AdjustUnsigned(v,delta,60u,60u,61200u); }
        else { A3AdjustUnsigned(&a3_charge_pwm_pct,delta,5u,5u,50u); }
    }
}

#if 0 /* UNUSED L16: GUI_A3SetNext; retained for future work */
void GUI_A3SetNext(void)
{
    /* Kept for the F4 soft key.  Editing itself is done with +/- now. */
}
#endif /* UNUSED: GUI_A3SetNext */

void GUI_A3ToggleMode(void)
{
    if(!a3_running) a3_mode_discharge=(unsigned char)!a3_mode_discharge;
}

unsigned char GUI_A3GetModeDischarge(void) { return a3_mode_discharge; }
unsigned int GUI_A3GetTargetCurrent(void)
{
    return a3_mode_discharge?a3_discharge_current_ma:a3_charge_current_ma;
}
unsigned int GUI_A3GetVoltageLimit(void)
{
    return a3_mode_discharge?a3_discharge_voltage_mv:a3_charge_voltage_mv;
}
unsigned char GUI_A3IsPwmEdit(void) { return 0u; }
unsigned int GUI_A3GetDischargeCompare(void)
{
    unsigned int ma=a3_discharge_current_ma;
    unsigned long y=((unsigned long)(ma+83u)*10420UL)>>16;
    return (unsigned int)(0xFBFFu+(unsigned int)y);
}
#if 0 /* UNUSED L16: GUI_A3GetChargePwmPercent; retained for future work */
unsigned int GUI_A3GetChargePwmPercent(void) { return a3_charge_pwm_pct; }
#endif /* UNUSED: GUI_A3GetChargePwmPercent */
unsigned int GUI_A3GetChargeCompare(void)
{
    /* Timer2 period is FBFF..FFFF = 0x400 = 1024 counts.
       LOW duty is the displacement above FBFF.  Integer division deliberately
       truncates, matching the manual test table (5%=51 counts, 50%=512). */
    unsigned long counts=((unsigned long)a3_charge_pwm_pct*1024UL)/100UL;
    return (unsigned int)(0xFBFFu+(unsigned int)counts);
}
/* V1.60: estimated discharge current from the *applied* CC3 value.
 * No A3 discharge-current ADC exists. Inverse of the verified mapping
 * Y=floor((I+83)*10420/65536). Quantisation means an estimate only.
 * Zero compare displacement is treated as zero during the ramp. */
unsigned int GUI_A3DischargeEstimatedMa(unsigned int cc)
{
    unsigned int delta,ma;
    unsigned long scaled;
    if(cc<=0xFBFFu) return 0u;
    delta=(unsigned int)(cc-0xFBFFu);
    /* Nearest centre of the integer-quantised CC3 interval. */
    scaled=(((unsigned long)delta*65536UL)+32768UL)/10420UL;
    if(scaled<=83UL) return 0u;
    scaled-=83UL;
    ma=scaled>400UL?400u:(unsigned int)scaled;
    return ma;
}
void GUI_A3SetRunning(unsigned char running) { a3_running=running?1u:0u; }
static unsigned int a3_live_cc=0xFBFFu;
/* A3 charge ADC zero offset captured with power output OFF. */
static unsigned int a3_charge_zero_raw=13u;
void GUI_A3SetChargeZeroRaw(unsigned int raw)
{
    a3_charge_zero_raw=raw;
}

static unsigned long a3_total_mah=0UL,a3_total_sec=0UL;
static unsigned char a3_status=0u;
void GUI_A3SetLive(unsigned int cc,unsigned long mah,unsigned long sec,unsigned char status)
{ a3_live_cc=cc;a3_total_mah=mah;a3_total_sec=sec;a3_status=status; }
unsigned int GUI_A3CapacitySet(void) {return a3_mode_discharge?a3_discharge_capacity_mah:a3_charge_capacity_mah;}
unsigned int GUI_A3TimeSet(void) {return a3_mode_discharge?a3_discharge_time_s:a3_charge_time_s;}



static char XTOOLS_HexNibble(unsigned char v)
{
    v &= 0x0Fu;
    return (v < 10u) ? (char)('0' + v) : (char)('A' + v - 10u);
}

static void XTOOLS_WordHex(char *s, unsigned int v)
{
    s[0]=XTOOLS_HexNibble((unsigned char)(v >> 12));
    s[1]=XTOOLS_HexNibble((unsigned char)(v >> 8));
    s[2]=XTOOLS_HexNibble((unsigned char)(v >> 4));
    s[3]=XTOOLS_HexNibble((unsigned char)v);
    s[4]='\0';
}

#include "timekeeper.h"
#include "adc.h"
#include "hardware.h"

static char HexDigit(unsigned char v){v&=0x0Fu;return(v<10u)?('0'+v):('A'+v-10u);}
static void Hex8(unsigned char v,char *s){s[0]=HexDigit(v>>4);s[1]=HexDigit(v);s[2]=0;}
#if 0 /* UNUSED L16: Dec8; retained for future work */
static void Dec8(unsigned char v,char *s)
{
    unsigned char n=0,i; char t[4];
    do{t[n++]=(char)('0'+v%10u);v/=10u;}while(v);
    for(i=0;i<n;i++)s[i]=t[n-1u-i];s[n]=0;
}
#endif /* UNUSED: Dec8 */
static void Dec16(unsigned int v,char *s)
{
    unsigned char n=0,i; char t[6];
    do{t[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<5u);
    for(i=0;i<n;i++)s[i]=t[n-1u-i];s[n]=0;
}

static void DecSL(signed long v,char *s)
{
    char t[12]; unsigned char n=0u,i=0u; unsigned long u;
    if(v<0L){s[i++]='-';u=(unsigned long)(-v);}
    else u=(unsigned long)v;
    do{t[n++]=(char)('0'+(u%10UL));u/=10UL;}while(u&&n<10u);
    while(n>0u)s[i++]=t[--n];
    s[i]=0;
}

static void FormatVolts3(signed long mv,char *s)
{
    unsigned long u;
    unsigned int whole,frac;
    unsigned char p=0u;
    char w[6];

    if(mv<0L) mv=0L;
    u=(unsigned long)mv;
    whole=(unsigned int)(u/1000UL);
    frac=(unsigned int)(u%1000UL);
    Dec16(whole,w);
    while(w[p]!=0) { s[p]=w[p]; ++p; }
    s[p++]='.';
    s[p++]=(char)('0'+((frac/100u)%10u));
    s[p++]=(char)('0'+((frac/10u)%10u));
    s[p++]=(char)('0'+(frac%10u));
    s[p]=0;
}

static void Two(unsigned char v,char *s){s[0]='0'+((v/10u)%10u);s[1]='0'+(v%10u);s[2]=0;}

static const char code *DayName(unsigned char d)
{
    switch(d){case 1:return "MON";case 2:return "TUE";case 3:return "WED";
    case 4:return "THU";case 5:return "FRI";case 6:return "SAT";case 7:return "SUN";}
    return "---";
}

static void Header(const char *title,const TIMEKEEPER_INFO *t)
{
    char tm[9]; unsigned char h,m,s;
    h=Timekeeper_BcdToBin(t->hour_bcd);m=Timekeeper_BcdToBin(t->minute_bcd);s=Timekeeper_BcdToBin(t->second_bcd);
    Two(h,&tm[0]);tm[2]=':';Two(m,&tm[3]);tm[5]=':';Two(s,&tm[6]);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,title);
    LCD_PrintInverse(188u,0u,tm);
}
#if 0 /* UNUSED L16: StatusAt; retained for future work */
static void StatusAt(unsigned int x,unsigned char y,unsigned char ok)
{
    LCD_Print(x,y,ok?"OK":"FAULT");
}
#endif /* UNUSED: StatusAt */

#if 0 /* UNUSED L16: GUI_ShowSplash; retained for future work */
void GUI_ShowSplash(void)
{
    LCD_ClearBuffer();
    LCD_DrawSchulzeLogo(8u,20u);
    LCD_Print(55u,16u,"XTOOLS for ISL8");
    LCD_Print(61u,20u,"  and SERVICE");
    LCD_FillRect(55u,29u,126u,38u);
    LCD_PrintInverse(61u,30u,"SERVICE");
    LCD_Print(55u,46u,"V2.00");
    LCD_Refresh();
}
#endif /* UNUSED: GUI_ShowSplash */

static const char code *DayName2(unsigned char d)
{
    switch(d){case 1:return "MO";case 2:return "TU";case 3:return "WE";
    case 4:return "TH";case 5:return "FR";case 6:return "SA";case 7:return "SU";}
    return "--";
}

static void PrintLiveVoltage(unsigned int x,unsigned char y,char id,signed long mv)
{
    char s[12];
    LCD_Print(x,y,"(");
    LCD_DrawChar(x+6u,y,(unsigned char)id);
    LCD_Print(x+12u,y,":");
    if(mv<0L) mv=0L;
    DecSL(mv,s);
    LCD_Print(x+18u,y,s);
    LCD_Print(x+48u,y,")");
}

static void PageStatus(const TIMEKEEPER_INFO *t)
{
    char date[11],tm[6];
    unsigned char d,m,y,h,mi;
    ADC_VOLTAGE_RESULT a1,a2,a3;
    ADC_BATT_RESULT bp;

    /* First/final-style XTOOLS screen. Coordinates are real LCD pixels. */
    LCD_Rect(0u,0u,239u,63u);
    LCD_Rect(0u,53u,239u,63u);       /* bottom soft-key strip */

    LCD_DrawSchulzeLogo(5u,4u);
            
    LCD_FillRect(48u,  53u, 48u,  63u);
    LCD_FillRect(96u,  53u, 96u,  63u);
    LCD_FillRect(144u, 53u, 144u, 63u);
    LCD_FillRect(192u, 53u, 192u, 63u); 

    LCD_ClearPixel (48u,54u);
    LCD_ClearPixel (96u,54u);
    LCD_ClearPixel (144u,54u);
    LCD_ClearPixel (192u,54u);

    LCD_ClearPixel (48u,62u);
    LCD_ClearPixel (96u,62u);
    LCD_ClearPixel (144u,62u);
    LCD_ClearPixel (192u,62u);
            

    d=Timekeeper_BcdToBin(t->day_bcd);
    m=Timekeeper_BcdToBin(t->month_bcd);
    y=Timekeeper_BcdToBin(t->year_bcd);
    h=Timekeeper_BcdToBin(t->hour_bcd);
    mi=Timekeeper_BcdToBin(t->minute_bcd);

    Two(d,&date[0]); date[2]='.';
    Two(m,&date[3]); date[5]='.';
    date[6]='2'; date[7]='0'; Two(y,&date[8]);
    LCD_Print(177u,2u,date);

    LCD_Print(184u,11u,DayName2(t->weekday_bcd));
    Two(h,&tm[0]); tm[2]=':'; Two(mi,&tm[3]);
    LCD_Print(203u,11u,tm);

    /* Four live values, after the same calibration/conversion used by XTOOLS. */
    ADC_MeasureA1((signed char)t->copy[0].tol_a1,(signed char)t->copy[0].ofs_a1,&a1);
    ADC_MeasureA2((signed char)t->copy[0].tol_a2,(signed char)t->copy[0].ofs_a2,&a2);
    ADC_MeasureA3((signed char)t->copy[0].tol_a3,(signed char)t->copy[0].ofs_a3,&a3);
    ADC_MeasureBattPlus((signed char)t->batt_tol[0],&bp);

    PrintLiveVoltage(184u,20u,'1',a1.mv_cal);
    PrintLiveVoltage(184u,28u,'2',a2.mv_cal);
    PrintLiveVoltage(184u,36u,'3',a3.mv_cal);
    /* Confirmed: third byte is BATT+ TOL; OFS is fixed zero. */
    PrintLiveVoltage(184u,44u,'B',(signed long)bp.mv);

    LCD_Print(61u,8u,"XTOOLS for ISL8");
    LCD_Print(61u,20u,"  and SERVICE");
    LCD_Print(12u,30u,"V2.00");
    
    LCD_Print(9u,55u, "CALIB");
    LCD_Print(53u,55u, " USER");
    LCD_Print(102u,55u," TIME");
    LCD_Print(150u,55u,"EEPROM");
    LCD_Print(198u,55u," NEXT");
    
    LCD_Rect(12u,40u,168u,49u);
    LCD_PrintInverse(13u,41u,"Programed by Prometheuss80");
    
}

static void PrintSigned8(unsigned int x,unsigned char y,signed char v,unsigned char inverse)
{
    char s[6];
    signed long n=(signed long)v;
    unsigned char pos=0u;

    if(n>=0L) s[pos++]='+';
    DecSL(n,&s[pos]);
    if(inverse) LCD_PrintInverse(x,y,s);
    else LCD_Print(x,y,s);
}

/* DIFF % = (CAL - ADC) / CAL * 100, displayed to 0.001%.
 * Calculate with integer long division: no float and no 32-bit overflow
 * from multiplying the voltage difference by 100000.
 * ADC and CAL come from the same acquisition, not two independent reads.
 */
static void CalibDiffText(signed long adc_mv,signed long cal_mv,char *s)
{
    signed long difference;
    unsigned long magnitude,denom,whole,remainder,scaled;
    unsigned char i,digit,next_digit;
    unsigned char pos=0u;

    if(cal_mv<=0L) { s[0]='-';s[1]='-';s[2]='-';s[3]=0;return; }
    difference=cal_mv-adc_mv;
    s[pos++]=(difference<0L)?'-':'+';
    magnitude=(difference<0L)?(unsigned long)(-difference):(unsigned long)difference;
    denom=(unsigned long)cal_mv;
    whole=magnitude/denom;
    remainder=magnitude%denom;

    /* More than 999.999% does not fit the reserved display region. */
    if(whole>=10UL) { s[0]='O';s[1]='V';s[2]='R';s[3]=0;return; }

    /* Five decimal places of the ratio are three decimal places in %. */
    scaled=whole;
    for(i=0u;i<5u;i++)
    {
        remainder*=10UL;
        digit=(unsigned char)(remainder/denom);
        remainder%=denom;
        scaled=scaled*10UL+(unsigned long)digit;
    }
    /* Round the last displayed digit, half up. */
    remainder*=10UL;
    next_digit=(unsigned char)(remainder/denom);
    if(next_digit>=5u) ++scaled;
    if(scaled>999999UL) scaled=999999UL;

    whole=scaled/1000UL;
    if(whole>=100UL) s[pos++]=(char)('0'+(whole/100UL)%10UL);
    if(whole>=10UL) s[pos++]=(char)('0'+(whole/10UL)%10UL);
    s[pos++]=(char)('0'+whole%10UL);
    s[pos++]='.';
    magnitude=scaled%1000UL;
    s[pos++]=(char)('0'+magnitude/100UL);
    s[pos++]=(char)('0'+(magnitude/10UL)%10UL);
    s[pos++]=(char)('0'+magnitude%10UL);
    s[pos++]='%';
    s[pos]=0;
}

static void CalibSoftKeys(void)
{
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);
    LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);
    LCD_FillRect(192u,53u,192u,63u);

    LCD_ClearPixel(48u,54u);  LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u);  LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);

    LCD_Print(17u,55u,"A1");
    LCD_Print(65u,55u,"A2");
    LCD_Print(113u,55u,"A3");
    LCD_Print(153u,55u,"BATT+");
    LCD_Print(201u,55u,"CLEAR");
}

static void PageCalibration(const TIMEKEEPER_INFO *t,unsigned char channel,unsigned char edit)
{
    char h[3],s[12],diff_text[12];
    signed char tol=0,ofs=0;
    signed long mv_uncal=0L,mv_cal=0L;
    ADC_VOLTAGE_RESULT vr;
    ADC_BATT_RESULT bp;

    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"CALIBRATION");

    if(channel==0u) LCD_PrintInverse(210u,1u,"A1");
    else if(channel==1u) LCD_PrintInverse(210u,1u,"A2");
    else if(channel==2u) LCD_PrintInverse(210u,1u,"A3");
    else LCD_PrintInverse(198u,1u,"BATT+");

    /* Physical seven-byte calibration block from copy 1. Byte 2 is BATT+ TOL; BATT+ OFS=FIX0. */
    LCD_Print(3u,11u,"RAW:");
    Hex8(t->copy[0].tol_a1,h); LCD_Print(31u,11u,h);
    Hex8(t->copy[0].tol_a2,h); LCD_Print(49u,11u,h);
    Hex8(t->copy[0].zero,h);   LCD_Print(67u,11u,h);
    Hex8(t->copy[0].ofs_a1,h); LCD_Print(85u,11u,h);
    Hex8(t->copy[0].ofs_a2,h); LCD_Print(103u,11u,h);
    Hex8(t->copy[0].ofs_a3,h); LCD_Print(121u,11u,h);
    Hex8(t->copy[0].tol_a3,h); LCD_Print(139u,11u,h);

    if(channel==0u)
    {
        tol=(signed char)t->copy[0].tol_a1; ofs=(signed char)t->copy[0].ofs_a1;
        ADC_MeasureA1(tol,ofs,&vr); mv_uncal=vr.mv_uncal; mv_cal=vr.mv_cal;
    }
    else if(channel==1u)
    {
        tol=(signed char)t->copy[0].tol_a2; ofs=(signed char)t->copy[0].ofs_a2;
        ADC_MeasureA2(tol,ofs,&vr); mv_uncal=vr.mv_uncal; mv_cal=vr.mv_cal;
    }
    else if(channel==2u)
    {
        tol=(signed char)t->copy[0].tol_a3; ofs=(signed char)t->copy[0].ofs_a3;
        ADC_MeasureA3(tol,ofs,&vr); mv_uncal=vr.mv_uncal; mv_cal=vr.mv_cal;
    }
    else
    {
        tol=(signed char)t->batt_tol[0];
        ofs=0;
        /* One physical BATT+ acquisition only.  CAL uses the stored TOL,
           while ADC is reconstructed from the SAME averaged RAW sample with
           TOL=0.  This makes the calibration effect visible without adding
           sample-to-sample ADC noise.  BATT+ OFS is fixed zero. */
        ADC_MeasureBattPlus(tol,&bp);
        mv_uncal=(signed long)ADC_BattPlusRawToMv(bp.raw,0);
        mv_cal=(signed long)bp.mv;
    }

    LCD_Print(3u,22u,"TOL:");
    PrintSigned8(34u,22u,tol,(edit==0u)?1u:0u);

    LCD_Print(103u,22u,"OFS:");
    if(channel==3u) LCD_Print(134u,22u,"FIX0");
    else PrintSigned8(134u,22u,ofs,(edit==1u)?1u:0u);

    LCD_Print(3u,33u,"ADC:");
    DecSL(mv_uncal,s); LCD_Print(34u,33u,s); LCD_Print(70u,33u,"mV");
    LCD_Print(103u,33u,"CAL:");
    DecSL(mv_cal,s); LCD_Print(134u,33u,s); LCD_Print(170u,33u,"mV");

    LCD_Print(3u,44u,"CALIB:");
    LCD_Print(45u,44u,t->cal_ok?"OK":"FAULT");
    LCD_Print(88u,44u,"B+:");
    LCD_Print(112u,44u,t->batt_tol_ok?"OK":"FAULT");
    CalibDiffText(mv_uncal,mv_cal,diff_text);
    LCD_Print(151u,44u,"DIFF:");
    LCD_Print(183u,44u,diff_text);

    CalibSoftKeys();
}

static void UserSoftKeys(void)
{
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);
    LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);
    LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u);  LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u);  LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);
    LCD_Print(11u,55u,"USER");
    LCD_Print(59u,55u,"PASS");
    LCD_Print(105u,55u,"ERASE");
}

static void PageUser(const TIMEKEEPER_INFO *t)
{
    char sn[6];
    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"USER / PASSWORD");

    LCD_Print(3u,12u,"USER:");
    LCD_Print(39u,12u,t->copy[0].owner);
    LCD_Print(183u,12u,t->owner_ok?"OK":"FAULT");

    LCD_Print(3u,24u,"PASS:");
    LCD_Print(39u,24u,t->copy[0].password);
    LCD_Print(183u,24u,t->password_ok?"OK":"FAULT");

    LCD_Print(3u,35u,"SN:");
    Dec16(t->copy[0].serial_number,sn);
    LCD_Print(27u,35u,sn);
    LCD_Print(183u,35u,t->sn_ok?"OK":"FAULT");

    LCD_Print(3u,45u,"F1 USER  F2 PASS  F3 ERASE");
    UserSoftKeys();
}


static void TimeSoftKeys(const TIMEKEEPER_INFO *t)
{
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u); LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u); LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u); LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u); LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);
    LCD_Print(10u,55u,"H/M/S");
    LCD_Print(57u,55u,"DAY/M");
    LCD_Print(104u,55u,"YR/WK");
    LCD_Print(203u,55u,t->rtc_running?"STOP":" RUN");
}

static void PrintTimeValue(unsigned int x,unsigned char y,unsigned char v,unsigned char inverse)
{
    char s[3]; Two(v,s);
    if(inverse) LCD_PrintInverse(x,y,s); else LCD_Print(x,y,s);
}

static void PageTime(const TIMEKEEPER_INFO *t,unsigned char field)
{
    unsigned char h,mi,se,d,m,y,w;
    char ys[5];
    h=Timekeeper_BcdToBin(t->hour_bcd); mi=Timekeeper_BcdToBin(t->minute_bcd);
    se=Timekeeper_BcdToBin(t->second_bcd); d=Timekeeper_BcdToBin(t->day_bcd);
    m=Timekeeper_BcdToBin(t->month_bcd); y=Timekeeper_BcdToBin(t->year_bcd);
    w=Timekeeper_BcdToBin(t->weekday_bcd);

    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"TIME / DATE");
    LCD_Print(3u,12u,"TIME:");
    PrintTimeValue(45u,12u,h,field==TMK_RTC_HOUR); LCD_Print(57u,12u,":");
    PrintTimeValue(63u,12u,mi,field==TMK_RTC_MINUTE); LCD_Print(75u,12u,":");
    PrintTimeValue(81u,12u,se,field==TMK_RTC_SECOND);

    LCD_Print(3u,23u,"DATE:");
    PrintTimeValue(45u,23u,d,field==TMK_RTC_DAY); LCD_Print(57u,23u,".");
    PrintTimeValue(63u,23u,m,field==TMK_RTC_MONTH); LCD_Print(75u,23u,".20");
    Two(y,&ys[0]);
    if(field==TMK_RTC_YEAR) LCD_PrintInverse(93u,23u,ys); else LCD_Print(93u,23u,ys);

    LCD_Print(3u,34u,"WEEK:");
    if(field==TMK_RTC_WEEKDAY) LCD_PrintInverse(45u,34u,DayName2(t->weekday_bcd));
    else LCD_Print(45u,34u,DayName2(t->weekday_bcd));
    LCD_Print(177u,2u,"RTC:");
    LCD_Print(201u,2u,t->rtc_running?"RUN":"STOP");
    LCD_Print(3u,43u,"+/- SET");
    TimeSoftKeys(t);
}

#if 0 /* UNUSED L16: A3Fixed4; kept for reference */
static void A3Fixed4(unsigned int v,char *s)
{
    if(v>9999u) v=9999u;
    s[0]=(char)('0'+(v/1000u)%10u);
    s[1]=(char)('0'+(v/100u)%10u);
    s[2]=(char)('0'+(v/10u)%10u);
    s[3]=(char)('0'+v%10u);
    s[4]='\0';
}
#endif /* UNUSED: A3Fixed4 */

#if 0 /* UNUSED L16: A3Fixed5; retained for future work */
static void A3Fixed5(unsigned int v,char *s)
{
    if(v>99999u) v=99999u;
    s[0]=(char)('0'+(v/10000u)%10u);
    s[1]=(char)('0'+(v/1000u)%10u);
    s[2]=(char)('0'+(v/100u)%10u);
    s[3]=(char)('0'+(v/10u)%10u);
    s[4]=(char)('0'+v%10u);
    s[5]='\0';
}
#endif /* UNUSED: A3Fixed5 */

/* Forward declaration: used by PageMeasurements below. */
static unsigned int A3RawToMa(unsigned int raw);
static unsigned int PowerTenthsW(unsigned int mv,unsigned int ma);
static void PrintTenthsW(unsigned char x,unsigned char y,unsigned int v);

static unsigned int A2Put(unsigned int x,unsigned char y,const char *v);
static unsigned int A2Num(unsigned int x,unsigned char y,unsigned int n);
static unsigned int A3RawToMa(unsigned int raw);
static unsigned int PowerTenthsW(unsigned int mv,unsigned int ma);
static void PageMeasurements(const TIMEKEEPER_INFO *t)
{
    char h[5];
    unsigned int raw,ma,mv,si,sv,sq,st,x,power10,minutes,seconds;
    const char *state="STOP";
    ADC_VOLTAGE_RESULT v;
    ADC_BATT_RESULT bp;
    unsigned int bv,bdec;
    char batt_text[8];
    ADC_MeasureA3((signed char)t->copy[0].tol_a3,(signed char)t->copy[0].ofs_a3,&v);
    ADC_MeasureBattPlus((signed char)t->batt_tol[0],&bp);
    raw=ADC_MeasureA3Current();
    ma=a3_mode_discharge?(a3_running?GUI_A3DischargeEstimatedMa(a3_live_cc):0u):A3RawToMa(raw); /* DIS: CC3 estimate; CHG: ADC11 measured */
    mv=(v.mv_cal>0L && v.mv_cal<65535L)?(unsigned int)v.mv_cal:0u;
    si=GUI_A3GetTargetCurrent();sv=GUI_A3GetVoltageLimit();
    sq=GUI_A3CapacitySet();st=GUI_A3TimeSet();
    minutes=(unsigned int)(a3_total_sec/60UL);
    seconds=(unsigned int)(a3_total_sec%60UL);
    if(a3_mode_discharge && a3_running && mv>0u)
    {
        unsigned int power_cap=(unsigned int)(2500000UL/(unsigned long)mv);
        if(ma>power_cap) ma=power_cap;
        if(ma>si) ma=si;
    }
    power10=PowerTenthsW(mv,ma);
    if(a3_status==1u)state=a3_mode_discharge?"DIS":"CC";
    else if(a3_status==2u)state="CV";
    else if(a3_status==3u)state="END";
    else if(a3_status==4u)state="TIME";
    else if(a3_status==5u)state="QUANT";
    else if(a3_status==6u)state="FAULT";
    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"TESTING PORT A3");
    /* Identical top-row layout to A2: leave room for both voltage and current. */
    x=104u;x=A2Num(x,1u,mv);x=A2Put(x,1u,"mV ");
    {x=A2Num(x,1u,ma);A2Put(x,1u,"mA");}
    LCD_PrintInverse(231u,1u,a3_mode_discharge?"D":"C");
    x=2u;x=A2Put(x,11u,"Q: ");x=A2Num(x,11u,(unsigned int)a3_total_mah);
    x=A2Put(x,11u,"mAh / T: ");x=A2Num(x,11u,minutes);
    x=A2Put(x,11u,"m:");x=A2Num(x,11u,seconds);x=A2Put(x,11u,"s / ");
    x=A2Put(x,11u,"P: ");x=A2Num(x,11u,power10/10u);
    x=A2Put(x,11u,".");x=A2Num(x,11u,power10%10u);A2Put(x,11u,"W");
    x=2u;x=A2Put(x,21u,"U");x=A2Num(x,21u,sv);
    x=A2Put(x,21u," / I");x=A2Num(x,21u,si);
    x=A2Put(x,21u," / Q");x=A2Num(x,21u,sq);
    x=A2Put(x,21u," / T");A2Num(x,21u,st);
    x=2u;x=A2Put(x,31u,"CC3: ");XTOOLS_WordHex(h,a3_live_cc);x=A2Put(x,31u,h);
    x=A2Put(x,31u," / RAW I: ");x=A2Num(x,31u,raw);
    x=A2Put(x,31u," / FAN: ");A2Num(x,31u,measurement.raw[8]);
    x=2u;x=A2Put(x,41u,"EDIT ");
    if(a3_edit_field==0u) {x=A2Put(x,41u,a3_mode_discharge?"UMIN: ":"UMAX: ");x=A2Num(x,41u,sv);A2Put(x,41u,"mV");}
    else if(a3_edit_field==1u) {x=A2Put(x,41u,"CURRENT: ");x=A2Num(x,41u,si);A2Put(x,41u,"mA");}
    else if(a3_edit_field==2u) {x=A2Put(x,41u,"QUANT: ");x=A2Num(x,41u,sq);A2Put(x,41u,"mAh");}
    else {x=A2Put(x,41u,"TIME: ");x=A2Num(x,41u,st/60u);A2Put(x,41u,"min");}
    /* Same calibrated BATT+ readout as A1/A2, e.g. B13.63V. */
    bv=bp.mv/1000u;
    bdec=(bp.mv%1000u)/10u;
    batt_text[0]='B';
    batt_text[1]=(char)('0'+((bv/10u)%10u));
    batt_text[2]=(char)('0'+(bv%10u));
    batt_text[3]='.';
    batt_text[4]=(char)('0'+(bdec/10u));
    batt_text[5]=(char)('0'+(bdec%10u));
    batt_text[6]='V';batt_text[7]=0;
    LCD_Print(153u,41u,batt_text);
    LCD_PrintInverse(204u,41u,state);
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u); LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u); LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);
    LCD_Print(9u,55u,"START");LCD_Print(59u,55u,"STOP");
    LCD_Print(107u,55u,"EDIT");LCD_Print(155u,55u,"MODE");
    if(a3_mode_discharge)
        DrawModeOrCurrent(a3_running,a3_auto,a3_live_cc,83u,10420u,1u);
    else
        DrawModeOrCurrent(a3_running,a3_auto,a3_live_cc,6u,38375u,0u);
}

static unsigned int A2RawToMa(unsigned int raw)
{
    unsigned long v;
    if(raw<=10u) return 0u;
    v=((unsigned long)raw*1463UL)/1000UL+30UL;
    return v>65535UL?65535u:(unsigned int)v;
}
static unsigned int A3RawToMa(unsigned int raw)
{
    unsigned long v;
    if(!a3_running || raw<=a3_charge_zero_raw) return 0u;
    /* Do not display idle current as a real charge current.
       The original scale remains provisional; subtract measured zero. */
    v=((unsigned long)(raw-a3_charge_zero_raw)*700UL)/1000UL;
    return v>65535UL?65535u:(unsigned int)v;
}
static unsigned int PowerTenthsW(unsigned int mv,unsigned int ma)
{
    unsigned long v=((unsigned long)mv*(unsigned long)ma)/100000UL;
    return v>65535UL?65535u:(unsigned int)v;
}
#if 0 /* UNUSED L16: PrintTenthsW; retained for future work */
static void PrintTenthsW(unsigned char x,unsigned char y,unsigned int v)
{
    char s[12];
    A3Fixed4(v/10u,s); LCD_Print(x,y,s);
    LCD_Print((unsigned char)(x+24u),y,".");
    Dec16(v%10u,s); LCD_Print((unsigned char)(x+30u),y,s);
}
#endif /* UNUSED: PrintTenthsW */
/* 5x7 font uses a six-pixel character cell. Dynamic placement avoids
 * fixed-width leading zeroes and prevents old digits from remaining. */
static unsigned int A2Put(unsigned int x,unsigned char y,const char *v)
{
    const char *q=v;
    LCD_Print(x,y,v);
    while(*q) {x+=6u;++q;}
    return x;
}
static unsigned int A2Num(unsigned int x,unsigned char y,unsigned int n)
{
    char s[8];Dec16(n,s);return A2Put(x,y,s);
}
static void PageA2Test(const TIMEKEEPER_INFO *t)
{
    ADC_VOLTAGE_RESULT a2;
    unsigned int i_raw,actual_ma,power10,x,fan_raw,elapsed_min,elapsed_sec;
    const char *state="STOP";
    ADC_MeasureA2((signed char)t->copy[0].tol_a2,
                  (signed char)t->copy[0].ofs_a2,&a2);
    i_raw=ADC_Measure(ADC_CH_A2_CURRENT);
    actual_ma=A2RawToMa(i_raw);
    power10=PowerTenthsW((a2.mv_cal>0L)?(unsigned int)a2.mv_cal:0u,actual_ma);
    fan_raw=measurement.raw[8];
    elapsed_min=(unsigned int)(a2_elapsed_s/60UL);
    elapsed_sec=(unsigned int)(a2_elapsed_s%60UL);
    if(a2_status==1u)state="CC";
    else if(a2_status==2u)state="CV";
    else if(a2_status==3u)state="END";
    else if(a2_status==4u)state="TIME";
    else if(a2_status==5u)state="QUANT";
    else if(a2_status==6u)state="FAULT";
    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"TESTING PORT A2");
    x=104u;x=A2Num(x,1u,(a2.mv_cal>0L)?(unsigned int)a2.mv_cal:0u);
    x=A2Put(x,1u,"mV ");x=A2Num(x,1u,actual_ma);A2Put(x,1u,"mA");
    LCD_PrintInverse(231u,1u,"C");
    x=2u;x=A2Put(x,11u,"Q: ");x=A2Num(x,11u,(unsigned int)a2_delivered_mah);
    x=A2Put(x,11u,"mAh / T: ");x=A2Num(x,11u,elapsed_min);
    x=A2Put(x,11u,"m:");x=A2Num(x,11u,elapsed_sec);
    x=A2Put(x,11u,"s / P: ");x=A2Num(x,11u,power10/10u);
    x=A2Put(x,11u,".");x=A2Num(x,11u,power10%10u);A2Put(x,11u,"W");
    x=2u;x=A2Put(x,21u,"U");x=A2Num(x,21u,a2_voltage_mv);
    x=A2Put(x,21u," / I");x=A2Num(x,21u,a2_current_ma);
    x=A2Put(x,21u," / Q");x=A2Num(x,21u,a2_capacity_mah);
    x=A2Put(x,21u," / T");A2Num(x,21u,a2_time_s);
    x=2u;x=A2Put(x,31u,"CC2:");
    {char h[5];XTOOLS_WordHex(h,a2_live_compare);x=A2Put(x,31u,h);}
    x=A2Put(x,31u," / RAW I: ");x=A2Num(x,31u,i_raw);
    x=A2Put(x,31u," / FAN: ");A2Num(x,31u,fan_raw);
    x=2u;x=A2Put(x,41u,"EDIT ");
    if(a2_edit_field==0u) {x=A2Put(x,41u,"TIME: ");x=A2Num(x,41u,a2_time_s/60u);x=A2Put(x,41u,"min");}
    else if(a2_edit_field==1u) {x=A2Put(x,41u,"QUANT: ");x=A2Num(x,41u,a2_capacity_mah);x=A2Put(x,41u,"mAh");}
    else if(a2_edit_field==2u) {x=A2Put(x,41u,"CURRENT: ");x=A2Num(x,41u,a2_current_ma);x=A2Put(x,41u,"mA");}
    else {x=A2Put(x,41u,"VOLT: ");x=A2Num(x,41u,a2_voltage_mv);x=A2Put(x,41u,"mV");}
    LCD_PrintInverse(204u,41u,state);
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u); LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u); LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);
    LCD_Print(9u,55u,"START");LCD_Print(59u,55u,"STOP");
    LCD_Print(107u,55u,"EDIT");
    DrawModeOrCurrent(a2_running_ui,a2_auto,a2_live_compare,10u,11261u,0u);
}

/* A1 ANALYSIS ONLY: N is an unverified internal firmware quantity.
   For display we temporarily assume N equals selected mA. NOT a calibration. */
#if 0 /* UNUSED L16: A1_CandidateCC1; retained for future work */
static unsigned int A1_CandidateCC1(unsigned char mode,unsigned int n)
{
    unsigned int offset,scale,limit;
    unsigned long x;
    if(mode==0u) {offset=25u;scale=20108u;limit=2545u;}
    else if(mode==1u) {offset=30u;scale=5578u;limit=10000u;}
    else {offset=83u;scale=4492u;limit=10045u;}
    /* 10000 for resistor branch is diagnostic only, not verified. */
    if(n<=offset) return 0xFBFFu;
    if(n>limit) n=limit;
    x=((unsigned long)(n-offset)*(unsigned long)scale)>>16;
    if(x>1024UL) x=1024UL;
    return (unsigned int)(0xFBFFu+(unsigned int)x);
}
#endif /* UNUSED: A1_CandidateCC1 */
/* A1 safe UI-only bring-up: no CC1 writes, no latch enables. */
static unsigned int a1_bench_cc=0xFBFFu;
static unsigned int a1_bench_raw=0u;
static unsigned char a1_bench_running=0u,a1_bench_fault=0u;
static unsigned char a1_bench_disch=0u; /* 0 CH, 1 RES, 2 RECUP */
static unsigned long a1_bench_elapsed_ticks=0UL;
static unsigned long a1_bench_capacity_x100=0UL;
void GUI_A1BenchTotals(unsigned long elapsed_ticks,unsigned long capacity_x100)
{
    a1_bench_elapsed_ticks=elapsed_ticks;
    a1_bench_capacity_x100=capacity_x100;
}
void GUI_A1BenchMode(unsigned char discharge) {a1_bench_disch=discharge;}
void GUI_A1BenchLive(unsigned int cc,unsigned int raw,unsigned char running,unsigned char fault)
{
    a1_bench_cc=cc;a1_bench_raw=raw;
    a1_bench_running=running;a1_bench_fault=fault;
}
static unsigned char a1_mode=0u; /* 0 CHARGE, 1 RES, 2 RECUP (manual diagnostic) */
static unsigned char a1_edit=0u;
static unsigned int a1_v[3]={8400u,6600u,18000u};
static unsigned int a1_i[3]={2000u,250u,100u};
static unsigned int a1_q[3]={2000u,2000u,2000u};
static unsigned int a1_t[3]={3600u,3600u,60u};
static unsigned char a1_auto=0u;
/* RECUP requires a battery able to absorb power; disabled by default. */
static unsigned char a1_recup_enabled=0u;
static unsigned int a1_recup_batt_limit_mv=14700u;
void GUI_A1EditNext(void) {a1_edit=(unsigned char)((a1_edit+1u)%(a1_mode==0u?5u:6u));}
/* F4 selects CHARGE / DISCHARGE; D/R are selected automatically at START. */
void GUI_A1ModeNext(void) {a1_mode=(unsigned char)((a1_mode==0u)?1u:0u);a1_edit=0u;}
void GUI_A1SelectDischargePath(unsigned char path)
{
    unsigned char old=a1_mode;
    if(path!=1u && path!=2u) return;
    if(old==1u || old==2u)
    {
        a1_v[path]=a1_v[old];a1_i[path]=a1_i[old];
        a1_q[path]=a1_q[old];a1_t[path]=a1_t[old];
    }
    a1_mode=path;
}
unsigned char GUI_A1IsCurrentEdit(void) {return (unsigned char)(a1_edit==1u);}
unsigned char GUI_A1IsToggleEdit(void) {return (unsigned char)(a1_edit==4u);}
unsigned int GUI_A1CurrentSet(void) {return a1_i[a1_mode];}
unsigned int GUI_A1VoltageSet(void) {return a1_v[a1_mode];}
unsigned int GUI_A1CapacitySet(void) {return a1_q[a1_mode];}
unsigned int GUI_A1TimeSet(void) {return a1_t[a1_mode];}
unsigned char GUI_A1IsAuto(void) {return a1_auto;}
void GUI_A1ToggleAuto(void) {a1_auto=(unsigned char)!a1_auto;}
unsigned char GUI_A1RecupEnabled(void) {return a1_recup_enabled;}
unsigned int GUI_A1RecupBattLimit(void) {return a1_recup_batt_limit_mv;}
void GUI_A1Adjust(int delta)
{
    unsigned int *v;
    unsigned int lo,hi,step;
    unsigned long next;
    if(a1_edit==4u) {a1_recup_enabled=(unsigned char)!a1_recup_enabled;return;}
    if(a1_edit==5u && a1_mode!=0u)
    {
        if(delta>0 && a1_recup_batt_limit_mv<14700u) a1_recup_batt_limit_mv+=100u;
        else if(delta<0 && a1_recup_batt_limit_mv>14200u) a1_recup_batt_limit_mv-=100u;
        return;
    }
    if(a1_edit==0u) {v=&a1_v[a1_mode];lo=1000u;hi=(a1_mode==0u?15000u:60000u);step=100u;}
    else if(a1_edit==1u) {v=&a1_i[a1_mode];lo=250u;hi=(a1_mode==0u?2000u:10000u);step=250u;}
    else if(a1_edit==2u) {v=&a1_q[a1_mode];lo=100u;hi=25000u;step=100u;}
    else {v=&a1_t[a1_mode];lo=300u;hi=14400u;step=300u;}
    if(delta>0) {next=(unsigned long)*v+step;*v=(unsigned int)(next>hi?hi:next);}
    else if(delta<0) {*v=(*v<=lo+step)?lo:(unsigned int)(*v-step);}
}
static void PageA1Test(const TIMEKEEPER_INFO *t)
{
    ADC_VOLTAGE_RESULT vr;
    ADC_BATT_RESULT bp;
    unsigned int mv,ma,x,power10,mins,secs;
    char cc_text[5];
    char batt_text[8];
    unsigned int bv,bdec;
    ADC_MeasureBattPlus((signed char)t->batt_tol[0],&bp);
    ADC_MeasureA1((signed char)t->copy[0].tol_a1,
                  (signed char)t->copy[0].ofs_a1,&vr);
    mv=(vr.mv_cal>0L)?(unsigned int)vr.mv_cal:0u;
    if(!a1_bench_running) ma=0u;
    else if(a1_bench_disch==2u)
        ma=(a1_bench_raw<=7u)?0u:(unsigned int)((((unsigned long)a1_bench_raw*3482UL+500UL)/1000UL)+65UL);
    else if(a1_bench_disch)
        ma=(unsigned int)(((unsigned long)a1_bench_raw*1140UL+500UL)/1000UL);
    else
        ma=(a1_bench_raw<=7u)?0u:(unsigned int)((unsigned long)a1_bench_raw*3UL+15UL);
    power10=PowerTenthsW(mv,ma);
    mins=(unsigned int)(a1_bench_elapsed_ticks/6000UL);
    secs=(unsigned int)((a1_bench_elapsed_ticks/100UL)%60UL);
    LCD_Rect(0u,0u,239u,63u);
    LCD_PrintInverse(2u,1u,"TESTING PORT A1");
    x=104u;x=A2Num(x,1u,mv);x=A2Put(x,1u,"mV ");
    x=A2Num(x,1u,ma);A2Put(x,1u,"mA");
    LCD_PrintInverse(231u,1u,(a1_mode==0u)?"C":((a1_mode==2u)?"R":"D"));
    x=2u;x=A2Put(x,11u,"Q: ");
    x=A2Num(x,11u,(unsigned int)(a1_bench_capacity_x100/360000UL));
    x=A2Put(x,11u,"mAh / T: ");x=A2Num(x,11u,mins);
    x=A2Put(x,11u,"m:");x=A2Num(x,11u,secs);
    x=A2Put(x,11u,"s / P: ");x=A2Num(x,11u,power10/10u);
    x=A2Put(x,11u,".");x=A2Num(x,11u,power10%10u);A2Put(x,11u,"W");
    x=2u;x=A2Put(x,21u,"U");x=A2Num(x,21u,a1_v[a1_mode]);
    x=A2Put(x,21u," / I");x=A2Num(x,21u,a1_i[a1_mode]);
    x=A2Put(x,21u," / Q");x=A2Num(x,21u,a1_q[a1_mode]);
    x=A2Put(x,21u," / T");A2Num(x,21u,a1_t[a1_mode]);
    XTOOLS_WordHex(cc_text,a1_bench_cc);
    x=2u;x=A2Put(x,31u,"CC1:");x=A2Put(x,31u,cc_text);
    x=A2Put(x,31u," RAW:");x=A2Num(x,31u,a1_bench_raw);
    x=A2Put(x,31u," / FAN: ");A2Num(x,31u,measurement.raw[8]);
    x=2u;x=A2Put(x,41u,"EDIT ");
    if(a1_edit==0u) {x=A2Put(x,41u,"VOLT: ");x=A2Num(x,41u,a1_v[a1_mode]);A2Put(x,41u,"mV");}
    else if(a1_edit==1u) {x=A2Put(x,41u,"CURRENT: ");x=A2Num(x,41u,a1_i[a1_mode]);A2Put(x,41u,"mA");}
    else if(a1_edit==2u) {x=A2Put(x,41u,"QUANT: ");x=A2Num(x,41u,a1_q[a1_mode]);A2Put(x,41u,"mAh");}
    else if(a1_edit==3u) {x=A2Put(x,41u,"TIME: ");x=A2Num(x,41u,a1_t[a1_mode]/60u);A2Put(x,41u,"min");}
    else if(a1_edit==4u) {x=A2Put(x,41u,"RECUP: ");A2Put(x,41u,a1_recup_enabled?"ON":"OFF");}
    else {x=A2Put(x,41u,"B LIM: ");x=A2Num(x,41u,a1_recup_batt_limit_mv/1000u);x=A2Put(x,41u,".");x=A2Num(x,41u,(a1_recup_batt_limit_mv%1000u)/100u);A2Put(x,41u,"V");}
    /* Compact calibrated BATT+ voltage, fixed position ahead of status.
       Use Bxx.xxV; 2-digit volts for 12 V Pb test. */
    bv=bp.mv/1000u;
    bdec=(bp.mv%1000u)/10u;
    batt_text[0]='B';
    batt_text[1]=(char)('0'+((bv/10u)%10u));
    batt_text[2]=(char)('0'+(bv%10u));
    batt_text[3]='.';
    batt_text[4]=(char)('0'+(bdec/10u));
    batt_text[5]=(char)('0'+(bdec%10u));
    batt_text[6]='V';batt_text[7]=0;
    LCD_Print(153u,41u,batt_text);
    if(a1_bench_fault) LCD_PrintInverse(204u,41u,"FAULT");
    else if(!a1_bench_running) LCD_PrintInverse(211u,41u,"STOP");
    else if(a1_bench_disch==2u) LCD_PrintInverse(204u,41u,"RECUP");
    else if(a1_bench_disch) LCD_PrintInverse(198u,41u,"DISCHAR");
    else LCD_PrintInverse(204u,41u,"CHARGE");
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u); LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u); LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);
    LCD_Print(9u,55u,"START");LCD_Print(59u,55u,"STOP");
    LCD_Print(107u,55u,"EDIT");LCD_Print(155u,55u,"MODE");
    /* A1 F5: show applied CC1 reference in MAN while running.
       Keep the measured ADC current in the top row unchanged. */
    if(a1_auto) LCD_PrintInverse(203u,55u,"AUT");
    else if(!a1_bench_running) LCD_PrintInverse(203u,55u,"MAN");
    else if(a1_bench_disch==2u)
        DrawModeOrCurrent(1u,0u,a1_bench_cc,83u,4492u,0u);
    else if(a1_bench_disch==1u)
        DrawModeOrCurrent(1u,0u,a1_bench_cc,25u,20108u,0u);
    else
        DrawModeOrCurrent(1u,0u,a1_bench_cc,30u,5578u,0u);
}

static void PageHardware(const TIMEKEEPER_INFO *t)
{
    char date[11],tm[6];
    unsigned char d,m,y,h,mi;
    ADC_VOLTAGE_RESULT a1,a2,a3;
    ADC_BATT_RESULT bp;


    /* Keep the second main screen visually identical to the SERVICE screen. */
    LCD_Rect(0u,0u,239u,63u);
    LCD_Rect(0u,53u,239u,63u);
    LCD_DrawSchulzeLogo(5u,4u);

    LCD_FillRect(48u,53u,48u,63u);
    LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);
    LCD_FillRect(192u,53u,192u,63u);

    LCD_ClearPixel(48u,54u);  LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u);  LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);

    d=Timekeeper_BcdToBin(t->day_bcd);
    m=Timekeeper_BcdToBin(t->month_bcd);
    y=Timekeeper_BcdToBin(t->year_bcd);
    h=Timekeeper_BcdToBin(t->hour_bcd);
    mi=Timekeeper_BcdToBin(t->minute_bcd);

    Two(d,&date[0]); date[2]='.';
    Two(m,&date[3]); date[5]='.';
    date[6]='2'; date[7]='0'; Two(y,&date[8]);
    LCD_Print(177u,2u,date);

    LCD_Print(184u,11u,DayName2(t->weekday_bcd));
    Two(h,&tm[0]); tm[2]=':'; Two(mi,&tm[3]);
    LCD_Print(203u,11u,tm);

    ADC_MeasureA1((signed char)t->copy[0].tol_a1,(signed char)t->copy[0].ofs_a1,&a1);
    ADC_MeasureA2((signed char)t->copy[0].tol_a2,(signed char)t->copy[0].ofs_a2,&a2);
    ADC_MeasureA3((signed char)t->copy[0].tol_a3,(signed char)t->copy[0].ofs_a3,&a3);
    ADC_MeasureBattPlus((signed char)t->batt_tol[0],&bp);

    PrintLiveVoltage(184u,20u,'1',a1.mv_cal);
    PrintLiveVoltage(184u,28u,'2',a2.mv_cal);
    PrintLiveVoltage(184u,36u,'3',a3.mv_cal);
    PrintLiveVoltage(184u,44u,'B',(signed long)bp.mv);

    LCD_Print(61u,8u,"XTOOLS for ISL8");
    LCD_Print(61u,20u,"  and SERVICE");
    LCD_Print(12u,30u,"V2.00");

    LCD_Print(3u,55u,  "TEST HW");
    LCD_Print(57u,55u, "CH A1");
    LCD_Print(105u,55u,"CH A2");
    LCD_Print(153u,55u,"CH A3");
    LCD_Print(198u,55u,"CHARGE");

    LCD_Rect(12u,40u,168u,49u);
    LCD_PrintInverse(13u,41u,"Programed by Prometheuss80");
}

static void HWTestState(unsigned char x,unsigned char y,unsigned char id,unsigned char selected,const char *label)
{
    if(selected) LCD_PrintInverse(x,y,label); else LCD_Print(x,y,label);
    if(HW_TestGet(id)) LCD_Print((unsigned char)(x+72u),y,"ON ");
    else LCD_Print((unsigned char)(x+72u),y,"OFF");
}

static void PageHWTest(const TIMEKEEPER_INFO *t,unsigned char cursor)
{
    ADC_VOLTAGE_RESULT a1,a2;
    char s[8];

    LCD_Rect(0u,0u,239u,63u);
    LCD_Rect(0u,53u,239u,63u);
    LCD_PrintInverse(87u,2u,"TESTING HW");

    /* Left column: four latch-B hardware functions. */
    HWTestState(3u,13u,HWTEST_FAN_INT,(cursor==HWTEST_FAN_INT),"FAN INT");
    HWTestState(3u,23u,HWTEST_FAN_OUT,(cursor==HWTEST_FAN_OUT),"FAN OUT");
    HWTestState(3u,33u,HWTEST_BLINK,(cursor==HWTEST_BLINK),"BLINK");
    HWTestState(3u,43u,HWTEST_DISCH_RES,(cursor==HWTEST_DISCH_RES),"DISCH RES");

    /* Right column: 5V tests.  Keep ADC indication tied to the named port. */
    if(cursor==HWTEST_5V_A1) LCD_PrintInverse(123u,13u,"5V A1");
    else LCD_Print(123u,13u,"5V A1");
    LCD_Print(159u,13u,Test5V_GetMode(0u)==TEST5V_IDLE?"IDL":(Test5V_GetMode(0u)==TEST5V_ON?"ON ":"OFF"));
    ADC_MeasureA1((signed char)t->copy[0].tol_a1,(signed char)t->copy[0].ofs_a1,&a1);
    FormatVolts3(a1.mv_cal,s); LCD_Print(183u,13u,s); LCD_Print(219u,13u,"V");

    if(cursor==HWTEST_5V_A2) LCD_PrintInverse(123u,23u,"5V A2");
    else LCD_Print(123u,23u,"5V A2");
    LCD_Print(159u,23u,Test5V_GetMode(1u)==TEST5V_IDLE?"IDL":(Test5V_GetMode(1u)==TEST5V_ON?"ON ":"OFF"));
    ADC_MeasureA2((signed char)t->copy[0].tol_a2,(signed char)t->copy[0].ofs_a2,&a2);
    FormatVolts3(a2.mv_cal,s); LCD_Print(183u,23u,s); LCD_Print(219u,23u,"V");

    /* FAN INT sensor P8.0 / ADC8: show the same cached RAW sample used
       by the background fan task. No blocking ADC read during LCD paint. */
    LCD_Print(123u,33u,"RAW IN FAN");
    if(measurement.valid_mask & (1u<<8))
    {
        unsigned int fan_raw=measurement.raw[8];
        char fan_digits[6];
        fan_digits[0]=(char)('0'+((fan_raw/10000u)%10u));
        fan_digits[1]=(char)('0'+((fan_raw/1000u)%10u));
        fan_digits[2]=(char)('0'+((fan_raw/100u)%10u));
        fan_digits[3]=(char)('0'+((fan_raw/10u)%10u));
        fan_digits[4]=(char)('0'+(fan_raw%10u));
        fan_digits[5]='\0';
        LCD_Print(198u,33u,fan_digits);
    }
    else LCD_Print(198u,33u,"-----");

    /* Fuse is indication only, not part of the selectable test sequence. */
    LCD_Print(123u,43u,"FUSE");
    LCD_Print(159u,43u,(P6&0x80u)?"OK":"FAULT");

    LCD_FillRect(48u,53u,48u,63u);
    LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);
    LCD_FillRect(192u,53u,192u,63u);
    LCD_ClearPixel(48u,54u);  LCD_ClearPixel(48u,62u);
    LCD_ClearPixel(96u,54u);  LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(144u,62u);
    LCD_ClearPixel(192u,54u); LCD_ClearPixel(192u,62u);

    LCD_Print(17u,55u,"UP");
    LCD_Print(59u,55u,"DOWN");
    LCD_Print(112u,55u,"ON");
    LCD_Print(157u,55u,"OFF");
    LCD_Print(196u,55u,"ALL OFF");
}

static void PageTimekeeper(const TIMEKEEPER_INFO *t)
{
    char h[3],s[8],date[9];
    unsigned char d,m,y;
    Header("TIMEKEEPER",t);
    d=Timekeeper_BcdToBin(t->day_bcd);m=Timekeeper_BcdToBin(t->month_bcd);y=Timekeeper_BcdToBin(t->year_bcd);
    Two(d,&date[0]);date[2]='.';Two(m,&date[3]);date[5]='.';Two(y,&date[6]);
    LCD_Print(2u,13u,"DATE:");LCD_Print(38u,13u,date);
    LCD_Print(116u,13u,"DAY:");LCD_Print(146u,13u,DayName(t->weekday_bcd));
    LCD_Print(2u,24u,"COPY1 SN:");Dec16(t->copy[0].serial_number,s);LCD_Print(62u,24u,s);
    LCD_Print(2u,33u,"COPY2 SN:");Dec16(t->copy[1].serial_number,s);LCD_Print(62u,33u,s);
    LCD_Print(2u,42u,"COPY3 SN:");Dec16(t->copy[2].serial_number,s);LCD_Print(62u,42u,s);
    LCD_Print(130u,33u,t->sn_ok?"SN COPIES OK":"SN FAULT");
    LCD_Print(2u,53u,"RAW CAL:");
    Hex8(t->copy[0].tol_a1,h);LCD_Print(56u,53u,h);
    Hex8(t->copy[0].tol_a2,h);LCD_Print(74u,53u,h);
    /* ZERO is intentionally skipped on the GUI. */
    Hex8(t->copy[0].ofs_a1,h);LCD_Print(98u,53u,h);
    Hex8(t->copy[0].ofs_a2,h);LCD_Print(116u,53u,h);
    Hex8(t->copy[0].ofs_a3,h);LCD_Print(134u,53u,h);
    Hex8(t->copy[0].tol_a3,h);LCD_Print(152u,53u,h);
}

static unsigned int EepromLogicalWord(unsigned int raw)
{
    return (unsigned int)((raw << 8) | (raw >> 8));
}

static unsigned char EepromCopiesOK(void)
{
    unsigned char i;
    for(i=0u;i<21u;i++)
    {
        if(g_eeprom_words[0x01u+i] != g_eeprom_words[0x16u+i]) return 0u;
        if(g_eeprom_words[0x01u+i] != g_eeprom_words[0x2Bu+i]) return 0u;
    }
    return 1u;
}

static void EepromSoftkeys(void)
{
    LCD_Rect(0u,53u,239u,63u);
    LCD_FillRect(48u,53u,48u,63u);
    LCD_FillRect(96u,53u,96u,63u);
    LCD_FillRect(144u,53u,144u,63u);
    LCD_FillRect(192u,53u,192u,63u);

    LCD_ClearPixel(48u,54u);  LCD_ClearPixel(96u,54u);
    LCD_ClearPixel(144u,54u); LCD_ClearPixel(192u,54u);
    LCD_ClearPixel(48u,62u);  LCD_ClearPixel(96u,62u);
    LCD_ClearPixel(144u,62u); LCD_ClearPixel(192u,62u);

    LCD_Print(11u,55u,"DUMP");
    LCD_Print(59u,55u," SN");
    LCD_Print(106u,55u,"SYNC");
    LCD_Print(152u,55u,"ERASE");
    LCD_Print(203u,55u,"BACK");
}

static void PageEeprom(const TIMEKEEPER_INFO *t)
{
    char hx[5],sn[6];
    unsigned int esn,tksn;
    unsigned char copy_ok,tk_ok,match_ok;

    LCD_Rect(0u,0u,239u,63u);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,"EEPROM 93C46B");

    if(!g_eeprom_full_valid)
    {
        LCD_Print(4u,20u,"EEPROM NOT READ");
        EepromSoftkeys();
        return;
    }

    esn=EepromLogicalWord(g_eeprom_words[0x06u]);
    tksn=t->copy[0].serial_number;
    copy_ok=EepromCopiesOK();
    tk_ok=(unsigned char)((t->copy[0].serial_number==t->copy[1].serial_number) &&
                          (t->copy[0].serial_number==t->copy[2].serial_number));
    match_ok=(unsigned char)(tk_ok && (esn==tksn));

    LCD_Print(4u,11u,"HEADER:");
    XTOOLS_WordHex(hx,EepromLogicalWord(g_eeprom_words[0x00u]));
    LCD_Print(52u,11u,hx);

    LCD_Print(112u,11u,"COPY:");
    LCD_Print(148u,11u,copy_ok ? "OK" : "FAULT");

    LCD_Print(4u,23u,"EEPROM SN:");
    Dec16(esn,sn);
    LCD_Print(70u,23u,sn);

    LCD_Print(112u,23u,"TK SN:");
    Dec16(tksn,sn);
    LCD_Print(154u,23u,sn);

    LCD_Print(4u,35u,"TK COPY:");
    LCD_Print(58u,35u,tk_ok ? "OK" : "FAULT");

    LCD_Print(112u,35u,"MATCH:");
    LCD_Print(154u,35u,match_ok ? "OK" : "FAULT");

    if(g_eeprom_sync_status==1u) LCD_PrintInverse(4u,45u,"SYNC EEPROM -> TK OK");
    else if(g_eeprom_sync_status==2u) LCD_PrintInverse(4u,45u,"SYNC FAULT");
    else LCD_Print(4u,45u,"128 BYTE READ OK");

    EepromSoftkeys();
}

void GUI_EepromDumpFirst(void){eeprom_dump_page=0u;}
#if 0 /* UNUSED L16: GUI_EepromDumpNext; retained for future work */
void GUI_EepromDumpNext(void){if(eeprom_dump_page<2u)eeprom_dump_page++;else eeprom_dump_page=0u;}
#endif /* UNUSED: GUI_EepromDumpNext */
void GUI_EepromDumpPrev(void){if(eeprom_dump_page>0u)eeprom_dump_page--;else eeprom_dump_page=2u;}

static void PageUserErase(void)
{
    LCD_Rect(0u,0u,239u,63u);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,"TIMEKEEPER ERASE");
    LCD_Print(4u,14u,"FILL 8000-9FF7 = 55");
    LCD_Print(4u,26u,"RTC  9FF8-9FFF SAFE");
    if(g_timekeeper_erase_status==1u) LCD_PrintInverse(4u,38u,"ERASE OK - RESTART");
    else if(g_timekeeper_erase_status==2u) LCD_PrintInverse(4u,38u,"ERASE FAULT");
    else LCD_Print(4u,38u,"ENTER ERASE / ESC CANCEL");
}

static void SnFive(char *s,unsigned int v)
{
    s[0]=(char)('0'+((v/10000u)%10u));
    s[1]=(char)('0'+((v/1000u)%10u));
    s[2]=(char)('0'+((v/100u)%10u));
    s[3]=(char)('0'+((v/10u)%10u));
    s[4]=(char)('0'+(v%10u));
    s[5]=0;
}

static void PageEepromSn(void)
{
    char s[6],one[2];
    unsigned int current;
    unsigned char i;
    LCD_Rect(0u,0u,239u,63u);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,"EEPROM SERIAL EDIT");
    current=EepromLogicalWord(g_eeprom_words[0x06u]);
    LCD_Print(4u,11u,"SN:"); SnFive(s,current); LCD_Print(28u,11u,s);
    LCD_Print(70u,11u,"NEW:"); SnFive(s,g_eeprom_sn_edit);
    for(i=0u;i<5u;i++) { one[0]=s[i]; one[1]=0; if(i==g_eeprom_sn_digit) LCD_PrintInverse((unsigned int)(100u+i*6u),11u,one); else LCD_Print((unsigned int)(100u+i*6u),11u,one); }
    LCD_Print(145u,11u,"F2 DIGIT +/-");

    LCD_Print(4u,27u,"F2 SELECT DIGIT   +/- CHANGE");
    LCD_Print(4u,39u,"ENTER WRITE SERIAL NUMBER");
    if(g_eeprom_sn_write_status==1u)
    {
        LCD_PrintInverse(4u,55u,"ENTER WRITE");
        LCD_Print(100u,55u,"ESC CANCEL");
    }
    else if(g_eeprom_sn_write_status==2u) LCD_PrintInverse(4u,55u,"SN WRITE OK");
    else if(g_eeprom_sn_write_status==3u) LCD_PrintInverse(4u,55u,"SN WRITE FAULT");
    else
    {
        LCD_Print(4u,55u,"ENTER SAVE");
        LCD_Print(184u,55u,"ESC BACK");
    }
}

static void PageEepromErase(void)
{
    LCD_Rect(0u,0u,239u,63u);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,"EEPROM ERASE");
    LCD_Print(4u,14u,"ALL 64 WORDS -> FFFF");
    LCD_Print(4u,26u,"SERIAL NUMBER WILL ERASE");
    LCD_Print(4u,36u,"TIMEKEEPER NOT TOUCHED");
    if(g_eeprom_erase_status==1u) LCD_PrintInverse(4u,47u,"ERASE OK");
    else if(g_eeprom_erase_status==2u) LCD_PrintInverse(4u,47u,"ERASE FAULT");
    else LCD_Print(4u,47u,"ENTER ERASE / ESC CANCEL");
    LCD_Print(184u,55u,"ESC BACK");
}

static void PageEepromDump(void)
{
    char a[4],v[5];
    unsigned char row,i,word,x,y;
    static const char code hex[]="0123456789ABCDEF";

    LCD_Rect(0u,0u,239u,63u);
    LCD_FillRect(0u,0u,239u,7u);
    LCD_PrintInverse(2u,0u,"EEPROM COPY1 HEX");

    /*
     * COPY1 = words 01h..15h (21 x 16-bit).
     * Six words per row:
     * 01: xxxx xxxx xxxx xxxx xxxx xxxx
     * 07: xxxx xxxx xxxx xxxx xxxx xxxx
     * 0D: xxxx xxxx xxxx xxxx xxxx xxxx
     * 13: xxxx xxxx xxxx
     *
     * SN is word 06h, therefore its value is drawn inverse.
     */
    for(row=0u;row<4u;row++)
    {
        word=(unsigned char)(0x01u + row*6u);
        if(word>0x15u) break;

        /* Spread four data rows over the available display height. */
        y=(unsigned char)(11u + row*13u);

        a[0]=hex[(word>>4)&0x0Fu];
        a[1]=hex[word&0x0Fu];
        a[2]=':';
        a[3]=0;
        LCD_Print(2u,y,a);

        x=25u;
        for(i=0u;i<6u;i++)
        {
            unsigned char adr=(unsigned char)(word+i);
            if(adr>0x15u) break;

            XTOOLS_WordHex(v,EepromLogicalWord(g_eeprom_words[adr]));

            if(adr==0x06u)
                LCD_PrintInverse(x,y,v);
            else
                LCD_Print(x,y,v);

            x=(unsigned char)(x+34u);
        }
    }

    LCD_Print(184u,55u,"ESC BACK");
}


/* V1.46: CHARGE user-interface only. No power switching from this page. */
static unsigned char charge_selected=0u;
static unsigned char charge_edit=0u;
void GUI_ChargeSelect(void) { charge_selected=(unsigned char)((charge_selected+1u)%3u); }
void GUI_ChargeEdit(void) { charge_edit=(unsigned char)((charge_edit+1u)%4u); }
void GUI_ChargeMode(void) { charge[charge_selected].mode^=1u; }
void GUI_ChargeStartDemo(void) { /* Intentionally no RUN state or hardware enable. */ }
void GUI_ChargeStop(void) { charge[charge_selected].state=CHARGE_STOPPED; }
void GUI_ChargeAdjust(signed char delta)
{
    CHARGE_CHANNEL xdata *c=&charge[charge_selected];
    unsigned long *v=0;
    unsigned int *w=0;
    unsigned long step=1UL,maxv=0UL,minv=0UL,n;
    if(charge_edit==0u) {v=&c->u_set_mv;step=100UL;minv=500UL;maxv=58800UL;}
    else if(charge_edit==1u) {w=&c->i_set_ma;step=10UL;minv=10UL;maxv=5000UL;}
    else if(charge_edit==2u) {w=&c->time_set_min;step=1UL;minv=1UL;maxv=999UL;}
    else {w=&c->cap_set_mah;step=25UL;minv=25UL;maxv=25000UL;}
    n=v?*v:(unsigned long)*w;
    if(delta>0) n=(n+step>maxv)?maxv:n+step;
    else n=(n<=minv+step)?minv:n-step;
    if(v) *v=n; else *w=(unsigned int)n;
}
static void ChargeNum(unsigned long n,unsigned char digits,char *out)
{
    unsigned char i;
    for(i=0u;i<digits;i++) {out[digits-1u-i]=(char)('0'+n%10UL);n/=10UL;}
    out[digits]=0;
}
static void PageCharge(const TIMEKEEPER_INFO *t)
{
    unsigned char i,y;
    char s[9];
    CHARGE_CHANNEL xdata *c;
    ADC_VOLTAGE_RESULT vr;
    ADC_BATT_RESULT bp;
    LCD_PrintInverse(0u,0u,"ISL8-936G XTOOLS");
    LCD_Print(166u,0u,"CHARGE");
    LCD_Print(0u,8u,"BATT:");
    ADC_MeasureBattPlus((signed char)t->batt_tol[0],&bp);
    ChargeNum((unsigned long)bp.mv,5u,s);
    LCD_Print(36u,8u,s);
    LCD_Print(110u,8u,"SCR:--.-");
    for(i=0u;i<3u;i++)
    {
        y=(unsigned char)(16u+i*8u);
        c=&charge[i];
        s[0]='A';s[1]=(char)('1'+i);s[2]=0;
        if(i==charge_selected) LCD_PrintInverse(0u,y,s);else LCD_Print(0u,y,s);
        if(i==0u) ADC_MeasureA1((signed char)t->copy[0].tol_a1,(signed char)t->copy[0].ofs_a1,&vr);
        else if(i==1u) ADC_MeasureA2((signed char)t->copy[0].tol_a2,(signed char)t->copy[0].ofs_a2,&vr);
        else ADC_MeasureA3((signed char)t->copy[0].tol_a3,(signed char)t->copy[0].ofs_a3,&vr);
        ChargeNum(vr.mv_cal>0L?(unsigned long)vr.mv_cal:0UL,5u,s);
        LCD_Print(16u,y,s);
        LCD_Print(46u,y,"mV");
        LCD_Print(72u,y,"I:00000mA");
        LCD_Print(144u,y,"CAP:00000mAh");
    }
    c=&charge[charge_selected];
    LCD_Print(0u,40u,c->mode==CHARGE_CHARGE?"CHARGE":"DISCH ");
    LCD_Print(58u,40u,"U:");ChargeNum(c->u_set_mv,5u,s);LCD_Print(76u,40u,s);
    LCD_Print(121u,40u,"I:");ChargeNum(c->i_set_ma,4u,s);LCD_Print(139u,40u,s);
    LCD_Print(0u,48u,"EDIT:");
    LCD_Print(36u,48u,charge_edit==0u?"VOLT":charge_edit==1u?"CURR":charge_edit==2u?"TIME":"CAP ");
    LCD_Print(95u,48u,"+/- SET");
    LCD_Print(157u,48u,"NO POWER");
    LCD_Print(0u,56u,"F1 SEL F2 EDIT F3 GO F4 STOP F5 MODE");
}
void GUI_DrawPage(GUI_PAGE page,const TIMEKEEPER_INFO *t,unsigned char hw_cursor,
                  unsigned char calib_channel,unsigned char calib_edit,unsigned char time_field)
{
    LCD_ClearBuffer();
    switch(page)
    {
        case PAGE_STATUS:PageStatus(t);break;
        case PAGE_CHARGE:PageCharge(t);break;
        case PAGE_CALIB:PageCalibration(t,calib_channel,calib_edit);break;
        case PAGE_USER:PageUser(t);break;
        case PAGE_USER_ERASE:PageUserErase();break;
        case PAGE_TIME:PageTime(t,time_field);break;
        case PAGE_MEASUREMENTS:PageMeasurements(t);break;
        case PAGE_A2_TEST:PageA2Test(t);break;
        case PAGE_A1_TEST:PageA1Test(t);break;
        case PAGE_HARDWARE:PageHardware(t);break;
        case PAGE_HW_TEST:PageHWTest(t,hw_cursor);break;
        case PAGE_TIMEKEEPER:PageTimekeeper(t);break;
        case PAGE_EEPROM:PageEeprom(t);break;
        case PAGE_EEPROM_DUMP:PageEepromDump();break;
        case PAGE_EEPROM_SN:PageEepromSn();break;
        case PAGE_EEPROM_ERASE:PageEepromErase();break;
        default:PageStatus(t);break;
    }
    if(page==PAGE_A2_TEST) LCD_RefreshDiff();
    else LCD_Refresh();
}
