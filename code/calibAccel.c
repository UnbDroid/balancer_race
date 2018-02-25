#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <math.h>

#define PWR_MGMT_1 0x6b
#define PWR_MGMT_2 0x6c
#define GYRO_CONFIG 0x1b
#define CONFIG 0x1a
#define SMPLRT_DIV 0x19
#define ACCEL_CONFIG 0x1c
#define ACCEL_CONFIG2 0x1d
#define INT_PIN_CFG 0x37
#define INT_ENABLE 0x38
#define FIFO_EN 0x23
#define I2C_MST_CTRL 0x24
#define USER_CTRL 0x6a
#define FIFO_COUNTH 0x72
#define FIFO_COUNTL 0x73
#define FIFO_R_W 0x74
#define XG_OFFSET_H 0X13
#define XG_OFFSET_L 0X14
#define YG_OFFSET_H 0X15
#define YG_OFFSET_L 0X16
#define ZG_OFFSET_H 0X17
#define ZG_OFFSET_L 0X18
#define XA_OFFSET_H 0X77
#define XA_OFFSET_L 0X78
#define YA_OFFSET_H 0X7a
#define YA_OFFSET_L 0X7b
#define ZA_OFFSET_H 0X7d
#define ZA_OFFSET_L 0X7e

#define ACCEL_X_OFFSET_HI 0x00
#define ACCEL_X_OFFSET_LO 0x00
#define ACCEL_Y_OFFSET_HI 0x00
#define ACCEL_Y_OFFSET_LO 0x00
#define ACCEL_Z_OFFSET_HI 0x00
#define ACCEL_Z_OFFSET_LO 0x00

#define ACCELX_BIAS 0.016481
#define ACCELY_BIAS 0.020262
#define ACCELZ_BIAS 0.11054
#define ACCEL_GAIN 1/2048.0 // 0.00006103515 // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9

double accel_val[500][3];
double accel_mag[500];
double mag_mean = 0, mag_std;
double accel_mean[3] = {0, 0, 0};

int16_t convertTo15bit(int16_t n)
{
	int i;
	int16_t bit[16];
	bit[0] = 0x0001;
	for(i = 1; i <= 15; ++i)
	{

	}
}

int main()
{
	printf("Please make sure that the robot is at rest (its attitude doesn't matter as long\n");
	printf("as it is not moving or subject to strong vibrations) and press enter.");
	while(getchar() != '\n');

	int MPU9250addr;
	uint8_t data[6];
	uint16_t i, j, packet_count, fifo_count;
	int32_t accel_bias[3] = {0, 0, 0};

	MPU9250addr = wiringPiI2CSetup(0x68);

	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x80); // reset MPU9250 registers to default configurations
	delay(200);

	/*
	wiringPiI2CWriteReg8(MPU9250addr, XA_OFFSET_H, ACCEL_X_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, XA_OFFSET_L, ACCEL_X_OFFSET_LO);
	wiringPiI2CWriteReg8(MPU9250addr, YA_OFFSET_H, ACCEL_Y_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, YA_OFFSET_L, ACCEL_Y_OFFSET_LO);
	wiringPiI2CWriteReg8(MPU9250addr, ZA_OFFSET_H, ACCEL_Z_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, ZA_OFFSET_L, ACCEL_Z_OFFSET_LO);
	*/

	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x01);
	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_2, 0x00);

	wiringPiI2CWriteReg8(MPU9250addr, INT_ENABLE, 0x00);
	wiringPiI2CWriteReg8(MPU9250addr, FIFO_EN, 0x00);
	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x00);
	wiringPiI2CWriteReg8(MPU9250addr, I2C_MST_CTRL, 0x00);
	wiringPiI2CWriteReg8(MPU9250addr, USER_CTRL, 0x00);
	wiringPiI2CWriteReg8(MPU9250addr, USER_CTRL, 0x0c);
    delay(15);    

    wiringPiI2CWriteReg8(MPU9250addr, CONFIG, 0x01);
    wiringPiI2CWriteReg8(MPU9250addr, SMPLRT_DIV, 0x00);
    wiringPiI2CWriteReg8(MPU9250addr, ACCEL_CONFIG, 0x18);

	printf("Waiting for sensor to take the measurements...\n");

	wiringPiI2CWriteReg8(MPU9250addr, USER_CTRL, 0x40);
	wiringPiI2CWriteReg8(MPU9250addr, FIFO_EN, 0x08);
	
	delay(80);

	wiringPiI2CWriteReg8(MPU9250addr, FIFO_EN, 0x00);
	delay(80);

	data[0] = wiringPiI2CReadReg8(MPU9250addr, FIFO_COUNTH);
	data[1] = wiringPiI2CReadReg8(MPU9250addr, FIFO_COUNTL);
	fifo_count = ((uint16_t)data[0] << 8) | data[1];
	packet_count = fifo_count/6;

	printf("%d measurements taken.\n", packet_count);

	printf("Fetching packets from sensor memory...\n");

	for(i = 0; i < packet_count; ++i)
	{
		data[0] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[1] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[2] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[3] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[4] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[5] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		accel_val[i][0] = (ACCEL_GAIN*(int16_t)(((int16_t)data[0] << 8) | data[1]))-ACCELX_BIAS;
		accel_val[i][1] = (ACCEL_GAIN*(int16_t)(((int16_t)data[2] << 8) | data[3]))-ACCELY_BIAS;
		accel_val[i][2] = (ACCEL_GAIN*(int16_t)(((int16_t)data[4] << 8) | data[5]))-ACCELZ_BIAS;
		accel_mag[i] = sqrt(pow(accel_val[i][0], 2) + pow(accel_val[i][1], 2) + pow(accel_val[i][2], 2));
		mag_mean += accel_mag[i];
		//printf("Fetched packet number %d.\n", i+1);
	}
	
	mag_mean /= packet_count;

	printf("Calculating standard deviation and mean values.\n");

	for(i = 0; i < packet_count; ++i)
	{
		mag_std += pow((accel_mag[i] - mag_mean), 2);
		accel_mean[0] += accel_val[i][0];
		accel_mean[1] += accel_val[i][1];
		accel_mean[2] += accel_val[i][2];
	}

	mag_std = sqrt(mag_std/(packet_count-1));
	accel_mean[0] /= packet_count;
	accel_mean[1] /= packet_count;
	accel_mean[2] /= packet_count;

	printf("X: %f Y: %f Z: %f Magnitude: %f\n", accel_mean[0], accel_mean[1], accel_mean[2], mag_mean);

	printf("#define GRAVITY %f\n", mag_mean);
	printf("#define ACC_TOLERANCE %f\n", 3*mag_std);

	return 0;
}