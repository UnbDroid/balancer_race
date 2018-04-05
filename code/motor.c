#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>   // using the termios.h library
#include <stdbool.h>

#define LMOTOR 0
#define RMOTOR 1

#define ARDUINO_RST -1 // pin which will reset the Arduino board whenever the code starts

#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

#define MEDIAN_SIZE 5;

// Declaracao das funcoes.
void setupCommSerial();
void getValidData();
void storeValidData();

// Variaveis globais.
char msg[MSG_MAX];
bool newMsg = false;
int arduino;

struct motor {
	double displacement, speed;
	double set_speed;
	unsigned long int last_update;
	double vec_speed[MEDIAN_SIZE], vec_disp[MEDIAN_SIZE];
	unsigned long int n_disp, n_speed;
};

struct motor left_motor, right_motor;

void init_motors()
{
	int i = 0;
	char devpath[20];
	
	setupCommSerial();				// Setup comunicacao serial.

	unsigned long int now = micros();
	left_motor.displacement = 0;
	left_motor.speed = 0;
	left_motor.set_speed = 0;
	left_motor.last_update = now;
	left_motor.displacement = 0;
	left_motor.speed = 0;
	left_motor.set_speed = 0;
	left_motor.last_update = now;
	left_motor.n_speed = 0;
	left_motor.n_disp = 0;

	right_motor.displacement = 0;
	rightmotor.speed = 0;
	rightmotor.set_speed = 0;
	rightmotor.last_update = now;
	rightmotor.displacement = 0;
	rightmotor.speed = 0;
	rightmotor.set_speed = 0;
	right_motor.last_update = now;
	left_motor.n_speed = 0;
	left_motor.n_disp = 0;
}

void setupCommSerial()
{
	int ok = 0;

	do
	{
		arduino = serialOpen("/dev/ttyUSB0", BAUDRATE);	// Inicia comunicacao serial com baud rate 115200bps.
		//printf("no setup\n");
	} while (arduino <= 0);

	//while (wiringPiSetup() == -1);		// Necessita de sudo.

	do										// Espera a estabilizacao da comunicacao serial.
   	{
   		while(serialDataAvail(arduino))
   		{
   			msg[0] = serialGetchar(arduino);
   			//printf("%c...\n",   msg[0]);			// Apenas para testes.
   			if(msg[0] == 'c')						// Msg de nao recebimento.
   			{
   				serialPutchar(arduino, 'b');		// Reenvia msg de confirmacao de recebimento.
   			}
   			else
   			if(msg[0] == 'a')						// Msg esperada receber, envio e recibo de msg funcionando.
   			{
   				ok = 1;
   			}
   		}
   	} while (!ok);
}

static int lenght = 0;
void getValidData()
{
	static int msgp = 0;		// pontero da msg.
	char startChar = ':';		// char define inicio da msg.
	char endChar = ';';			// char define fim da msg.

	while((serialDataAvail(arduino)) && (!newMsg))
	{
		msg[msgp] = serialGetchar(arduino);
		//printf("%c\n", msg[msgp]);			// Apenas para testes.
		if (msg[msgp] == startChar)			// Ageita para comecar a colocar a msg no inicio de msg[].
		{
			msgp = -1;
		}
		else
		if (msg[msgp] == endChar)			// Finaliza a obtencao da msg encerrando a string msg[].
		{
			msg[msgp] = '\0';
			lenght = msgp - 1;
			newMsg = true;						// Sinaliza uma nova msg recebida.
		}
		
		msgp++;
		if (msgp >= MSG_MAX)				// Evitar a escrita em memoria inacessivel.
		{
			msgp = MSG_MAX - 1;
		}
	}
}

void storeValidData()	// Aqui que sera mudado para nossas necessidades. No caso espera receber apenas um float.
{
	if (newMsg)
	{
		//printf("msg: %s...\n", msg);
		if(msg[lenght] == 'l')
		{
			if(msg[lenght-1] == 'd')
			{
				left_motor.displacement = atof(msg);
				left_motor.n_disp++;
			} else if(msg[lenght-1] == 's'){
				left_motor.speed = atof(msg);
				left_motor.n_speed++;
			}
		} else if(msg[lenght] == 'r') {
			if(msg[lenght-1] == 'd')
			{
				right_motor.displacement = atof(msg);
				right_motor.n_disp++;
			} else if(msg[lenght-1] == 's'){
				right_motor.speed = atof(msg);
				right_motor.n_speed++;
			}
		}
		//valor = atof(msg);						// Transforma double, 0 para entradas invalidos.
		//printf("msg: %s...\n", msg);				// Apenas para testes.
		//printf("valor: %f...\n", velocidade);		// Apenas para testes.
		newMsg = false;
	}
}

void write_motors()
{
	char deliver[MSG_MAX];
	int i = 0;

	snprintf(deliver, MSG_MAX, ":%.*fl;", SEND_PRECISION, left_motor.set_speed);		// Prepara a msg a ser enviada.
	while (deliver[i] != '\0')
	{
		serialPutchar (arduino, deliver[i]);		// Envio da msg char por char ate fim da string. Obs: existe na biblioteca a funcao "serialPrintf" mas ela nao funcionou.
		//printf("%c", deliver[i]);
		i++;
		if (i >= MSG_MAX)							// Evitar ir alem da string.
		{
			break;
		}
	}
	//printf("\n");
	i = 0;
	snprintf(deliver, MSG_MAX, ":%.*fr;", SEND_PRECISION, right_motor.set_speed);		// Prepara a msg a ser enviada.
	while (deliver[i] != '\0')
	{
		serialPutchar (arduino, deliver[i]);		// Envio da msg char por char ate fim da string. Obs: existe na biblioteca a funcao "serialPrintf" mas ela nao funcionou.
		//printf("%c", deliver[i]);
		i++;
		if (i >= MSG_MAX)							// Evitar ir alem da string.
		{
			break;
		}
	}
	//printf("\n");
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
