/*------------------------------------------------
XTOOLS for charger Schulze Chamaleon ISL8-963G
programed by Prometheuss80
Martin Stransky 1980
https://spirit-pro.com/Martenss/Isl_8/Isl_8.html
project 8/2022-5/2026
------------------------------------------------*/

#include <REG80C537_ISL8.H>
#define XBYTE ((unsigned char volatile xdata *) 0)                  
            
/* PERSONAL ISL8-936G */

//#define	ZERO_WDT   	      P1^4;
//#define	PIEZO	      P4^4;
//#define	OE_373BC	      P4^5;

// KEY F1  = 0x05 / F2 = 0x0B / F3 = 0x03 / F4 = 0x0D / F5 = 0x07
// KEY ESC = 0x06 / ENT = / PLUS = / MINUS = 

// define PORT 6 PINS
#define     E_PIN             0x01
#define	RW_PIN	      0x02
#define     RS_PIN	      0x04
#define	CS_PIN	      0x08
#define	RESET_PIN         0x10
#define	LATCH_B_PIN	      0x20
#define	LATCH_C_PIN	      0x40
#define	FUSE	      0x80

#define     H_PIN             0x02
#define     L_PIN             0x04
#define     A1_PIN            0x10
#define     A2_PIN            0x20
#define     A3_PIN            0x40

#define	TMKPR_R_PIN	      0x40
#define	TMKPR_W_PIN	      0x80
#define	TMKPR_C_PIN	      0x80

#define     LATCH_B_DATA      0x01 //0x01 = OFF / 0x05 = light = ON/ 
#define     LATCH_C_DATA      0xFF

// SAVE CODE AREA ORIGINAL SN and CALIBRATION for SPECIFIC CHARGER

unsigned char  code FW_0  = 0x31;   //FW        1.49
unsigned char  code FW_1  = 0x2E;
unsigned char  code FW_2  = 0x34;
unsigned char  code FW_3  = 0x39;

unsigned char  data SAVE_A1H  = 0x00;
unsigned char  data SAVE_A2H  = 0x00;
unsigned char  data SAVE_A1L  = 0x00;
unsigned char  data SAVE_A2L  = 0x00;
unsigned char  data SAVE_A3L  = 0x00;
unsigned char  data SAVE_A3H  = 0x00;

unsigned long  data  i = 0x00;
unsigned int   data  x = 0x00;
unsigned int   data  w = 0x00;   

unsigned char  data  read_data  = 0x00;
unsigned char  data  write_data = 0x00;

unsigned char  data  timerCount = 0x00;
unsigned char  data  key        = 0x00;

unsigned char  data  setupmenu   = 0x00;
unsigned char  data  setupcalib  = 0x00;

unsigned int   data  dptr_adress  = 0x00;
unsigned char  data  dptr_data    = 0x00;
unsigned int   data  dptr_counter = 0x00;
unsigned char  data  dptr_loop    = 0x00;

unsigned char  data  TMKPR_SETUP = 0x00;
unsigned char  data  TMKPR_CLOCK = 0x00;

unsigned char  data  SN_LOW      = 0x00;
unsigned char  data  SN_HIGH     = 0x00;
unsigned int   data  SN_NUMBER   = 0x00;
unsigned char  data  BYTE_NUMBER = 0x00;
unsigned char  data  NUM_0 = 0x00;
unsigned char  data  NUM_1 = 0x00;
unsigned char  data  NUM_2 = 0x00;
unsigned char  data  NUM_3 = 0x00;
unsigned char  data  NUM_4 = 0x00;
unsigned char  data  HEX_0 = 0x00;
unsigned char  data  HEX_1 = 0x00;

unsigned char  data  CALIB_A1H;
unsigned char  data  CALIB_A2H;
unsigned char  data  CALIB_ZERO;
unsigned char  data  CALIB_A1L;
unsigned char  data  CALIB_A2L;
unsigned char  data  CALIB_A3L;
unsigned char  data  CALIB_A3H;
unsigned char  data  TMKPR_SN_H;
unsigned char  data  TMKPR_SN_L;

/* ------------------------------------------------*/

void MAIN         (void);
void SETUP        (void);
void TIMEKEEPER   (void);
void CALIB        (void);
void EDIT         (void);

void KEY_TMKPR    (void);
void KEY_CALIB    (void);
void KEY_EDIT     (void);

void LCD_INIT     (void);
void LCD_CLR      (void);
void LCD_START    (void);
void LCD_MENU_S   (void);           // START
void LCD_MENU_T   (void);           // TIMEKEEPER
void LCD_MENU_C   (void);           // CALIBRATION
void LCD_MENU_E   (void);           // EDIT

void SOUND        (void);
void SOUND_SHORT  (void);
void LATCH        (void);
void INFO         (void);
void ERASE        (void);
void WRITE        (void);

void E_LOW        (void);
void E_HIGH       (void);
void RW_LOW       (void);
void RW_HIGH      (void);
void RS_LOW       (void);
void RS_HIGH      (void);
void CS_LOW       (void);
void CS_HIGH      (void);
void RESET_LOW    (void);
void RESET_HIGH   (void);
void LATCH_B_LOW  (void);
void LATCH_B_HIGH (void);
void LATCH_C_LOW  (void);
void LATCH_C_HIGH (void);

