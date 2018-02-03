#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include "sensors.c"

int main()
{
	printf("You have 60 seconds to spin the robot around as much as you can. Preferably in a figure 8.\n");
	wiringPiSetupPhys();
	init_sensors();
	
	int16_t max[3] = {-32000}, min[3] = {32000};

	unsigned long int begin = millis();
	while(millis() - begin < 60000)
	{
		update_imu();
		
		if(imu.magnet.rawX > max[0])
		{
			max[0] = imu.magnet.rawX;
		}
		if(imu.magnet.rawY > max[1])
		{
			max[1] = imu.magnet.rawY;
		}
		if(imu.magnet.rawZ > max[2])
		{
			max[2] = imu.magnet.rawZ;
		}

		if(imu.magnet.rawX < min[0])
		{
			min[0] = imu.magnet.rawX;
		}
		if(imu.magnet.rawY < min[1])
		{
			min[1] = imu.magnet.rawY;
		}
		if(imu.magnet.rawZ < min[2])
		{
			min[2] = imu.magnet.rawZ;
		}
	}
	printf("Min Values: (%i, %i, %i)\n", min[0], min[1], min[2]);
	printf("Max Values: (%i, %i, %i)\n", max[0], max[1], max[2]);

	return 0;
}