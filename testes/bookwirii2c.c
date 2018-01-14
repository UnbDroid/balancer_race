/* *************************************************************** *\
|																	|
|	PROGRAMA FEITO PARA LEITURA DO SENSOR MPU-60X0 USANDO 			|
|	O PROTOCOLO DE COMUNICACAO I2C.									|
|	EH FORTEMENTE RECOMENDADO A LEITURA DO CODIGO COM O 			|
|	DATASHEET E O MAPA DE REGISTRADORES	FORNECIDOS PELO FABRICANTE.	|
|																	|
\* *************************************************************** */

#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define OFFSET_SAMPLES 1000.0
#define R_GAIN (1.0/65.5)	// ajustado para uma sensibilidade de 2000 DPS, para outras sensibilidades consultar pagina 12 do datasheet.
#define G_GAIN (1.0/1370)	// ajustado para uma sensibilidade de +-1.3 Ga, para outras sensibilidades consultar pagina 12 do datasheet.
#define DT 1.000
#define M_PI 3.14159265358979323846
#define RAD2DEG 57.29578
#define AA 0.98

int mymillis();

int main(){
	int endeMPU6050 = wiringPiI2CSetup(0x68);
	int endeHMC5883L = wiringPiI2CSetup(0x1e);

	int gyrXhi, gyrXlo;
	int16_t gyrXhilo;
	int gyrYhi, gyrYlo;
	int16_t gyrYhilo;
	int gyrZhi, gyrZlo;
	int16_t gyrZhilo;

	int magXhi, magXlo;
	int16_t magXhilo;
	int magYhi, magYlo;
	int16_t magYhilo;
	int magZhi, magZlo;
	int16_t magZhilo;

	long int gyrXoffset = 0, gyrYoffset = 0, gyrZoffset = 0;

	int accXhi, accXlo;
	int16_t accXhilo;
	int accYhi, accYlo;
	int16_t accYhilo;
	int accZhi, accZlo;
	int16_t accZhilo;

	int temphi, templo, temphilo;

	float gyrXrate = 0, gyrXang = 0, accXang = 0, angX = 0, magXgaus = 0;
	float gyrYrate = 0, gyrYang = 0, accYang = 0, angY = 0, magYgaus = 0; 
	float gyrZrate = 0, gyrZang = 0, accZang = 0, angZ = 0, magZgaus = 0;

	int i;

	int startInt = mymillis();

	clock_t last_read, current_read;
	double dt;

	// setando o bit do registrador para tirar o giroscópio/acelerômetro do sleep.
	temphi = wiringPiI2CReadReg8(endeMPU6050, 0x6b);
	temphi = temphi & 0xbf;
	wiringPiI2CWriteReg8(endeMPU6050, 0x6b, temphi);

	// setando o bit do registrador para tirar o magnetômetro do sleep.
	temphi = wiringPiI2CReadReg8(endeHMC5883L, 0x02);
	temphi = temphi & 0xfc;
	wiringPiI2CWriteReg8(endeHMC5883L, 0x02, temphi);

	// setando o sensor para 500 DPS (sim, ele é um carry).
	wiringPiI2CWriteReg8(endeMPU6050, 0x1b, 0x08);

	// setando o sensor para 0.88 Gauss.
	wiringPiI2CWriteReg8(endeHMC5883L, 0x01, 0x00);

	for(i = 0; i < OFFSET_SAMPLES; ++i)
	{
		gyrXhi = wiringPiI2CReadReg8(endeMPU6050, 0x43);
		gyrXlo = wiringPiI2CReadReg8(endeMPU6050, 0x44);
		gyrYhi = wiringPiI2CReadReg8(endeMPU6050, 0x45);
        gyrYlo = wiringPiI2CReadReg8(endeMPU6050, 0x46);
		gyrZhi = wiringPiI2CReadReg8(endeMPU6050, 0x47);
        gyrZlo = wiringPiI2CReadReg8(endeMPU6050, 0x48);

		gyrXhilo = ((gyrXhi<<8)|gyrXlo);
		gyrYhilo = ((gyrYhi<<8)|gyrYlo);
		gyrZhilo = ((gyrZhi<<8)|gyrZlo);
		
		gyrXoffset += gyrXhilo;
		gyrYoffset += gyrYhilo;
		gyrZoffset += gyrZhilo;

		usleep(1000);
	}
	gyrXoffset = gyrXoffset/OFFSET_SAMPLES;
	gyrYoffset = gyrYoffset/OFFSET_SAMPLES;
	gyrZoffset = gyrZoffset/OFFSET_SAMPLES;

	int last_print = mymillis();
	current_read = clock();
	while(1){
		startInt = mymillis();

		// leitura dos valores brutos do sensor.
		last_read = current_read;
		current_read = clock();
		gyrXhi = wiringPiI2CReadReg8(endeMPU6050, 0x43);
		gyrXlo = wiringPiI2CReadReg8(endeMPU6050, 0x44);
		gyrYhi = wiringPiI2CReadReg8(endeMPU6050, 0x45);
        gyrYlo = wiringPiI2CReadReg8(endeMPU6050, 0x46);
		gyrZhi = wiringPiI2CReadReg8(endeMPU6050, 0x47);
        gyrZlo = wiringPiI2CReadReg8(endeMPU6050, 0x48);
		
		accXhi = wiringPiI2CReadReg8(endeMPU6050, 0x3b);
        accXlo = wiringPiI2CReadReg8(endeMPU6050, 0x3c);
		accYhi = wiringPiI2CReadReg8(endeMPU6050, 0x3d);
        accYlo = wiringPiI2CReadReg8(endeMPU6050, 0x3e);
		accZhi = wiringPiI2CReadReg8(endeMPU6050, 0x3f);
        accZlo = wiringPiI2CReadReg8(endeMPU6050, 0x40);

		temphi = wiringPiI2CReadReg8(endeMPU6050, 0x41);
        templo = wiringPiI2CReadReg8(endeMPU6050, 0x42);

        magXhi = wiringPiI2CReadReg8(endeHMC5883L, 0x03);
		magXlo = wiringPiI2CReadReg8(endeHMC5883L, 0x04);
		magYhi = wiringPiI2CReadReg8(endeHMC5883L, 0x07);
        magYlo = wiringPiI2CReadReg8(endeHMC5883L, 0x08);
		magZhi = wiringPiI2CReadReg8(endeHMC5883L, 0x05);
        magZlo = wiringPiI2CReadReg8(endeHMC5883L, 0x06);

		gyrXhilo = ((gyrXhi<<8)|gyrXlo) - (int16_t)gyrXoffset;
		gyrYhilo = ((gyrYhi<<8)|gyrYlo) - (int16_t)gyrYoffset;
		gyrZhilo = ((gyrZhi<<8)|gyrZlo) - (int16_t)gyrZoffset;
		accXhilo = ((accXhi<<8)|accXlo);
		accYhilo = ((accYhi<<8)|accYlo);
		accZhilo = ((accZhi<<8)|accZlo);
		temphilo = ((temphi<<8)|templo);

		magXhilo = ((magXhi<<8)|magXlo);
		magYhilo = ((magYhi<<8)|magYlo);
		magZhilo = ((magZhi<<8)|magZlo);

		// tratamentos dos dados brutos do sensor.
		gyrXrate = (float)gyrXhilo * R_GAIN;
		gyrYrate = (float)gyrYhilo * R_GAIN;
		gyrZrate = (float)gyrZhilo * R_GAIN;

		magXgaus = (float)magXhilo * G_GAIN;
		magYgaus = (float)magYhilo * G_GAIN;
		magZgaus = (float)magZhilo * G_GAIN;

		dt = ((double)(current_read-last_read)/(double)CLOCKS_PER_SEC);

		gyrXang += gyrXrate * dt;
		gyrYang += gyrYrate * dt;
		gyrZang += gyrZrate * dt;

		/*
		accXang = (atan2f(accYhilo, accZhilo) + M_PI)*RAD2DEG;
		accYang = (atan2f(accZhilo, accXhilo) + M_PI)*RAD2DEG;
		accZang = (atan2f(accXhilo, accYhilo) + M_PI)*RAD2DEG;

		// angulo combinado do gyro e accel e filtrado por um filtro complementar.
		angX = AA*(angX + gyrXrate * DT) + (1 - AA) * accXang;
		angY = AA*(angX + gyrYrate * DT) + (1 - AA) * accYang;
		angZ = AA*(angX + gyrZrate * DT) + (1 - AA) * accZang;
		
		angX = atan(accXhi/(sqrtf(accYhi*accYhi + accZhi*accZhi)))*180/M_PI;
		angY = atan(accYhi/(sqrtf(accXhi*accXhi + accZhi*accZhi)))*180/M_PI;
		angZ = atan(accZhi/(sqrtf(accXhi*accXhi + accYhi*accYhi)))*180/M_PI;
		
		*/
		//printf("%f\t %f\t %f\t %f\t %f\t %f\t", angX, accYhi*accYhi + accZhi*accZhi, angY, accXhi*accXhi + accZhi*accZhi, angZ, accXhi*accXhi + accYhi*accYhi);
		while(mymillis() - startInt < DT){
			usleep(10);
		}
		if(mymillis() - last_print > 200)
		{
			last_print = mymillis();
			system("clear");
			printf("GyroX:%.3lf GyroY:%.3lf GyroZ:%.3lf\nMagX:%.3f MagY:%.3f MagZ:%.3f\nTime:%.3lfms\n", 
					gyrXang, gyrYang, gyrZang, magXgaus, magYgaus, magZgaus, dt*1000);
		}

		//printf("%d\n", mymillis() - startInt);
	}
	return 0;
}

int mymillis(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}
