#include "hardware.h"

void HW_Delay(void)
{
    volatile unsigned int i;
    for (i = 0; i < 500u; i++) { ; }
}

void HW_Init(void)
{
    P7 = 0xFF;
    P6 = 0x80;
    P1 = 0x11;
    P3 = 0xFF;
    P4 = 0xCF;
    P5 = 0x00;

    /* Known safe ISL8 latch state. */
    OE_373BC = 1;

    P5 = 0x01;
    P6 |= LATCH_B;
    HW_Delay();
    P6 &= (unsigned char)~LATCH_B;

    P5 = 0xFF;
    P6 |= LATCH_C;
    HW_Delay();
    P6 &= (unsigned char)~LATCH_C;

    /* V1.47: keep shared LATCH B/C output enable asserted for HW TEST,
     * A1 and A2 operation. P4.5=1 is the operator-requested test polarity.
     * Verify physical OE polarity on hardware before power testing. */
    OE_373BC = 1;
    P5 = 0x00;
}

/* -------------------------------------------------------------------------
 * TEST HW latch control
 *
 * Latch shadows are mandatory because P5 is shared with the LCD data bus.
 * Only the requested test bits are changed; all other latch bits retain
 * their previous state.
 *
 * Initial shadows match HW_Init(): B=01h, C=FFh.
 * Active levels follow the verified/supplied ISL8 hardware mapping:
 *   B0 FAN INT  : active LOW
 *   B2 FAN OUT  : active HIGH
 *   B3 BLINK    : active HIGH
 *   B4 DISCH RES: active HIGH (safe startup state is LOW)
 *   C6 TEST 5V A2: active LOW (safe startup state is HIGH)
 *   C7 TEST 5V A1: active LOW (safe startup state is HIGH)
 * ------------------------------------------------------------------------- */
static unsigned char latch_b_shadow=0x01u;
static unsigned char latch_c_shadow=0xFFu;

static void HW_WriteLatchB(void)
{
    unsigned char et=ET0; ET0=0;
    P5=latch_b_shadow;
    P6|=LATCH_B;
    HW_Delay();
    P6&=(unsigned char)~LATCH_B;
    P5=0x00u; ET0=et;
}

static void HW_WriteLatchC(void)
{
    unsigned char et=ET0; ET0=0;
    P5=latch_c_shadow;
    P6|=LATCH_C;
    HW_Delay();
    P6&=(unsigned char)~LATCH_C;
    P5=0x00u; ET0=et;
}
/* Called from Timer0 ISR or with ET0 disabled. C6/C7 only. */
void HW_Test5V_ApplyISR(unsigned char active_mask)
{
    unsigned char old_p5=P5;
    latch_c_shadow|=0xC0u;
    if(active_mask&1u) latch_c_shadow&=(unsigned char)~0x80u;
    if(active_mask&2u) latch_c_shadow&=(unsigned char)~0x40u;
    P5=latch_c_shadow;
    P6|=LATCH_C;
    P6&=(unsigned char)~LATCH_C;
    P5=old_p5;
}

void HW_TestSet(unsigned char function_id,unsigned char on)
{
    unsigned char mask;
    unsigned char active_low;
    unsigned char use_c;
    unsigned char et;

    use_c=0u;
    active_low=0u;
    mask=0u;

    switch(function_id)
    {
        case HWTEST_FAN_INT:   mask=0x01u; active_low=1u; break;
        case HWTEST_FAN_OUT:   mask=0x04u; break;
        case HWTEST_BLINK:     mask=0x08u; break;
        case HWTEST_DISCH_RES: mask=0x10u; break;
        case HWTEST_5V_A1:     mask=0x80u; active_low=1u; use_c=1u; break;
        case HWTEST_5V_A2:     mask=0x40u; active_low=1u; use_c=1u; break;
        default: return;
    }

    et=ET0; ET0=0;
    if(use_c)
    {
        if((on && !active_low) || (!on && active_low)) latch_c_shadow|=mask;
        else latch_c_shadow&=(unsigned char)~mask;
        HW_WriteLatchC();
    }
    else
    {
        if((on && !active_low) || (!on && active_low)) latch_b_shadow|=mask;
        else latch_b_shadow&=(unsigned char)~mask;
        HW_WriteLatchB();
    }
    ET0=et;
}

