#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GYRO_GAIN 0.01526717557 //(1.0/65.5)	// gyro values ratio for 500 º/s full scale range. If in doubt consult datasheet page 8
#define ACCEL_GAIN 0.00006103515 //(1.0/16384.0) // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9
#define MAGNET_GAIN 0.15 // magnet values ratio for 16-bit output.
#define RAD2DEG 57.2957

#define GYRO_X_OFFSET_HI 0x00
#define GYRO_X_OFFSET_LO 0x0b
#define GYRO_Y_OFFSET_HI 0xff
#define GYRO_Y_OFFSET_LO 0xf8
#define GYRO_Z_OFFSET_HI 0x00
#define GYRO_Z_OFFSET_LO 0x04

#define MAGX_BIAS 18
#define MAGY_BIAS 73
#define MAGZ_BIAS -246.5

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
#define CNTL2 0x0b
#define ASAX 0x10
#define ASAY 0x11
#define ASAZ 0x12

#define IR_LEFT 22
#define IR_RIGHT 40


//numero escolhido com base em testes, para saber quando 
//ignorar leitura de um dos eixos do acelerometroou do gyro
#define MUITOMAIOR 3

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
int MPU9250addr, AK8963addr;
unsigned long long int last_update;
double magsensX, magsensY, magsensZ;
double relative_shift_x, relative_shift_y, relative_shift_z;
double NaN;

uint8_t gyrXhi, gyrXlo;
uint8_t gyrYhi, gyrYlo;
uint8_t gyrZhi, gyrZlo;

uint8_t accXhi, accXlo;
uint8_t accYhi, accYlo;
uint8_t accZhi, accZlo;

uint8_t mag_overflow, mag_data_ready;
uint8_t magXhi, magXlo;
uint8_t magYhi, magYlo;
uint8_t magZhi, magZlo;

int16_t temp;
double tempX, tempY, tempZ;

unsigned long long int now_time;
double dt;

void update_imu();

