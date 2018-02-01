#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GYRO_GAIN (1.0/65.5)	// gyro values ratio for 500 º/s full scale range. If in doubt consult datasheet page 8
#define ACCEL_GAIN (1.0/16384.0) // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9
#define MAGNET_GAIN 1.0
#define RAD2DEG 57.2957

#define GYRO_X_OFFSET_HI 0x00
#define GYRO_X_OFFSET_LO 0x0b
#define GYRO_Y_OFFSET_HI 0xff
#define GYRO_Y_OFFSET_LO 0xf8
#define GYRO_Z_OFFSET_HI 0x00
#define GYRO_Z_OFFSET_LO 0x04

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

#define IR_LEFT 22
#define IR_RIGHT 40

struct infrared {
	int left, right;
};

struct gyro {
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double posX;
	double posY;
	double posZ;
	double velX;
	double velY;
	double velZ;
	double accX;
	double accY;
	double accZ;
};

struct accel {
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double posX;
	double posY;
	double posZ;
	double velX;
	double velY;
	double velZ;
	double accX;
	double accY;
	double accZ;
};

struct magnet {
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double posX;
	double posY;
	double posZ;
	double velX;
	double velY;
	double velZ;
	double accX;
	double accY;
	double accZ;
};

struct imu {
	struct gyro gyro;
	struct accel accel;
	struct magnet magnet;
	long int dt;
};

struct infrared ir;
struct imu imu;
int endeMPU9250;
unsigned long long int last_update;

void initMPU9250()
{
	endeMPU9250 = wiringPiI2CSetup(0x68);

	// set PWR_MGMT_1 register
	// [7] H_RESET - if set to 1 it will auto clear and reset the chip registers to default values
	// [6] SLEEP - if set, the chip is set to sleep mode
	// [5] CYCLE - if set, chip will alternate between sleep and taking samples
	// [4] GYRO_STANDBY - if set, gyro will be always partially on so that it's faster to enable
	// [3] PD_PTAT - power down the Proportional To Absolute Temperature voltage regulator
	// [2:0] CLKSEL - set to 000 to use internal clock, 001 to auto select clock
	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_1, 0x80); // reset MPU9250 registers to default configurations
	delay(100);
	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_1, 0x01); // disable sleep, set clock to auto

	// set PWR_MGMT_2 register
	// [7, 6] - Reserved
	// [5, 4, 3] - Set to 0 to enable Accel [X, Y, Z]
	// [2, 1, 0] - Set to 0 to enable Gyro  [X, Y, Z]
	wiringPiI2CWriteReg8(endeMPU9250, PWR_MGMT_2, 0x00);

	// set CONFIG register
	// [7] - Reserved
	// [6] - FIFO_MODE - when set to 0, discard readings if FIFO is full. Otherwise, oldest data will be replaced.
	// [5:3] - EXT_SYNC_SET - just disable with 000. For more info, check page 13 of the register map
	// [2:0] - DLPF_CFG - Differential Low Pass Filter Configuration. Check page 13 of the register map
	wiringPiI2CWriteReg8(endeMPU9250, CONFIG, 0x03);

	// set sample rate to 200Hz
	// sample rate defined by 1/(1+SMPLRT_DIV)
	wiringPiI2CWriteReg8(endeMPU9250, SMPLRT_DIV, 0x04);

	// set gyro max range to 500 degrees/second
	// GYRO_CONFIG
	// [7, 6, 5] - Gyro self-test for [X, Y, Z] axis
	// [4:3] - Gyro full scale value (250, 500, 1000, 2000 degrees/second)
	// [2] - Reserved
	// [1:0] - Setting to bypass DLPF(Differential Low Pass Filter). Disable with 00.
	wiringPiI2CWriteReg8(endeMPU9250, GYRO_CONFIG, 0x08);

	// set gyro offset registers (0x13 to 0x18) to values obtained by the calibGyro.c program
	wiringPiI2CWriteReg8(endeMPU9250, XG_OFFSET_H, GYRO_X_OFFSET_HI);
	wiringPiI2CWriteReg8(endeMPU9250, XG_OFFSET_L, GYRO_X_OFFSET_LO);
	wiringPiI2CWriteReg8(endeMPU9250, YG_OFFSET_H, GYRO_Y_OFFSET_HI);
	wiringPiI2CWriteReg8(endeMPU9250, YG_OFFSET_L, GYRO_Y_OFFSET_LO);
	wiringPiI2CWriteReg8(endeMPU9250, ZG_OFFSET_H, GYRO_Z_OFFSET_HI);
	wiringPiI2CWriteReg8(endeMPU9250, ZG_OFFSET_L, GYRO_Z_OFFSET_LO);

	// set accel max range to 2 g
	// ACCEL_CONFIG
	// [7, 6, 5] - Accel self-test for [X, Y, Z] axis
	// [4:3] - Accel full scale value (2, 4, 8, 16 g)
	// [2:0] - Reserved
	wiringPiI2CWriteReg8(endeMPU9250, ACCEL_CONFIG, 0x00);

	/*
	int i;
	int32_t gyro_bias[3] = {0, 0, 0};
	for(i = 0; i < 1000; ++i)
	{
		int16_t gyro_temp[3] = {0, 0, 0};
		int8_t data[6];
		data[0] = wiringPiI2CReadReg8(endeMPU9250, 0x43);
		data[1] = wiringPiI2CReadReg8(endeMPU9250, 0x44);
		data[2] = wiringPiI2CReadReg8(endeMPU9250, 0x45);
		data[3] = wiringPiI2CReadReg8(endeMPU9250, 0x46);;
		data[4] = wiringPiI2CReadReg8(endeMPU9250, 0x47);
		data[5] = wiringPiI2CReadReg8(endeMPU9250, 0x48);
		gyro_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]);
		gyro_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
		gyro_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
		gyro_bias[0] += (int32_t) gyro_temp[0];
		gyro_bias[1] += (int32_t) gyro_temp[1];
		gyro_bias[2] += (int32_t) gyro_temp[2];
	}
	gyro_bias[0] /= (int32_t) 1000;
	gyro_bias[1] /= (int32_t) 1000;
	gyro_bias[2] /= (int32_t) 1000;
	
	printf("%d, %d, %d\n", gyro_bias[0], gyro_bias[1], gyro_bias[2]);
	*/

	imu.gyro.posX = 0;
	imu.gyro.posY = 0;
	imu.gyro.posZ = 0;

	imu.gyro.velX = 0;
	imu.gyro.velY = 0;
	imu.gyro.velZ = 0;

	imu.accel.posX = 0;
	imu.accel.posY = 0;
	imu.accel.posZ = 0;

	imu.magnet.posX = 0;
	imu.magnet.posY = 0;
	imu.magnet.posZ = 0;

	last_update = micros();
}

