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

#define WHEEL_RADIUS 0.05
#define TICKS2METERS WHEEL_RADIUS*3.14159265358979323846/200

struct motor {
	int a_port, b_port;
	int pwm;
	volatile long long int posCounter;
	volatile unsigned long int tickFreq;
	volatile unsigned long int lastTick;
	volatile unsigned long int currTick;
	double displacement, speed;
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
    right_motor.posCounter -= 1;
  } else {
    right_motor.posCounter += 1;
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
	    	digitalWrite(M_LEFT_A, LOW);
			digitalWrite(M_LEFT_B, HIGH);
	    	pwmWrite(PWM_LEFT, power);
	    	left_motor.a_port = 0;
	    	left_motor.b_port = 1;
	    	left_motor.pwm = power;
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, LOW);
        	digitalWrite(M_RIGHT_B, HIGH);
        	pwmWrite(PWM_RIGHT, power);
        	right_motor.a_port = 0;
	    	right_motor.b_port = 1;
	    	right_motor.pwm = power;
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M_LEFT_A, HIGH);
			digitalWrite(M_LEFT_B, LOW);
	    	pwmWrite(PWM_LEFT, power);
	    	left_motor.a_port = 1;
	    	left_motor.b_port = 0;
	    	left_motor.pwm = power;
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, HIGH);
        	digitalWrite(M_RIGHT_B, LOW);
        	pwmWrite(PWM_RIGHT, power);
        	right_motor.a_port = 1;
	    	right_motor.b_port = 0;
	    	right_motor.pwm = power;
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
	    	digitalWrite(M_LEFT_A, HIGH);
			digitalWrite(M_LEFT_B, LOW);
	    	pwmWrite(PWM_LEFT, power);
	    	left_motor.a_port = 1;
	    	left_motor.b_port = 0;
	    	left_motor.pwm = power;
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, HIGH);
        	digitalWrite(M_RIGHT_B, LOW);
        	pwmWrite(PWM_RIGHT, power);
        	right_motor.a_port = 1;
	    	right_motor.b_port = 0;
	    	right_motor.pwm = power;
		}
	} else {
		power = -power;
		if(power > 1023) power = 1023;
		if(motor == LMOTOR)
		{
	    	digitalWrite(M_LEFT_A, LOW);
			digitalWrite(M_LEFT_B, HIGH);
	    	pwmWrite(PWM_LEFT, power);
	    	left_motor.a_port = 0;
	    	left_motor.b_port = 1;
	    	left_motor.pwm = power;
		} else if(motor == RMOTOR) {
			digitalWrite(M_RIGHT_A, LOW);
        	digitalWrite(M_RIGHT_B, HIGH);
        	pwmWrite(PWM_RIGHT, power);
        	right_motor.a_port = 0;
	    	right_motor.b_port = 1;
	    	right_motor.pwm = power;
		}
	}
}

void Brake()
{
	digitalWrite(M_LEFT_A, HIGH);
	digitalWrite(M_LEFT_B, HIGH);
	pwmWrite(PWM_LEFT, 0);
	digitalWrite(M_RIGHT_A, HIGH);
	digitalWrite(M_RIGHT_B, HIGH);
	pwmWrite(PWM_RIGHT, 0);
}

void Coast()
{
	digitalWrite(M_LEFT_A, LOW);
	digitalWrite(M_LEFT_B, LOW);
	pwmWrite(PWM_LEFT, 0);
	digitalWrite(M_RIGHT_A, LOW);
	digitalWrite(M_RIGHT_B, LOW);
	pwmWrite(PWM_RIGHT, 0);
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

void update_motors()
{
	left_motor.displacement = left_motor.posCounter*TICKS2METERS;
	left_motor.speed = left_motor.tickFreq*TICKS2METERS;
	right_motor.displacement = right_motor.posCounter*TICKS2METERS;
	right_motor.speed = right_motor.tickFreq*TICKS2METERS;
}