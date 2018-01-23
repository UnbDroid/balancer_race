#include <wiringPi.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define PWM1 12
#define M1A 11
#define M1B 13

#define PWM2 33
#define M2A 29
#define M2B 31

#define LMOTOR 0
#define RMOTOR 1

#define DEBUG_MOTORS 0
#define MOTOR_DEBUG_DELAY_TIME 200
#define DEBUG_PRINT_FIRST_LINE 32

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

void init_motors()
{
	pinMode(DIR_PINA, INPUT);
	pinMode(DIR_PINB, INPUT);
	wiringPiISR(ENC_PINA, INT_EDGE_FALLING, &encoderInterruptA);
	wiringPiISR(ENC_PINB, INT_EDGE_FALLING, &encoderInterruptB);
	pinMode(PWM1, PWM_OUTPUT);
	pinMode(PWM2, PWM_OUTPUT);
	pinMode(M1A, OUTPUT);
	pinMode(M1B, OUTPUT);
	pinMode(M2A, OUTPUT);
	pinMode(M2B, OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(1023);
	pwmSetClock(2);
	if(DEBUG_MOTORS)
	{
		printf("\033[%d;%dHLeft Motor\nPos:            Freq:\nRight Motor\nPos:            Freq:", DEBUG_PRINT_FIRST_LINE, 0);
	}
}

void OnFwd(int motor, int power)
{
	if(power >= 0)
	{
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M1A, HIGH);
			digitalWrite(M1B, LOW);
	    	pwmWrite(PWM1, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M2A, LOW);
        	digitalWrite(M2B, HIGH);
        	pwmWrite(PWM2, power);
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M1A, LOW);
			digitalWrite(M1B, HIGH);
	    	pwmWrite(PWM1, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M2A, HIGH);
        	digitalWrite(M2B, LOW);
        	pwmWrite(PWM2, power);
		}
	}
}

void OnRev(int motor, int power)
{
	if(power >= 0)
	{
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M1A, LOW);
			digitalWrite(M1B, HIGH);
	    	pwmWrite(PWM1, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M2A, HIGH);
        	digitalWrite(M2B, LOW);
        	pwmWrite(PWM2, power);
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M1A, HIGH);
			digitalWrite(M1B, LOW);
	    	pwmWrite(PWM1, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M2A, LOW);
        	digitalWrite(M2B, HIGH);
        	pwmWrite(PWM2, power);
		}
	}
}

void Brake(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M1A, HIGH);
		digitalWrite(M1B, HIGH);
		pwmWrite(PWM1, 0);
	} else if (motor == RMOTOR) {
		digitalWrite(M2A, HIGH);
		digitalWrite(M2B, HIGH);
		pwmWrite(PWM2, 0);
	}
}

void Coast(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M1A, LOW);
		digitalWrite(M1B, LOW);
		pwmWrite(PWM1, 0);
	} else if (motor == RMOTOR) {
		digitalWrite(M2A, LOW);
		digitalWrite(M2B, LOW);
		pwmWrite(PWM2, 0);
	}
}

void update_debug_encoders()
{
		printf("\033[%d;%dH%'010lld\n", DEBUG_PRINT_FIRST_LINE+1, 6, posCounterA);
	    printf("\033[%d;%dH%05ld\n", DEBUG_PRINT_FIRST_LINE+1, 22, tickFreqA);
	    printf("\033[%d;%dH%'010lld\n", DEBUG_PRINT_FIRST_LINE+3, 6, posCounterB);
	    printf("\033[%d;%dH%05ld\n", DEBUG_PRINT_FIRST_LINE+3, 22, tickFreqB);
	    delay(MOTOR_DEBUG_DELAY_TIME);
}

int TachoCount(int motor)
{
	if(motor == LMOTOR)
	{
		return posCounterA;
	} else if(motor == RMOTOR)
	{
		return posCounterB;
	} else {
		return 0;
	}
}

int TachoSpeed(int motor)
{
	if(motor == LMOTOR)
	{
		return tickFreqA;
	} else if(motor == RMOTOR)
	{
		return tickFreqB;
	} else {
		return 0;
	}	
}