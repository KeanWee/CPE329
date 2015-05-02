#include <msp430.h>
#include <legacymsp430.h>
int main(void) {
 WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
 P1DIR |= BIT6 + BIT0; // P1.4 and P1.0 output bits for toggling
 P1OUT |= BIT6 + BIT0; // Start with LEDs illuminated
 P1IE |= BIT3 + BIT2; // Enable interrupts for P1.3 and P1.2
 P1REN |= BIT3 + BIT2; // Add an internal pullup resistor to P1.3 and P1.2
 P1IES |= BIT3 + BIT2; // Select high to low edge Interrupt on P1.3 and P1.2
 P1IFG &= ~(BIT3 + BIT2); // Clear the interrupt flags to ensure system
 // starts with no interrupts
 // LOW POWER MODE VERSION
 _BIS_SR(LPM4_bits + GIE); // Let's Go (to Sleep) in Low Power Mode 4 with
 // interrupts enabled!
 // MAIN LOOP VERSION, GIVES SAME RESULTS
 //_enable_interrupts(); // enable global interrupts
 //while(1); // main loop, looking for an interrupt,
 // just wasting CPU cycles otherwise
}
#pragma vector=PORT1_VECTOR // Interrupt vector
__interrupt void Port_1(void){ // ISR
 // CHECK AND CLEAR EACH POSSIBLE BUTTON INTERRUPT SEPARATELY
 // NOTE THAT BOTH ARE ALWAYS CHECKED AND NO 'ELSE' IS USED.
 if (P1IFG & BIT2){
 P1OUT ^= BIT0; 
 __delay_cycles(160000);// Toggle P1.0
 P1IFG &= ~BIT2; // Clear the Bit 2 interrupt flag,
 // leave all other bits untouched
 }
 if (P1IFG & BIT3){
 P1OUT ^= BIT6; // Toggle P1.4
 P1IFG &= ~BIT3; // Clear the Bit 3 interrupt flag,
 // leave all other bits untouched
 }
}