void TMKPR_R_ON   (void);
void TMKPR_W_ON   (void);
void TMKPR_R_OFF  (void);
void TMKPR_W_OFF  (void);
void TMKPR_STOP   (void);
void TMKPR_START  (void);
void TMKPR_RUN    (void);
void INIT_RTC     (void);
void SN_CONVERT   (void);
void BYTE_CONVERT (void);
void READ_CAL_SN  (void);
void WRITE_CAL_SN (void);
void SAVE_ORIGIN  (void);
void PLUS_CALIB   (void);
void MINUS_CALIB  (void);

void delay_1ms    (void);
void DELAY_1S     (void);
void DELAY_SHORT  (void);

/*------------------------------------------------
MAIN C Function
------------------------------------------------*/
void        MAIN() {
            while (1) {

            if (setupmenu == 0x00)  
            { 
              SETUP       ();                     
            }            
           
            if (setupmenu == 0x01)  
            { 
              TIMEKEEPER  ();                    
            }    
            
            if (setupmenu == 0x02)  
            { 
              CALIB       ();                  
            } 
            
            if (setupmenu == 0x03)  
            { 
              EDIT        ();                                        
            }
            
            }
            }

 /*------------------------------------------------
TIMEKEEPER C Function
------------------------------------------------*/
      
void TIMEKEEPER ()
            { 
            LCD_MENU_T  ();
            KEY_TMKPR   ();           
            } 
            
/*------------------------------------------------
CALIBRATION C Function
------------------------------------------------*/
      
void CALIB ()
            {                          
            LCD_MENU_C  (); 
            KEY_CALIB   ();           
            }    
                         
 /*------------------------------------------------
EDIT C Function
------------------------------------------------*/
      
void EDIT ()
            {                          
            LCD_MENU_E   ();  
            KEY_EDIT     ();            
            }    
                       
/*------------------------------------------------
SETUP C Function
------------------------------------------------*/

void SETUP  ()
            {            
WDT  = 0x00;                                    // WDT disable								
EAL  = 0x01;                                    // enable i n t e r r u p t 
ET0  = 0x01;                                    // enable timer0 i n t e r r u p t
TMOD = 0x01;                                    // mode 0
TH0  = (65535 - 41666) / 256;                   // 1000ms
TL0  = (65535 - 41666) % 256;      
TR0  = 0x01;                                    // enable timer                            
            
DPSEL = 0x00;                                   // DPSEL 0
                       
P1 = 0x11;                                      //* Setup P1 for Input I=input/O=output - 00010001b */
P3 = 0xFF;				//* Setup P3 for Input I=input/O=output - 11101111b */FF		 
P4 = 0xCF;				//* Setup P4 for Input I=input/O=output - 11001111b */
P5 = 0x00;				//* Setup P5 for Input I=input/O=output - 00000000b */
P6 = 0x80;				//* Setup P6 for Input I=input/O=output - 10000000b */
P7 = 0xFF;				//* Setup P7 ADC for Input I=input      - 11111111b */
P8 = 0x0F;				//* Setup P8 ADC for Input I=input      - 00001111b */	
				
LATCH        ();                                 // setup periferie charger                 
READ_CAL_SN  ();                                 // read calib + SN and save to ram cpu
SN_CONVERT   ();                                 // read - decode serial number           
TMKPR_RUN    ();                                 // M48T08 normal mode

LCD_INIT     ();                                 // setup LCD
LCD_CLR      ();                                 // erase LCD  
LCD_START    ();                                 // lcd main logo / lcd "programmed by Prometheus80" / lcd "FW: x.xx"

SOUND_SHORT  ();                                 // sound      
DELAY_SHORT  ();                                 // delay
SOUND_SHORT  ();                                 // sound      
DELAY_SHORT  ();                                 // delay
SOUND_SHORT  ();                                 // sound      
DELAY_SHORT  ();                                 // delay
DELAY_1S      ();                                 // delay 1s

LCD_CLR      ();                                 // NEXT TIMEKEEPER
setupmenu = 0x01;                                // end setup
            }       

/*------------------------------------------------
MAIN C interrupt 1
------------------------------------------------*/

void WATCHDOG(void) interrupt 1
            {
            TH0 = (65535 - 41666) / 256;     // 1000ms
	TL0 = (65535 - 41666) % 256;
	if (++timerCount==16) 
	{
	timerCount = 0x00;
	ZERO_WDT = !ZERO_WDT;
	}
            }

/*------------------------------------------------
KEY C Function
------------------------------------------------*/
      
void KEY_TMKPR ()
            {
            key = P4;    
            key &= ~0x80;        
            key &= ~0x40;     
            key &= ~0x20;
            key &= ~0x10;     
            
            if (key == 0x06)  // ESC
            {
            setupmenu  =  0x00;                                             
            SOUND       (); 
            LCD_CLR      ();           
            }               
                        
            if (key == 0x05)  // F1
            {                          
            INFO ();                                   
            }     

            if (key == 0x0B)  // F2
            {                 
            ERASE ();           
            }

            if (key == 0x03)  // F3
            {                  
            WRITE ();
            } 

            // NEXT CALIB
            if (key == 0x07)  // F5
            {
            setupmenu  =  0x02;  
            SOUND       ();            
            DELAY_1S    ();
            LCD_CLR      ();                                                                              
            }      
            }     

