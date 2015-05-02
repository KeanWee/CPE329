#include <msp430g2452.h>
#include <msp430.h>
#define RShigh P2OUT |= BIT0 //set RS high (send command)
#define RSlow P2OUT &= ~BIT0 //set RS low (send data)
#define READ P2OUT |= BIT1 //set R/W high (read mode)
#define WRITE P2OUT &= ~BIT1 //set R/W low (write mode)
#define ENABLEhigh P2OUT |= BIT2 //set EN pin to 1
#define ENABLElow P2OUT &= ~BIT2 //set EN pin to 0
void init(void){
//16Mhz
if (CALBC1_16MHZ==0xFF) // If calibration constant erased
{
while(1); // do not load, trap CPU!!
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
void ENABLEdelay(void){
__delay_cycles(4);
ENABLEhigh;
__delay_cycles(500);
ENABLElow;
__delay_cycles(1000);
}
void SENDcommand(int command){
RSlow;
WRITE;
// ============= 8­bit mode ==============
P1OUT &= 0x00;
P1OUT |= command;
ENABLEdelay();

/* =======================================
// ============ nibble mode ==============
P1OUT &= 0x00;
P1OUT |= command;
ENABLEdelay();
P1OUT &= 0x00;
P1OUT |= (command << 4);
ENABLEdelay();
// =======================================
*/}
void SENDdata(char data){
RShigh;
WRITE;
// ============= 8­bit mode ==============
P1OUT &= 0x00;
P1OUT |= data;
ENABLEdelay();

// =======================================
// =========== nibble mode ===============
/*P1OUT &= 0x00;
P1OUT |= data;
ENABLEdelay();
P1OUT &= 0x00;
P1OUT |= (data << 4);
ENABLEdelay();
// =======================================
*/}
void SENDstring(char *string){
while (*string){
SENDdata(*string);
string++;
}
}

void LCDstartup(void){
init();
__delay_cycles(16000); //20 ms
// SENDcommand(0x38); //set function 8­bit
SENDcommand(0x33);
__delay_cycles(1000);
SENDcommand(0x32);
__delay_cycles(1000);
SENDcommand(0x28); //set function nibble mode
__delay_cycles(592); //37 us
SENDcommand(0x0C); //display & cursor on
__delay_cycles(592); //37 using
SENDcommand(0x01); //clears the display
__delay_cycles(24320); //1.52 ms
}
void SENDscrollstringleft(char *text){
while (*text){
SENDcommand(0x1B);
text++;
__delay_cycles(2000000);
}
SENDcommand(0x01);
__delay_cycles(25000);
}
void SENDscrollstringright(char *word){
while (*word){
SENDcommand(0x1F);
word++;
__delay_cycles(2000000);
}
SENDcommand(0x01);
__delay_cycles(25000);
}
void ISroutine(void){
__delay_cycles(500);
SENDcommand(0x01);
__delay_cycles(25000);
SENDstring(" Never gonna give you up");
SENDscrollstringleft("Never gonna give you up ");
SENDcommand(0xC0);
__delay_cycles(500);
SENDstring(" Never gonna let you down");
SENDscrollstringright("Never gonna let you down ");
SENDcommand(0x02);
__delay_cycles(2000000);
SENDstring(" Never gonna run around and desert you");
SENDscrollstringleft("Never gonna run around and ");
SENDcommand(0xC0);
__delay_cycles(500);
SENDstring(" Never gonna make you cry");
SENDscrollstringright("Never gonna make you cry ");
SENDcommand(0x02);
__delay_cycles(5000);
SENDstring(" Never gonna say goodbye");
SENDscrollstringleft("Never gonna say goodbye ");

SENDcommand(0xC0);
__delay_cycles(5000);
SENDstring(" Never gonna tell a lie and hurt you");
SENDscrollstringright("Never gonna tell a lie and ");
}
void main(void) {
WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
LCDstartup();
SENDstring("Hello World!");
// =============== BONUS=================
P2IE |= BIT3;
P2REN |= BIT3;
P2IES |= BIT3;
P2IFG &= ~BIT3;
_enable_interrupts();
// ======================================
while(1);
}
#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void){
ISroutine();
P2IFG &= ~BIT3;
}