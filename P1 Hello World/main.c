#include <msp430.h>
#define RShigh P2OUT |= BIT0
#define RSlow P2OUT &= ~BIT0
#define READ P2OUT |= BIT1
#define WRITE P2OUT &= ~BIT1
#define ENABLE P2OUT |= BIT2
#define DISABLE P2OUT &= ~BIT2

void init(void){
  if (CALBC1_16MHZ==0xFF)     // If calibration constant erased
      while(1);               // do not load, trap CPU!!
   DCOCTL = 0;                // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_16MHZ;    // Set range
   DCOCTL = CALDCO_16MHZ; 

   P1DIR |= 0xFF;
   P2DIR |= 0x07;

   P1OUT &= 0x00;
   P2OUT &= 0x00;
}

void ENABLEdelay(void){
   __delay_cycles(4);
   ENABLE;
   __delay_cycles(500);
   DISABLE;
   __delay_cycles(1000);
}

void SENDcommand(int command){
   RSlow;
   WRITE;
   P1OUT &= 0x00;
   P1OUT |= command;
   ENABLEdelay();
}
void SENDdata(int data){
   RShigh;
   WRITE;
   P1OUT &= 0x00;
   P1OUT |= data;
   ENABLEdelay();
}
void SENDstring(char *string){
   while (*string){
      SENDdata(*string);
   }
}

void LCDstartup(void){
   init();
   __delay_cycles(16000);
   SENDcommand(0x38);
   __delay_cycles(1000);
   SENDcommand(0x32);
   __delay_cycles(1000);
   SENDcommand(0x28);
   __delay_cycles(592);
   SENDcommand(0x0C);
   __delay_cycles(592);
   SENDcommand(0x01);
   __delay_cycles(24320);
}

void main(void){
   WDTCTL = WDTPW |WDTHOLD;
   LCDstartup();
   SENDstring("Hello world!");
}