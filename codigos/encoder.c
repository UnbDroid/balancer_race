#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>

#define DEBUG_ENCODERS 0
#define DELAY_TIME 200000

#define ENC_PINA 18
#define DIR_PINA 16
#define ENC_PINB 38
#define DIR_PINB 36

volatile long long int posCounterA = 0, posCounterB = 0;
volatile unsigned long int tickFreqA = 0, tickFreqB = 0;
volatile unsigned long int lastTickA = 0, lastTickB = 0;
volatile unsigned long int currTickA = 0, currTickB = 0;

void encoderInterruptA(void) {
  if(digitalRead(DIR_PINA))
  {
    ++posCounterA;
  } else {
    --posCounterA;
  }
  currTickA = micros();
  tickFreqA = 1000000/(currTickA-lastTickA);
  lastTickA = currTickA;
}

void encoderInterruptB(void) {
  if(digitalRead(DIR_PINB))
  {
    ++posCounterB;
  } else {
    --posCounterB;
  }
  currTickB = micros();
  tickFreqB = 1000000/(currTickB-lastTickB);
  lastTickB = currTickB;
}

void init_encoders()
{
	pinMode(DIR_PINA, INPUT);
	pinMode(DIR_PINB, INPUT);
	wiringPiISR(ENC_PINA, INT_EDGE_FALLING, &encoderInterruptA);
	wiringPiISR(ENC_PINB, INT_EDGE_FALLING, &encoderInterruptB);
}

void print_debug_encoders()
{
	while(1)
	{
		printf("\033[%d;%dH%'010lld\n", 1, 0, posCounterA);
	    printf("\033[%d;%dH%05ld\n", 2, 0, tickFreqA);
	    printf("\033[%d;%dH%'010lld\n", 1, 20, posCounterB);
	    printf("\033[%d;%dH%05ld\n", 2, 20, tickFreqB);
	    delayMicroseconds(DELAY_TIME);		
	}
}