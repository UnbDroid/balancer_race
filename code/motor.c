#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>   // using the termios.h library

#define LMOTOR 0
#define RMOTOR 1

#define ARDUINO_RST -1 // pin which will reset the Arduino board whenever the code starts

#define MOTOR_RCV_MESS_SIZE 20
#define MOTOR_SND_MESS_SIZE 15

#define MAX_MSG 5

struct motor {
	double displacement, speed;
	double set_speed;
	unsigned long int last_update;
};

char motor_sent_message[MOTOR_SND_MESS_SIZE], motor_received_message[MAX_MSG][MOTOR_RCV_MESS_SIZE];
struct motor left_motor, right_motor;
int arduino;

void init_motors()
{
	int i = 0;
	char devpath[20];
	do {
		snprintf(devpath, 20, "/dev/ttyUSB%d", i%10);
		arduino = open(devpath, O_RDWR | O_NOCTTY | O_NDELAY);
		++i;
	} while(!arduino);

	struct termios options;       // the termios structure is vital
	tcgetattr(arduino, &options);    // sets the parameters for the file

   	// Set up the communications options:
   	// 115200 baud, 8-bit, enable receiver, no modem control lines
   	options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   	options.c_iflag = IGNPAR | ICRNL;   // ignore partity errors
   	tcflush(arduino, TCIFLUSH);            // discard file information
   	tcsetattr(arduino, TCSANOW, &options); // changes occur immmediately

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

// Envia uma string para o arduino.
// Argumentos:	transmit = menssagem a ser enviada.
// Retorno: 	count = .
// Atualmente envia até 100 char.
int send_to_arduino(char* transmit)
{
	int count;
	if ((count = write(arduino, transmit, 30)) < 0)
	{
		perror("Failed to write to the output\n");
    	return -1;
	}
	return count;
}

// Recebe uma menssagem do arduino até um caractere especial.
// Argumentos:	message = local de armazenamento da mensagem.
//						  obs: conteúdo de message será alterado!
// Retorno:		total_count = número de caracteres total recebido.
// Caractere especial fim segmento mensagem = ';'.
// Caractere especial fim mensagem = ':'.
int receive_from_arduino(char message[MAX_MSG][MOTOR_RCV_MESS_SIZE])
{
	char receive[100] = "";
	int end = 0;
	int total_count = 0;
	int count;
	int c; 
	int d = 0;
	int e = 0;

	for (c = 0; c < MAX_MSG; c++)
	{
		for (d = 0; d < MOTOR_RCV_MESS_SIZE; d++)
		{
			message[c][d] = '\0';
		}
	}

	d = 0;

	do
	{
		if ((count = read(arduino, (void*)receive, 30)) < 0)	// ERRO.
		{
			//perror("Erro: Failed to read from the input lol\n");
        	//return -1;
		} 
		else if (count == 0)	// RECEBEU NADA.
		{
			delayMicroseconds(100);
		} 
		else
		{
			for (c = 0; c < count; c++)
			{
				if (receive[c] == ';')
				{
					message[d][e] = '\0';
					d++;
					e = 0;
				} 
				else if (receive[c] == '\n')
				{
					end = 1;
					break;
				}
				else
				{
					message[d][e] = receive[c];
					e++;
					total_count++;
				}
			}
		}
	} while (!end);

	return total_count;
}


void read_motors()
{
	unsigned long int now;
	int packet_count;
	char disp[10], speed[7];
	int i;

	receive_from_arduino(motor_received_message);

	// ldisplacement;lspeed;rdisplacement;rspeed;
	// +0000.00;+0.00;+0000.00;+0.00;\n
	left_motor.displacement = strtod(motor_received_message[0], NULL);
	left_motor.speed = strtod(motor_received_message[1], NULL);

	right_motor.displacement = strtod(motor_received_message[2], NULL);
	right_motor.speed = strtod(motor_received_message[3], NULL);
}

void write_motors()
{
	//lspeed;rspeed;
	//+0.000;+0.000;
	snprintf(motor_sent_message, MOTOR_SND_MESS_SIZE, "%+6.3f;%+6.3f;", left_motor.set_speed, right_motor.set_speed);
	send_to_arduino(motor_sent_message);
}