#include <wiringPi.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define PWM1 12
#define PWM2 33
#define M1A 35
#define M1B 36
#define M2A 37
#define M2B 38

#define LMOTOR 0
#define RMOTOR 1

void init_motors()
{
	pinMode(PWM1, PWM_OUTPUT);
	pinMode(PWM2, PWM_OUTPUT);
	pinMode(M1A, OUTPUT);
	pinMode(M1B, OUTPUT);
	pinMode(M2A, OUTPUT);
	pinMode(M2B, OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(1023);//range
	pwmSetClock(2);
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