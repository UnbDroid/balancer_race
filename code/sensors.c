#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define R_GAIN (1.0/65.5)	// ajustado para uma sensibilidade de 2000 DPS, para outras sensibilidades consultar pagina 12 do datasheet.
#define G_GAIN (1.0/1370)	// ajustado para uma sensibilidade de +-1.3 Ga, para outras sensibilidades consultar pagina 12 do datasheet.

#define IR_LEFT 40
#define IR_RIGHT 22

struct infrared {
	int left, right;
};

struct infrared ir;

void init_sensors()
{
	int endeMPU9250 = wiringPiI2CSetup(0x68);
	int8_t tempbyte;

	// turning sleep off on MPU9250 chip (accel/gyro)
	tempbyte = wiringPiI2CReadReg8(endeMPU9250, 0x6b);
	tempbyte = tempbyte & 0xbf;
	wiringPiI2CWriteReg8(endeMPU9250, 0x6b, tempbyte);

	// setando o sensor para 500 DPS (sim, ele é um carry).
	wiringPiI2CWriteReg8(endeMPU9250, 0x1b, 0x08);

	pinMode(IR_LEFT, INPUT);
	pinMode(IR_RIGHT, INPUT);
}

void update_ir()
{
	ir.left = digitalRead(IR_LEFT);
	ir.right = digitalRead(IR_RIGHT);
}