void KEY_CALIB ()
// if press key?
            {
            key = P4;    
            key &= ~0x80;        
            key &= ~0x40;     
            key &= ~0x20;
            key &= ~0x10; 
            
            if (key == 0x06)  // ESC
            { 
            setupmenu  =  0x01;            
            SOUND       (); 
            LCD_CLR      ();                                                                                                  
            }               
                        
            if (key == 0x05)  //F1
            {                  
//          ERASE CALIB 
                        
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// ERASE

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            CALIB_A1H  = 0x00;
            CALIB_A2H  = 0x00;                      
            CALIB_ZERO = 0x00;
            CALIB_A1L  = 0x00; 
            CALIB_A2L  = 0x00;
            CALIB_A3L  = 0x00;
            CALIB_A3H  = 0x00;  
                        
            WRITE_CAL_SN ();                 
            SOUND        ();         // sound 
            LCD_CLR      ();
            }            
      
            if (key == 0x0B) //F2
            {                 
//          DEFAULT CALIB   
        
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// DEFAULT

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x44;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x55;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            CALIB_A1H  = 0x01;
            CALIB_A2H  = 0x01;                      
            CALIB_ZERO = 0x00;
            CALIB_A1L  = 0x01; 
            CALIB_A2L  = 0x01;
            CALIB_A3L  = 0x01;
            CALIB_A3H  = 0x01;
                        
            WRITE_CAL_SN             ();
            SOUND                    ();         // sound   
            LCD_CLR                  ();
            }      
            
            if (key == 0x03) //F3
            {                 
//          ORIGIN CALIB 
                                               
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// ORIGIN

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x47;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
            CALIB_A1H  = XBYTE [0x9C55] ;
            CALIB_A2H  = XBYTE [0x9C56] ;                     
            CALIB_ZERO = 0x00;
            CALIB_A1L  = XBYTE [0x9C57] ;
            CALIB_A2L  = XBYTE [0x9C58] ;
            CALIB_A3L  = XBYTE [0x9C59] ;
            CALIB_A3H  = XBYTE [0x9C5A] ;  
                        
            WRITE_CAL_SN            ();       
            SOUND                   ();         // sound  
            LCD_CLR                 ();            
            }  

            if (key == 0x0D)  // F4
            {                        
            DELAY_1S ();
            SOUND_SHORT (); 
                        
                         if (key == 0x0D)  // F4
                        { 
                        key = P4;    
                        key &= ~0x80;        
                        key &= ~0x40;     
                        key &= ~0x20;
                        key &= ~0x10;                                     
                        DELAY_1S ();
                        SOUND_SHORT (); 
                       
                                    if (key == 0x0D)  // F4
                                    {  
                                    key = P4;    
                                    key &= ~0x80;        
                                    key &= ~0x40;     
                                    key &= ~0x20;
                                    key &= ~0x10;                                                 
                                    DELAY_1S ();
                                    SOUND_SHORT (); 
                                    SAVE_ORIGIN ();                         
                                    }                                                
                        }                                    
            }            
          
            if (key == 0x07)  //F5
            {                 
//          NEXT - EDIT;
            setupmenu  =  0x03;   
            SOUND        ();            
            DELAY_1S     ();
            LCD_CLR      ();                                                                                                              
            }                                                   
            }  
 
void KEY_EDIT ()                  
// EDIT TEXT                          
            {
            key = P4;    
            key &= ~0x80;        
            key &= ~0x40;     
            key &= ~0x20;
            key &= ~0x10;     
            
            if (key == 0x06)  // ESC
            {
            setupmenu  =  0x02;              
            SOUND       (); 
            LCD_CLR      ();                                                     
            }  

            if (key == 0x01)  // PLUS
            {
            PLUS_CALIB  ();                        
            SOUND_SHORT ();
            DELAY_SHORT ();            
            } 

            if (key == 0x09)  // MINUS
            { 
            MINUS_CALIB ();                        
            SOUND_SHORT ();
            DELAY_SHORT ();           
            }             

            if (key == 0x0E)  // ENTER
            {                  
            SOUND_SHORT ();
            DELAY_SHORT ();            
            } 
                        
            if (key == 0x05)  // F1
            {           
              
// SET OFFSET :A:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xDE;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

//A1
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            setupcalib |=  A1_PIN; // H
            setupcalib &= ~A2_PIN; // L
            setupcalib &= ~A3_PIN; // L    

            SOUND_SHORT ();
            DELAY_SHORT ();
            }      
            
            if (key == 0x0B)  // F2
            {          
            
// SET OFFSET :A2:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xDE;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

//A2
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
           
            setupcalib |=  A2_PIN; // H
            setupcalib &= ~A1_PIN; // L
            setupcalib &= ~A3_PIN; // L    

            SOUND_SHORT (); 
            DELAY_SHORT ();            
            }  
      
            if (key == 0x03)  // F3
              {          
            
// SET OFFSET :A3:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xDE;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

//A2
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
            setupcalib |=  A3_PIN; // H
            setupcalib &= ~A2_PIN; // L
            setupcalib &= ~A1_PIN; // L    

            SOUND_SHORT ();
            DELAY_SHORT ();            
            }     
      
            if (key == 0x0D)  // F4
            {     
                        
// SET OFFSET :H:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xE0;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

//-H
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            setupcalib |=  H_PIN; // H
            setupcalib &= ~L_PIN; // L   

            SOUND_SHORT ();
            DELAY_SHORT ();            
            }    
      
            if (key == 0x07)   // F5
            {     
                        
// SET OFFSET :L:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xE0;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

//-L
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
            setupcalib |=  L_PIN; // H
            setupcalib &= ~H_PIN; // L   
            
            SOUND_SHORT (); 
            DELAY_SHORT ();            
            }       
            }   
 
/*------------------------------------------------
ROUTINE C Function
------------------------------------------------*/
            
void SOUND_SHORT ()
            {           
            for ( x = 0; x < 30; x++)                  
            {
            PIEZO = !PIEZO;  
            delay_1ms ();
            }            
            PIEZO = 0;        
            }
 
