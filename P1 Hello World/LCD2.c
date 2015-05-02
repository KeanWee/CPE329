#include <msp430.h>
#include <msp430g2452.h>
#include <msp430g2553.h>

#define RSHigh    P2OUT |= BIT0
#define RSLow     P2OUT &= ~BIT0
#define READ      P2OUT |= BIT1
#define Write     P2OUT &= ~BIT1
#define EHigh     P2OUT |= BIT2
#define ELow      P2OUT &= ~BIT2

void EPulse(void);
void SendCommand(char cmd);
void SendString(char *string);

void LCD_Init(void)           // Initializes LCD with byte mode
{
   // Setup LCD
   P1DIR |= 0xFF;             // Enables Port 1 for outputs
   P2DIR |= 0x7;              // Enables Port 2 for outputs
   P1OUT &= 0x00;             // Clears Port 1
   P2OUT &= 0x00;             // Clears Port 2
   __delay_cycles(21000);    // Wait 20ms
   SendCommand(0x38);
   __delay_cycles(100);       // Wait 37us
   SendCommand(0x0F);
   __delay_cycles(100);       // Wait 37us
   SendCommand(0x01);
   __delay_cycles(2000);     // Wait 1.52ms
}

void LCD_Write(char input)    // Write character to DDRAM location
{
   RSHigh;
   Write;
   P1OUT = 0x00;
   P1OUT |= input;             // Set Data bus to input
   __delay_cycles(1);         // Wait 40ns
   EPulse();
}

void LCD_DDRAM_SET(char addr) // Sets DDRAM location to address
{
   P1OUT = 0x00;
   P1OUT = addr;              // Set Data bus to address
   P2OUT &= ~BIT1 + ~BIT2;    // Set Port 2 pins
   P1OUT |= BIT7;             // Force DB7 to 1
   __delay_cycles(1);         // Wait 40ns
   EPulse();
}

void EPulse(void)
{
   ELow;
   __delay_cycles(1);
   EHigh;
   __delay_cycles(5);
   ELow;
   __delay_cycles(5);
}

void SendCommand(char cmd)
{
   RSLow;
   Write;
   P1OUT = 0x00;
   P1OUT |= cmd;
   EPulse();
}

void SendString(char *string)
{
   while(*string)
   {
      LCD_Write(*string);
      string++;
   }
}

int main(void)
{
   WDTCTL = WDTPW + WDTHOLD;  // Shuts off watchdog timer

  if (CALBC1_1MHZ==0xFF)     // If calibration constant erased
      while(1);               // do not load, trap CPU!!
   DCOCTL = 0;                // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_1MHZ;    // Set range
   DCOCTL = CALDCO_1MHZ;     // Set DCO step + modulation
   LCD_Init();                // Calls LCD initilization
   // LCD_DDRAM_SET(0x01);       // Sets DDRAM locaiton to the first spot
   // LCD_Write('b');           // Writes an "@" to the cursor
   SendString("Hello world,");
   LCD_DDRAM_SET(0x40);
   SendString("This is Kean");
   while(1);
}



