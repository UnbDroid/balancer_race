#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GYRO_GAIN 0.01526717557 //(1.0/65.5)	// gyro values ratio for 500 º/s full scale range. If in doubt consult datasheet page 8
#define ACCEL_GAIN 0.00006103515 //(1.0/16384.0) // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9
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
#define XG_OFFSET_H 0x13
#define XG_OFFSET_L 0x14
#define YG_OFFSET_H 0x15
#define YG_OFFSET_L 0x16
#define ZG_OFFSET_H 0x17
#define ZG_OFFSET_L 0x18
#define CNTL1 0x0a
#define ASAX 0x10
#define ASAY 0x11
#define ASAZ 0x12

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

void update_imu();

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

	// set magnetometer Control 1 register
	// CTNL1
	// [7:5] - Reserved
	// [4] - If 0, output is 14-bits two's complement. If 1, output is 16-bits two's complement
	// [3:0] - Magnetometer mode selection. Check page 51 of the register map for more info
	wiringPiI2CWriteReg8(endeMPU9250, CNTL1, 0x12);

	// set magnetometer sensitivity value
	// ASAX, ASAY, ASAZ
	// [7:0] - ASA value. Hadj = H*((ASA-128)/64 + 1)
	wiringPiI2CWriteReg8(endeMPU9250, ASAX, 0x80);
	wiringPiI2CWriteReg8(endeMPU9250, ASAY, 0x80);
	wiringPiI2CWriteReg8(endeMPU9250, ASAZ, 0x80);

	update_imu();

	imu.gyro.posX = imu.accel.posX;
	imu.gyro.posY = imu.accel.posY;
	imu.gyro.posZ = 0;
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

	/*

	double tempX, tempY, tempZ;
	double temp2X, temp2Y, temp2Z;

	tempX = imu.gyro.velX;
	tempY = imu.gyro.velY;
	tempZ = imu.gyro.velZ;

	*/

	imu.gyro.velX = -GYRO_GAIN*(double)imu.gyro.rawX;
	imu.gyro.velY = -GYRO_GAIN*(double)imu.gyro.rawY;
	imu.gyro.velZ = -GYRO_GAIN*(double)imu.gyro.rawZ;

	/*

	imu.gyro.accX = (imu.gyro.velX-tempX)/dt;
	imu.gyro.accY = (imu.gyro.velY-tempY)/dt;
	imu.gyro.accZ = (imu.gyro.velZ-tempZ)/dt;

	*/

	imu.gyro.posX += imu.gyro.velX*dt;
	imu.gyro.posY += imu.gyro.velY*dt;
	imu.gyro.posZ += imu.gyro.velZ*dt;	

	/*
	tempX = imu.accel.posX;
	tempY = imu.accel.posY;
	tempZ = imu.accel.posZ;

	temp2X = imu.accel.velX;
	temp2Y = imu.accel.velY;
	temp2Z = imu.accel.velZ;
	*/

	imu.accel.posX = RAD2DEG*atan2(-(double)imu.accel.rawZ, -(double)imu.accel.rawY);
	imu.accel.posY = RAD2DEG*atan2(-(double)imu.accel.rawX, -(double)imu.accel.rawZ);
	imu.accel.posZ = RAD2DEG*atan2(-(double)imu.accel.rawY, -(double)imu.accel.rawX);

	/*
	imu.accel.velX = (imu.gyro.posX-tempX)/dt;
	imu.accel.velY = (imu.gyro.posY-tempY)/dt;
	imu.accel.velZ = (imu.gyro.posZ-tempZ)/dt;

	imu.accel.accX = (imu.accel.velX-temp2X)/dt;
	imu.accel.accY = (imu.accel.velY-temp2Y)/dt;
	imu.accel.accZ = (imu.accel.velZ-temp2Z)/dt;
	*/
}

void update_ir()
{
	ir.left = !digitalRead(IR_LEFT);
	ir.right = !digitalRead(IR_RIGHT);
}