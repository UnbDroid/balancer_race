#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LBD 0
#define GYRO_GAIN 1.0/131.0 // gyro values ratio for 500 º/s full scale range. If in doubt consult datasheet page 8
#define ACCEL_GAIN 0.00006103515 //(1.0/16384.0) // accel values ratio for 2048 g full scale range. If in doubt consult datasheet page 9
#define MAGNET_GAIN 0.15 // magnet values ratio for 16-bit output.
#define RAD2DEG 57.2957

#define GRAVITY 1.039682
#define ACC_TOLERANCE 0.011163

#define STD_DEV_GYRO_X 0.1628
#define STD_DEV_GYRO_Y 0.2210
#define STD_DEV_GYRO_Z 0.2206
#define STD_DEV_TRIAD_X 2.6934
#define STD_DEV_TRIAD_Y 0.3951
#define STD_DEV_TRIAD_Z 0.5645

#define GYRO_X_OFFSET_HI 0x00
#define GYRO_X_OFFSET_LO 0x00
#define GYRO_Y_OFFSET_HI 0x00
#define GYRO_Y_OFFSET_LO 0x00
#define GYRO_Z_OFFSET_HI 0x00
#define GYRO_Z_OFFSET_LO 0x00

#define MAGX_BIAS 18
#define MAGY_BIAS 73
#define MAGZ_BIAS -246.5

#define ACCELX_BIAS 0.016481
#define ACCELY_BIAS 0.020262
#define ACCELZ_BIAS 0.11054
#define ACCEL_ALPHA 0
#define MEDIAN_SIZE 5

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
#define XA_OFFSET_H 0X77
#define XA_OFFSET_L 0X78
#define YA_OFFSET_H 0X7a
#define YA_OFFSET_L 0X7b
#define ZA_OFFSET_H 0X7d
#define ZA_OFFSET_L 0X7e
#define CNTL1 0x0a
#define CNTL2 0x0b
#define ASAX 0x10
#define ASAY 0x11
#define ASAZ 0x12

#define IR_LEFT 22
#define IR_RIGHT 40

struct infrared {
	int left, right;
};

struct k_filter {
	float yaw, pitch, roll;//valores tratados
	float R[3];//matriz de covariamcia da tried
	float Wk[3];//covariancia gyro
	float Xk[3];//resultado do filtro
	float Pk[3];//matriz de covariancia da predição
	float Qk[3];//covariancia ...
	float K[3];//ganho de kalman	
};

struct gyro {
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double treatedX;
	double treatedY;
	double treatedZ;
};

struct accel {
	unsigned long int n_measurements;
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double treatedX;
	double treatedY;
	double treatedZ;
	double Xvec[MEDIAN_SIZE];
	double Yvec[MEDIAN_SIZE];
	double Zvec[MEDIAN_SIZE];
	double Xmedian;
	double Ymedian;
	double Zmedian;
	double filteredX;
	double filteredY;
	double filteredZ;
	double magnitude;
};

struct magnet {
	int data_ready;
	int overflow;
	int16_t rawX;
	int16_t rawY;
	int16_t rawZ;
	double treatedX;
	double treatedY;
	double treatedZ;
	double magnitude;	
};

struct imu {
	struct gyro gyro;
	struct accel accel;
	struct magnet magnet;
	double yaw, pitch, roll;
	double dt;
	unsigned long long int last_update;
	int update;
};

struct complementar {
	double pitch;
};
struct complementar compFilt;
#define COMP_CONST 0.98

double u[3], v[3], tempvec[3], tempmag;
double i_n[3], j_n[3], k_n[3];
double i_b[3], j_b[3], k_b[3];
double rot_matrix[3][3];

struct k_filter kalman;
struct infrared ir;
struct imu imu;
int MPU9250addr, AK8963addr;
double magsensX, magsensY, magsensZ;

uint8_t gyrXhi, gyrXlo;
uint8_t gyrYhi, gyrYlo;
uint8_t gyrZhi, gyrZlo;

