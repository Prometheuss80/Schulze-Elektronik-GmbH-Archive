#include <REG80C537_ISL8.H>
#include "adc.h"
#include "serial.h"
#include "watchdog.h"
#include "timekeeper.h"

/*
 * ISL8 ADC reconstruction.
 *
 * Original ROM anchors:
 *   A_77D3 = channel select + 3x A_E181 + normal-channel filter
 *   A_E181 = 32-sample adaptive DAPR conversion
 *   A_796F = A1 voltage conversion, K=0x4580 = 17792
 *   A_79BC = A2 voltage conversion, K=0x3A2F = 14895
 *   A_7A11 = A3 voltage conversion, K=0x0F34 = 3892
 *
 * V1.09: A3 path keeps the original 32-bit D5F6 arithmetic order.
 *
 * V1.04 test purpose:
 *   Compare all three A_E181 results, their median/filter result, and the
 *   reconstructed Schulze integer conversion against the original firmware.
 *   No charger power-stage outputs are touched here.
 */

#define ADC_K_A1 17792UL
#define ADC_K_A2 14895UL
#define ADC_K_A3  3888UL

static void ADC_Wait(void)
{
    while(BSY) WDT_Kick();
}

void ADC_Init(void)
{
    IEN1 &= 0xFEu;
    ADC_Wait();
    ADCON0 &= 0x3Fu;
}

static void ADC_SelectChannel(unsigned char channel)
{
    IEN1 &= 0xFEu;
    ADC_Wait();
    ADCON0 &= 0x80u;
    ADCON1 &= 0x80u;
    ADCON1 |= (unsigned char)(channel & 0x0Fu);
}

static unsigned int ADC_E181(ADC_DEBUG *dbg)
{
    unsigned char i,coarse,range,dapr_value,fine;
    unsigned int value,sum=0u,raw;

    /* Channel is selected ONCE by the A_77D3-level caller, exactly as ROM. */
    for(i=0u;i<32u;i++)
    {
        DAPR=0x00u;
        ADC_Wait();
        coarse=ADDAT;

        if(coarse<0x18u) range=0u;
        else
        {
            range=(unsigned char)((coarse-8u)>>4);
            if(range>=0x0Eu) range=0x0Eu;
        }

        dapr_value=(unsigned char)((unsigned char)(range*0x11u)+0x20u);
        DAPR=dapr_value;
        ADC_Wait();
        fine=ADDAT;

        value=(unsigned int)range*128u+(unsigned int)fine;
        sum+=value;

        if(dbg!=0)
        {
            dbg->coarse[i]=coarse;
            dbg->range[i]=range;
            dbg->dapr[i]=dapr_value;
            dbg->fine[i]=fine;
        }
        WDT_Kick();
    }

    if(sum<0x25u) raw=0u;
    else raw=(unsigned int)((sum-0x25u)>>4);

    if(dbg!=0) { dbg->sum=sum; dbg->raw=raw; }
    return raw;
}

static unsigned int Median3(unsigned int a,unsigned int b,unsigned int c)
{
    unsigned int t;
    if(a>b){t=a;a=b;b=t;}
    if(b>c){t=b;b=c;c=t;}
    if(a>b){t=a;a=b;b=t;}
    return b;
}

unsigned int ADC_Measure(unsigned char channel)
{
    unsigned int a,b,c;
    /* ROM A_77D3: select channel once, then call A_E181 three times. */
    ADC_SelectChannel(channel);
    a=ADC_E181(0);
    b=ADC_E181(0);
    c=ADC_E181(0);
    return Median3(a,b,c);
}

