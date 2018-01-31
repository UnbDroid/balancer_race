#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

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

int main()
{
	int endeMPU9250;
	uint8_t data[6];
	uint16_t ii, jj, packet_count, fifo_count;
	int32_t gyro_bias[3] = {0, 0, 0};

	endeMPU9250 = wiringPiI2CSetup(0x68);

	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_1, 0x80); // reset MPU9250 registers to default configurations
	delay(100);

	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_1, 0x01);
	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_2, 0x00);

	wiringPiI2CWriteReg8(endeMPU9250, INT_ENABLE, 0x00);
	wiringPiI2CWriteReg8(endeMPU9250, FIFO_EN, 0x00);
	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_1, 0x00);
	wiringPiI2CWriteReg8(endeMPU9250, I2C_MST_CTRL, 0x00);
	wiringPiI2CWriteReg8(endeMPU9250, USER_CTRL, 0x00);
	wiringPiI2CWriteReg8(endeMPU9250, USER_CTRL, 0x0c);
    delay(15);    

    wiringPiI2CWriteReg8(endeMPU9250, CONFIG, 0x01);
    wiringPiI2CWriteReg8(endeMPU9250, SMPLRT_DIV, 0x00);
    wiringPiI2CWriteReg8(endeMPU9250, GYRO_CONFIG, 0x00);
 
	uint16_t gyrosensitivity = 131;   // = 131 LSB/degrees/sec

	wiringPiI2CWriteReg8(endeMPU9250, USER_CTRL, 0x40);
	wiringPiI2CWriteReg8(endeMPU9250, FIFO_EN, 0x70);
	delay(60);

	wiringPiI2CWriteReg8(endeMPU9250, FIFO_EN, 0x00);

	data[0] = wiringPiI2CReadReg8(endeMPU9250, FIFO_COUNTH);
	data[1] = wiringPiI2CReadReg8(endeMPU9250, FIFO_COUNTL);
	fifo_count = ((uint16_t)data[0] << 8) | data[1];
	packet_count = fifo_count/6;

	for(ii = 0; ii < packet_count; ++ii)
	{
		int16_t gyro_temp[3] = {0, 0, 0};
		data[0] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		data[1] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		data[2] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		data[3] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		data[4] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		data[5] = wiringPiI2CReadReg8(endeMPU9250, FIFO_R_W);
		gyro_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]);
		gyro_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
		gyro_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
		//printf("%x %x, %x %x, %x %x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
		//printf("%i, %i, %i\n\n", gyro_temp[0], gyro_temp[1], gyro_temp[2]);
		gyro_bias[0] += (int32_t) gyro_temp[0];
		gyro_bias[1] += (int32_t) gyro_temp[1];
		gyro_bias[2] += (int32_t) gyro_temp[2];
	}

	gyro_bias[0] /= (int32_t) packet_count;
	gyro_bias[1] /= (int32_t) packet_count;
	gyro_bias[2] /= (int32_t) packet_count;
	printf("Gyro Bias: {%i, %i, %i}\n\n", gyro_bias[0], gyro_bias[1], gyro_bias[2]);

	data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
	data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
	data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
	data[3] = (-gyro_bias[1]/4)       & 0xFF;
	data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
	data[5] = (-gyro_bias[2]/4)       & 0xFF;

	printf("#define GYRO_X_OFFSET_HI 0x%02x\n", data[0]);
	printf("#define GYRO_X_OFFSET_LO 0x%02x\n", data[1]);
	printf("#define GYRO_Y_OFFSET_HI 0x%02x\n", data[2]);
	printf("#define GYRO_Y_OFFSET_LO 0x%02x\n", data[3]);
	printf("#define GYRO_Z_OFFSET_HI 0x%02x\n", data[4]);
	printf("#define GYRO_Z_OFFSET_LO 0x%02x\n\n", data[5]);

	printf("X offset: %f\n", (float)gyro_bias[0] / (float)gyrosensitivity);
	printf("Y offset: %f\n", (float)gyro_bias[1] / (float)gyrosensitivity);
	printf("Z offset: %f\n\n", (float)gyro_bias[2] / (float)gyrosensitivity);

	return 0;
}