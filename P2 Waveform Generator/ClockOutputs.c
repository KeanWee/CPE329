#include <msp430.h>
#include <msp430g2553.h>

int main()
{
   WDTCTL = WDTPW + WDTHOLD;
   if (CALBC1_16MHZ==0xFF)                                              // If calibration constant erased
      while(1);                                                         // do not load, trap CPU!!
   DCOCTL = 0;                                                          // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_16MHZ;                                              // Set range
   BCSCTL2 |= 0x10;
   DCOCTL = CALDCO_16MHZ; 

   P1DIR |= BIT0 + BIT4;
   P1SEL = BIT7 + BIT5 + BIT4;
   P1SEL2 = BIT7 + BIT5;

   CCTL0 = CCIE;                                                        // CCR0 interrupt enabled
   CCR0 = (100);                                            // Sets interrupt trigger at 5 ms
   TACTL = TASSEL_2 + MC_1; 

   UCB0CTL0 |= UCCKPL + UCMSB + UCMST + /* UCMODE_2 */ + UCSYNC;


   UCB0CTL1 |= UCSSEL_2;

   UCB0CTL1 &= ~UCSWRST; 
   _BIS_SR(LPM0_bits + GIE);
   return 0;
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timera_A(void)
{
   P1OUT ^= BIT0;
}