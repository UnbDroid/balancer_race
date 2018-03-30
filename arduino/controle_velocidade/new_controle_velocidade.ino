
#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

// Variaveis globais.
char msg[MSG_MAX];
boolean newMsg = false;
float valor;
float old_valor = 0;

void setup()
{
	int ok = 0;

	noInterrupts();					// Nao permite interrupcoes.

	pinMode(LED_BUILTIN, OUTPUT);		// LED imbutido no arduino. 
	Serial.begin(BAUDRATE);				// Inicia comunicacao serial com baud rate 115200bps.

	interrupts();					// Volta a permitir interrupcoes.

	while(!ok)						// Espera a estabilizacao da comunicacao serial.
	{
		if (Serial.available() > 0)
		{
			msg[0] = Serial.read();
			if(msg[0] == 'b')			// Msg esperada receber, envio e recibo de msg funcionando.
			{
				Serial.print("a");		// Resposta de que obteve comunicacao, 
				ok = 1;
			}
		}
		else
		{
			Serial.print("c");			// Msg de persistencia na espera.
		}
		delay(100);					// Simplesmente para nao enviar muitas msgs desnecessarias.
	}
}

void loop()
{
	if (valor != old_valor)
	{
		old_valor = valor;
		Serial.print(":");
		Serial.print(valor + 0.001, SEND_PRECISION);
		Serial.print(";");
	}
}

void serialEvent()	// Obs: ocorre apenas depois de loop() ser executado.
{
	getValidData();			// Obtem msg sem parar o codigo, importante que se tenha uma boa frequencia de execucao de loop().
	storeValidData();		// Armazena msg caso tenha tido uma msg completa recebida.
}



void getValidData()
{
	static int msgp = 0;		// pontero da msg.
	char startChar = ':';		// char define inicio da msg.
	char endChar = ';';			// char define fim da msg.

	while((Serial.available()) && (!newMsg))
	{
		msg[msgp] = Serial.read();
		//Serial.print(msg[msgp]);			// Apenas para testes.
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
		valor = String(msg).toFloat();		// Transforma float, 0 para entradas invalidos.
		//Serial.println(msg);				// Apenas para testes.
		//Serial.println(valor, 5);			// Apenas para testes.
		newMsg = false;
	}
}