void initMPU9250()
{
	MPU9250addr = wiringPiI2CSetup(0x68);

	// set PWR_MGMT_1 register
	// [7] H_RESET - if set to 1 it will auto clear and reset the chip registers to default values
	// [6] SLEEP - if set, the chip is set to sleep mode
	// [5] CYCLE - if set, chip will alternate between sleep and taking samples
	// [4] GYRO_STANDBY - if set, gyro will be always partially on so that it's faster to enable
	// [3] PD_PTAT - power down the Proportional To Absolute Temperature voltage regulator
	// [2:0] CLKSEL - set to 000 to use internal clock, 001 to auto select clock
	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x80); // reset MPU9250 registers to default configurations
	delay(100);
	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_1, 0x01); // disable sleep, set clock to auto

	// enable magnetometer bypass
	// INT_PIN_CFG
	// [7] - ACTL - Check page 29 of register map for more info
	// [6] - OPEN - Check page 29 of register map for more info
	// [5] - LATCH_INT_EN - Check page 29 of register map for more info
	// [4] - INT_ANYRD_2CLEAR - Check page 29 of register map for more info
	// [3] - ACTL_FSYNC - Check page 29 of register map for more info
	// [2] - FSYNC_INT_MODE_EN - Check page 29 of register map for more info
	// [1] - BYPASS_EN - Check page 29 of register map for more info
	// [0] - Reserved
	wiringPiI2CWriteReg8(MPU9250addr, INT_PIN_CFG, 0x02);

	AK8963addr = wiringPiI2CSetup(0x0c);

	// set PWR_MGMT_2 register
	// [7, 6] - Reserved
	// [5, 4, 3] - Set to 0 to enable Accel [X, Y, Z]
	// [2, 1, 0] - Set to 0 to enable Gyro  [X, Y, Z]
	wiringPiI2CWriteReg8(MPU9250addr, PWR_MGMT_2, 0x00);

	// set CONFIG register
	// [7] - Reserved
	// [6] - FIFO_MODE - when set to 0, discard readings if FIFO is full. Otherwise, oldest data will be replaced.
	// [5:3] - EXT_SYNC_SET - just disable with 000. For more info, check page 13 of the register map
	// [2:0] - DLPF_CFG - Differential Low Pass Filter Configuration. Check page 13 of the register map
	wiringPiI2CWriteReg8(MPU9250addr, CONFIG, 0x03);

	// set sample rate to 200Hz
	// sample rate defined by 1/(1+SMPLRT_DIV)
	wiringPiI2CWriteReg8(MPU9250addr, SMPLRT_DIV, 0x04);

	// set gyro max range to 500 degrees/second
	// GYRO_CONFIG
	// [7, 6, 5] - Gyro self-test for [X, Y, Z] axis
	// [4:3] - Gyro full scale value (250, 500, 1000, 2000 degrees/second)
	// [2] - Reserved
	// [1:0] - Setting to bypass DLPF(Differential Low Pass Filter). Disable with 00.
	wiringPiI2CWriteReg8(MPU9250addr, GYRO_CONFIG, 0x08);

	// set gyro offset registers (0x13 to 0x18) to values obtained by the calibGyro.c program
	wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_H, GYRO_X_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_L, GYRO_X_OFFSET_LO);
	wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_H, GYRO_Y_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_L, GYRO_Y_OFFSET_LO);
	wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_H, GYRO_Z_OFFSET_HI);
	wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_L, GYRO_Z_OFFSET_LO);

	// set accel max range to 2 g
	// ACCEL_CONFIG
	// [7, 6, 5] - Accel self-test for [X, Y, Z] axis
	// [4:3] - Accel full scale value (2, 4, 8, 16 g)
	// [2:0] - Reserved
	wiringPiI2CWriteReg8(MPU9250addr, ACCEL_CONFIG, 0x00);

	// soft reset magnetometer
	// CNTL2
	// [7:1] - Reserved
	// [0] - When set to 1, magnetometer automatically resets and sets this bit to 0
	wiringPiI2CWriteReg8(AK8963addr, CNTL2, 0x01);
	delay(50);

	// get magnetometer sensitivity value
	// ASAX, ASAY, ASAZ
	// [7:0] - ASA value. Hadj = H*((ASA-128)/256 + 1)
	wiringPiI2CWriteReg8(AK8963addr, CNTL1, 0x0F);
	delay(50);
	magsensX = (double)(wiringPiI2CReadReg8(AK8963addr, ASAX)-128.0)/256.0 + 1;
	magsensY = (double)(wiringPiI2CReadReg8(AK8963addr, ASAY)-128.0)/256.0 + 1;
	magsensZ = (double)(wiringPiI2CReadReg8(AK8963addr, ASAZ)-128.0)/256.0 + 1;
	wiringPiI2CWriteReg8(AK8963addr, CNTL1, 0x00);
	delay(50);

	// set magnetometer Control 1 register
	// CTNL1
	// [7:5] - Reserved
	// [4] - If 0, output is 14-bits two's complement. If 1, output is 16-bits two's complement
	// [3:0] - Magnetometer mode selection. Check page 51 of the register map for more info
	wiringPiI2CWriteReg8(AK8963addr, CNTL1, 0x16);
	delay(50);
	
	relative_shift_x = 0;
	relative_shift_y = 0;
	relative_shift_z = 0;

	update_imu();

	// Checking if accel and magnet values are not NaN and setting gyroscope position initial values
	if(imu.accel.posX == imu.accel.posX) imu.gyro.posX = imu.accel.posX; else imu.gyro.posX = 0;
	if(imu.accel.posY == imu.accel.posY) imu.gyro.posY = imu.accel.posY; else imu.gyro.posY = 0;
	if(imu.magnet.posZ == imu.magnet.posZ) imu.gyro.posZ = imu.magnet.posZ; else imu.gyro.posZ = 0;

	// Checking if sensor values are NaN and setting relative shift values
	if((imu.accel.posX == imu.accel.posX) && (imu.magnet.posX == imu.magnet.posX))
			relative_shift_x = imu.accel.posX - imu.magnet.posX;
	if((imu.accel.posY == imu.accel.posY) && (imu.magnet.posY == imu.magnet.posY))
			relative_shift_y = imu.accel.posY - imu.magnet.posY;
	if((imu.accel.posZ == imu.accel.posZ) && (imu.magnet.posZ == imu.magnet.posZ))
			relative_shift_z = imu.magnet.posZ - imu.accel.posZ;


}

