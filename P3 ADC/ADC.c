#include <msp430.h>
#include <msp430g2553.h>

#define CSBIT BIT6
#define SIMOBIT BIT7
#define CLKBIT BIT5

void Drive_DAC(unsigned int level);
void GenerateRamp();
volatile unsigned int Flag = 0;
volatile unsigned char ComparatorValue;

int main(void)
{
   WDTCTL = WDTPW + WDTHOLD;                                            // Stop watchdog timer

   if (CALBC1_16MHZ==0xFF)                                              // If calibration constant erased
      while(1);                                                         // do not load, trap CPU!!
   DCOCTL = 0;                                                          // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_16MHZ;                                              // Set range
   BCSCTL2 |= 0x10;
   DCOCTL = CALDCO_16MHZ;                                               // Set DCO step + modulation

   CAPD |= BIT2 + BIT3 + BIT4;                                          // Turns off GPIO function of pins
   
   P1DIR |= CSBIT;                                                      // Activate /CS on the DAC

   P1SEL  = SIMOBIT + CLKBIT;                                           // These two lines dedicate P1.7 and P1.5
   P1SEL2 = SIMOBIT + CLKBIT;                                           // for UCB0SIMO and UCB0CLK respectively

   CCTL0 = CCIE;                                                        // CCR0 interrupt enabled
   CCR0 = (16000);                                            // Sets interrupt trigger at 5 ms
   TACTL = TASSEL_2 + MC_1;                                             // SMCLK, UP mode

   // UCB0BR0 &= 0x00;                                                     // (low divider byte)
   // UCB0BR1 &= 0x10;  

   // SPI Setup
   // clock inactive state = low,
   // MSB first, 8-bit SPI master,
   // 4-pin active Low STE, synchronous
   //
   // 4-bit mode disabled for now
   UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


   UCB0CTL1 |= UCSSEL_2;                                                // UCB0 will use SMCLK as the basis for
                                                                        // the SPI bit clock

   UCB0CTL1 &= ~UCSWRST;                                                // **Initialize USCI state machine**

   P2SEL &= ~(BIT7 + BIT6);                                             // Enable Xin and Xout pins
   P2SEL2 &= ~(BIT7 + BIT6);                                            //

   P2DIR |= 0xFF;
   CACTL2 |= P2CA4;

   // while(1){
   //    Drive_DAC(ComparatorValue);
   //    ComparatorValue++;
   //    __delay_cycles(1000);
   // }
   _BIS_SR(LPM0_bits + GIE);
   return 0;
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
   P1OUT |= CSBIT;                                                      // Set P1.4   (drive /CS high on DAC)
   return;
}

void GenerateRamp()
{
   CACTL2 ^= CASHORT + P2CA2 + P2CA4;                                   // Sets comparator input to resistor network
   while((!Flag) && (P2OUT < 0xFF)){
      P2OUT = P2OUT + 1;
   }
   CACTL2 &= P2CA4;
   P2OUT = 0x00;
   return;
}

/* Timer interrupt to update waveform */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
   CACTL2 |= CASHORT + P2CA2;                                           // Shorts transmission gate and fills Capacitor
   __delay_cycles(100000);
   _BIS_SR(GIE);                                                        // Enable interrupts
   CACTL1 |= CAIE + CAON;                                               // Comparator Interrupts enabled and Comparator enabled
   GenerateRamp(); 
   Drive_DAC(ComparatorValue);
   CACTL1 &= ~(CAIE + CAON);
   Flag = 0;
}

#pragma vector=COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR(void) {
      ComparatorValue = P2OUT;
      Flag = 1;
      CACTL1 &= ~CAIE;
      Drive_DAC(500);
}