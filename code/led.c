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
#define NO_COLOR -1
#define OFF_COLOR 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define CYAN 5
#define MAGENTA 6
#define YELLOW 7

#define LED_RANGE 4095

int endPCA9685;

int last_color = NO_COLOR;
int led_state = NO_STATE;
int led_color = OFF_COLOR;
int last_update = 0;
int led_state_flag = 0;
unsigned int led_delay = 100;

int r_dutycycle; 
int g_dutycycle;
int b_dutycycle;

void init_led()
{
	endPCA9685 = wiringPiI2CSetup(0x40);
	initPCA9685(endPCA9685);
}

void set_led_state(int state)
{
	led_state = state;
	led_color = NO_COLOR;
}

void set_color(int color)
{
	led_color = color;
	led_state = NO_STATE;
}

void force_led()
{
	last_update = millis();
	if(led_state != NO_STATE)
	{
		switch(led_state)
		{
			case STANDBY:
				led_delay = 600;
				if(led_state_flag)
				{
					led_state_flag = 0;
					r_dutycycle = 0;
					g_dutycycle = LED_RANGE;
					b_dutycycle = LED_RANGE;
				} else {
					led_state_flag = 1;
					r_dutycycle = 0;
					g_dutycycle = 1260;
					b_dutycycle = LED_RANGE;
				}
				break;
			case BLUETOOTH:
				led_delay = 10;
				r_dutycycle = 0;
				if(led_state_flag)
				{
					b_dutycycle += 3;
					if(b_dutycycle >= 4095) led_state_flag = 0;
				} else {
					b_dutycycle -= 3;
					if(b_dutycycle <= 4095) led_state_flag = 1;
				}
				if(b_dutycycle < 3600)
				{
					g_dutycycle = 3600;
				} else {
					g_dutycycle = b_dutycycle;
				}
				break;
		}
	}
	if(led_color != NO_COLOR && led_color != last_color)
	{		
		last_color = led_color;
		led_delay = 100;
		switch(led_color)
		{
			case WHITE:
				r_dutycycle = 0;
				g_dutycycle = 0;
				b_dutycycle = 0;
				break;
			case RED:
				r_dutycycle = 0; 
				g_dutycycle = LED_RANGE; 
				b_dutycycle = LED_RANGE;
				break;
			case GREEN:
				r_dutycycle = LED_RANGE; 
				g_dutycycle = 0; 
				b_dutycycle = LED_RANGE;
				break;
			case BLUE:
				r_dutycycle = LED_RANGE;
				g_dutycycle = LED_RANGE;
				b_dutycycle = 0;
				break;
			case CYAN:
				r_dutycycle = LED_RANGE;
				g_dutycycle = 0;
				b_dutycycle = 0;
				break;
			case MAGENTA:
				r_dutycycle = 0;
				g_dutycycle = LED_RANGE;
				b_dutycycle = 0;
				break;
			case YELLOW:
				r_dutycycle = 0; 
				g_dutycycle = 0; 
				b_dutycycle = LED_RANGE;
				break;
			case OFF_COLOR:
				r_dutycycle = LED_RANGE;
				g_dutycycle = LED_RANGE;
				b_dutycycle = LED_RANGE;
				break;
		}
	}
	pwmPCA9685(endPCA9685, LED_R, r_dutycycle);
	pwmPCA9685(endPCA9685, LED_G, g_dutycycle);
	pwmPCA9685(endPCA9685, LED_B, b_dutycycle);
}

void update_led()
{
	
	if(millis() - last_update > led_delay)
	{
		force_led();
	}
}

void light_channels(int red, int green, int blue)
{
	// dutycycle = 0 -> led aceso totalmente
	r_dutycycle = LED_RANGE - 255 * red * red / LED_RANGE; 
	g_dutycycle = LED_RANGE - 255 * green * green / LED_RANGE; 
	b_dutycycle = LED_RANGE - 255 * blue * blue / LED_RANGE;

	led_delay = 500;
	led_color = NO_COLOR;
	led_state = NO_STATE;

	pwmPCA9685(endPCA9685, LED_R, r_dutycycle);
	pwmPCA9685(endPCA9685, LED_G, g_dutycycle);
	pwmPCA9685(endPCA9685, LED_B, b_dutycycle);
}