uint8_t accXhi, accXlo;
uint8_t accYhi, accYlo;
uint8_t accZhi, accZlo;

uint8_t mag_data_ready;
uint8_t magXhi, magXlo;
uint8_t magYhi, magYlo;
uint8_t magZhi, magZlo;

unsigned long long int now_time;

double median[MEDIAN_SIZE];

void update_imu();

void init_complementar();
void init_kalman();
void update_kalman();
void QuickSort(double array[], unsigned size);
double getMediana(double array[MEDIAN_SIZE]);

void initMPU9250()
{
	FILE *fp;
	uint8_t data[6];
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

	// set sample rate to 1kHz
	// sample rate defined by 1000/(1+SMPLRT_DIV)
	wiringPiI2CWriteReg8(MPU9250addr, SMPLRT_DIV, 0x00);

	// set gyro max range to 500 degrees/second
	// GYRO_CONFIG
	// [7, 6, 5] - Gyro self-test for [X, Y, Z] axis
	// [4:3] - Gyro full scale value (250, 500, 1000, 2000 degrees/second)
	// [2] - Reserved
	// [1:0] - Setting to bypass DLPF(Differential Low Pass Filter). Disable with 00.
	wiringPiI2CWriteReg8(MPU9250addr, GYRO_CONFIG, 0x00);

	// set gyro offset registers (0x13 to 0x18) to values obtained by the calibGyro.c program
	if(fp = fopen("gyro.calib", "r"))
	{
		fread(data, sizeof(uint8_t), 6, fp);
		wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_H, data[0]);
		wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_L, data[1]);
		wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_H, data[2]);
		wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_L, data[3]);
		wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_H, data[4]);
		wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_L, data[5]);
	} else {
		wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_H, GYRO_X_OFFSET_HI);
		wiringPiI2CWriteReg8(MPU9250addr, XG_OFFSET_L, GYRO_X_OFFSET_LO);
		wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_H, GYRO_Y_OFFSET_HI);
		wiringPiI2CWriteReg8(MPU9250addr, YG_OFFSET_L, GYRO_Y_OFFSET_LO);
		wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_H, GYRO_Z_OFFSET_HI);
		wiringPiI2CWriteReg8(MPU9250addr, ZG_OFFSET_L, GYRO_Z_OFFSET_LO);
	}
	

	// set accel max range to 2 g
	// ACCEL_CONFIG
	// [7, 6, 5] - Accel self-test for [X, Y, Z] axis
	// [4:3] - Accel full scale value (2, 4, 8, 16 g)
	// [2:0] - Reserved
	wiringPiI2CWriteReg8(MPU9250addr, ACCEL_CONFIG, 0x00);

	// set accel DLPF bandwidth
	// ACCEL_CONFIG
	// [7:4] - Reserved
	// [3] - ACCEL_FCHOICE_B - Inverse of ACCEL_FCHOICE.
	// [2:0] - Accel DLPF bandwidth configuration. (4 - 21.2Hz; 5 - 10.2Hz; 6 - 5.05Hz)
	wiringPiI2CWriteReg8(MPU9250addr, ACCEL_CONFIG2, 0x06);

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
	wiringPiI2CReadReg8(AK8963addr, 0x09); // Clear register for first magnetometer reading.
	delay(10);

	int c;
	for(c = 0; c < 3; ++c)
	{
		i_n[c] = 0;
		j_n[c] = 0;
		k_n[c] = 0;
	}

	imu.accel.n_measurements = 0;

	// Preparing the windows for median filter
	imu.accel.Xvec[1] = -2;
	imu.accel.Yvec[1] = -2;
	imu.accel.Zvec[1] = -2;
	imu.accel.Xvec[2] = 2;
	imu.accel.Yvec[2] = 2;
	imu.accel.Zvec[2] = 2;
	imu.accel.Xvec[3] = -2;
	imu.accel.Yvec[3] = -2;
	imu.accel.Zvec[3] = -2;
	imu.accel.Xvec[4] = 2;
	imu.accel.Yvec[4] = 2;
	imu.accel.Zvec[4] = 2;

	update_imu();

	imu.accel.filteredX = imu.accel.treatedX;
	imu.accel.filteredY = imu.accel.treatedY;
	imu.accel.filteredZ = imu.accel.treatedZ;	
}

