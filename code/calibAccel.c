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

#define ACCEL_GAIN 0.00006103515 //(1.0/16384.0) // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9

double accel_val[100][3];
double accel_mag[100];
double mag_mean = 0, mag_std;

int main()
{
	printf("Please make sure that the robot is at rest (its attitude doesn't matter as long\n
			as it is not moving or subject to strong vibrations) and press enter.");
	getchar();
	while(getchar() != '\n');

	int MPU9250addr;
	uint8_t data[6];
	uint16_t i, j, packet_count, fifo_count;
	int32_t accel_bias[3] = {0, 0, 0};

	MPU9250addr = wiringPiI2CSetup(0x68);

	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x80); // reset MPU9250 registers to default configurations
	delay(100);

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
    wiringPiI2CWriteReg8(MPU9250addr, ACCEL_CONFIG, 0x00);

	wiringPiI2CWriteReg8(MPU9250addr, USER_CTRL, 0x40);
	wiringPiI2CWriteReg8(MPU9250addr, FIFO_EN, 0x08);
	delay(80);

	wiringPiI2CWriteReg8(MPU9250addr, FIFO_EN, 0x00);

	data[0] = wiringPiI2CReadReg8(MPU9250addr, FIFO_COUNTH);
	data[1] = wiringPiI2CReadReg8(MPU9250addr, FIFO_COUNTL);
	fifo_count = ((uint16_t)data[0] << 8) | data[1];
	packet_count = fifo_count/6;

	for(i = 0; i < packet_count; ++i)
	{
		int16_t gyro_temp[3] = {0, 0, 0};
		data[0] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[1] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[2] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[3] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[4] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		data[5] = wiringPiI2CReadReg8(MPU9250addr, FIFO_R_W);
		accel_val[i][0] = ACCEL_GAIN*(int16_t)(((int16_t)data[0] << 8) | data[1]);
		accel_val[i][1] = ACCEL_GAIN*(int16_t)(((int16_t)data[2] << 8) | data[3]);
		accel_val[i][2] = ACCEL_GAIN*(int16_t)(((int16_t)data[4] << 8) | data[5]);
		accel_mag[i] = sqrt(pow(accel_val[0], 2) + pow(accel_val[1], 2) + pow(accel_val[2], 2));
		mag_mean += accel_mag[i];
	}
	
	mag_mean /= packet_count;
	
	for(i = 0; i < packet_count; ++i)
	{
		mag_std += pow((accel_mag[i] - mag_mean), 2);
	}

	mag_std = sqrt(mag_std/(packet_count-1));

	printf("#define GRAVITY %f\n", mag_mean);
	printf("#define ACC_TOLERANCE %f\n", mag_std);

	return 0;
}