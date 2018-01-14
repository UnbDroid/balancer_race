/*
isr4pi.c
D. Thiebaut
based on isr.c from the WiringPi library, authored by Gordon Henderson
https://github.com/WiringPi/WiringPi/blob/master/examples/isr.c

Compile as follows:

    gcc -o isr4pi isr4pi.c -lwiringPi

Run as follows:

    sudo ./isr4pi

 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define ENC_PINA 15
#define DIR_PINA 16
#define ENC_PINB 31
#define DIR_PINB 32

#define DELAY_TIME 200000

// the event counter 
volatile long int posCounterA = 0, posCounterB = 0;
volatile unsigned long int ticksCounterA = 0, ticksCounterB = 0;
volatile unsigned long int tickFreqA = 0, tickFreqB = 0;
volatile unsigned int lastTickA = 0, currTickA = 0, lastTickB = 0, currTickB = 0;
//volatile unsigned int numTicksA = 0, numTicksB = 0;

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void encoderInterruptA(void) {
  //++ticksCounter;
  
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
  //++ticksCounter;
  
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

// -------------------------------------------------------------------------
// main
int main(void) {
  char resposta;

  printf("Are you su?(y/n)\n");
  scanf("%c", &resposta);
  while(resposta!='y' && resposta!='n')
  {
    printf("y or n:\n");
    scanf("%c", &resposta);
  }
  if(resposta!='y')
    return 0;

  // sets up the wiringPi library
  if (wiringPiSetupPhys() < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }
  pinMode(DIR_PINA, INPUT);
  pinMode(DIR_PINB, INPUT);
  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (ENC_PINA, INT_EDGE_FALLING, &encoderInterruptA) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

   if ( wiringPiISR (ENC_PINB, INT_EDGE_FALLING, &encoderInterruptB) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }


  

  // display counter value every second.
  printf("\e[2J\e[H");
  while(1) {
    
    printf("\033[%d;%dH%'010d\n", 1, 0, posCounterA);
    //posCounter = 0;
    //printf("\033[%d;%dH%010d\n", 2, 0, ticksCounter);
    printf("\033[%d;%dH%05ld\n", 2, 0, tickFreqA);
    //tickFreq = 0;
    //numTicks = 0;
    printf("\033[%d;%dH%'010d\n", 1, 20, posCounterB);
    //posCounter = 0;
    //printf("\033[%d;%dH%010d\n", 2, 0, ticksCounter);
    printf("\033[%d;%dH%05ld\n", 2, 20, tickFreqB);
    delayMicroseconds(DELAY_TIME); // wait 1 milissecond
  }

  return 0;
}