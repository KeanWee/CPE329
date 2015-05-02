#ifndef LCDHeader
#define LCDHeader

// Need to define: RSBIT, RWBIT, EBIT
// Defaults:       0    , 1    , 2

#define RSHigh    P2OUT |= RSBIT
#define RSLow     P2OUT &= ~RSBIT
#define READ      P2OUT |= RWBIT
#define Write     P2OUT &= ~RWBIT
#define EHigh     P2OUT |= EBIT
#define ELow      P2OUT &= ~EBIT

void EPulse(void);
void SendCommand(char cmd);
void SendString(char *string);
void LCD_Init(void);
void LCD_Write(char input);
void LCD_DDRAM_SET(char addr);
void PrintScroll(char *string);

#endif