void init_sensors()
{
	initMPU9250();
	init_kalman();
	init_complementar();
	pinMode(IR_LEFT, INPUT);
	pinMode(IR_RIGHT, INPUT);
}

void update_imu()
{
	now_time = micros();
	imu.dt = (now_time - imu.last_update)/1000000.0;
	imu.last_update = now_time;
	
	// Reading gyroscope
	gyrXhi = wiringPiI2CReadReg8(MPU9250addr, 0x43);
	gyrXlo = wiringPiI2CReadReg8(MPU9250addr, 0x44);
	imu.gyro.rawX = (int16_t)((int16_t)gyrXhi<<8 | gyrXlo);

	gyrYhi = wiringPiI2CReadReg8(MPU9250addr, 0x45);
    gyrYlo = wiringPiI2CReadReg8(MPU9250addr, 0x46);
	imu.gyro.rawY = (int16_t)((int16_t)gyrYhi<<8 | gyrYlo);

	gyrZhi = wiringPiI2CReadReg8(MPU9250addr, 0x47);
    gyrZlo = wiringPiI2CReadReg8(MPU9250addr, 0x48);
	imu.gyro.rawZ = (int16_t)((int16_t)gyrZhi<<8 | gyrZlo);

	// Reading accelerometer
	accXhi = wiringPiI2CReadReg8(MPU9250addr, 0x3b);
    accXlo = wiringPiI2CReadReg8(MPU9250addr, 0x3c);
    imu.accel.rawX = (int16_t)((int16_t)accXhi<<8 | accXlo);

	accYhi = wiringPiI2CReadReg8(MPU9250addr, 0x3d);
    accYlo = wiringPiI2CReadReg8(MPU9250addr, 0x3e);
	imu.accel.rawY = (int16_t)((int16_t)accYhi<<8 | accYlo);

	accZhi = wiringPiI2CReadReg8(MPU9250addr, 0x3f);
    accZlo = wiringPiI2CReadReg8(MPU9250addr, 0x40);
    imu.accel.rawZ = (int16_t)((int16_t)accZhi<<8 | accZlo);

    // Reading magnetometer
	imu.magnet.data_ready = (wiringPiI2CReadReg8(AK8963addr, 0x02) & 0x01);
	if(imu.magnet.data_ready)
	{
		magXlo = wiringPiI2CReadReg8(AK8963addr, 0x03);
	    magXhi = wiringPiI2CReadReg8(AK8963addr, 0x04);
	    imu.magnet.rawX = (int16_t)((int16_t)magXhi<<8 | magXlo);
	    
	    magYlo = wiringPiI2CReadReg8(AK8963addr, 0x05);
		magYhi = wiringPiI2CReadReg8(AK8963addr, 0x06);
		imu.magnet.rawY = (int16_t)((int16_t)magYhi<<8 | magYlo);

	    magZlo = wiringPiI2CReadReg8(AK8963addr, 0x07);
		magZhi = wiringPiI2CReadReg8(AK8963addr, 0x08);
		imu.magnet.rawZ = (int16_t)((int16_t)magZhi<<8 | magZlo);
		
		imu.magnet.overflow = (wiringPiI2CReadReg8(AK8963addr, 0x09) & 0x08);
	}

    // Unit corrections for the gyroscope
	imu.gyro.treatedX = GYRO_GAIN*(double)imu.gyro.rawX;
	imu.gyro.treatedY = GYRO_GAIN*(double)imu.gyro.rawY;
	imu.gyro.treatedZ = GYRO_GAIN*(double)imu.gyro.rawZ;

	// Unit corrections for the accelerometer
	imu.accel.treatedX = (ACCEL_GAIN*(double)imu.accel.rawX)-ACCELX_BIAS;
	imu.accel.treatedY = (ACCEL_GAIN*(double)imu.accel.rawY)-ACCELY_BIAS;
	imu.accel.treatedZ = (ACCEL_GAIN*(double)imu.accel.rawZ)-ACCELZ_BIAS;
	
	// Median and low-pass filtering for the accelerometer
	imu.accel.Xvec[imu.accel.n_measurements%MEDIAN_SIZE] = imu.accel.treatedX;
	imu.accel.Yvec[imu.accel.n_measurements%MEDIAN_SIZE] = imu.accel.treatedY;
	imu.accel.Zvec[imu.accel.n_measurements%MEDIAN_SIZE] = imu.accel.treatedZ;
	imu.accel.Xmedian = getMediana(imu.accel.Xvec);
	imu.accel.Ymedian = getMediana(imu.accel.Yvec);
	imu.accel.Zmedian = getMediana(imu.accel.Zvec);
	imu.accel.filteredX = ACCEL_ALPHA*imu.accel.filteredX + (1-ACCEL_ALPHA)*imu.accel.Xmedian;
	imu.accel.filteredY = ACCEL_ALPHA*imu.accel.filteredY + (1-ACCEL_ALPHA)*imu.accel.Ymedian;
	imu.accel.filteredZ = ACCEL_ALPHA*imu.accel.filteredZ + (1-ACCEL_ALPHA)*imu.accel.Zmedian;
	++(imu.accel.n_measurements);

	// Calculating acceleration magnitude
	imu.accel.magnitude = sqrt(pow(imu.accel.filteredX, 2) + pow(imu.accel.filteredY, 2) + pow(imu.accel.filteredZ, 2));

	// Axis inversions and unit corrections for the magnetometer.
	// For some reason it is mounted to the MPU9250 module with X and Y axis switched and Z axis inverted.
	imu.magnet.treatedX = ((double)imu.magnet.rawY-MAGY_BIAS)*magsensY;
	imu.magnet.treatedY = ((double)imu.magnet.rawX-MAGX_BIAS)*magsensX;
	imu.magnet.treatedZ = (MAGZ_BIAS-(double)imu.magnet.rawZ)*magsensZ;
	imu.magnet.magnitude = sqrt(pow(imu.magnet.treatedX, 2) + pow(imu.magnet.treatedY, 2) + pow(imu.magnet.treatedZ, 2));
	
	imu.update = 1;
	
	// Conditions for changing the value of imu.update should be put in here

	if(!imu.update) return;

	
	// TRIAD algorithm code

	// Defining u and v vectors.
	// These are unit vectors corresponding to gravitational force and magnetic field respectively.
	u[0] = imu.accel.filteredX/imu.accel.magnitude;
	u[1] = imu.accel.filteredY/imu.accel.magnitude;
	u[2] = imu.accel.filteredZ/imu.accel.magnitude;
	
	v[0] = imu.magnet.treatedX/imu.magnet.magnitude;
	v[1] = imu.magnet.treatedY/imu.magnet.magnitude;
	v[2] = imu.magnet.treatedZ/imu.magnet.magnitude;
	
	// Defining i, j and k as described in the robot coordinate system.
	int c;
	for(c = 0; c < 3; ++c)
	{
		tempvec[c] = u[c] + v[c];
	}
	tempmag = sqrt(pow(tempvec[0], 2) + pow(tempvec[1], 2) + pow(tempvec[2], 2));
	for(c = 0; c < 3; ++c)
	{
		i_b[c] = (u[c]+v[c])/tempmag;
	}		
	
	for(c = 0; c < 3; ++c)
	{
		tempvec[c] = u[c] - v[c];
	}
	j_b[0] = i_b[1]*tempvec[2] - i_b[2]*tempvec[1];
	j_b[1] = i_b[2]*tempvec[0] - i_b[0]*tempvec[2];
	j_b[2] = i_b[0]*tempvec[1] - i_b[1]*tempvec[0];
	tempmag = sqrt(pow(j_b[0], 2) + pow(j_b[1], 2) + pow(j_b[2], 2));
	for(c = 0; c < 3; ++c)
	{
		j_b[c] /= tempmag;
	}

	k_b[0] = i_b[1]*j_b[2] - i_b[2]*j_b[1];
	k_b[1] = i_b[2]*j_b[0] - i_b[0]*j_b[2];
	k_b[2] = i_b[0]*j_b[1] - i_b[1]*j_b[0];

	// Strongly recommend that the section below be done during calibration instead.
	if(	i_n[0] == 0 && i_n[1] == 0 && i_n[2] == 0 &&
		j_n[0] == 0 && j_n[1] == 0 && j_n[2] == 0 &&
		k_n[0] == 0 && k_n[1] == 0 && k_n[2] == 0)
	{
		/* Since this is run when we first call update_imu() function, set
		 * the TRIAD vectors in reference to the robot coordinate system
		 * as equal to the TRIAD vectors in reference to the navigational
		 * coordinate system. This means the starting position of the robot
		 * will define its navigational coordinate system.
		*/

		i_n[0] = 0.056063;	i_n[1] = -0.493376;	i_n[2] = -0.868007;
		j_n[0] = 0.969514;	j_n[1] = 0.234605;	j_n[2] = -0.070731;
		k_n[0] = 0.238536;	k_n[1] = -0.837580;	k_n[2] = 0.491488;

		//for calibration
		/*
			printf("i_n[0] = %lf;\ti_n[1] = %lf;\ti_n[2] = %lf;\n", i_n[0], i_n[1], i_n[2]);
			printf("j_n[0] = %lf;\tj_n[1] = %lf;\tj_n[2] = %lf;\n", j_n[0], j_n[1], j_n[2]);
			printf("k_n[0] = %lf;\tk_n[1] = %lf;\tk_n[2] = %lf;\n", k_n[0], k_n[1], k_n[2]);

			for(c = 0; c < 3; ++c)
			{
				i_n[c] = i_b[c];
				j_n[c] = j_b[c];
				k_n[c] = k_b[c];
			}
		*/
	}


	// Calculating the rotation matrix from the navigational coordinate
	// system to the robot coordinate system using the TRIAD algorithm.
	int i, j;
	for(i = 0; i < 3; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			rot_matrix[i][j] = i_b[i]*i_n[j] + j_b[i]*j_n[j] + k_b[i]*k_n[j];
		}
	}

	// Calculating yaw, pitch and roll angles based on the rotation matrix
	// obtained above using the TRIAD algorithm.
	imu.yaw = RAD2DEG*atan2(	rot_matrix[0][1], 
								sqrt(1-pow(rot_matrix[1][0], 2) - pow(rot_matrix[2][0], 2)));

	imu.pitch = RAD2DEG*atan2(	-rot_matrix[0][2],
								sqrt(pow(rot_matrix[1][2], 2) + pow(rot_matrix[2][2], 2)));

	imu.roll = RAD2DEG*atan2(	rot_matrix[1][2], 
								sqrt(1-pow(rot_matrix[2][0], 2) - pow(rot_matrix[2][1], 2)));


}

