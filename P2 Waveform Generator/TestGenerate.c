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

#include "/Users/Kean/Documents/CPE329/LCD.h"
#include "/Users/Kean/Documents/CPE329/LCD.c"

void GenerateSquareWave();
void GenerateSineWave();
void GenerateSawtoothWave();
void Drive_DAC(unsigned int level);

volatile int toggleMode = 0;
volatile int freqMode = 1;
volatile int dutyMode = 5;
volatile unsigned int TempDAC_Value = 0;
volatile unsigned int Counter = 0;


volatile int sineValues[50] = {0, 125, 249, 368, 482, 588, 685, 771, 844, 905, 951,
982, 998, 998, 982, 951, 905, 844, 771, 685, 588, 482, 368, 249, 125, 0, -125, -249,
-368, -482, -588, -685, -771, -844, -905, -951, -982, -998, -998, -982, -951, -905,
-844, -771, -685, -588, -482, -368, -249, -125};


int main(void){
   WDTCTL = WDTPW + WDTHOLD;                                            // Stop watchdog timer

   if (CALBC1_16MHZ==0xFF)                                              // If calibration constant erased
      while(1);                                                         // do not load, trap CPU!!
   DCOCTL = 0;                                                          // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_16MHZ;                                              // Set range
   BCSCTL2 |= 0x10;
   DCOCTL = CALDCO_16MHZ;                                               // Set DCO step + modulation

   LCD_Init();                                                          // Initilizes LCD

   // Init Ports
   P1DIR |= CSBIT;                                                      // Activate /CS on the DAC

   P1SEL  = SIMOBIT + CLKBIT;                                           // These two lines dedicate P1.7 and P1.5
   P1SEL2 = SIMOBIT + CLKBIT;                                           // for UCB0SIMO and UCB0CLK respectively

   CCTL0 = CCIE;                                                        // CCR0 interrupt enabled
   CCR0 = (16000000 / 5000);                                            // Sets interrupt trigger at 5 ms
   TACTL = TASSEL_2 + MC_1;                                             // SMCLK, UP mode

   // SPI Setup
   // clock inactive state = low,
   // MSB first, 8-bit SPI master,
   // 4-pin active Low STE, synchronous
   //
   // 4-bit mode disabled for now
   UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


   UCB0CTL1 |= UCSSEL_2;                                                // UCB0 will use SMCLK as the basis for
                                                                        // the SPI bit clock
   // Sets SMCLK divider to default, so 16MHz clock
   UCB0BR0 &= 0x00;                                                     // (low divider byte)
   UCB0BR1 &= 0x00;                                                     // (high divider byte)

   UCB0CTL1 &= ~UCSWRST;                                                // **Initialize USCI state machine**

   P2IE |= BIT3 + BIT4 + BIT5;                                          // Enables interrupts on P2.3 - 2.5
   P2REN |= BIT3 + BIT4 + BIT5;                                         // Sets up resistors on P2.3 - p2.5
   P2OUT &= ~(BIT3 + BIT4 + BIT5);                                      // Sets resistors to pull down
   P2IES &= ~(BIT3 + BIT4 + BIT5);                                      // Interrupts rising edge triggered
   P2IFG &= ~(BIT3 + BIT4 + BIT5);                                      // Clears interrupt flags

   LCD_DDRAM_SET(0x40);                                                 //
   SendString("CPE 329 Demo");                                          // Writes text to bottom line
   LCD_DDRAM_SET(0x00);                                                 //

   _BIS_SR(LPM0_bits + GIE);                                            // Enable interrupts and set lower power mode

   return 0;
}

/* Generates the next portion of the square wave */
void GenerateSquareWave()
{
   Counter++;                                                       // Increments counter
   if(Counter <= dutyMode*5)                                        // Sets voltage to 5V if the counter is 
      TempDAC_Value = 2000;                                             // within the range of the duty cycle

   else
      TempDAC_Value = 0;                                                // Sets voltage to 0V otherwise

   if(Counter == 50)                                                // Restricts bounds of counter
      Counter = 0;

   Drive_DAC(TempDAC_Value);                                            // Drives voltage to DAC
}