/* Complete voltage paths used by the original service/calibration display.
 *
 * A1/A2:
 *   A_9C23 -> 3 complete A_77D3 results.
 *   Each A_77D3 does 3x A_E181, selects the middle ADC result and then
 *   performs the channel conversion (including TOL/OFS).  A_9C23 selects
 *   the middle of the three already-converted results.
 *
 * A3 is deliberately NOT routed through A_E181 here.  The original
 * service path A_4FBF(index 2) selects CH7 and calls A_D5F6: 128 adaptive
 * samples, subtract 0x81 from the accumulated value, multiply by 0x0F30
 * (3888), shift right 16, then apply TOL/OFS.
 */

static signed long ApplyCal(signed long mv,signed char tol,signed char ofs)
{
    signed long v;
    v=(mv*(1000L-(signed long)tol))/1000L;

    /* ROM A_501E..A_504B: positive OFS cannot drive A3 below zero.
       Using the same saturation for all voltage channels is harmless and
       matches the intended signed-offset semantics. */
    if(ofs>=0 && v<(signed long)ofs) return 0L;
    return v-(signed long)ofs;
}

static signed long Median3SL(signed long a,signed long b,signed long c)
{
    signed long t;
    if(a>b){t=a;a=b;b=t;}
    if(b>c){t=b;b=c;c=t;}
    if(a>b){t=a;a=b;b=t;}
    return b;
}

static unsigned int Measure77D3Raw(unsigned char channel)
{
    unsigned int a,b,c;
    ADC_SelectChannel(channel);
    a=ADC_E181(0);
    b=ADC_E181(0);
    c=ADC_E181(0);
    return Median3(a,b,c);
}

static signed long ConvertRawMv(unsigned int raw,unsigned long k,
                                signed char tol,signed char ofs)
{
    signed long base=((signed long)((unsigned long)raw*k))/1024L;
    return ApplyCal(base,tol,ofs);
}

static void MeasureA12(unsigned char channel,unsigned long k,
                       signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r)
{
    unsigned int raw1,raw2,raw3,raw_mid;
    signed long u1,u2,u3,c1,c2,c3;

    raw1=Measure77D3Raw(channel);
    raw2=Measure77D3Raw(channel);
    raw3=Measure77D3Raw(channel);
    raw_mid=Median3(raw1,raw2,raw3);

    /* No-calibration value: same three complete groups, conversion before
       the outer A_9C23 middle selection. */
    u1=ConvertRawMv(raw1,k,0,0);
    u2=ConvertRawMv(raw2,k,0,0);
    u3=ConvertRawMv(raw3,k,0,0);

    c1=ConvertRawMv(raw1,k,tol,ofs);
    c2=ConvertRawMv(raw2,k,tol,ofs);
    c3=ConvertRawMv(raw3,k,tol,ofs);

    r->raw=raw_mid;
    r->mv_uncal=Median3SL(u1,u2,u3);
    r->mv_cal=Median3SL(c1,c2,c3);
}

static unsigned long ADC_D5F6_Sum128(void)
{
    unsigned char i,coarse,range,dapr_value,fine;
    unsigned int value;
    unsigned long sum=0UL;

    ADC_SelectChannel(ADC_CH_A3_VOLTAGE);

    for(i=0u;i<128u;i++)
    {
        DAPR=0x00u;
        ADC_Wait();
        coarse=ADDAT;

        if(coarse<0x18u) range=0u;
        else
        {
            range=(unsigned char)((coarse-8u)>>4);
            if(range>=0x0Eu) range=0x0Eu;
        }

        dapr_value=(unsigned char)((unsigned char)(range*0x11u)+0x20u);
        DAPR=dapr_value;
        ADC_Wait();
        fine=ADDAT;

        value=(unsigned int)range*128u+(unsigned int)fine;
        sum+=(unsigned long)value;
        WDT_Kick();
    }

    /* A_D5F6: if accumulator >= 0x81 return accumulator-0x81, else 0. */
    if(sum>=0x81UL) sum-=0x81UL;
    else sum=0UL;
    return sum;
}

void ADC_MeasureA1(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r)
{
    MeasureA12(ADC_CH_A1_VOLTAGE,ADC_K_A1,tol,ofs,r);
}

