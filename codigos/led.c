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

// Constantes de referência para cores
#define UNDEFINED_COLOR 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define YELLOW 5

#define LED_DELAY 100

int endPCA9685;
int current_color = UNDEFINED_COLOR;

void light_off()
{
	current_color = UNDEFINED_COLOR;
	pwmPCA9685(endPCA9685, LED_R, 4095);
	pwmPCA9685(endPCA9685, LED_G, 4095);
	pwmPCA9685(endPCA9685, LED_B, 4095);
}

void init_led()
{
	endPCA9685 = wiringPiI2CSetup(0x40);
	initPCA9685(endPCA9685);
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
				g_dutycicle = 4095; 
				b_dutycicle = 4095;
				break;
			case GREEN:
				r_dutycicle = 4095; 
				g_dutycicle = 0; 
				b_dutycicle = 4095;
				break;
			case BLUE:
				r_dutycicle = 4095; 
				g_dutycicle = 4095; 
				b_dutycicle = 0;
				break;
			case YELLOW:
				r_dutycicle = 0; 
				g_dutycicle = 0; 
				b_dutycicle = 4095;
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
	int r_dutycicle = 4095 - 255 * red * red / 4095; 
	int g_dutycicle = 4095 - 255 * green * green / 4095; 
	int b_dutycicle = 4095 - 255 * blue * blue / 4095;

	current_color = UNDEFINED_COLOR;

	pwmPCA9685(endPCA9685, LED_R, r_dutycicle);
	pwmPCA9685(endPCA9685, LED_G, g_dutycicle);
	pwmPCA9685(endPCA9685, LED_B, b_dutycicle);
}