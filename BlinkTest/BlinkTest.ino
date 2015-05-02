

int main()
{
  WDTCTL = WDTPW | WDTHOLD;
  
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;
  while(1)
  {
    P1OUT ^= BIT0;
    delay(1000);
  }
  return 0;
}