void ADC_MeasureA2(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r)
{
    MeasureA12(ADC_CH_A2_VOLTAGE,ADC_K_A2,tol,ofs,r);
}


/* A3 current / P8.3 / ADC11.
 *
 * Original ROM anchor:
 *   A_984E: when A3 is active -> R7=#0Bh -> A_9C23
 *   result is stored to 5CE0:5CE1 and copied to 002B:002C.
 *
 * A_9C23 performs three complete A_77D3 measurements and selects the
 * middle result.  Each reconstructed Measure77D3Raw() already performs
 * the channel select + 3x A_E181 + middle selection.
 *
 * IMPORTANT: this function deliberately returns the original internal
 * current value.  Do not label it mA in protection code until the scale
 * is verified on hardware.
 */
unsigned int ADC_MeasureA3Current(void)
{
    unsigned int a,b,c;
    a=Measure77D3Raw(ADC_CH_A3_CURRENT);
    b=Measure77D3Raw(ADC_CH_A3_CURRENT);
    c=Measure77D3Raw(ADC_CH_A3_CURRENT);
    return Median3(a,b,c);
}

/* BATT+ / P7.3 / ADC channel 3.
 *
 * Original ROM path:
 *   A_9C23 -> A_77D3 -> A_790D -> A_7A05
 *
 * A_790D first applies the signed byte at 0x5D7F as the factor
 *     raw2 = raw * (1000-factor) / 1000
 * then A_7A05 converts with K=0x0F34=3892 and /1024.
 *
 * Channel 3 has its own A_9C23 filtering when bit 0x51 is clear:
 *   >=10000 mV : one complete A_77D3 result
 *    6000..9999 : average of two complete converted results
 *   < 6000 mV  : average of four complete converted results
 *
 * The third Timekeeper byte was historically labelled ZERO. Hardware tests
 * confirm Schulze uses it as the BATT+ TOL factor. BATT+ OFS is FIX0.
 */
#define ADC_K_BATT_PLUS 3892UL

unsigned int ADC_BattPlusRawToMv(unsigned int raw,signed char factor)
{
    signed long scaled;
    unsigned long mv;

    scaled=((signed long)raw*(1000L-(signed long)factor))/1000L;
    if(scaled<0L) scaled=0L;
    mv=((unsigned long)scaled*ADC_K_BATT_PLUS)/1024UL;
    if(mv>65535UL) mv=65535UL;
    return (unsigned int)mv;
}

void ADC_MeasureBattPlus(signed char batt_factor,ADC_BATT_RESULT *r)
{
    unsigned int raw1,raw2,raw3,raw4;
    unsigned int mv1,mv2,mv3,mv4;
    unsigned long sum;

    raw1=Measure77D3Raw(ADC_CH_BATT_PLUS);
    mv1=ADC_BattPlusRawToMv(raw1,batt_factor);

    if(mv1>=10000u)
    {
        r->raw=raw1;
        r->mv=mv1;
        return;
    }

    raw2=Measure77D3Raw(ADC_CH_BATT_PLUS);
    mv2=ADC_BattPlusRawToMv(raw2,batt_factor);

    if(mv1>=6000u)
    {
        r->raw=(unsigned int)(((unsigned long)raw1+(unsigned long)raw2)>>1);
        sum=(unsigned long)mv1+(unsigned long)mv2;
        r->mv=(unsigned int)(sum>>1);
        return;
    }

    raw3=Measure77D3Raw(ADC_CH_BATT_PLUS);
    mv3=ADC_BattPlusRawToMv(raw3,batt_factor);
    raw4=Measure77D3Raw(ADC_CH_BATT_PLUS);
    mv4=ADC_BattPlusRawToMv(raw4,batt_factor);

    sum=(unsigned long)raw1+(unsigned long)raw2+(unsigned long)raw3+(unsigned long)raw4;
    r->raw=(unsigned int)(sum>>2);
    sum=(unsigned long)mv1+(unsigned long)mv2+(unsigned long)mv3+(unsigned long)mv4;
    r->mv=(unsigned int)(sum>>2);
}