unsigned char HW_TestGet(unsigned char function_id)
{
    unsigned char mask;
    unsigned char active_low;
    unsigned char v;

    active_low=0u;
    if(function_id==HWTEST_FAN_INT) { mask=0x01u; active_low=1u; }
    else if(function_id==HWTEST_FAN_OUT) mask=0x04u;
    else if(function_id==HWTEST_BLINK) mask=0x08u;
    else if(function_id==HWTEST_DISCH_RES) mask=0x10u;
    else if(function_id==HWTEST_5V_A1) { mask=0x80u; active_low=1u; }
    else if(function_id==HWTEST_5V_A2) { mask=0x40u; active_low=1u; }
    else return 0u;

    if(function_id>=HWTEST_5V_A1) v=(unsigned char)(latch_c_shadow&mask);
    else v=(unsigned char)(latch_b_shadow&mask);

    if(active_low) return v?0u:1u;
    return v?1u:0u;
}

void HW_TestAllOff(void)
{
    /* B: FAN INT=1, FAN OUT=0, BLINK=0, DISCH=0. Preserve B1/B5/B6/B7. */
    latch_b_shadow&=(unsigned char)~(0x04u|0x08u|0x10u);
    latch_b_shadow|=0x01u;
    HW_WriteLatchB();

    /* C: TEST 5V A1/A2 OFF = HIGH. Preserve C0..C5 exactly. */
    latch_c_shadow|=(0x40u|0x80u);
    HW_WriteLatchC();
}


/* -------------------------------------------------------------------------
 * A3 SERVICE POWER TEST
 *
 * Physical mapping confirmed on ISL8:
 *   Latch C / P5.1 = ENABLE CHARGE A3
 *   Latch C / P5.2 = ENABLE DISCHARGE A3
 * Original ROM semantics confirmed: A_FFE1 SETS the selected latch-C bit.
 * Original zero/OFF path A_8884..A_888F applies FFE1 to BOTH 04h and 02h.
 * Therefore the safe A3 state is C1=C2=HIGH; an active direction is selected
 * only by clearing exactly one of those bits (FFE6-equivalent).
 *
 * Timer2/CC3 values follow the original ROM initialization:
 *   reload FBFFh, initial CC3 FFFFh, T2CON=11h.
 * Once the A3 regulator is active, original 0 mA resolves to CC3=FBFFh
 * with CCEN.7 left enabled: latch HIGH/HIGH is the real power OFF state
 * while P1.3 keeps producing the minimum compare pulse.
 * ------------------------------------------------------------------------- */
void HW_A3AllOff(void)
{
    /* Full A3 shutdown, used when entering/leaving the service page.
     * Exact original FFE1/FFE1-equivalent power state: BOTH bits HIGH. */
    latch_c_shadow|=0x06u;
    HW_WriteLatchC();
    CCH3=0xFFu;
    CCL3=0xFFu;
    CCEN&=0x7Fu;
}

void HW_A3PowerOffKeepPWM(void)
{
    /* Original A3 zero-current state: power paths HIGH/HIGH first, then
     * CC3=FBFFh while Timer2/CC3 remains enabled.  Scope measurement on the
     * original charger confirms a continuous extremely narrow LOW pulse in
     * OFF state rather than a static output. */
    latch_c_shadow|=0x06u;
    HW_WriteLatchC();
    CCH3=0xFBu;
    CCL3=0xFFu;
    CCEN|=0x80u;
}

void HW_A3SetDirection(unsigned char discharge)
{
    /* Exact original transition principle: first FFE1 both bits (HIGH/OFF),
       then FFE6 only the requested direction (clear one bit). */
    latch_c_shadow|=0x06u;
    HW_WriteLatchC();

    /* Physical first-power test V1.24 proved the logical functions are reversed:
     * UI CHARGE was physically discharging. Swap the two A3 enables here. */
    if(discharge) latch_c_shadow&=(unsigned char)~0x02u;
    else          latch_c_shadow&=(unsigned char)~0x04u;
    HW_WriteLatchC();
}