void init_complementar()
{
	compFilt.pitch = imu.accel.treatedX;
}

void update_complementar()
{
	compFilt.pitch = COMP_CONST*(compFilt.pitch + imu.dt*imu.gyro.treatedX) + (1 - COMP_CONST)*(imu.accel.treatedX);
	//printf("complementar pitch: %f...\n", compFilt.pitch*RAD2DEG);
}

void init_kalman()
{
	kalman.R[0] = STD_DEV_TRIAD_X;
	kalman.R[1] = STD_DEV_TRIAD_Y;
	kalman.R[2] = STD_DEV_TRIAD_Z;
	kalman.Wk[0] = STD_DEV_GYRO_X;
	kalman.Wk[1] = STD_DEV_GYRO_Y;
	kalman.Wk[2] = STD_DEV_GYRO_Z;

	kalman.Xk[0] = imu.roll;
	kalman.Xk[1] = imu.pitch;
	kalman.Xk[2] = imu.yaw;

	kalman.Pk[0] = kalman.R[0];
	kalman.Pk[1] = kalman.R[1];
	kalman.Pk[2] = kalman.R[2];

	kalman.Qk[0] = kalman.Wk[0] + LBD;
	kalman.Qk[1] = kalman.Wk[1] + LBD;
	kalman.Qk[2] = kalman.Wk[2] + LBD;
}


