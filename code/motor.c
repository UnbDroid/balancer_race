#include <wiringPi.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define PWM_LEFT 12
#define M_LEFT_A 11
#define M_LEFT_B 13

#define PWM_RIGHT 33
#define M_RIGHT_A 29
#define M_RIGHT_B 31

#define LMOTOR 0
#define RMOTOR 1

#define ENC_PIN_LEFT 18
#define DIR_PIN_LEFT 16
#define ENC_PIN_RIGHT 38
#define DIR_PIN_RIGHT 36

struct motor {
	volatile long long int posCounter;
	volatile unsigned long int tickFreq;
	volatile unsigned long int lastTick;
	volatile unsigned long int currTick;
};

struct motor left_motor, right_motor;

void encoderInterruptLeft(void) {
  if(digitalRead(DIR_PIN_LEFT))
  {
    left_motor.posCounter += 1;
  } else {
    left_motor.posCounter -= 1;
  }
  left_motor.currTick = micros();
  left_motor.tickFreq = 1000000/(left_motor.currTick-left_motor.lastTick);
  left_motor.lastTick = left_motor.currTick;
}

void encoderInterruptRight(void) {
  if(digitalRead(DIR_PIN_RIGHT))
  {
    right_motor.posCounter += 1;
  } else {
    right_motor.posCounter -= 1;
  }
  right_motor.currTick = micros();
  right_motor.tickFreq = 1000000/(right_motor.currTick-right_motor.lastTick);
  right_motor.lastTick = right_motor.currTick;
}

void init_motors()
{
	pinMode(DIR_PIN_LEFT, INPUT);
	pinMode(DIR_PIN_RIGHT, INPUT);
	wiringPiISR(ENC_PIN_LEFT, INT_EDGE_FALLING, &encoderInterruptLeft);
	wiringPiISR(ENC_PIN_RIGHT, INT_EDGE_FALLING, &encoderInterruptRight);
	pinMode(PWM_LEFT, PWM_OUTPUT);
	pinMode(PWM_RIGHT, PWM_OUTPUT);
	pinMode(M_LEFT_A, OUTPUT);
	pinMode(M_LEFT_B, OUTPUT);
	pinMode(M_RIGHT_A, OUTPUT);
	pinMode(M_RIGHT_B, OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(1023);
	pwmSetClock(2);
}

void OnFwd(int motor, int power)
{
	if(power >= 0)
	{
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M_LEFT_A, HIGH);
			digitalWrite(M_LEFT_B, LOW);
	    	pwmWrite(PWM_LEFT, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, LOW);
        	digitalWrite(M_RIGHT_B, HIGH);
        	pwmWrite(PWM_RIGHT, power);
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M_LEFT_A, LOW);
			digitalWrite(M_LEFT_B, HIGH);
	    	pwmWrite(PWM_LEFT, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, HIGH);
        	digitalWrite(M_RIGHT_B, LOW);
        	pwmWrite(PWM_RIGHT, power);
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
	    	digitalWrite(M_LEFT_A, LOW);
			digitalWrite(M_LEFT_B, HIGH);
	    	pwmWrite(PWM_LEFT, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, HIGH);
        	digitalWrite(M_RIGHT_B, LOW);
        	pwmWrite(PWM_RIGHT, power);
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M_LEFT_A, HIGH);
			digitalWrite(M_LEFT_B, LOW);
	    	pwmWrite(PWM_LEFT, power);
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, LOW);
        	digitalWrite(M_RIGHT_B, HIGH);
        	pwmWrite(PWM_RIGHT, power);
		}
	}
}

void Brake(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M_LEFT_A, HIGH);
		digitalWrite(M_LEFT_B, HIGH);
		pwmWrite(PWM_LEFT, 0);
	} else if (motor == RMOTOR) {
		digitalWrite(M_RIGHT_A, HIGH);
		digitalWrite(M_RIGHT_B, HIGH);
		pwmWrite(PWM_RIGHT, 0);
	}
}

void Coast(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M_LEFT_A, LOW);
		digitalWrite(M_LEFT_B, LOW);
		pwmWrite(PWM_LEFT, 0);
	} else if (motor == RMOTOR) {
		digitalWrite(M_RIGHT_A, LOW);
		digitalWrite(M_RIGHT_B, LOW);
		pwmWrite(PWM_RIGHT, 0);
	}
}

int TachoCount(int motor)
{
	if(motor == LMOTOR)
	{
		return left_motor.posCounter;
	} else if(motor == RMOTOR)
	{
		return right_motor.posCounter;
	} else {
		return 0;
	}
}

int TachoSpeed(int motor)
{
	if(motor == LMOTOR)
	{
		return left_motor.tickFreq;
	} else if(motor == RMOTOR)
	{
		return right_motor.tickFreq;
	} else {
		return 0;
	}	
}