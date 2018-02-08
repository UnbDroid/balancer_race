#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pca9685pwmcontrol.c>

// Ports used by the RBG LED on the PCA9685 board
#define LED_R 15
#define LED_G 14
#define LED_B 13

#define ON 1
#define OFF 0

// LED state constant values
#define BLUETOOTH 0
#define STANDBY 1
#define NUM_STATES 2

// Color constant values
#define OFF_COLOR 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define CYAN 5
#define MAGENTA 6
#define YELLOW 7
#define ORANGE 8

#define LED_RANGE 4095

int PCA9685addr; // PCA9685 board I2C address

/* The state_flags vector holds the flags that determine whether each state
 * is on or off. That way, it is possible to define a priority sequence.
 * To properly set a state on, see the set_led_state() function.
 */
int state_flags[NUM_STATES] = {0};
int led_state = -1;

int last_led_update = 0;
int led_state_flag = 0;
unsigned int led_delay = 100;

int r, g, b, i;

void init_led()
{
	PCA9685addr = wiringPiI2CSetup(0x40);
	initPCA9685(PCA9685addr);
}

void set_led_state(int state, int on_off)
{
	if(on_off == ON)
	{
		state_flags[state] = 1;
	} else if(on_off == OFF) {
		state_flags[state] = 0;
	}

	int i = 0;
	led_state = -1;
	while(i < NUM_STATES && led_state != i)
	{
		if(state_flags[i])
			led_state = i;
		else
			++i;
	}
	if(i == NUM_STATES)
		led_state = -1;
}

void set_color(int color, int intensity)
{
	i = intensity;
	switch(color)
	{
		case WHITE:
			r = 255;
			g = 255;
			b = 255;
			break;
		case RED:
			r = 255; 
			g = 0; 
			b = 0;
			break;
		case GREEN:
			r = 0;
			g = 255;
			b = 0;
			break;
		case BLUE:
			r = 0;
			g = 0;
			b = 255;
			break;
		case CYAN:
			r = 0;
			g = 255;
			b = 255;
			break;
		case MAGENTA:
			r = 255;
			g = 0;
			b = 255;
			break;
		case YELLOW:
			r = 255;
			g = 180;
			b = 0;
			break;
		case ORANGE:
			r = 180;
			g = 80;
			b = 0;
		case OFF_COLOR:
			r = 0;
			g = 0;
			b = 0;
			break;
	}
}

void light_rgb()
{
	int r_dutycycle = LED_RANGE - i * r * r / LED_RANGE; 
	int g_dutycycle = LED_RANGE - i * g * g / LED_RANGE; 
	int b_dutycycle = LED_RANGE - i * b * b / LED_RANGE;

	pwmPCA9685(PCA9685addr, LED_R, r_dutycycle);
	pwmPCA9685(PCA9685addr, LED_G, g_dutycycle);
	pwmPCA9685(PCA9685addr, LED_B, b_dutycycle);
}

void update_led()
{
	if(millis() - last_led_update > led_delay)
	{
		last_led_update = millis();
		switch(led_state)
		{
			case STANDBY:
				led_delay = 600;
				if(led_state_flag)
				{
					led_state_flag = 0;
					set_color(RED, 255);
				} else {
					led_state_flag = 1;
					set_color(YELLOW, 255);
				}
				break;
			case BLUETOOTH:
				led_delay = 50;
				if(led_state_flag)
				{
					i += 15;
					if(i >= 255) led_state_flag = 0;
				} else {
					i -= 15;
					if(i <= -50) led_state_flag = 1;
				}
				set_color(BLUE, i);
				break;
			default: // no state set
				i = 0;
				break;
		}
		light_rgb(r, g, b, i);
	}
}

/*
yellow = R: 255 G: 175 B:   0
cyan = R:   0 G: 255 B: 255
magenta = R: 255 G:   0 B: 255
orange = R: 180 G:  81 B:   0
*/