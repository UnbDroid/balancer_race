#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "sensors.c"

#define SEC 1000000

int am_i_su()
{
    if(geteuid())
    	return 0;
    return 1;
}

int main()
{
	if(!am_i_su())
	{
		printf("Restricted area. Super users only.\n");
		return 0;
	}

	int i, j;
	FILE* fp;
	FILE* fptxt;
	double triad_n[3][3] = {0};
	int n_measurements = 0;

    wiringPiSetupPhys();
    init_sensors();

    unsigned long int begin = micros();
    while(micros() - begin < 10*SEC && n_measurements < 1000)
    {
    	update_imu();
    	if(imu.update)
    	{
    		triad_n[0][0] += i_b[0];
    		triad_n[0][1] += i_b[1];
    		triad_n[0][2] += i_b[2];
    		triad_n[1][0] += j_b[0];
    		triad_n[1][1] += j_b[1];
    		triad_n[1][2] += j_b[2];
    		triad_n[2][0] += k_b[0];
    		triad_n[2][1] += k_b[1];
    		triad_n[2][2] += k_b[2];
    		++n_measurements;
    	}
    }
    triad_n[0][0] /= n_measurements;
	triad_n[0][1] /= n_measurements;
	triad_n[0][2] /= n_measurements;
	triad_n[1][0] /= n_measurements;
	triad_n[1][1] /= n_measurements;
	triad_n[1][2] /= n_measurements;
	triad_n[2][0] /= n_measurements;
	triad_n[2][1] /= n_measurements;
	triad_n[2][2] /= n_measurements;
	
	fp = fopen("triad.calib", "w");
	fptxt = fopen("triad.calib.txt", "w");

	for(i = 0; i < 3; ++i)
	{
		for(j = 0; j < 3; ++j)
		{
			fwrite(&triad_n[i][j], sizeof(triad_n[i][j]), 1, fp);
		}
		fprintf(fptxt, "i_n[%d] = %+010.7lf\n", i, triad_n[0][i]);
		fprintf(fptxt, "j_n[%d] = %+010.7lf\n", i, triad_n[1][i]);
		fprintf(fptxt, "k_n[%d] = %+010.7lf\n", i, triad_n[2][i]);
	}

	fclose(fp);

	return 0;
}