void SOUND ()
            {           
            for ( x = 0; x < 250; x++)                
            {
            PIEZO = !PIEZO;  
            delay_1ms ();
            }            
            PIEZO = 0;        
            }
           
void LATCH  ()                            // setup charger internal periferie
            {
                  
OE_373BC = 1;
                  
P5 = LATCH_B_DATA;
LATCH_B_HIGH ();
delay_1ms ();
LATCH_B_LOW ();
delay_1ms ();

P5 = LATCH_C_DATA;
LATCH_C_HIGH ();
delay_1ms ();
LATCH_C_LOW (); 
delay_1ms (); 

OE_373BC = 0;                  

P5 = 0x00;                                    
            }
             
/*------------------------------------------------
DELAY C Function
------------------------------------------------*/
            
void delay_1ms()
	{
	for (i=0; i<50; i++);  //19 - 1ms
	}
 
void DELAY_1S ()
	{
            for (i=0; i<21725; i++);
            }
            
void DELAY_SHORT ()
	{
            for (i=0; i<7000; i++); // 330ms
            }
 
/*------------------------------------------------
READ and WRITE C Function
------------------------------------------------*/
 
void SAVE_ORIGIN ()          
            {
XBYTE [0x9C55] = CALIB_A1H;
XBYTE [0x9C56] = CALIB_A2H;                       
XBYTE [0x9C57] = CALIB_A1L; 
XBYTE [0x9C58] = CALIB_A2L;
XBYTE [0x9C59] = CALIB_A3L;
XBYTE [0x9C5A] = CALIB_A3H; 
                        
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// SAVE
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            SOUND         ();         // sound  
            LCD_CLR       ();         // clear lcd               
            }
           
void WRITE_CAL_SN ()
            {
XBYTE [0x9F82] = TMKPR_SN_H;
XBYTE [0x9F83] = TMKPR_SN_L;                        
                        
XBYTE [0x9F9D] = CALIB_A1H;
XBYTE [0x9F9E] = CALIB_A2H;                       
XBYTE [0x9F9F] = CALIB_ZERO;
XBYTE [0x9FA0] = CALIB_A1L; 
XBYTE [0x9FA1] = CALIB_A2L;
XBYTE [0x9FA2] = CALIB_A3L;
XBYTE [0x9FA3] = CALIB_A3H;   
                        
XBYTE [0x9FAC] = TMKPR_SN_H;
XBYTE [0x9FAD] = TMKPR_SN_L; 

XBYTE [0x9FC7] = CALIB_A1H;
XBYTE [0x9FC8] = CALIB_A2H;                       
XBYTE [0x9FC9] = CALIB_ZERO;
XBYTE [0x9FCA] = CALIB_A1L; 
XBYTE [0x9FCB] = CALIB_A2L;
XBYTE [0x9FCC] = CALIB_A3L;
XBYTE [0x9FCD] = CALIB_A3H; 
                        
XBYTE [0x9FD6] = TMKPR_SN_H;
XBYTE [0x9FD7] = TMKPR_SN_L;
                        
XBYTE [0x9FF1] = CALIB_A1H;
XBYTE [0x9FF2] = CALIB_A2H;                       
XBYTE [0x9FF3] = CALIB_ZERO;
XBYTE [0x9FF4] = CALIB_A1L; 
XBYTE [0x9FF5] = CALIB_A2L;
XBYTE [0x9FF6] = CALIB_A3L;
XBYTE [0x9FF7] = CALIB_A3H;     
            }
           
void READ_CAL_SN ()
            {
CALIB_A1H  = XBYTE [0x9F9D];
CALIB_A2H  = XBYTE [0x9F9E];
CALIB_ZERO = XBYTE [0x9F9F];
CALIB_A1L  = XBYTE [0x9FA0];
CALIB_A2L  = XBYTE [0x9FA1];
CALIB_A3L  = XBYTE [0x9FA2];
CALIB_A3H  = XBYTE [0x9FA3];  
TMKPR_SN_H = XBYTE [0x9F82]; 
TMKPR_SN_L = XBYTE [0x9F83];    
            }            
      
void SN_CONVERT ()  
      {                 
    SN_LOW  = XBYTE [0x9F83];            
    SN_HIGH = XBYTE [0x9F82];  

    SN_NUMBER = (SN_HIGH << 8) | SN_LOW;                  
                  
    NUM_0 = SN_NUMBER % 10;  
    SN_NUMBER /= 10;
    NUM_1 = SN_NUMBER % 10;
    SN_NUMBER /= 10;
    NUM_2 = SN_NUMBER % 10;
    SN_NUMBER /= 10;
    NUM_3 = SN_NUMBER % 10;
    SN_NUMBER /= 10;
    NUM_4 = SN_NUMBER % 10;
    
    NUM_0 += 0x30;
    NUM_1 += 0x30;
    NUM_2 += 0x30;
    NUM_3 += 0x30;
    NUM_4 += 0x30;                                              
      }
      
void BYTE_CONVERT ()  
      {
    HEX_0 = (BYTE_NUMBER >> 4) & 0x0F; // Horní 4 bity
    HEX_1 = BYTE_NUMBER & 0x0F; // Dolní 4 bity

    HEX_0 = (HEX_0 < 10) ? (HEX_0 + '0') : (HEX_0 - 10 + 'A');
    HEX_1 = (HEX_1 < 10) ? (HEX_1 + '0') : (HEX_1 - 10 + 'A');                 
      }   
    
