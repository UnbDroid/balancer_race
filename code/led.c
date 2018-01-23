#include <stdio.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pca9685pwmcontrol.c>

// Portas usadas pelo LED no PCA9685
#define LED_R 15
#define LED_G 14
#define LED_B 13
#define CATHODE 12

// Constantes de referência para cores
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
int current_color = UNDEFINED_COLOR;

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
	pwmPCA9685(endPCA9685, CATHODE, LED_RANGE);
	light_off();
}

void light_color(int color)
{
	if(color != current_color)
	{		
		int r_dutycicle; 
		int g_dutycicle;
		int b_dutycicle;

		current_color = color;

		switch(color)
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