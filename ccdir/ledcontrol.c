//bibliotecas utilizadas
#include <stdio.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pca9685pwmcontrol.c>

void light_color(int red, int green, int blue);

int main()
{
	int red, green, blue;
	printf("Escolha valores para RGB: \n");
	scanf("%d %d %d", &red, &green, &blue);
	light_color(red, green, blue);
}

void light_color(int red, int green, int blue)
{
	int endPCA9685 = wiringPiI2CSetup(0x40);
	initPCA9685(endPCA9685);

	// dutycicle = 0 -> led aceso totalmente
	int r_dutycicle = 4095 - 4095*red/255; 
	int g_dutycicle = 4095 - 4095*green/255; 
	int b_dutycicle = 4095 - 4095*blue/255; 

	pwmPCA9685(endPCA9685, 15, r_dutycicle);
	pwmPCA9685(endPCA9685, 14, g_dutycicle);
	pwmPCA9685(endPCA9685, 13, b_dutycicle);
}