void  INIT_RTC    ()
      {
      TMKPR_W_ON  (); 
      XBYTE [0x9FFA] = 0x00;  // minute
      XBYTE [0x9FFB] = 0x12;  // hours
      XBYTE [0x9FFC] = 0x01;  // weekly
      XBYTE [0x9FFD] = 0x01;  // day
      XBYTE [0x9FFE] = 0x06;  // month
      XBYTE [0x9FFF] = 0x25;  // year
      TMKPR_W_OFF  ();           
      }
      
void INFO ()
            {
      LCD_CLR     ();   

//  text info
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//info text 1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();                        

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();   

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();                        

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();                        

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x62;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

//////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x28;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// text 2
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x68;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x67;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x67;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x75;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x35;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

/////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x50;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// text 3                       
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x36;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

/////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x78;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// text 4                       
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x34;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

///////// 

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xA0;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

// text 5                       
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x36;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

//////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xC8;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

// text 6

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x75;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x75;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x68;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x76;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x67;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

///////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xF0;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

// text 7
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x76;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//////////

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x18;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

// text 8
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x25;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();


// text LCD

// reset calib and set all ports for
// charging lipol and current 250mA.
// A1 - 3S lipol / 12.60V 
// A2 - 2S lipol /  8.40V
// A3 - 1S lipol /  3.60V
// measure and calculate the voltage
// difference in millivolts.
// TOL = 0.1% / OFS = 1mV

            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            DELAY_1S ();
            SOUND             ();         // sound  
            LCD_CLR           ();         // clear lcd            
          }
         
void ERASE ()
      {           
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// ERASE

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

SAVE_A1H = XBYTE [0x9C55];
SAVE_A2H = XBYTE [0x9C56];
SAVE_A1L = XBYTE [0x9C57];
SAVE_A2L = XBYTE [0x9C58];
SAVE_A3L = XBYTE [0x9C59];
SAVE_A3H = XBYTE [0x9C5A];

            dptr_adress  = 0x8000;     
            dptr_counter = 0x1FFF;        
            dptr_data = 0x55;

            {           
            for ( x = 0; x < dptr_counter; x++)      
            {
            XBYTE [dptr_adress] = dptr_data;                          
            dptr_adress++;         
            }        
            }
            
XBYTE [0x9C55] = SAVE_A1H;
XBYTE [0x9C56] = SAVE_A2H;                       
XBYTE [0x9C57] = SAVE_A1L; 
XBYTE [0x9C58] = SAVE_A2L;
XBYTE [0x9C59] = SAVE_A3L;
XBYTE [0x9C5A] = SAVE_A3H; 
        
/*
       WRITE_CAL_SN      ();  

            // serial number  1
            XBYTE [0x9F82] = 0x55;
            XBYTE [0x9F83] = 0x55; 
       
            // serial number  2
            XBYTE [0x9FAC] = 0x55;
            XBYTE [0x9FAD] = 0x55; 

            // serial number  3
            XBYTE [0x9FD6] = 0x55;
            XBYTE [0x9FD7] = 0x55;  
 */           
       SN_CONVERT        (); 
       SOUND             ();               // sound  
       LCD_CLR           ();         // clear lcd       
            }
      