void init_sensors()
{
	NaN = sqrt(-1);
	initMPU9250();

	pinMode(IR_LEFT, INPUT);
	pinMode(IR_RIGHT, INPUT);
}

void update_imu()
{
	now_time = micros();
	imu.dt = now_time - last_update;
	dt = (double)imu.dt/1000000.0;
	last_update = now_time;

	//reading gyro
	gyrXhi = wiringPiI2CReadReg8(MPU9250addr, 0x43);
	gyrXlo = wiringPiI2CReadReg8(MPU9250addr, 0x44);
	imu.gyro.rawX = (int16_t)((int16_t)gyrXhi<<8 | gyrXlo);

	gyrYhi = wiringPiI2CReadReg8(MPU9250addr, 0x45);
    gyrYlo = wiringPiI2CReadReg8(MPU9250addr, 0x46);
	imu.gyro.rawY = (int16_t)((int16_t)gyrYhi<<8 | gyrYlo);

	gyrZhi = wiringPiI2CReadReg8(MPU9250addr, 0x47);
    gyrZlo = wiringPiI2CReadReg8(MPU9250addr, 0x48);
	imu.gyro.rawZ = (int16_t)((int16_t)gyrZhi<<8 | gyrZlo);

	//readig acc
	accXhi = wiringPiI2CReadReg8(MPU9250addr, 0x3b);
    accXlo = wiringPiI2CReadReg8(MPU9250addr, 0x3c);
    imu.accel.rawX = (int16_t)((int16_t)accXhi<<8 | accXlo);

	accYhi = wiringPiI2CReadReg8(MPU9250addr, 0x3d);
    accYlo = wiringPiI2CReadReg8(MPU9250addr, 0x3e);
	imu.accel.rawY = (int16_t)((int16_t)accYhi<<8 | accYlo);

	accZhi = wiringPiI2CReadReg8(MPU9250addr, 0x3f);
    accZlo = wiringPiI2CReadReg8(MPU9250addr, 0x40);
    imu.accel.rawZ = (int16_t)((int16_t)accZhi<<8 | accZlo);

    //mag_data_ready = wiringPiI2CReadReg8(AK8963addr, 0x02);
    //if(mag_data_ready & 0x01)
    //{
    mag_overflow = wiringPiI2CReadReg8(AK8963addr, 0x09);

    if(!(mag_overflow & 0x08))
    {
    	//reding mag
		magXlo = wiringPiI2CReadReg8(AK8963addr, 0x03);
	    magXhi = wiringPiI2CReadReg8(AK8963addr, 0x04);
	    imu.magnet.rawX = (int16_t)((int16_t)magXhi<<8 | magXlo);

	    magYlo = wiringPiI2CReadReg8(AK8963addr, 0x05);
		magYhi = wiringPiI2CReadReg8(AK8963addr, 0x06);
		imu.magnet.rawY = (int16_t)((int16_t)magYhi<<8 | magYlo);

	    magZlo = wiringPiI2CReadReg8(AK8963addr, 0x07);
		magZhi = wiringPiI2CReadReg8(AK8963addr, 0x08);
		imu.magnet.rawZ = (int16_t)((int16_t)magZhi<<8 | magZlo);

		tempX = ((double)imu.magnet.rawX-MAGX_BIAS)*magsensX;
		tempY = ((double)imu.magnet.rawY-MAGY_BIAS)*magsensY;
		tempZ = ((double)imu.magnet.rawZ-MAGZ_BIAS)*magsensZ;

		//calculando o angulo com base na IMU
		if(abs(tempX) > (MUITOMAIOR*(abs(tempZ)+abs(tempY))))
			imu.magnet.posX = NaN;
		else
		{
			imu.magnet.posX = (RAD2DEG*atan2(tempZ, tempY)) + relative_shift_x;
			if(imu.magnet.posX > 180) imu.magnet.posX -= 360;
			else if(imu.magnet.posX < -180) imu.magnet.posX += 360;
		}
		
		if(abs(tempY) > (MUITOMAIOR*(abs(tempZ)+abs(tempX))))
			imu.magnet.posY = NaN;
		else
		{
			imu.magnet.posY = (RAD2DEG*atan2(tempX, tempZ)) + relative_shift_y;
			if(imu.magnet.posY > 180) imu.magnet.posY -= 360;
			else if(imu.magnet.posY < -180) imu.magnet.posY += 360;
		}
		
		if(abs(tempZ) > (MUITOMAIOR*(abs(tempX)+abs(tempY))))
			imu.magnet.posZ = NaN;
		else
			imu.magnet.posZ = RAD2DEG*atan2(tempY, tempX);		
    }
    else
    {
    	imu.magnet.posX = NaN;
		imu.magnet.posY = NaN;
		imu.magnet.posZ = NaN;
    }
    //}

    // Axis inversions and unit corrections for the gyroscope
	imu.gyro.velX = -GYRO_GAIN*(double)imu.gyro.rawY;
	imu.gyro.velY = -GYRO_GAIN*(double)imu.gyro.rawX;
	imu.gyro.velZ = GYRO_GAIN*(double)imu.gyro.rawZ;

	// Gyroscope integration
	imu.gyro.posX += imu.gyro.velX*dt;
	if(imu.gyro.posX > 180) imu.gyro.posX -= 360;
	else if(imu.gyro.posX < -180) imu.gyro.posX += 360;
	
	imu.gyro.posY += imu.gyro.velY*dt;
	if(imu.gyro.posY > 180) imu.gyro.posY -= 360;
	else if(imu.gyro.posY < -180) imu.gyro.posY += 360;
	
	imu.gyro.posZ += imu.gyro.velZ*dt;
	if(imu.gyro.posZ > 180) imu.gyro.posZ -= 360;
	else if(imu.gyro.posZ < -180) imu.gyro.posZ += 360;	

	// Accelerometer angular position measurement calculations
	tempX = (double)imu.accel.rawY;
	tempY = (double)imu.accel.rawX;
	tempZ = -1.0*(double)imu.accel.rawZ;

	if(abs(tempX) > (MUITOMAIOR*(abs(tempZ)+abs(tempY))))
		imu.accel.posX = NaN;
	else
		imu.accel.posX = RAD2DEG*atan2(tempZ, tempY);
	
	if(abs(tempY) > (MUITOMAIOR*(abs(tempX)+abs(tempZ))))
		imu.accel.posY = NaN;
	else
		imu.accel.posY = RAD2DEG*atan2(tempX, tempZ);
	
	if(abs(tempZ) > (MUITOMAIOR*(abs(tempY)+abs(tempX))))
	{
		imu.accel.posZ = NaN;
	} else {
		imu.accel.posZ = (RAD2DEG*atan2(tempY, tempX)) + relative_shift_z;
		if(imu.accel.posZ > 180) imu.accel.posZ -= 360;
		else if(imu.accel.posZ < -180) imu.accel.posZ += 360;
	}

	// Checking if sensor values are NaN and setting relative shift values if they are not yet set
	if((relative_shift_x == 0)&&((imu.accel.posX == imu.accel.posX) && (imu.magnet.posX == imu.magnet.posX))) 
	{
		relative_shift_x = imu.accel.posX - imu.magnet.posX;
		imu.gyro.posX = imu.accel.posX;
	}
	if((relative_shift_y == 0)&&((imu.accel.posY == imu.accel.posY) && (imu.magnet.posY == imu.magnet.posY))) 
	{
		relative_shift_y = imu.accel.posY - imu.magnet.posY;
		imu.gyro.posY = imu.accel.posY;
	}
	if((relative_shift_z == 0)&&((imu.accel.posZ == imu.accel.posZ) && (imu.magnet.posZ == imu.magnet.posZ))) 
	{
		relative_shift_z = imu.magnet.posZ - imu.accel.posZ;
		imu.gyro.posZ = imu.magnet.posZ;
	}

}

void update_ir()
{
	ir.left = !digitalRead(IR_LEFT);
	ir.right = !digitalRead(IR_RIGHT);
}