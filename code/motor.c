#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define LMOTOR 0
#define RMOTOR 1

#define ARDUINO_RST -1 // pin which will reset the Arduino board whenever the code starts

struct motor {
	double displacement, speed;
	double set_speed;
	unsigned long int last_update;
};

struct motor left_motor, right_motor;
int arduino;

void init_motors()
{
	int i = 0;
	char devpath[20];
	do {
		snprintf(devpath, 20, "/dev/ttyUSB%d", i);
		arduino = serialOpen(devpath, 2000000);
	} while(!arduino);

	unsigned long int now = micros();
	left_motor.displacement = 0;
	left_motor.speed = 0;
	left_motor.set_speed = 0;
	left_motor.last_update = now;
	left_motor.displacement = 0;
	left_motor.speed = 0;
	left_motor.set_speed = 0;
	left_motor.last_update = now;
}

void setMotorSpeed(int motor, double speed)
{
	if(motor == LMOTOR)
	{
		left_motor.set_speed = speed;
	} else if (motor == RMOTOR) {
		right_motor.set_speed = speed;
	}
}

#define MOTOR_RCV_MESS_SIZE 35
void read_motors()
{
	unsigned long int now;
	int packet_count;
	char rcv_msg[MOTOR_RCV_MESS_SIZE];
	char disp[10], speed[7];
	int i;

	packet_count = serialDataAvail(arduino);
	if(packet_count == MOTOR_RCV_MESS_SIZE)
	{
		now = micros();
		left_motor.last_update = now;
		right_motor.last_update = now;

		for(i = 0; i < packet_count; ++i)
		{
			rcv_msg[i] = serialGetchar(arduino);
		}
		
		// ldisplacement;lspeed;rdisplacement;rspeed;
		// +0000.000;+0.000;+0000.000;+0.000;
		strncpy(disp, &rcv_msg[0], 9);
		disp[9] = '\0';
		strncpy(speed, &rcv_msg[10], 6);
		speed[6] = '\0';

		left_motor.displacement = strtod(disp, NULL);
		left_motor.speed = strtod(speed, NULL);

		strncpy(disp, &rcv_msg[17], 9);
		disp[9] = '\0';
		strncpy(speed, &rcv_msg[27], 6);
		speed[6] = '\0';

		right_motor.displacement = strtod(disp, NULL);
		right_motor.speed = strtod(speed, NULL);
	} else {
		serialFlush(arduino);
	}
}

#define MOTOR_SND_MESS_SIZE 15
void write_motors()
{
	char snd_msg[MOTOR_SND_MESS_SIZE];

	//lspeed;rspeed;
	//+0.000;+0.000;
	snprintf(snd_msg, MOTOR_SND_MESS_SIZE, "%+6.3f;%+6.3f;", left_motor.set_speed, right_motor.set_speed);
	serialPuts(arduino, snd_msg);
}