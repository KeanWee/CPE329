#include <msp430.h>
#include <msp430g2452.h>
#include <msp430g2553.h>
#include <stdio.h>

#define RSBIT BIT0
#define RWBIT BIT1
#define EBIT  BIT2
#define CSBIT BIT4
#define SIMOBIT BIT7
#define CLKBIT BIT5
#define PI 3.14159


#define RSHigh    P2OUT |= RSBIT
#define RSLow     P2OUT &= ~RSBIT
#define READ      P2OUT |= RWBIT
#define Write     P2OUT &= ~RWBIT
#define EHigh     P2OUT |= EBIT
#define ELow      P2OUT &= ~EBIT

void EPulse(void);
void SendCommand(char cmd);
void SendString(char *string);
void LCD_Init(void);
void LCD_Write(char input);
void LCD_DDRAM_SET(char addr);
void GenerateSquareWave();
void GenerateSineWave();
void Drive_DAC(unsigned int level);

volatile int count = 0;
volatile int flag = 0;
volatile int toggleMode = 0;
volatile int freqMode = 1;
volatile int dutyMode = 1;
volatile unsigned int TempDAC_Value = 0;
volatile unsigned int Rising = 1;
volatile unsigned int SquareCount = 0;
volatile unsigned int SinCount = 0;

int main(void){
      WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

      if (CALBC1_16MHZ==0xFF)    // If calibration constant erased
         while(1);              // do not load, trap CPU!!
      DCOCTL = 0;               // Select lowest DCOx and MODx settings
      BCSCTL1 = CALBC1_16MHZ;    // Set range
      BCSCTL2 |= 0x10;
      DCOCTL = CALDCO_16MHZ;     // Set DCO step + modulation





   LCD_Init();

   // Init Ports
  P1DIR |= CSBIT;                     // Activate /CS on the DAC

  P1SEL  = SIMOBIT + CLKBIT;             // These two lines dedicate P1.7 and P1.5
  P1SEL2 = SIMOBIT + CLKBIT;             // for UCB0SIMO and UCB0CLK respectively

  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = (16000000 / 1000);                             // Sets interrupt trigger at 10 ms
  TACTL = TASSEL_2 + MC_1;                  // ACLK, contmode

  // SPI Setup
  // clock inactive state = low,
  // MSB first, 8-bit SPI master,
  // 4-pin active Low STE, synchronous
  //
  // 4-bit mode disabled for now
  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


  UCB0CTL1 |= UCSSEL_2;               // UCB0 will use SMCLK as the basis for
                                      // the SPI bit clock

  // Sets SMCLK divider to 16,
  // hence making SPI SCLK equal
  // to SMCLK/16 = 1MHz
  UCB0BR0 &= 0x00;             // (low divider byte)
  UCB0BR1 &= 0x00;             // (high divider byte)

  // An example of creating another lower frequency SPI SCLK
  //UCB0BR0 |= 0x00;           // (low byte)  This division caused divide by 256
  //UCB0BR1 |= 0x01;           // (high byte) for an SPI SCLK of ~62.5 KHz.

  //UCB0MCTL = 0;              // No modulation => NOT REQUIRED ON B PORT,
                               // would be required if used UCA0

  UCB0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**

   // P1DIR = 0x00;
   // P2DIR = 0x00;
   P1DIR |= 0x0F;
   P2DIR |= 0x07;

   P2IE |= BIT3 + BIT4 + BIT5;
   P2REN |= BIT3 + BIT4 + BIT5;
   P2OUT &= ~(BIT3 + BIT4 + BIT5);
   P2IES &= ~(BIT3 + BIT4 + BIT5);
   P2IFG &= ~(BIT3 + BIT4 + BIT5);

   SendCommand(0x06);
   LCD_DDRAM_SET(0x40);
   SendString("CPE 329 Demo");
   LCD_DDRAM_SET(0x00);

   _BIS_SR(LPM0_bits + GIE); //enable interrupts and set lower power mode
   //_BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt

   return 0;
}



/*Initialize the LCD to Nibble Mode*/
void LCD_Init(void)
{

   P1DIR |= 0x0F;                         // Enables Port 1 for outputs
   P2DIR |= RSBIT + RWBIT + EBIT;         // Enables Port 2 for outputs
   P1OUT &= 0xF0;                         // Clears Port 1
   P2OUT &= ~(RSBIT + RWBIT + EBIT);                         // Clears Port 2
   //__delay_cycles(21000);                 // Wait 20ms
   __delay_cycles(160000);
   __delay_cycles(160000);
   __delay_cycles(160000);
   SendCommand(0x28);
   //__delay_cycles(100);                   // Wait 37us
   __delay_cycles(2000);
   SendCommand(0x0C);
   //__delay_cycles(100);                   // Wait 37us
   __delay_cycles(2000);
   SendCommand(0x01);
   //__delay_cycles(2000);                  // Wait 1.52ms
   __delay_cycles(16*2000+500);
}


/*Writes a single character to the LCD*/
void LCD_Write(char input)    // Write character to DDRAM location
{
   RSHigh;
   Write;
   __delay_cycles(400);
   P1OUT &= 0xF0;
   P1OUT |= ((input >> 4) & 0x0F);            // Set Data bus to input
   __delay_cycles(40);         // Wait 40ns
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (input & 0x0F);
   __delay_cycles(40);
   EPulse();
}

