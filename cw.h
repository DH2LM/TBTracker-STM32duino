#ifndef _CW_H_
#define _CW_H_

//This header file contains CW blink codes for the red LED in case of an error occuring.

void dit()
{
  digitalWrite(LED_RED, HIGH);
  delay(100);
  digitalWrite(LED_RED, LOW);
  delay(100);
}

void dah()
{
  digitalWrite(LED_RED, HIGH);
  delay(300);
  digitalWrite(LED_RED, LOW);
  delay(100);
}

void giveL()
{
  dit();
  dah();
  dit();
  dit();
}

void give4()
{
  dit();
  dit();
  dit();
  dit();
  dah();
}

void giveF()
{
  dit();
  dit();
  dah();
  dit();
}

void giveS()
{
  dit();
  dit();
  dit();
}

void giveR()
{
  dit();
  dah();
  dit();
}

void giveH()
{
  dit();
  dit();
  dit();
  dit();
}

#endif