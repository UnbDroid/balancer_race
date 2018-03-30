// Apenas copiei os includes de motor.c, talvez nao tenha a necessidade de tudo.
#include <wiringPi.h>
#include <wiringSerial.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

// Declaracao das funcoes.
void setupCommSerial();
void getValidData();
void storeValidData();
void sendDoubleSerial(double send);

// Variaveis globais.
char msg[MSG_MAX];
bool newMsg = false;
double valor = 0.001;
double old_valor = 0.001;
int arduino;

unsigned long long int time = 0, old_time = 0;		// Apenas para testes.

int main()
{
	setupCommSerial();				// Setup comunicacao serial.
	printf("Setup ok...\n");		// Apenas para testes.
	getchar();						// Apenas para testes.

	sendDoubleSerial(valor);		// Como eh o rasp que recebe a ultima confirmacao parecia melhor o rasp comecar o teste.
	
	while (1)
	{
		getValidData();					// Obtem msg sem parar o codigo, importante que se tenha uma boa frequencia de execucao do loop que esta inserido.
		storeValidData();				// Armazena msg caso tenha tido uma msg completa recebida.

		// Apartir daqui feito uma logica apenas para demonstrar
		// a comunicacao funcionando e mostrar envio de msg.
		if (valor != old_valor)			// Obs: o arduino que esta mudando o valor.
		{
			time = micros();
			printf("time: %10lli...   ", time - old_time);

			printf("valor: %.*f...\n", SEND_PRECISION, valor);
			sendDoubleSerial(valor);			// Envia um valor double.
			old_valor = valor;

			old_time = micros();
		}
	}
}



void setupCommSerial()
{
	int ok = 0;

	do
	{
		arduino = serialOpen("/dev/ttyUSB0", BAUDRATE);	// Inicia comunicacao serial com baud rate 115200bps.
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
		valor = atof(msg);							// Transforma double, 0 para entradas invalidos.
		//printf("msg: %s...\n", msg);				// Apenas para testes.
		//printf("valor: %f...\n", velocidade);		// Apenas para testes.
		newMsg = false;
	}
}

void sendDoubleSerial(double send)
{
	char deliver[MSG_MAX];
	int i = 0;

	snprintf(deliver, MSG_MAX, ":%.*f;", SEND_PRECISION, send);		// Prepara a msg a ser enviada.
	while (deliver[i] != '\0')
	{
		serialPutchar (arduino, deliver[i]);		// Envio da msg char por char ate fim da string. Obs: existe na biblioteca a funcao "serialPrintf" mas ela nao funcionou.
		i++;
		if (i >= MSG_MAX)							// Evitar ir alem da string.
		{
			break;
		}
	}
}