/*Sets the LCDDDRAM address*/
void LCD_DDRAM_SET(char addr) // Sets DDRAM location to address
{
   P1OUT &= 0xF0;
   P1OUT |= (((addr >> 4) & 0x0F) | 0x08);              // Set Data bus to address
   //P2OUT &= ~BIT2;
   //P2OUT &= ~BIT1;    // Set Port 2 pins
   ELow;
   Write;
   RSLow;
   __delay_cycles(50);         // Wait 40ns
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (addr & 0x0F);
   __delay_cycles(50);
   EPulse();
}

/*Pulses the Enable pin on the LCD*/
void EPulse(void)
{
   ELow;
   __delay_cycles(20);
   EHigh;
   __delay_cycles(5*20);
   ELow;
   __delay_cycles(5*20);
}

/*Sends a command to the LCD*/
void SendCommand(char cmd)
{
   RSLow;
   Write;
   __delay_cycles(40);
   P1OUT &= 0xF0;
   P1OUT |= ((cmd >> 4) & 0x0F);
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (cmd & 0x0F);
   EPulse();
}


/*Sends a string characters to be displayed on the LCD*/
void SendString(char *string)
{
   while(*string)
   {
      __delay_cycles(100000);//80000 to look nice
      LCD_Write(*string);
      string++;
      
   }
}

void GenerateSquareWave(void)
{
   SquareCount++;
   if(SquareCount <= dutyMode){
      TempDAC_Value = 2000;
   }
   else{
      TempDAC_Value = 0;
   }

   if(SquareCount == 10)
      SquareCount = 0;
   Drive_DAC(TempDAC_Value);
}

void GenerateSineWave()
{
   SinCount++;
   if(SinCount == 100)
      SinCount = 0;
   TempDAC_Value = sine(2*PI*freqMode*100*SinCount)*400 + 1000;
   Drive_DAC(TempDAC_Value);
}

void Drive_DAC(unsigned int level){
  unsigned int DAC_Word = 0;

  DAC_Word = (0x1000) | (level & 0x0FFF);   // 0x1000 sets DAC for Write
                                            // to DAC, Gain = 2, /SHDN = 1
                                            // and put 12-bit level value
                                            // in low 12 bits.

  P1OUT &= ~CSBIT;                    // Clear P1.4 (drive /CS low on DAC)
                                     // Using a port output to do this for now

  UCB0TXBUF = (DAC_Word >> 8);       // Shift upper byte of DAC_Word
                                     // 8-bits to right

  //while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?

  UCB0TXBUF = (unsigned char)(DAC_Word & 0x00FF);  // Transmit lower byte to DAC

  //while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?
  __delay_cycles(13);               // Delay 200 16 MHz SMCLK periods
                                     // (12.5 us) to allow SIMO to complete
  P1OUT |= CSBIT;                     // Set P1.4   (drive /CS high on DAC)
  return;
}


/*Interrupt Service Routine. Increases count variable by one and displays it on the LCD*/
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){
   __delay_cycles(1600000);
   SendCommand(0x01);

   char string[16];
   char toggleString[10];
   char dutyString[16];
   TempDAC_Value = 0;
   if(P2IFG & BIT3){
      toggleMode++;
      if(toggleMode == 3)
         toggleMode = 0;
   }
   else if(P2IFG & BIT4){
      freqMode++;
      if(freqMode == 6)
         freqMode = 1;
      CCR0 = 16000000 / (freqMode * 1000);
   }
   else if(P2IFG & BIT5 && toggleMode == 0){
      dutyMode++;
      if(dutyMode == 11)
         dutyMode = 1;
      {
         /* code */
      }
   }

   

      if (toggleMode == 0){
        sprintf(toggleString,"Square");
      }
      else if (toggleMode == 1){
        sprintf(toggleString,"Triangle");
      }
      else if (toggleMode == 2){
        sprintf(toggleString,"Sawtooth");
      }



      sprintf(string,"%s %d Hz",toggleString,freqMode * 100);
      sprintf(dutyString,"Duty: %d %%",dutyMode * 10);
      LCD_DDRAM_SET(0x00);
      SendString(string);
      if(toggleMode ==0){
        LCD_DDRAM_SET(0x40);
        SendString(dutyString);
      }
      __delay_cycles(160);
      P2IFG &= ~(BIT3+BIT4+BIT5);              //toggle interrupt flag
}

 // Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
   //GenerateSquareWave(freqMode, dutyMode);
   //Drive_DAC(800);

   // if(Rising){
   //    TempDAC_Value = 800;
   //    Drive_DAC(TempDAC_Value);
   //    Rising = 0;
   //  }
   //  else{
   //    TempDAC_Value = 0;
   //    Drive_DAC(TempDAC_Value);
   //    Rising = 1;
   //  }
   if(toggleMode == 0)
      GenerateSquareWave();
   else if(toggleMode == 1)
      GenerateSineWave();
   else;



   // SquareCount++;
   // if(SquareCount <= dutyMode){
   //    TempDAC_Value = 2000;
   // }
   // else{
   //    TempDAC_Value = 0;
   // }

   // if(SquareCount == 10)
   //    SquareCount = 0;
   // Drive_DAC(TempDAC_Value);

   // if(Rising){
   //    TempDAC_Value += 20;
   //    Drive_DAC(TempDAC_Value);
   //    if(TempDAC_Value == 800)
   //    Rising = 0;
   //  }
   //  else{
   //    TempDAC_Value -= 20;
   //    Drive_DAC(TempDAC_Value);
   //    if(TempDAC_Value == 0)
   //    Rising = 1;
   //  }
}



