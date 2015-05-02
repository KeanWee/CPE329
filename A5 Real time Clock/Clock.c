#include <msp430.h>
#include <msp430g2452.h>
#include <stdio.h>

#define RSHigh    P2OUT |= BIT0
#define RSLow     P2OUT &= ~BIT0
#define READ      P2OUT |= BIT1
#define Write     P2OUT &= ~BIT1
#define EHigh     P2OUT |= BIT2
#define ELow      P2OUT &= ~BIT2

void EPulse(void);
void SendCommand(char cmd);
void SendString(char *string);
void LCD_Init(void);
void LCD_Write(char input);
void LCD_DDRAM_SET(char addr);
void PrintScroll(char *string);

volatile int count = 0;
volatile int seconds = 0;
volatile int minutes = 0;
volatile int hours = 0;



int main(void){
   WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
   
   if (CALBC1_1MHZ==0xFF)    // If calibration constant erased
      while(1);              // do not load, trap CPU!!
   DCOCTL = 0;               // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_1MHZ;    // Set range
   DCOCTL = CALDCO_1MHZ;     // Set DCO step + modulation

   LCD_Init();

   P1DIR = 0x00;
   P2DIR = 0x00;
   P1DIR |= 0xF0;
   P2DIR |= 0x07;



   SendCommand(0x06);


   P1DIR |= BIT0 + BIT1;                            // P1.0 output
   CCTL0 = CCIE;                             // CCR0 interrupt enabled
   CCR0 = 32768;                             // Sets interrupt trigger at 1 second
   TACTL = TASSEL_1 + MC_2;                  // ACLK, contmode


   _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt

   return 0;
}

/*Prints scrolling text to the LCD*/
void PrintScroll(char *string){
   SendCommand(0x06);
   int delay = 0;
   char *str = string;
   char *start = str;
   char *p = start;
   int length = 0;
   while (*p++)                    //calculate length of string
      length++;
   if(length <= 16){
      SendString(string);
      return;
   }
   char *end = start + length;
   while (start + 16 <= end){
      char *blockStart=start;
      char *blockEnd=start + 16;
      while(blockEnd - blockStart >= 0){
         LCD_Write(*blockStart);   //prints to LCD
         blockStart++;
      }
      LCD_DDRAM_SET(0x00);         //resets DDRAM
      __delay_cycles(160000);
      if(!delay)
      {
         __delay_cycles(160000);
         delay++;
      }
      start++;
   }
}


/*Initialize the LCD to Nibble Mode*/
void LCD_Init(void)
{
  
   P1DIR |= 0xF0;             // Enables Port 1 for outputs
   P2DIR |= 0x7;              // Enables Port 2 for outputs
   P1OUT &= 0x00;             // Clears Port 1
   P2OUT &= 0x00;             // Clears Port 2
   __delay_cycles(21000);     // Wait 20ms
   SendCommand(0x28);
   __delay_cycles(100);       // Wait 37us
   SendCommand(0x0C);
   __delay_cycles(100);       // Wait 37us
   SendCommand(0x01);
   __delay_cycles(2000);      // Wait 1.52ms
}


/*Writes a single character to the LCD*/
void LCD_Write(char input)    // Write character to DDRAM location
{
   RSHigh;
   Write;
   P1OUT = 0x00;
   P1OUT |= input;            // Set Data bus to input
   __delay_cycles(1);         // Wait 40ns
   EPulse();
   P1OUT = 0x00;
   P1OUT |= (input << 4);
   __delay_cycles(1);
   EPulse();
}

/*Sets the LCDDDRAM address*/
void LCD_DDRAM_SET(char addr) // Sets DDRAM location to address
{
   P1OUT = 0x00;
   P1OUT = addr;              // Set Data bus to address
   P2OUT &= ~BIT1 + ~BIT2;    // Set Port 2 pins
   P1OUT |= BIT7;             // Force DB7 to 1
   __delay_cycles(1);         // Wait 40ns
   EPulse();
   P1OUT = 0x00;
   P1OUT |= (addr << 4);
   __delay_cycles(1);
   EPulse();
}

/*Pulses the Enable pin on the LCD*/
void EPulse(void)
{
   ELow;
   __delay_cycles(1);
   EHigh;
   __delay_cycles(5);
   ELow;
   __delay_cycles(5);
}

/*Sends a command to the LCD*/
void SendCommand(char cmd)
{
   RSLow;
   Write;
   P1OUT = 0x00;
   P1OUT |= cmd;
   EPulse();
   P1OUT = 0x00;
   P1OUT |= (cmd << 4);
   EPulse();
}


/*Sends a string characters to be displayed on the LCD*/
void SendString(char *string)
{
   while(*string)
   {
      LCD_Write(*string);
      string++;
      __delay_cycles(80000);
   }
}




// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    char OutString[16];      // String for output to LCD
    P1OUT ^= BIT0;           // Toggles LED for testing
    seconds++;               // Increments seconds and checks whether to increment minutes and hours
    if (seconds == 60){
      seconds = 0;
      minutes++;
    }
    if (minutes == 60){
      minutes = 0;
      hours++;
    }
    if (hours == 24){
      hours = 0;
    }

    sprintf(OutString, "%02d : %02d : %02d",hours, minutes, seconds);  // Converts variables into the string
    LCD_DDRAM_SET(0x00);
    SendString(OutString);    // Writes string to LCD
    CCR0+=32768;


}
