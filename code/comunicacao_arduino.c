#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>   // using the termios.h library

int set_velocidade(double vel);
double get_velocidade();
char leitura;
unsigned long long int pos_times = 0, now_times = 0, pos_timeg = 0, now_timeg = 0;

int arduino;
int i = 0;
double velocidade = 0.00001;

PI_THREAD(setT)
{
	while (1)
	{
		set_velocidade(velocidade);
	}
}

PI_THREAD(getT)
{
	while (1)
	{
		velocidade = get_velocidade();
		printf("velocidade: %f...\n", velocidade);
	}
}

int main()
{
	//double velocidade;

	do
	{
		arduino = serialOpen("/dev/ttyUSB0", 2000000);
	} while(!arduino || arduino == -1);

	if (wiringPiSetup () == -1)
	{
		printf("setup error");
		return -1;
	}
   	
   	printf("Conectado...\n");

   	//velocidade = 00.001;

   	while(!i)
   	{
   		while(serialDataAvail(arduino))
   		{
   			printf("%c...\n", leitura = serialGetchar(arduino));
   			serialFlush(arduino);
   			if(leitura == 'c')
   			{
   				serialPutchar(arduino, 'b');
   			}
   			else
   			if(leitura == 'a')
   			{
   				i = 1;
   			}
   		}
   	}
   	printf("estabilizou comm...\n");
   	getchar();
   	
   	wiringPiSetupPhys();

   	piThreadCreate(setT);
   	piThreadCreate(getT);

   	while(1)
   	{
   		sleep(100000);
   	}

   	/*
   	while(1)
   	{
   		pos_time = micros();				// start tempo.

	   	set_velocidade(velocidade);			// função que envia valor.

	   	now_time = micros();				// end tempo.
	   	pos_time = now_time - pos_time;
	   	printf("set time: %llu...\n", pos_time);

	   	//printf("Enviou...\n");
	   	velocidade = -1;

	   	pos_time = micros();				// start tempo.

	   	velocidade = get_velocidade();		// função que recebe valor.

	   	now_time = micros();				// end tempo.
	   	pos_time = now_time - pos_time;
	   	printf("get time: %llu...\n", pos_time);

	   	printf("velocidade: %f...\n", velocidade);
	   	getchar();
	   	velocidade += velocidade;
   	}
   	*/
}

int set_velocidade(double vel)
{
	char msg[100], receive[100];
	int count;
	int ok = 0;
	int i_msg = 0;

	/*
	if ((count = write(arduino, ":set", 5)) < 0)
	{
		perror("Failed to write to the output\n");
    	return -1;
	}
	*/
	//serialPrintf(arduino, ":set");

	/*
	serialPutchar (arduino, ':') ;
	serialPutchar (arduino, 's') ;
	serialPutchar (arduino, 'e') ;
	serialPutchar (arduino, 't') ;

	do
	{
		while ((count = serialDataAvail(arduino)) && (!ok))
		{
			/*
			if (count == -1)
			{
				perror("Failed to read the archive...\n");
			}
			
			printf("count dentro while: %d...\n", count);
			getchar();
			*/
			/*
			msg[i_msg] = serialGetchar(arduino);
			//printf("%c\n", msg[i_msg]);
			if (msg[i_msg] == ':')			// Reseta a escrita para o inicio de msg[].
			{
				i_msg = -1;
			}
			if (msg[i_msg] == ';')			// Indica fim de String.
			{
				msg[i_msg] = '\0';
				ok = 1;
			}
			i_msg++;
		}
		ok = 0;

		if(!memcmp(msg, "ok", 2))
		{
			ok = 1;
		}
	} while(!ok);
	ok = 0;
	*/
	snprintf(msg, 15, ":%7f;", vel);
	/*
	if ((count = write(arduino, &msg, 10)) < 0)
	{
		perror("Failed to write to the output\n");
    	return -1;
	}
	*/
	//printf("%s...\n", msg);
	i_msg = 0;
	while (msg[i_msg] != '\0')
	{
		serialPutchar (arduino, msg[i_msg]);
		i_msg++;
	}

	return 0;
}

double get_velocidade()
{
	char msg[100], receive[100];
	int count;
	int ok = 0;
	int i_msg = 0;

	/*
	if ((count = write(arduino, ":get", 5)) < 0)
	{
		perror("Failed to write to the output\n");
    	return -1;
	}
	*/
	/*
	serialPutchar (arduino, ':') ;
	serialPutchar (arduino, 'g') ;
	serialPutchar (arduino, 'e') ;
	serialPutchar (arduino, 't') ;

	*/
	do
	{
		while ((count = serialDataAvail(arduino) && (!ok)))
		{
			msg[i_msg] = serialGetchar(arduino);
			//printf("%c...\n", msg[i_msg]);
			if (msg[i_msg] == ':')			// Reseta a escrita para o inicio de msg[].
			{
				i_msg = -1;
			}
			if (msg[i_msg] == ';')			// Indica fim de String.
			{
				msg[i_msg] = '\0';
				ok = 1;
			}
			i_msg++;
		}
	} while(!ok);
	ok = 0;

	return atof(msg);
}