/* Last A3 intermediates are retained for the ENTER/UART diagnostic. */
static unsigned long a3_sum_after_81=0UL;
static unsigned long a3_product_low32=0UL;
static unsigned int  a3_base16=0u;
static signed char   a3_last_tol=0;
static signed char   a3_last_ofs=0;
static signed long   a3_last_cal=0L;

/* A_3E87 returns the low 32 bits of the multiply.  unsigned long on C51 is
   32 bit, so the cast/order below intentionally preserves modulo-2^32 math. */
static unsigned long MulLow32(unsigned long a,unsigned long b)
{
    return (unsigned long)(a*b);
}

void ADC_MeasureA3(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r)
{
    unsigned long acc;
    unsigned long product;
    unsigned int base16;
    signed long calibrated;

    /* Exact ROM order:
       D5F6 128-sample 32-bit accumulator -> -0x81 -> *0x0F30
       (low 32 bits) -> logical >>16 -> *(1000-TOL) -> /1000 -> OFS. */
    acc=ADC_D5F6_Sum128();
    product=MulLow32(acc,ADC_K_A3);
    base16=(unsigned int)(product>>16);
    calibrated=ApplyCal((signed long)base16,tol,ofs);

    a3_sum_after_81=acc;
    a3_product_low32=product;
    a3_base16=base16;
    a3_last_tol=tol;
    a3_last_ofs=ofs;
    a3_last_cal=calibrated;

    /* RAW on LCD = low 16 bits of the corrected 32-bit D5F6 accumulator. */
    r->raw=(unsigned int)acc;
    r->mv_uncal=(signed long)base16;
    r->mv_cal=calibrated;
}

#if 0 /* UNUSED L16: ADC_MeasureDebug; retained for future work */
unsigned int ADC_MeasureDebug(unsigned char channel, ADC_DEBUG *dbg)
{
    ADC_SelectChannel(channel);
    return ADC_E181(dbg);
}
#endif /* UNUSED: ADC_MeasureDebug */

/* Reconstructed integer path for A1/A2/A3:
 * BASE = (RAW*K)/1024
 * CAL  = (BASE*(1000-TOL))/1000
 * mV   = CAL-OFS
 * TOL/OFS are signed 8-bit values in the original code.
 */
#if 0 /* UNUSED: legacy ADC serial diagnostic conversion and formatters */
signed long ADC_SchulzeMv(unsigned int raw,unsigned long k,
                          signed char tol,signed char ofs,
                          unsigned long *base_out,signed long *cal_out)
{
    unsigned long base;
    signed long factor;
    signed long cal;

    base=((unsigned long)raw*k)/1024UL;
    factor=1000L-(signed long)tol;
    cal=((signed long)base*factor)/1000L;

    if(base_out!=0) *base_out=base;
    if(cal_out!=0) *cal_out=cal;
    return cal-(signed long)ofs;
}

static char HexDigit(unsigned char v)
{
    v&=0x0Fu;
    return(v<10u)?(char)('0'+v):(char)('A'+v-10u);
}

static void PrintHex8(unsigned char v)
{
    char s[3]; s[0]=HexDigit(v>>4);s[1]=HexDigit(v);s[2]=0;Serial_Print(s);
}

static void PrintDecUL(unsigned long v)
{
    char s[11],t[11]; unsigned char n=0u,i;
    do{t[n++]=(char)('0'+(v%10UL));v/=10UL;}while(v&&n<10u);
    for(i=0u;i<n;i++)s[i]=t[n-1u-i];s[n]=0;Serial_Print(s);
}

static void PrintDecSL(signed long v)
{
    if(v<0L){Serial_PutChar('-');v=-v;}
    PrintDecUL((unsigned long)v);
}