void WRITE ()
      {          
LCD_CLR     ();         // clear lcd 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// WRITE

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x57;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// keyword   1
XBYTE [0x9F7A] = 0x6B;
XBYTE [0x9F7B] = 0x65;
XBYTE [0x9F7C] = 0x79;
XBYTE [0x9F7D] = 0x77; 
XBYTE [0x9F7E] = 0x6F; 
XBYTE [0x9F7F] = 0x72; 
XBYTE [0x9F80] = 0x64;
XBYTE [0x9F81] = 0x20;

// serial number  1
XBYTE [0x9F82] = TMKPR_SN_H;
XBYTE [0x9F83] = TMKPR_SN_L; 

// owner    1
XBYTE [0x9F84] = 0x73;
XBYTE [0x9F85] = 0x63;
XBYTE [0x9F86] = 0x68;
XBYTE [0x9F87] = 0x75;
XBYTE [0x9F88] = 0x6C;
XBYTE [0x9F89] = 0x7A;
XBYTE [0x9F8A] = 0x65;
XBYTE [0x9F8B] = 0x20;
XBYTE [0x9F8C] = 0x65; 
XBYTE [0x9F8D] = 0x6C;
XBYTE [0x9F8E] = 0x65;
XBYTE [0x9F8F] = 0x6B; 
XBYTE [0x9F90] = 0x74; 
XBYTE [0x9F91] = 0x72;
XBYTE [0x9F92] = 0x6F;
XBYTE [0x9F93] = 0x6E;
XBYTE [0x9F94] = 0x69;
XBYTE [0x9F95] = 0x6B;
XBYTE [0x9F96] = 0x20;
XBYTE [0x9F97] = 0x67;
XBYTE [0x9F98] = 0x6D;
XBYTE [0x9F99] = 0x62; 
XBYTE [0x9F9A] = 0x68;
XBYTE [0x9F9B] = 0x00;

// char     1
XBYTE [0x9F9C] = 0x52; 

// calibration    1
XBYTE [0x9F9D] = CALIB_A1H;
XBYTE [0x9F9E] = CALIB_A2H;                       
XBYTE [0x9F9F] = CALIB_ZERO;
XBYTE [0x9FA0] = CALIB_A1L; 
XBYTE [0x9FA1] = CALIB_A2L;
XBYTE [0x9FA2] = CALIB_A3L;
XBYTE [0x9FA3] = CALIB_A3H; 

// keyword   2
XBYTE [0x9FA4] = 0x6B;
XBYTE [0x9FA5] = 0x65;
XBYTE [0x9FA6] = 0x79;
XBYTE [0x9FA7] = 0x77;
XBYTE [0x9FA8] = 0x6F;
XBYTE [0x9FA9] = 0x72; 
XBYTE [0x9FAA] = 0x64; 
XBYTE [0x9FAB] = 0x20; 

// serial number  2
XBYTE [0x9FAC] = TMKPR_SN_H;
XBYTE [0x9FAD] = TMKPR_SN_L; 

// owner    2
XBYTE [0x9FAE] = 0x73;
XBYTE [0x9FAF] = 0x63;
XBYTE [0x9FB0] = 0x68;
XBYTE [0x9FB1] = 0x75;
XBYTE [0x9FB2] = 0x6C;
XBYTE [0x9FB3] = 0x7A;
XBYTE [0x9FB4] = 0x65;
XBYTE [0x9FB5] = 0x20; 
XBYTE [0x9FB6] = 0x65;
XBYTE [0x9FB7] = 0x6C; 
XBYTE [0x9FB8] = 0x65;
XBYTE [0x9FB9] = 0x6B;
XBYTE [0x9FBA] = 0x74;
XBYTE [0x9FBB] = 0x72; 
XBYTE [0x9FBC] = 0x6F;
XBYTE [0x9FBD] = 0x6E;
XBYTE [0x9FBE] = 0x69; 
XBYTE [0x9FBF] = 0x6B;
XBYTE [0x9FC0] = 0x20;
XBYTE [0x9FC1] = 0x67; 
XBYTE [0x9FC2] = 0x6D;
XBYTE [0x9FC3] = 0x62; 
XBYTE [0x9FC4] = 0x68;
XBYTE [0x9FC5] = 0x00; 

// char     2
XBYTE [0x9FC6] = 0x52;

// calibration    2
XBYTE [0x9FC7] = CALIB_A1H;
XBYTE [0x9FC8] = CALIB_A2H;                       
XBYTE [0x9FC9] = CALIB_ZERO;
XBYTE [0x9FCA] = CALIB_A1L; 
XBYTE [0x9FCB] = CALIB_A2L;
XBYTE [0x9FCC] = CALIB_A3L;
XBYTE [0x9FCD] = CALIB_A3H; 

// keyword  3
XBYTE [0x9FCE] = 0x6B;
XBYTE [0x9FCF] = 0x65;
XBYTE [0x9FD0] = 0x79;
XBYTE [0x9FD1] = 0x77; 
XBYTE [0x9FD2] = 0x6F;
XBYTE [0x9FD3] = 0x72;
XBYTE [0x9FD4] = 0x64; 
XBYTE [0x9FD5] = 0x20;

// serial number  3
XBYTE [0x9FD6] = TMKPR_SN_H;
XBYTE [0x9FD7] = TMKPR_SN_L;

// owner    3
XBYTE [0x9FD8] = 0x73;
XBYTE [0x9FD9] = 0x63;
XBYTE [0x9FDA] = 0x68;
XBYTE [0x9FDB] = 0x75;
XBYTE [0x9FDC] = 0x6C;
XBYTE [0x9FDD] = 0x7A; 
XBYTE [0x9FDE] = 0x65; 
XBYTE [0x9FDF] = 0x20; 
XBYTE [0x9FE0] = 0x65;
XBYTE [0x9FE1] = 0x6C; 
XBYTE [0x9FE2] = 0x65;
XBYTE [0x9FE3] = 0x6B; 
XBYTE [0x9FE4] = 0x74; 
XBYTE [0x9FE5] = 0x72;
XBYTE [0x9FE6] = 0x6F; 
XBYTE [0x9FE7] = 0x6E;
XBYTE [0x9FE8] = 0x69; 
XBYTE [0x9FE9] = 0x6B; 
XBYTE [0x9FEA] = 0x20;
XBYTE [0x9FEB] = 0x67;
XBYTE [0x9FEC] = 0x6D; 
XBYTE [0x9FED] = 0x62; 
XBYTE [0x9FEE] = 0x68;
XBYTE [0x9FEF] = 0x00; 

// char     3
XBYTE [0x9FF0] = 0x00;

// calibration    3
XBYTE [0x9FF1] = CALIB_A1H;
XBYTE [0x9FF2] = CALIB_A2H;                       
XBYTE [0x9FF3] = CALIB_ZERO;
XBYTE [0x9FF4] = CALIB_A1L; 
XBYTE [0x9FF5] = CALIB_A2L;
XBYTE [0x9FF6] = CALIB_A3L;
XBYTE [0x9FF7] = CALIB_A3H;   

       TMKPR_STOP        ();                                 // stop RTC
       INIT_RTC          ();                                 // 
       SN_CONVERT        ();
       SOUND             ();               // sound  
       LCD_CLR           ();         // clear lcd       
      }      
 
void E_HIGH ()
      {
      P6 |= E_PIN;
      }
        
/*void E_LOW ()
      {
      P6 &= ~E_PIN;   
      }           
*/
void RW_HIGH ()
      {
      P6 |= RW_PIN;
      }
        
void RW_LOW ()
      {
      P6 &= ~RW_PIN;   
      } 

void RS_HIGH ()
      {
      P6 |= RS_PIN;
      }
        
void RS_LOW ()
      {
      P6 &= ~RS_PIN;   
      }                 
      
/*void CS_HIGH ()
      {
      P6 |= CS_PIN;
     }
*/         
void CS_LOW ()
      {
      P6 &= ~CS_PIN;   
      }