void update_kalman()
{
	// Prediction
	kalman.Xk[0] = kalman.Xk[0] + imu.dt*imu.gyro.treatedX;
	kalman.Xk[1] = kalman.Xk[1] + imu.dt*imu.gyro.treatedY;
	kalman.Xk[2] = kalman.Xk[2] + imu.dt*imu.gyro.treatedZ;

	kalman.Pk[0] = kalman.Pk[0] + kalman.Qk[0];
	kalman.Pk[1] = kalman.Pk[1] + kalman.Qk[1];
	kalman.Pk[2] = kalman.Pk[2] + kalman.Qk[2];
	
	if(imu.update)
	{
		// Correction
		kalman.K[0] = kalman.Pk[0] / (kalman.Pk[0]+kalman.R[0]);
		kalman.K[1] = kalman.Pk[1] / (kalman.Pk[1]+kalman.R[1]);
		kalman.K[2] = kalman.Pk[2] / (kalman.Pk[2]+kalman.R[2]);

		kalman.Xk[0] = kalman.Xk[0] + kalman.K[0] * (imu.roll - kalman.Xk[0]);
		kalman.Xk[1] = kalman.Xk[1] + kalman.K[1] * (imu.pitch - kalman.Xk[1]);
		kalman.Xk[2] = kalman.Xk[2] + kalman.K[2] * (imu.yaw - kalman.Xk[2]);

		kalman.Pk[0] = (1-kalman.K[0])*kalman.Pk[0]*(1 - kalman.K[0]) + kalman.K[0]*kalman.R[0]*kalman.K[0];
		kalman.Pk[1] = (1-kalman.K[1])*kalman.Pk[1]*(1 - kalman.K[1]) + kalman.K[1]*kalman.R[1]*kalman.K[1];
		kalman.Pk[2] = (1-kalman.K[2])*kalman.Pk[2]*(1 - kalman.K[2]) + kalman.K[2]*kalman.R[2]*kalman.K[2];
	}

	kalman.roll = kalman.Xk[0];
	kalman.pitch = kalman.Xk[1];
	kalman.yaw = kalman.Xk[2];
}

void update_ir()
{
	ir.left = !digitalRead(IR_LEFT);
	ir.right = !digitalRead(IR_RIGHT);
}

// Abdullah's QuickSort implementation for usage with the Median Filter
unsigned Partition(double array[], unsigned f, unsigned l, double pivot)
{
    unsigned i = f-1, j = l+1;
    while(1)
    {
        while(pivot < array[--j]);
        while(array[++i] < pivot);
        if(i<j)
        {
            double tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;
        }
        else
            return j;
    }
}

void QuickSortImpl(double array[], unsigned f, unsigned l)
{
    while(f < l)
    {
        unsigned m = Partition(array, f, l, array[f]);
        QuickSortImpl(array, f, m);
        f = m+1;
    }
}

void QuickSort(double array[], unsigned size)
{
    QuickSortImpl(array, 0, size-1);
}

double getMediana(double array[MEDIAN_SIZE])
{
	int i;
	for (i = 0; i < MEDIAN_SIZE; i++)
	{
		median[i] = array[i];
	}

	QuickSort(median, MEDIAN_SIZE);
	return median[MEDIAN_SIZE/2];
}