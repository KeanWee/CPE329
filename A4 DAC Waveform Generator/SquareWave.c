#include <msp430g2553.h>
#include <msp430.h>

void Drive_DAC(unsigned int level);
volatile unsigned int TempDAC_Value = 0;
volatile unsigned int RisingEdge = 1;

int main(void)
{

  WDTCTL = WDTPW + WDTHOLD;        // Stop watchdog timer

  // 16Mhz SMCLK
  if (CALBC1_16MHZ==0xFF)        // If calibration constant erased
  {
    while(1);                        // do not load, trap CPU!!
  }
  DCOCTL = 0;                        // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_16MHZ;            // Set range
  DCOCTL = CALDCO_16MHZ;             // Set DCO step + modulation

  // Init Ports
  P1DIR |= BIT4;                     // Will use BIT4 to activate /CE on the DAC

  P1SEL  = BIT7 + BIT5;  // + BIT4;  // These two lines dedicate P1.7 and P1.5
  P1SEL2 = BIT7 + BIT5; // + BIT4;   // for UCB0SIMO and UCB0CLK respectively

  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 327;                             // Sets interrupt trigger at 10 ms
  TACTL = TASSEL_1 + MC_2;                  // ACLK, contmode

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
  UCB0BR0 |= 0x10;             // (low divider byte)
  UCB0BR1 |= 0x00;             // (high divider byte)

  // An example of creating another lower frequency SPI SCLK
  //UCB0BR0 |= 0x00;           // (low byte)  This division caused divide by 256
  //UCB0BR1 |= 0x01;           // (high byte) for an SPI SCLK of ~62.5 KHz.

  //UCB0MCTL = 0;              // No modulation => NOT REQUIRED ON B PORT,
                               // would be required if used UCA0

  UCB0CTL1 &= ~UCSWRST;        // **Initialize USCI state machine**
                               // SPI now Waiting for something to
                               // be placed in TXBUF.

  // No interrupts in this version, so commented out
  //IE2 = UCB0TXIE;            // Enable USCI0 TX interrupt
  //IE2 = UCB0RXIE;            // Enable USCI0 RX interrupt
  //_enable_interrupts();
  _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt
  return 0;

} // end of main


void Drive_DAC(unsigned int level){
  unsigned int DAC_Word = 0;

  DAC_Word = (0x1000) | (level & 0x0FFF);   // 0x1000 sets DAC for Write
                                            // to DAC, Gain = 2, /SHDN = 1
                                            // and put 12-bit level value
                                            // in low 12 bits.

  P1OUT &= ~BIT4;                    // Clear P1.4 (drive /CS low on DAC)
                                     // Using a port output to do this for now

  UCB0TXBUF = (DAC_Word >> 8);       // Shift upper byte of DAC_Word
                                     // 8-bits to right

  while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?

  UCB0TXBUF = (unsigned char)
           (DAC_Word & 0x00FF);  // Transmit lower byte to DAC

  while (!(IFG2 & UCB0TXIFG));       // USCI_A0 TX buffer ready?
  __delay_cycles(200);               // Delay 200 16 MHz SMCLK periods
                                     // (12.5 us) to allow SIMO to complete
  P1OUT |= BIT4;                     // Set P1.4   (drive /CS high on DAC)
  return;
}

// no interrupts for now
/*
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  // Test for valid RX and TX character
  volatile unsigned int i;
}
*/

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    P1OUT ^= BIT0;           // Toggles LED for testing
    if(RisingEdge){
      TempDAC_Value += 800;
      Drive_DAC(TempDAC_Value);
      RisingEdge = 0;
    }
    else{
      TempDAC_Value -= 800;
      Drive_DAC(TempDAC_Value);
      RisingEdge = 1;
    }
    CCR0+=327;
}