void RESET_HIGH ()
      {
      P6 |= RESET_PIN;
      }
        
void RESET_LOW ()
      {
      P6 &= ~RESET_PIN;   
      }                 
      
void LATCH_B_HIGH ()
      {
      P6 |= LATCH_B_PIN;
      }
        
void LATCH_B_LOW ()
      {
      P6 &= ~LATCH_B_PIN;   
      }                    
      
void LATCH_C_HIGH ()
      {
      P6 |= LATCH_C_PIN;
      }
        
void LATCH_C_LOW ()
      {
      P6 &= ~LATCH_C_PIN;   
      } 

void TMKPR_RUN ()
      {
      XBYTE [0x9FF8] = 0x00;       
      }         
/*     
void TMKPR_R_ON ()
      {
      XBYTE [0x9FF8] = 0x40;     
      TMKPR_SETUP = XBYTE [0x9FF8]; 
      TMKPR_SETUP |= TMKPR_R_PIN;
      XBYTE [0x9FF8] = TMKPR_SETUP;      
      }
           
void TMKPR_R_OFF ()
      {
      XBYTE [0x9FF8] = 0x00;
      TMKPR_SETUP = XBYTE [0x9FF8]; 
      TMKPR_SETUP &= ~ TMKPR_R_PIN;
      XBYTE [0x9FF8] = TMKPR_SETUP;      
      }
 */     
void TMKPR_W_ON ()
      {
      XBYTE [0x9FF8] = 0x80;
      TMKPR_SETUP = XBYTE [0x9FF8]; 
      TMKPR_SETUP |= TMKPR_W_PIN;
      XBYTE [0x9FF8] = TMKPR_SETUP;      
      }
           
void TMKPR_W_OFF ()
      {
      XBYTE [0x9FF8] = 0x00;     
      TMKPR_SETUP = XBYTE [0x9FF8]; 
      TMKPR_SETUP &= ~ TMKPR_W_PIN;
      XBYTE [0x9FF8] = TMKPR_SETUP;      
      }
 /*           
void TMKPR_START ()
      {
      TMKPR_CLOCK = XBYTE [0x9FF9];      
      TMKPR_CLOCK |= TMKPR_C_PIN;
      XBYTE [0x9FF9] = TMKPR_CLOCK;      
      }
 */          
void TMKPR_STOP ()
      {
      XBYTE [0x9FF9] = 0x80;    
      TMKPR_CLOCK = XBYTE [0x9FF9];       
      TMKPR_CLOCK &= ~ TMKPR_C_PIN;
      XBYTE [0x9FF9] = TMKPR_CLOCK;      
      }    
      
void LCD_INIT ()
      {
// LCD RESET / CS LOW / RESET LOW / DELAY / E HIGH / RESET HIGH 	
CS_LOW ();           
E_HIGH ();            
RESET_LOW ();
delay_1ms ();         
RESET_HIGH ();                         
                       
// INIT LCD
RW_LOW ();              //display off, master mode, cursor on/blink, text mode, internal CG-ROM
RS_HIGH ();
P5 = 0x00;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // 6x8 font
RS_HIGH ();
P5 = 0x01;
RW_LOW ();
RS_LOW ();
P5 = 0x75;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // 40 horizontal characters
RS_HIGH ();
P5 = 0x02;
RW_LOW ();
RS_LOW ();
P5 = 0x27;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // 64 lines
RS_HIGH ();
P5 = 0x03;
RW_LOW ();
RS_LOW ();
P5 = 0x40;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor on 8th line
RS_HIGH ();
P5 = 0x04;
RW_LOW ();
RS_LOW ();
P5 = 0x07;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // display start address low
RS_HIGH ();
P5 = 0x08;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // display start address high
RS_HIGH ();
P5 = 0x09;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  
            }
      
void LCD_CLR ()
            {            
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

            for ( x = 0; x < 320; x++)       // erase LCD      
            {
            
RW_LOW ();              //display off, master mode, cursor on/blink, text mode, internal CG-ROM
RS_HIGH ();
P5 = 0x00;
RW_LOW ();
RS_LOW ();
P5 = 0x10;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();           
            
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

RW_LOW ();              //display on, master mode, cursor on/blink, text mode, internal CG-ROM
RS_HIGH ();
P5 = 0x00;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            }      
            } 
  
void LCD_START ()
      {
//  start  LCD logo
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x56;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// calibration
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x43;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x42;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  XTOOL
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x58;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 
 
//** for
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** ISL8
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();   
       
//  text LCD version firmware
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x18;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** XTOOLS-FW:x.xx
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x58;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x57;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = FW_0;              //"x"
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = FW_1;              //"."
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = FW_2;              //"x"
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = FW_3;              //"x"
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x43;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x55;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x5A;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2E;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x35;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x39;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

      
//  text programmed by Prometheus80
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xCE;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** programmed by Prometheus80
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x67;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x62;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x79;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x50;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x68;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x30;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
      }

void LCD_MENU_T ()
      {
// first start text LCD 
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x06;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// calibration
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x43;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x42;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  XTOOL
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x58;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 
 
//** for
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** ISL8
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();     
                 
//  text serial number
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x50;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x38;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = NUM_4;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = NUM_3;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = NUM_2;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = NUM_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = NUM_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();    
                
//  text password
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x63;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x70;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
      
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x77;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FCE];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FCF];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD0];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD1];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD2];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD3];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD4];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD5];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
          
//  text owner
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x78;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x77;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD8];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FD9];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDA];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDB];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDC];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDD];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDE];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FDF];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE0];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE1];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE2];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE3];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE4];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE5];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE6];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE7];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE8];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FE9];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FEA];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FEB];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FEC];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FED];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FEE];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = XBYTE [0x9FEF];
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
         
