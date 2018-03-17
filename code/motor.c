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

#define MOTOR_RCV_MESS_SIZE 31
#define MOTOR_SND_MESS_SIZE 15

struct motor {
	double displacement, speed;
	double set_speed;
	unsigned long int last_update;
};

char motor_sent_message[MOTOR_SND_MESS_SIZE], motor_received_message[MOTOR_RCV_MESS_SIZE];
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


void read_motors()
{
	unsigned long int now;
	int packet_count;
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
			motor_received_message[i] = serialGetchar(arduino);
		}

		// ldisplacement;lspeed;rdisplacement;rspeed;
		// +0000.00;+0.00;+0000.00;+0.00;
		strncpy(disp, &motor_received_message[0], 8);
		disp[9] = '\0';
		strncpy(speed, &motor_received_message[9], 5);
		speed[5] = '\0';

		left_motor.displacement = strtod(disp, NULL);
		left_motor.speed = strtod(speed, NULL);

		strncpy(disp, &motor_received_message[15], 8);
		disp[9] = '\0';
		strncpy(speed, &motor_received_message[24], 5);
		speed[5] = '\0';

		right_motor.displacement = strtod(disp, NULL);
		right_motor.speed = strtod(speed, NULL);
	} else {
		serialFlush(arduino);
	}
}

void write_motors()
{
	//lspeed;rspeed;
	//+0.000;+1.000;
	snprintf(motor_sent_message, MOTOR_SND_MESS_SIZE, "%+6.3f;%+6.3f;", left_motor.set_speed, right_motor.set_speed);
	serialPuts(arduino, motor_sent_message);
}