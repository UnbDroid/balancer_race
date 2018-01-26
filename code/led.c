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

// LED state constant values
#define NO_STATE -1
#define STANDBY 0
#define BLUETOOTH 1

// Color constant values
#define OFF_COLOR -1
#define UNDEFINED_COLOR 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define CYAN 5
#define MAGENTA 6
#define YELLOW 7

#define LED_DELAY 100

#define LED_RANGE 4095

int endPCA9685;
int current_state = NO_STATE;
int current_color = UNDEFINED_COLOR;
int led_state = NO_STATE;
int led_color = OFF_COLOR;
int last_update = 0;

void light_off()
{
	if(current_color != OFF_COLOR)
	{
		current_color = OFF_COLOR;
		pwmPCA9685(endPCA9685, LED_R, LED_RANGE);
		pwmPCA9685(endPCA9685, LED_G, LED_RANGE);
		pwmPCA9685(endPCA9685, LED_B, LED_RANGE);
	}
}

void init_led()
{
	endPCA9685 = wiringPiI2CSetup(0x40);
	initPCA9685(endPCA9685);
	light_off();
}

void set_led_state(int state)
{
	led_state = state;
}

void set_color(int color)
{
	led_color = color;
}

void update_led()
{
	if(led_color != current_color)
	{		
		int r_dutycicle; 
		int g_dutycicle;
		int b_dutycicle;

		current_color = led_color;

		switch(led_color)
		{
			case WHITE:
				r_dutycicle = 0;
				g_dutycicle = 0;
				b_dutycicle = 0;
				break;
			case RED:
				r_dutycicle = 0; 
				g_dutycicle = LED_RANGE; 
				b_dutycicle = LED_RANGE;
				break;
			case GREEN:
				r_dutycicle = LED_RANGE; 
				g_dutycicle = 0; 
				b_dutycicle = LED_RANGE;
				break;
			case BLUE:
				r_dutycicle = LED_RANGE;
				g_dutycicle = LED_RANGE;
				b_dutycicle = 0;
				break;
			case CYAN:
				r_dutycicle = LED_RANGE;
				g_dutycicle = 0;
				b_dutycicle = 0;
				break;
			case MAGENTA:
				r_dutycicle = 0;
				g_dutycicle = LED_RANGE;
				b_dutycicle = 0;
				break;
			case YELLOW:
				r_dutycicle = 0; 
				g_dutycicle = 0; 
				b_dutycicle = LED_RANGE;
				break;
			case OFF_COLOR:
				r_dutycicle = LED_RANGE;
				g_dutycicle = LED_RANGE;
				b_dutycicle = LED_RANGE;
				break;
		}
		pwmPCA9685(endPCA9685, LED_R, r_dutycicle);
		pwmPCA9685(endPCA9685, LED_G, g_dutycicle);
		pwmPCA9685(endPCA9685, LED_B, b_dutycicle);
	}
}

void light_channels(int red, int green, int blue)
{
	// dutycicle = 0 -> led aceso totalmente
	int r_dutycicle = LED_RANGE - 255 * red * red / LED_RANGE; 
	int g_dutycicle = LED_RANGE - 255 * green * green / LED_RANGE; 
	int b_dutycicle = LED_RANGE - 255 * blue * blue / LED_RANGE;

	current_color = UNDEFINED_COLOR;

	pwmPCA9685(endPCA9685, LED_R, r_dutycicle);
	pwmPCA9685(endPCA9685, LED_G, g_dutycicle);
	pwmPCA9685(endPCA9685, LED_B, b_dutycicle);
}