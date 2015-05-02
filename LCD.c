#ifndef LCDCode
#define LCDCode

// Data Bits P1.0 - P1.3

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

/*Initialize the LCD to Nibble Mode*/
void LCD_Init(void)
{

   P1DIR |= 0x0F;                         // Enables Port 1 for outputs
   P2DIR |= RSBIT + RWBIT + EBIT;         // Enables Port 2 for outputs
   P1OUT &= 0xF0;                         // Clears Port 1
   P2OUT &= ~(RSBIT + RWBIT + EBIT);      // Clears Port 2
   __delay_cycles(160000);
   __delay_cycles(160000);
   __delay_cycles(160000);
   SendCommand(0x28);
   __delay_cycles(2000);
   SendCommand(0x0C);
   __delay_cycles(2000);
   SendCommand(0x01);
   __delay_cycles(32500);
   SendCommand(0x06);
}

/*Writes a single character to the LCD*/
void LCD_Write(char input)
{
   RSHigh;
   Write;
   __delay_cycles(400);
   P1OUT &= 0xF0;
   P1OUT |= ((input >> 4) & 0x0F);            // Set Data bus to input
   __delay_cycles(40);         // Wait 40ns
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (input & 0x0F);
   __delay_cycles(40);
   EPulse();
}

/*Sets the LCD DDRAM address*/
void LCD_DDRAM_SET(char addr)
{
   P1OUT &= 0xF0;
   P1OUT |= (((addr >> 4) & 0x0F) | 0x08);    // Set Data bus to address
   Write;
   RSLow;
   __delay_cycles(50);         // Wait 40ns
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (addr & 0x0F);
   __delay_cycles(50);
   EPulse();
}

/*Pulses the Enable pin on the LCD high then low*/
void EPulse(void)
{
   ELow;
   __delay_cycles(20);
   EHigh;
   __delay_cycles(100);
   ELow;
   __delay_cycles(100);
}

/*Sends a command to the LCD*/
void SendCommand(char cmd)
{
   RSLow;
   Write;
   __delay_cycles(40);
   P1OUT &= 0xF0;
   P1OUT |= ((cmd >> 4) & 0x0F);
   EPulse();
   P1OUT &= 0xF0;
   P1OUT |= (cmd & 0x0F);
   EPulse();
}

/*Sends a string characters to be displayed on the LCD*/
void SendString(char *string)
{
   while(*string)
   {
      __delay_cycles(100000);//80000 to look nice
      LCD_Write(*string);
      string++;

   }
}

#endif