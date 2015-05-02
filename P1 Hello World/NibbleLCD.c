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
void LCD_Init(void);
void LCD_Write(char input);
void LCD_DDRAM_SET(char addr);
void PrintScroll(char *string);



int main(void)
{
   WDTCTL = WDTPW + WDTHOLD;  // Shuts off watchdog timer

   if (CALBC1_1MHZ==0xFF)     // If calibration constant erased
      while(1);               // do not load, trap CPU!!
   DCOCTL = 0;                // Select lowest DCOx and MODx settings
   BCSCTL1 = CALBC1_1MHZ;    // Set range
   DCOCTL = CALDCO_1MHZ;     // Set DCO step + modulation
   LCD_Init();                // Calls LCD initilization
   SendCommand(0x01);
   LCD_DDRAM_SET(0x00);
   PrintScroll(" This string is pretty damn long. Like I mean this is a really really really really long string");
   LCD_DDRAM_SET(0x40);
   PrintScroll("Yes           No");
   // while(!(P1IN & 0x03));
   // if(P1IN & 0x01){
   //    PrintScroll(" You Die");
   //    return 0;
   // }
   // else{
   //    PrintScroll("part 2");
   //    while(!(P1IN & 0x03));
   //    if(P1IN & 0x01){
   //       PrintScroll(" You Die");
   //       return 0;
   //    }
   //    else{
   //       PrintScroll("Part 3");
   //       while(!(P1IN & 0x03));
   //       if(P1IN & 0x01){
   //          PrintScroll(" You die");
   //          return 0;
   //       }
   //       else{
   //          PrintScroll("Winning");
   //          return 0;
   //       }
   //    }
   // }
   return 0;
}

void PrintScroll(char *string){
    SendCommand(0x06);
    int delay = 0;
    char *str = string;
    char *start = str;
    char *p = start;
    int length = 0;
    while (*p++)
        length++;
    //printf("len: %d\n",length);
      if(length <= 16){
         SendString(string);
         return;
      }
    char *end = start + length;
    while (start + 16 <= end){
        char *blockStart=start;
        char *blockEnd=start + 16;
        while(blockEnd - blockStart >= 0){
        //    printf("%c",*blockStart);
            LCD_Write(*blockStart);
            blockStart++;
        }
        LCD_DDRAM_SET(0x00);
        __delay_cycles(160000);
        if(!delay)
        {
         __delay_cycles(160000);
         delay++;
        }
    //    printf("\n");
        
        start++;
    }
}

void LCD_Init(void)           // Initializes LCD with byte mode
{
   // Setup LCD
   P1DIR |= 0xF0;             // Enables Port 1 for outputs
   P2DIR |= 0x7;              // Enables Port 2 for outputs
   P1OUT &= 0x00;             // Clears Port 1
   P2OUT &= 0x00;             // Clears Port 2
   __delay_cycles(21000);    // Wait 20ms
   SendCommand(0x28);
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
   P1OUT = 0x00;
   P1OUT |= (input << 4);
   __delay_cycles(1);
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
   P1OUT = 0x00;
   P1OUT |= (addr << 4);
   __delay_cycles(1);
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
   P1OUT = 0x00;
   P1OUT |= (cmd << 4);
   EPulse();
}

void SendString(char *string)
{
   while(*string)
   {
      LCD_Write(*string);
      string++;
      __delay_cycles(80000);
   }
}




