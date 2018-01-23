//bibliotecas utilizadas
#include <stdio.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

//declaração das fincões
void initPCA9685(int end);
void pwmPCA9685(int end, int porta, int dutycicle);

// int main()
// {
// 	int endPCA9685 = wiringPiI2CSetup(0x40);
// 	char resposta;
// 	int dc;

// 	printf("Are you su? ");
// 	scanf("%c", &resposta);
// 	while (resposta != 'y' && resposta != 'n')
// 	{
// 		printf("y ou n: ");
// 		scanf("%c", &resposta);
// 	}
// 	if(resposta != 'y')
// 		return 0;

	
// 	initPCA9685(endPCA9685);

// 	scanf("%d", &dc);
	
// 	while(dc>=0)
// 	{
// 		pwmPCA9685(endPCA9685, 15, dc);
// 		scanf("%d", &dc);
// 	}
	

// 	return 1;
// }

//assinatura das funcões
void initPCA9685(int end)
{
	wiringPiI2CWriteReg8(end, 0x00, 0x01);
}

void pwmPCA9685(int end, int porta, int dutycicle)
{
	int ledxON_L = 0x06 + 4*porta;
	int ledxON_H = 0x07 + 4*porta;
	int ledxOFF_L = 0x08 + 4*porta;
	int ledxOFF_H = 0x09 + 4*porta;
	int valH, valL;
	int16_t aux;

	if(dutycicle <= 0)
	{
		valH = 0x00;
		valL = 0x00;
	}else if(dutycicle >= 4095)
	{
		valH = 0x0f;
		valL = 0xff;
	}
	else
	{
		//bitmagic
		aux = dutycicle << 8;
		valL = aux >> 8;
		valH = dutycicle >> 8;
	}

	wiringPiI2CWriteReg8(end, ledxON_H, 0x00);
	wiringPiI2CWriteReg8(end, ledxON_L, 0x00);
	wiringPiI2CWriteReg8(end, ledxOFF_H, valH);
	wiringPiI2CWriteReg8(end, ledxOFF_L, valL);
	//printf("%x\t%x\t%x\t%x\n%x\t%x\t%x\t%x\n\n", ledxON_L, ledxON_H, ledxOFF_L, ledxOFF_H, 0x00, 0x00, valL, valH);
}