{\rtf1\ansi\ansicpg1252\cocoartf1344\cocoasubrtf720
{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural

\f0\fs24 \cf0 #include <msp430g2452.h>\
#include <msp430.h>\
#define RShigh P2OUT |= BIT0 //set RS high (send command)\
#define RSlow P2OUT &= ~BIT0 //set RS low (send data)\
#define READ P2OUT |= BIT1 //set R/W high (read mode)\
#define WRITE P2OUT &= ~BIT1 //set R/W low (write mode)\
#define ENABLEhigh P2OUT |= BIT2 //set EN pin to 1\
#define ENABLElow P2OUT &= ~BIT2 //set EN pin to 0\
void init(void)\{\
//16Mhz\
   if (CALBC1_16MHZ==0xFF) \
   \{\
      while(1)\uc0\u894  \
   \}\
   DCOCTL = 0\uc0\u894  // Select lowest DCOx and MODx settings\
   BCSCTL1 = CALBC1_16MHZ\uc0\u894  // Set range   \
   DCOCTL = CALDCO_16MHZ\uc0\u894  // Set DCO step + modulation\
// 1 clock = 62.5 ns\
P1DIR |= 0xFF\uc0\u894 \
P2DIR |= 0x07\uc0\u894 \
P1OUT &= 0x00\uc0\u894 \
\
P2OUT &= 0x00\uc0\u894 \
\}\
void ENABLEdelay(void)\{\
__delay_cycles(4)\uc0\u894 \
ENABLEhigh\uc0\u894 \
__delay_cycles(500)\uc0\u894 \
ENABLElow\uc0\u894 \
__delay_cycles(1000)\uc0\u894 \
\}\
void SENDcommand(int command)\{\
RSlow\uc0\u894 \
WRITE\uc0\u894 \
// ============= 8\'adbit mode ==============\
P1OUT &= 0x00\uc0\u894 \
P1OUT |= command\uc0\u894 \
ENABLEdelay()\uc0\u894 \
\
/* =======================================\
// ============ nibble mode ==============\
P1OUT &= 0x00\uc0\u894 \
P1OUT |= command\uc0\u894 \
ENABLEdelay()\uc0\u894 \
P1OUT &= 0x00\uc0\u894 \
P1OUT |= (command << 4)\uc0\u894 \
ENABLEdelay()\uc0\u894 \
// =======================================\
*/\}\
void SENDdata(char data)\{\
RShigh\uc0\u894 \
WRITE\uc0\u894 \
// ============= 8\'adbit mode ==============\
P1OUT &= 0x00\uc0\u894 \
P1OUT |= data\uc0\u894 \
ENABLEdelay()\uc0\u894 \
\
// =======================================\
// =========== nibble mode ===============\
/*P1OUT &= 0x00\uc0\u894 \
P1OUT |= data\uc0\u894 \
ENABLEdelay()\uc0\u894 \
P1OUT &= 0x00\uc0\u894 \
P1OUT |= (data << 4)\uc0\u894 \
ENABLEdelay()\uc0\u894 \
// =======================================\
*/\}\
void SENDstring(char *string)\{\
while (*string)\{\
SENDdata(*string)\uc0\u894 \
string++\uc0\u894 \
\}\
\}\
\
void LCDstartup(void)\{\
init()\uc0\u894 \
__delay_cycles(16000)\uc0\u894  //20 ms\
// SENDcommand(0x38)\uc0\u894  //set function 8\'adbit\
SENDcommand(0x33)\uc0\u894 \
__delay_cycles(1000)\uc0\u894 \
SENDcommand(0x32)\uc0\u894 \
__delay_cycles(1000)\uc0\u894 \
SENDcommand(0x28)\uc0\u894  //set function nibble mode\
__delay_cycles(592)\uc0\u894  //37 us\
SENDcommand(0x0C)\uc0\u894  //display & cursor on\
__delay_cycles(592)\uc0\u894  //37 using\
SENDcommand(0x01)\uc0\u894  //clears the display\
__delay_cycles(24320)\uc0\u894  //1.52 ms\
\}\
void SENDscrollstringleft(char *text)\{\
while (*text)\{\
SENDcommand(0x1B)\uc0\u894 \
text++\uc0\u894 \
__delay_cycles(2000000)\uc0\u894 \
\}\
SENDcommand(0x01)\uc0\u894 \
__delay_cycles(25000)\uc0\u894 \
\}\
void SENDscrollstringright(char *word)\{\
while (*word)\{\
SENDcommand(0x1F)\uc0\u894 \
word++\uc0\u894 \
__delay_cycles(2000000)\uc0\u894 \
\}\
SENDcommand(0x01)\uc0\u894 \
__delay_cycles(25000)\uc0\u894 \
\}\
void ISroutine(void)\{\
__delay_cycles(500)\uc0\u894 \
SENDcommand(0x01)\uc0\u894 \
__delay_cycles(25000)\uc0\u894 \
SENDstring(" Never gonna give you up")\uc0\u894 \
SENDscrollstringleft("Never gonna give you up ")\uc0\u894 \
SENDcommand(0xC0)\uc0\u894 \
__delay_cycles(500)\uc0\u894 \
SENDstring(" Never gonna let you down")\uc0\u894 \
SENDscrollstringright("Never gonna let you down ")\uc0\u894 \
SENDcommand(0x02)\uc0\u894 \
__delay_cycles(2000000)\uc0\u894 \
SENDstring(" Never gonna run around and desert you")\uc0\u894 \
SENDscrollstringleft("Never gonna run around and ")\uc0\u894 \
SENDcommand(0xC0)\uc0\u894 \
__delay_cycles(500)\uc0\u894 \
SENDstring(" Never gonna make you cry")\uc0\u894 \
SENDscrollstringright("Never gonna make you cry ")\uc0\u894 \
SENDcommand(0x02)\uc0\u894 \
__delay_cycles(5000)\uc0\u894 \
SENDstring(" Never gonna say goodbye")\uc0\u894 \
SENDscrollstringleft("Never gonna say goodbye ")\uc0\u894 \
\
SENDcommand(0xC0)\uc0\u894 \
__delay_cycles(5000)\uc0\u894 \
SENDstring(" Never gonna tell a lie and hurt you")\uc0\u894 \
SENDscrollstringright("Never gonna tell a lie and ")\uc0\u894 \
\}\
void main(void) \{\
WDTCTL = WDTPW | WDTHOLD\uc0\u894  // Stop watchdog timer\
LCDstartup()\uc0\u894 \
SENDstring("Hello World!")\uc0\u894 \
// =============== BONUS=================\
P2IE |= BIT3\uc0\u894 \
P2REN |= BIT3\uc0\u894 \
P2IES |= BIT3\uc0\u894 \
P2IFG &= ~BIT3\uc0\u894 \
_enable_interrupts()\uc0\u894 \
// ======================================\
while(1)\uc0\u894 \
\}\
#pragma vector = PORT2_VECTOR\
__interrupt void Port_2(void)\{\
ISroutine()\uc0\u894 \
P2IFG &= ~BIT3\uc0\u894 \
\}}