static void PrintDec16(unsigned int v){PrintDecUL((unsigned long)v);}
static void PrintSigned8(signed char v){PrintDecSL((signed long)v);}

static void Print2(unsigned char v)
{
    Serial_PutChar((char)('0'+((v/10u)%10u)));
    Serial_PutChar((char)('0'+(v%10u)));
}

#endif /* UNUSED: legacy ADC serial diagnostic conversion and formatters */

#if 0 /* UNUSED L16: ADC_DumpOne; kept for reference */
static void ADC_DumpOne(const char *name,unsigned char channel,
                        unsigned long k,signed char tol,signed char ofs,
                        unsigned char do_conversion)
{
    static ADC_DEBUG xdata dbg;
    unsigned char i;
    unsigned int r1,r2,r3,filtered;
    unsigned int g2a,g2b,g2c,g2,g3a,g3b,g3c,g3,final9;
    unsigned long base=0UL;
    signed long cal=0L,result=0L;

    /* Parameters kept for the diagnostic API; V1.06 dump intentionally ignores calibration. */
    if ((tol == 127) && (ofs == 127)) result = 0L;

    Serial_Print("\r\n----------------------------------------\r\n");
    Serial_Print(name); Serial_Print("  CH="); PrintDec16(channel);
    Serial_Print("\r\n#  COARSE RANGE DAPR FINE  VALUE\r\n");

    /* Each A_77D3 group selects the channel ONCE, then runs 3x A_E181.
       Do not reprogram ADCON0/ADCON1 between the three E181 calls. */
    ADC_SelectChannel(channel);
    r1=ADC_E181(&dbg);
    r2=ADC_E181(0);
    r3=ADC_E181(0);
    filtered=Median3(r1,r2,r3);

    ADC_SelectChannel(channel);
    g2a=ADC_E181(0); g2b=ADC_E181(0); g2c=ADC_E181(0);
    g2=Median3(g2a,g2b,g2c);

    ADC_SelectChannel(channel);
    g3a=ADC_E181(0); g3b=ADC_E181(0); g3c=ADC_E181(0);
    g3=Median3(g3a,g3b,g3c);
    final9=Median3(filtered,g2,g3);

    for(i=0u;i<32u;i++)
    {
        Print2(i);Serial_Print("   ");
        PrintHex8(dbg.coarse[i]);Serial_Print("     ");
        PrintHex8(dbg.range[i]);Serial_Print("   ");
        PrintHex8(dbg.dapr[i]);Serial_Print("   ");
        PrintHex8(dbg.fine[i]);Serial_Print("    ");
        PrintDec16((unsigned int)dbg.range[i]*128u+(unsigned int)dbg.fine[i]);
        Serial_Print("\r\n");
        WDT_Kick();
    }

    Serial_Print("SUM #1=");PrintDec16(dbg.sum);Serial_Print("\r\n");
    Serial_Print("E181 #1 = ");PrintDec16(r1);Serial_Print("\r\n");
    Serial_Print("E181 #2 = ");PrintDec16(r2);Serial_Print("\r\n");
    Serial_Print("E181 #3 = ");PrintDec16(r3);Serial_Print("\r\n");
    Serial_Print("77D3 #1 = ");PrintDec16(filtered);Serial_Print("\r\n");
    Serial_Print("77D3 #2 E181 = ");PrintDec16(g2a);Serial_Print(" / ");PrintDec16(g2b);Serial_Print(" / ");PrintDec16(g2c);Serial_Print(" -> ");PrintDec16(g2);Serial_Print("\r\n");
    Serial_Print("77D3 #3 E181 = ");PrintDec16(g3a);Serial_Print(" / ");PrintDec16(g3b);Serial_Print(" / ");PrintDec16(g3c);Serial_Print(" -> ");PrintDec16(g3);Serial_Print("\r\n");
    Serial_Print("9C23 TEST = ");PrintDec16(final9);Serial_Print("\r\n");

    if(do_conversion)
    {
        /* V1.06 comparison is intentionally calibration-free. */
        result=ADC_SchulzeMv(final9,k,0,0,&base,&cal);
        Serial_Print("K        = ");PrintDecUL(k);Serial_Print("\r\n");
        Serial_Print("TOL/OFS  = 0/0 (V1.06 TEST)\r\n");
        Serial_Print("BASE     = ");PrintDecUL(base);Serial_Print("\r\n");
        Serial_Print("CAL      = ");PrintDecSL(cal);Serial_Print("\r\n");
        Serial_Print("RESULT   = ");PrintDecSL(result);Serial_Print(" mV\r\n");
    }
}
#endif /* UNUSED: ADC_DumpOne */

