#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include "sensors.c"

int main()
{
	printf("\e[2J\e[H");
	printf("You have 60 seconds to spin the robot around as much as you can.\nPreferably in a figure 8.\n");
	wiringPiSetupPhys();
	init_sensors();
	
	int16_t max[3] = {-32000}, min[3] = {32000};

	unsigned long int begin = millis(), last_print = millis();
	while(millis() - begin < 60000)
	{
		update_imu();
		
		if(imu.magnet.rawX > max[0] && imu.magnet.rawX != 0)
		{
			max[0] = imu.magnet.rawX;
		}
		if(imu.magnet.rawY > max[1] && imu.magnet.rawY != 0)
		{
			max[1] = imu.magnet.rawY;
		}
		if(imu.magnet.rawZ > max[2] && imu.magnet.rawZ != 0)
		{
			max[2] = imu.magnet.rawZ;
		}

		if(imu.magnet.rawX < min[0] && imu.magnet.rawX != 0)
		{
			min[0] = imu.magnet.rawX;
		}
		if(imu.magnet.rawY < min[1] && imu.magnet.rawY != 0)
		{
			min[1] = imu.magnet.rawY;
		}
		if(imu.magnet.rawZ < min[2] && imu.magnet.rawZ != 0)
		{
			min[2] = imu.magnet.rawZ;
		}
		if(millis() - last_print > 200)
		{
			last_print = millis();
			printf("\033[%d;%dHMin Values: (%i, %i, %i)\n", 3, 0, min[0], min[1], min[2]);
			printf("\033[%d;%dHMax Values: (%i, %i, %i)\n", 4, 0, max[0], max[1], max[2]);
			printf("\033[%d;%dHCurrent Values: (%i, %i, %i)\n\n", 5, 0, imu.magnet.rawX, imu.magnet.rawY, imu.magnet.rawZ);
		}
	}
	printf("#define MAGX_BIAS %f\n", ((double)max[0]+(double)min[0])/2.0);
	printf("#define MAGY_BIAS %f\n", ((double)max[1]+(double)min[1])/2.0);
	printf("#define MAGZ_BIAS %f\n\n", ((double)max[2]+(double)min[2])/2.0);

	printf("Copy these values into the sensors.c file and recompile main.c to update magnetometer bias.");

	return 0;
}