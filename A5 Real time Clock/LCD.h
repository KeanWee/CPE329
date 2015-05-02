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