#define RSHigh    P2OUT |= BIT0
#define RSLow     P2OUT &= ~BIT0
#define READ      P2OUT |= BIT1
#define Write     P2OUT &= ~BIT1
#define EHigh     P2OUT |= BIT2
#define ELow      P2OUT &= ~BIT2

void PrintScroll(char *string){
    SendCommand(0x06);
    int delay = 0;
    char *str = string;
    char *start = str;
    char *p = start;
    int length = 0;
    while (*p++)
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
   SendCommand(0x0C);
   __delay_cycles(100);       // Wait 37us
   SendCommand(0x01);
   __delay_cycles(2000);     // Wait 1.52ms
   SendCommand(0x06);
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