void init_sensors()
{
	initMPU9250();

	pinMode(IR_LEFT, INPUT);
	pinMode(IR_RIGHT, INPUT);
}

void update_imu()
{
	uint8_t gyrXhi, gyrXlo;
	uint8_t gyrYhi, gyrYlo;
	uint8_t gyrZhi, gyrZlo;

	uint8_t accXhi, accXlo;
	uint8_t accYhi, accYlo;
	uint8_t accZhi, accZlo;

	uint8_t magXhi, magXlo;
	uint8_t magYhi, magYlo;
	uint8_t magZhi, magZlo;

	unsigned long long int now_time = micros();
	imu.dt = now_time - last_update;
	double dt = (double)imu.dt/1000000.0;
	last_update = now_time;

	//bit magic
	gyrXhi = wiringPiI2CReadReg8(endeMPU9250, 0x43);
	gyrXlo = wiringPiI2CReadReg8(endeMPU9250, 0x44);
	imu.gyro.rawX = (int16_t)((int16_t)gyrXhi<<8 | gyrXlo);

	gyrYhi = wiringPiI2CReadReg8(endeMPU9250, 0x45);
    gyrYlo = wiringPiI2CReadReg8(endeMPU9250, 0x46);
	imu.gyro.rawY = (int16_t)((int16_t)gyrYhi<<8 | gyrYlo);

	gyrZhi = wiringPiI2CReadReg8(endeMPU9250, 0x47);
    gyrZlo = wiringPiI2CReadReg8(endeMPU9250, 0x48);
	imu.gyro.rawZ = (int16_t)((int16_t)gyrZhi<<8 | gyrZlo);

	accXhi = wiringPiI2CReadReg8(endeMPU9250, 0x3b);
    accXlo = wiringPiI2CReadReg8(endeMPU9250, 0x3c);
    imu.accel.rawX = (int16_t)((int16_t)accXhi<<8 | accXlo);

	accYhi = wiringPiI2CReadReg8(endeMPU9250, 0x3d);
    accYlo = wiringPiI2CReadReg8(endeMPU9250, 0x3e);
	imu.accel.rawY = (int16_t)((int16_t)accYhi<<8 | accYlo);

	accZhi = wiringPiI2CReadReg8(endeMPU9250, 0x3f);
    accZlo = wiringPiI2CReadReg8(endeMPU9250, 0x40);
    imu.accel.rawZ = (int16_t)((int16_t)accZhi<<8 | accZlo);

	magXlo = wiringPiI2CReadReg8(endeMPU9250, 0x03);
    magXhi = wiringPiI2CReadReg8(endeMPU9250, 0x04);
    imu.magnet.rawX = (int16_t)((int16_t)magXhi<<8 | magXlo);

    magYlo = wiringPiI2CReadReg8(endeMPU9250, 0x05);
	magYhi = wiringPiI2CReadReg8(endeMPU9250, 0x06);
	imu.magnet.rawY = (int16_t)((int16_t)magYhi<<8 | magYlo);

    magZlo = wiringPiI2CReadReg8(endeMPU9250, 0x07);
	magZhi = wiringPiI2CReadReg8(endeMPU9250, 0x08);
	imu.magnet.rawZ = (int16_t)((int16_t)magZhi<<8 | magZlo);

	double tempX, tempY, tempZ;

	tempX = imu.gyro.velX;
	tempY = imu.gyro.velY;
	tempZ = imu.gyro.velZ;

	imu.gyro.velX = -GYRO_GAIN*(double)imu.gyro.rawX;
	imu.gyro.velY = -GYRO_GAIN*(double)imu.gyro.rawY;
	imu.gyro.velZ = -GYRO_GAIN*(double)imu.gyro.rawZ;

	imu.gyro.accX = (imu.gyro.velX-tempX)/dt;
	imu.gyro.accY = (imu.gyro.velY-tempY)/dt;
	imu.gyro.accZ = (imu.gyro.velZ-tempZ)/dt;

	imu.gyro.posX += imu.gyro.velX*dt;
	imu.gyro.posY += imu.gyro.velY*dt;
	imu.gyro.posZ += imu.gyro.velZ*dt;

	imu.accel.posX = (double)imu.accel.rawX;
	imu.accel.posY = (double)imu.accel.rawY;
	imu.accel.posZ = (double)imu.accel.rawZ;
	
	//imu.accel.posX = RAD2DEG*atan2((double)imu.accel.rawZ, (double)imu.accel.rawY);
	//imu.accel.posY = RAD2DEG*atan2((double)imu.accel.rawX, (double)imu.accel.rawZ);
	//imu.accel.posZ = RAD2DEG*atan2((double)imu.accel.rawY, (double)imu.accel.rawX);

	/*
	if(imu.accel.rawY != 0)
	{
		imu.accel.posX = RAD2DEG*atan((double)imu.accel.rawZ/imu.accel.rawY);
	} else {
		if(imu.accel.rawZ >= 0)
		{
			imu.accel.posX = 90;
		} else {
			imu.accel.posX = -90;
		}
	}
	if(imu.accel.rawZ != 0)
	{
		imu.accel.posY = RAD2DEG*atan((double)imu.accel.rawX/imu.accel.rawZ);
	} else {
		if(imu.accel.rawX >= 0)
		{
			imu.accel.posY = 90;
		} else {
			imu.accel.posY = -90;
		}
	}
	if(imu.accel.rawX != 0)
	{
		imu.accel.posZ = RAD2DEG*atan((double)imu.accel.rawY/imu.accel.rawX);
	} else {
		if(imu.accel.rawY >= 0)
		{
			imu.accel.posZ = 90;
		} else {
			imu.accel.posZ = -90;
		}
	}
	*/
}

void update_ir()
{
	ir.left = !digitalRead(IR_LEFT);
	ir.right = !digitalRead(IR_RIGHT);
}