#if 0 /* UNUSED L16: ADC_DumpA3Exact; kept for reference */
static void ADC_DumpA3Exact(void)
{
    Serial_Print("\r\n----------------------------------------\r\n");
    Serial_Print("A3 D5F6 EXACT PATH (last LCD measurement)\r\n");
    Serial_Print("SUM128-0x81 = ");PrintDecUL(a3_sum_after_81);Serial_Print("\r\n");
    Serial_Print("LOW16 RAW   = ");PrintDec16((unsigned int)a3_sum_after_81);Serial_Print("\r\n");
    Serial_Print("PRODUCT3888 = ");PrintDecUL(a3_product_low32);Serial_Print("\r\n");
    Serial_Print("BASE >>16   = ");PrintDec16(a3_base16);Serial_Print(" mV\r\n");
    Serial_Print("TOL         = ");PrintDecSL((signed long)a3_last_tol);Serial_Print("\r\n");
    Serial_Print("OFS         = ");PrintDecSL((signed long)a3_last_ofs);Serial_Print("\r\n");
    Serial_Print("CAL RESULT  = ");PrintDecSL(a3_last_cal);Serial_Print(" mV\r\n");
}
#endif /* UNUSED: ADC_DumpA3Exact */

#if 0 /* UNUSED L16: ADC_SendDebugAll; retained for future work */
void ADC_SendDebugAll(void)
{
    static TIMEKEEPER_INFO xdata t;
    signed char a1tol,a1ofs,a2tol,a2ofs,a3tol,a3ofs;

    Timekeeper_ReadInfo(&t);
    a1tol=(signed char)t.copy[0].tol_a1; a1ofs=(signed char)t.copy[0].ofs_a1;
    a2tol=(signed char)t.copy[0].tol_a2; a2ofs=(signed char)t.copy[0].ofs_a2;
    a3tol=(signed char)t.copy[0].tol_a3; a3ofs=(signed char)t.copy[0].ofs_a3;

    Serial_Print("\r\n========================================\r\n");
    Serial_Print("XTOOLS ISL8 ADC DEBUG V1.09\r\n");
    Serial_Print("ROM PATH: A1/A2=9x E181, A3=D5F6 128x\r\n");
    Serial_Print("TEST: ROM ADC CHANNEL-SELECT TIMING, TOL/OFS=0\r\n");
    Serial_Print("========================================\r\n");

    ADC_DumpOne("A1 VOLT",ADC_CH_A1_VOLTAGE,ADC_K_A1,a1tol,a1ofs,1u);
    ADC_DumpOne("A2 VOLT",ADC_CH_A2_VOLTAGE,ADC_K_A2,a2tol,a2ofs,1u);
    ADC_DumpA3Exact();
    ADC_DumpOne("BATT+",ADC_CH_BATT_PLUS,0UL,0,0,0u);
    ADC_DumpOne("BATT-",ADC_CH_BATT_MINUS,0UL,0,0,0u);

    Serial_Print("========================================\r\n");
    Serial_Print("END ADC DEBUG\r\n");
    Serial_Print("========================================\r\n");
}
#endif /* UNUSED: ADC_SendDebugAll */
