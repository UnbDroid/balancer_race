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
#define TICKS2METERS 0.00078539816 //WHEEL_RADIUS*3.14159265358979323846/200

#define ALPHA_MOTORS 0.9

struct motor {
	int a_port, b_port;
	int pwm;
	volatile long long int posCounter;
	double displacement, raw_speed, filtered_speed, accel;
	double last_pos;
	double set_speed;
	unsigned long int last_update;
};

struct motor left_motor, right_motor;

void encoderInterruptLeft(void)
{
  if(digitalRead(DIR_PIN_LEFT))
  {
    ++left_motor.posCounter;
  } else {
    --left_motor.posCounter;
  }
}

void encoderInterruptRight(void)
{
  if(digitalRead(DIR_PIN_RIGHT))
  {
    --right_motor.posCounter;
  } else {
    ++right_motor.posCounter;
  }
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
	pwmWrite(PWM_LEFT, 0);
	pwmWrite(PWM_RIGHT, 0);
	unsigned long int now = micros();
	left_motor.pwm = 0;
	left_motor.posCounter = 0;
	left_motor.displacement = 0;
	left_motor.raw_speed = 0;
	left_motor.filtered_speed = 0;
	left_motor.last_pos = 0;
	left_motor.last_update = now;
	right_motor.pwm = 0;
	right_motor.posCounter = 0;
	right_motor.displacement = 0;
	right_motor.raw_speed = 0;
	right_motor.filtered_speed = 0;
	right_motor.last_pos = 0;
	right_motor.last_update = now;
}

void Brake(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M_LEFT_A, HIGH);
		digitalWrite(M_LEFT_B, HIGH);
		pwmWrite(PWM_LEFT, 0);
		left_motor.a_port = 1;
    	left_motor.b_port = 1;
    	left_motor.pwm = 0;
	} else if (motor == RMOTOR) {
		digitalWrite(M_RIGHT_A, HIGH);
		digitalWrite(M_RIGHT_B, HIGH);
		pwmWrite(PWM_RIGHT, 0);
		right_motor.a_port = 1;
    	right_motor.b_port = 1;
    	right_motor.pwm = 0;
	}
}

void Coast(int motor)
{
	if(motor == LMOTOR)
	{
		digitalWrite(M_LEFT_A, LOW);
		digitalWrite(M_LEFT_B, LOW);
		pwmWrite(PWM_LEFT, 0);
		left_motor.a_port = 0;
    	left_motor.b_port = 0;
    	left_motor.pwm = 0;
	} else if (motor == RMOTOR) {
		digitalWrite(M_RIGHT_A, LOW);
		digitalWrite(M_RIGHT_B, LOW);
		pwmWrite(PWM_RIGHT, 0);
		right_motor.a_port = 0;
    	right_motor.b_port = 0;
    	right_motor.pwm = 0;
	}
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
	if(power > 0)
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
	} else if(power < 0) {
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
	} else {
		if(motor == LMOTOR)
		{
			Brake(LMOTOR);
		} else if(motor == RMOTOR) {
			Brake(RMOTOR);
		}
	}
}

void setMotorSpeed(int motor, double speed)
{
	if(motor == LMOTOR)
	{
		left_motor.set_speed = speed;
	} else if (motor == RMOTOR) {
		right_motor.set_speed = speed;
	}
}

void speedControl()
{
	double err;
	int pot;
	double KP = 500;

	err = (left_motor.set_speed - left_motor.filtered_speed);
	pot = KP*err;
	OnFwd(LMOTOR, pot);

	err = (right_motor.set_speed - right_motor.filtered_speed);
	pot = KP*err;
	OnFwd(RMOTOR, pot);
}

void update_motors()
{
	unsigned long int now;
	double last_speed;

	now = micros();
	left_motor.displacement = left_motor.posCounter*TICKS2METERS;
	
	last_speed = left_motor.filtered_speed;
	left_motor.raw_speed = 1000000.0*(left_motor.displacement - left_motor.last_pos)/(now - left_motor.last_update);
	left_motor.filtered_speed = ALPHA_MOTORS*left_motor.filtered_speed + (1-ALPHA_MOTORS)*left_motor.raw_speed;
	
	left_motor.accel = 1000000.0*(left_motor.filtered_speed - last_speed)/(now - left_motor.last_update);

	left_motor.last_pos = left_motor.displacement;
	left_motor.last_update = now;


	now = micros();
	right_motor.displacement = right_motor.posCounter*TICKS2METERS;
	
	last_speed = right_motor.filtered_speed;
	right_motor.raw_speed = 1000000.0*(right_motor.displacement - right_motor.last_pos)/(now - right_motor.last_update);
	right_motor.filtered_speed = ALPHA_MOTORS*right_motor.filtered_speed + (1-ALPHA_MOTORS)*right_motor.raw_speed;
	
	right_motor.accel = 1000000.0*(right_motor.filtered_speed - last_speed)/(now - right_motor.last_update);

	right_motor.last_pos = right_motor.displacement;
	right_motor.last_update = now;

	speedControl();
}