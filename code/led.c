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
#define HALT 1
#define PLOT 2
#define GREENLIGHT 3
#define STANDBY 4
#define NUM_STATES 5

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

/*
 *	The state_flags vector holds the flags that determine whether each state
 *	is on or off. That way, it is possible to define a priority sequence.
 *	To properly set a state on, see the set_led_state() function.
 */
int state_flags[NUM_STATES] = {0};
int led_state = -1;	// Determines which state should the LED be on.
					// It is modified by the set_led_state() function.

// Flow control variables.
int last_led_update = 0;
int led_state_flag = 0;

// Time to wait between LED updates.
// It is different depending on the LED state. Set by update_led() function.
// Should be equal to or higher than 50 milliseconds.
unsigned int led_delay = 100;

// Red, Green and Blue values for lighting up the LED
// Should vary from 0 to 255.
int r, g, b;
// Intensity modifier. Multiplies every color channel.
// Should vary from 0 to 255 where 0 means black (LED off)
// and 255 means r, g and b won't be modified.
int i;

// Initializes I2C communication with the PCA9685 board.
// Should be called at the beginning of the code.
void init_led()
{
	PCA9685addr = wiringPiI2CSetup(0x40);
	initPCA9685(PCA9685addr);
}

/* This function sets a chosen state flag to the condition passed as
 * an argument. It automatically checks for higher priority states to
 * guarantee that the led_state variable is set correctly.
 * State constants are defined in the header and the on_off parameter
 * should be passed as ON or OFF.
 */
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

/* This function sets the r, g, b variables to pre-defined values
 * so that the LED shines with the color passed as argument and the
 * i variable passed as the intensity argument.
 */
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
			r = 220;
			g = 90;
			b = 0;
			break;
		case OFF_COLOR:
			r = 0;
			g = 0;
			b = 0;
			break;
	}
}

/* This function writes the r, g, b and i values to the PCA9685 registers
 * so that the LED shines accordingly. It is mainly used at the end of the
 * update_led() function.
 */
void light_rgb()
{
	int r_dutycycle = LED_RANGE - i * r * r / LED_RANGE; 
	int g_dutycycle = LED_RANGE - i * g * g / LED_RANGE; 
	int b_dutycycle = LED_RANGE - i * b * b / LED_RANGE;

	pwmPCA9685(PCA9685addr, LED_R, r_dutycycle);
	pwmPCA9685(PCA9685addr, LED_G, g_dutycycle);
	pwmPCA9685(PCA9685addr, LED_B, b_dutycycle);
}

/* This function should be periodically called (by the LED thread) with a
 * period equal to led_delay milliseconds. It encodes what each LED state
 * does to the LED and how it should be updated.
 */
void update_led()
{
	if(millis() - last_led_update > led_delay)
	{
		last_led_update = millis();
		switch(led_state)
		{
			case GREENLIGHT:
				led_delay = 10;
				set_color(GREEN, 255);
				break;
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
			case HALT:
				led_delay = 10;
				set_color(ORANGE, 255);
				break;
			case PLOT:
				led_delay = 10;
				set_color(MAGENTA, 255);
				break;
			default: // no state set
				i = 0;
				break;
		}
		light_rgb(r, g, b, i);
	}
}