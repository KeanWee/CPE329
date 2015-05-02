#include <msp430g2553.h>
#include <msp430.h>

#define RSBIT BIT0
#define RWBIT BIT1
#define EBIT  BIT2
#define CSBIT BIT4
#define SIMOBIT BIT7
#define CLKBIT BIT5

#include "LCD.h"
#include "LCD.c"

int main(void){
   WDTCTL = WDTPW + WDTHOLD;        // Stop watchdog timer

  // 16Mhz SMCLK
  if (CALBC1_1MHZ==0xFF)        // If calibration constant erased
  {
    while(1);                        // do not load, trap CPU!!
  }
  DCOCTL = 0;                        // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;            // Set range
  DCOCTL = CALDCO_1MHZ;             // Set DCO step + modulation

  LCD_Init();
  SendCommand(0x01);
  while(1);
}