void HW_A3PWMInit(void)
{
    /* Service A3 page starts directly in the original regulator 0 mA
     * compare state.  Do not start the power stage from FFFFh. */
    CCH3=0xFBu;
    CCL3=0xFFu;
    CRCH=0xFBu;
    CRCL=0xFFu;
    T2CON=0x11u;
    CCEN|=0x80u;
}

void HW_A3SetCompare(unsigned int compare_value)
{
    CCH3=(unsigned char)(compare_value>>8);
    CCL3=(unsigned char)compare_value;
}

/* A2 scope-only test. Latch C bit 3 stays HIGH/OFF. */
void HW_A2AllOff(void)
{
    latch_c_shadow|=0x08u;
    HW_WriteLatchC();
    CCEN&=(unsigned char)~0x20u;
    CCH2=0xFFu; CCL2=0xFFu;
}
void HW_A2PWMInit(void)
{
    latch_c_shadow|=0x08u;
    HW_WriteLatchC();
    CCH2=0xFBu; CCL2=0xFFu; /* A3-matched 0 mA idle compare, scope test */
    CRCH=0xFBu; CRCL=0xFFu;
    T2CON=0x11u;
    CCEN|=0x20u;
}
void HW_A2SetCompare(unsigned int value)
{
    /* PWM register update only. Power latch is controlled separately.
     * In this preflight build the A2 latch remains OFF at all times. */
    CCH2=(unsigned char)(value>>8);
    CCL2=(unsigned char)value;
}

/* A2 latch C3 active LOW, as physically confirmed by the operator.
 * ALWAYS set compare to idle and settle before enabling power. */
void HW_A2Enable(unsigned char on)
{
    /* A2 C3 active LOW: ON=LOW, OFF=HIGH. */
    if(on) latch_c_shadow&=(unsigned char)~0x08u;
    else latch_c_shadow|=0x08u;
    HW_WriteLatchC();
}

/* A1 CHARGE test: swapped C4/C5 mapping. C4 LOW enables CHARGE.
   C0 (RECUP) and C5 (RES) remain HIGH. All A1 outputs active LOW. */
void HW_A1AllOff(void)
{
    latch_c_shadow|=0x31u;
    HW_WriteLatchC();
    CCEN&=(unsigned char)~0x08u; /* CC1 compare enable */
    CCH1=0xFFu; CCL1=0xFFu;
}
void HW_A1PWMInit(void)
{
    latch_c_shadow|=0x31u;
    HW_WriteLatchC();
    CCH1=0xFBu; CCL1=0xFFu;
    CRCH=0xFBu; CRCL=0xFFu;
    T2CON=0x11u;
    CCEN|=0x08u;
}
void HW_A1SetCompare(unsigned int value)
{
    CCH1=(unsigned char)(value>>8);
    CCL1=(unsigned char)value;
}
void HW_A1ChargeEnable(unsigned char on)
{
    latch_c_shadow|=0x31u;
    HW_WriteLatchC();
    if(on) latch_c_shadow&=(unsigned char)~0x10u;
    HW_WriteLatchC();
}

/* A1 recuperation: C0 LOW, all other A1 power paths HIGH. */
void HW_A1RecupEnable(unsigned char on)
{
    latch_c_shadow |= 0x31u;
    HW_WriteLatchC();
    if(on) latch_c_shadow &= (unsigned char)~0x01u;
    HW_WriteLatchC();
}

/* A1 internal resistor discharge: C5 LOW; C4 CHARGE/C0 RECUP HIGH. */
void HW_A1DischResEnable(unsigned char on)
{
    latch_c_shadow |= 0x31u;
    HW_WriteLatchC();
    if(on) latch_c_shadow &= (unsigned char)~0x20u;
    HW_WriteLatchC();
}