//  calibration 7 byte data
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xC9;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
      
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x62;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H_0
BYTE_NUMBER = XBYTE [0x9F9D];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H_0
BYTE_NUMBER = XBYTE [0x9F9E];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_H
BYTE_NUMBER = XBYTE [0x9F9F];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_0
BYTE_NUMBER = XBYTE [0x9FA0];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-L_0
BYTE_NUMBER = XBYTE [0x9FA1];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2_L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_0
BYTE_NUMBER = XBYTE [0x9FA2];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
BYTE_NUMBER = XBYTE [0x9FA3];
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();         
 
//  text LCD INFO
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x1A;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** INFO
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();                     

//  text LCD ERASE
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x21;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** ERASE
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
        
//  text LCD WRITE
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x29;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** WRITE
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x57;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();       
           
//  text LCD -----
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x31;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** -----
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD NEXT
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x39;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** NEXT
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x58;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            }       

void LCD_MENU_C ()
            {                   
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x0E;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// calibration
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
      
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x62;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x59;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x28;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H_0
BYTE_NUMBER = CALIB_A1H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H_0
BYTE_NUMBER = CALIB_A2H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_H
BYTE_NUMBER = CALIB_ZERO;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_0
BYTE_NUMBER = CALIB_A1L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-L_0
BYTE_NUMBER = CALIB_A2L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2_L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_0
BYTE_NUMBER = CALIB_A3L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
BYTE_NUMBER = CALIB_A3H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x29;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();   
 
//  text LCD ERASE
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x19;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** ERASE
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//   text LCD    DEFAULT ();            
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x20;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** DEFAULT
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x44;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x55;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();    

//   text LCD    ORIGIN ();
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x29;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            
RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** ORIGIN
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x52;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x47;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x49;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

//  text -----
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x31;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** SAVE
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x56;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD NEXT
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x39;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** NEXT
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x45;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x58;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            }       
     
void LCD_MENU_E ()
     
            {
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x0E;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// calibration
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x63;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
      
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x62;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x72;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// data calib
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x51;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H_0
BYTE_NUMBER = CALIB_A1H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H_0
BYTE_NUMBER = CALIB_A2H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H-1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_H
BYTE_NUMBER = CALIB_ZERO;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//ZERO_L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_0
BYTE_NUMBER = CALIB_A1L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-L_0
BYTE_NUMBER = CALIB_A2L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2_L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_0
BYTE_NUMBER = CALIB_A3L;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L_1
BYTE_NUMBER = CALIB_A3H;
BYTE_CONVERT ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_0;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = HEX_1;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();  

// LABEL DATA
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x7B;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

//A1-H
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-H
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// ZERO
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4E;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x43;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A1-L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A2-L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//A3-H
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD EDIT A1
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x1B;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** A1
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x31;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD EDIT A2
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x23;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** A2
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x32;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD EDIT A3
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x2B;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** A3
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x41;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x33;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD EDIT H
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x33;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** H
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//  text LCD EDIT L    // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0x3B;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x01;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

//** L
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

// EDIT LCD EDIT VALUE:
RW_LOW ();              // cursor adress low
RS_HIGH ();
P5 = 0x0A;
RW_LOW ();
RS_LOW ();
P5 = 0xCB;              
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              // cursor adress high
RS_HIGH ();
P5 = 0x0B;
RW_LOW ();
RS_LOW ();
P5 = 0x00;
E_HIGH ();
RW_HIGH ();             
RS_HIGH (); 

// EDIT VALUE OFFSET: 
RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x64;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x69;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x76;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x61;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x75;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x6F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x66;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x73;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x65;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x74;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3A;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x48;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x54;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x2F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x20;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4C;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x3D;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x4F;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x46;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();

RW_LOW ();              
RS_HIGH ();
P5 = 0x0C;
RW_LOW ();
RS_LOW ();
P5 = 0x53;
E_HIGH ();
RW_HIGH ();             
RS_HIGH ();
            }         


void PLUS_CALIB () 
            { 
                        
            if (setupcalib == 0x12)  // A1H
            {
            CALIB_A1H++;          
            WRITE_CAL_SN ();                      
            } 
                       
            if (setupcalib == 0x22)  // A2H
            {
            CALIB_A2H++;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x42)  // A3H
            {
            CALIB_A3H++;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x14)  // A1L
            {
            CALIB_A1L++;          
            WRITE_CAL_SN ();                      
            } 
                       
            if (setupcalib == 0x24)  // A2L
            {
            CALIB_A2L++;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x44)  // A3L
            {
            CALIB_A3L++;          
            WRITE_CAL_SN ();                      
            }           
            
            }            

            
void MINUS_CALIB ()             
            { 
                        
            if (setupcalib == 0x12)  // A1H
            {
            CALIB_A1H--;          
            WRITE_CAL_SN ();                      
            } 
                       
            if (setupcalib == 0x22)  // A2H
            {
            CALIB_A2H--;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x42)  // A3H
            {
            CALIB_A3H--;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x14)  // A1L
            {
            CALIB_A1L--;          
            WRITE_CAL_SN ();                      
            } 
                       
            if (setupcalib == 0x24)  // A2L
            {
            CALIB_A2L--;          
            WRITE_CAL_SN ();                      
            } 
            
            if (setupcalib == 0x44)  // A3L
            {
            CALIB_A3L--;          
            WRITE_CAL_SN ();                      
            }           
            
            }            
     
     
            