/* Generates the next portion of the sine wave */
void GenerateSineWave()
{
   Drive_DAC(sineValues[Counter]+1000);                                // Drives voltage depending on lookup table
   Counter++;                                                          // Increments counter
   if (Counter == 50)                                                  // Restricts range of counter
     Counter = 0;
}

/* Genreates the next portion of the sawtooth wave */
void GenerateSawtoothWave()
{
   if(Counter == 0)                                               // Sets voltage to 5V at start
      TempDAC_Value = 2000;
   else                                                                 // Decrements voltage in steps
      TempDAC_Value -= 40;
   
   Drive_DAC(TempDAC_Value);                                            // Drives voltage to DAC

   Counter++;                                                     // Increments counter

   if(Counter == 50)                                              // Restricts range of counter
      Counter = 0;
}

/* Drives voltage to DAC */
void Drive_DAC(unsigned int level){
   unsigned int DAC_Word = 0;

   DAC_Word = (0x1000) | (level & 0x0FFF);                              // 0x1000 sets DAC for Write
                                                                        // to DAC, Gain = 2, /SHDN = 1
                                                                        // and put 12-bit level value
                                                                        // in low 12 bits.

   P1OUT &= ~CSBIT;                                                     // Drive /CS low on DAC

   UCB0TXBUF = (DAC_Word >> 8);                                         // Shift upper byte of DAC_Word
                                                                        // 8-bits to right

   UCB0TXBUF = (unsigned char)(DAC_Word & 0x00FF);                      // Transmit lower byte to DAC

   //__delay_cycles(13);                                                  // Delay 13 1 MHz SMCLK periods
                                                                        // (12.5 us) to allow SIMO to complete
   P1OUT |= CSBIT;                                                      // Set P1.4   (drive /CS high on DAC)
   return;
}


/* Interrupt Service Routine. Takes mode toggle button inputs */
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void){
   __delay_cycles(1600000);                                             // Delay for debouncing
   SendCommand(0x01);                                                   // Clears display

   char string[16];
   char toggleString[10];
   char dutyString[16];

   TempDAC_Value = 0;
   Counter = 0;

   if(P2IFG & BIT3){                                                    // Toggles waveform between three modes
      toggleMode++;
      if(toggleMode == 3)
         toggleMode = 0;
   }
   else if(P2IFG & BIT4){                                               // Toggles frequency between five modes
      freqMode++;
      if(freqMode == 6)
      freqMode = 1;
      CCR0 = 16000000 / (freqMode * 5000);                              // Sets CCR0 based on the frequency
   }
   else if(P2IFG & BIT5 && toggleMode == 0){                            // Toggles duty cycle if square wave is selected
      dutyMode++;
      if(dutyMode == 11)
         dutyMode = 1;
   }



   if (toggleMode == 0)
      sprintf(toggleString,"Square");                                   // Prepares part string to be output to LCD
      
   else if (toggleMode == 1)
      sprintf(toggleString,"Sine");
      
   else if (toggleMode == 2)
      sprintf(toggleString,"Sawtooth");
  
   sprintf(string,"%s %d Hz",toggleString,freqMode * 100);              // Assembles top line string
   sprintf(dutyString,"Duty: %d %%",dutyMode * 10);                     // Assembles bottom line string
   LCD_DDRAM_SET(0x00);
   SendString(string);                                                  // Writes top line
   if(toggleMode ==0){                                                  // Writes bottom line if the wave is a square
      LCD_DDRAM_SET(0x40);
      SendString(dutyString);
   }
   __delay_cycles(160);
   P2IFG &= ~(BIT3+BIT4+BIT5);                                          // Clear interrupt flag
}

/* Timer interrupt to update waveform */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
   if(toggleMode == 0)                                                  // Calls waveform functions based on mode
      GenerateSquareWave();
   else if(toggleMode == 1)
      GenerateSineWave();
  else if(toggleMode == 2)
     GenerateSawtoothWave();
}