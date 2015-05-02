#include <msp430.h>


void init(void){
//16Mhz
   if (CALBC1_16MHZ==0xFF) 
   {
      while(1); 
   }
   DCOCTL = 0; // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_16MHZ; // Set range   
   DCOCTL = CALDCO_16MHZ; // Set DCO step + modulation
// 1 clock = 62.5 ns
P1DIR |= 0xFF;
P2DIR |= 0x07;
P1OUT &= 0x00;

P2OUT &= 0x00;
}


void SENDstring(char *string){
while (*string){
SENDdata(*string);
string++;
}
}