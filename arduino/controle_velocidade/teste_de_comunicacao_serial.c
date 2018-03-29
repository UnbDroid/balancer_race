#define DEBUG 1//1 - ligado /// 0 - desligado

#define RAIO 0.100 // 100mm

//driver
#define PWM_L 10
#define INA_L 7
#define INB_L 6

#define PWM_R 9
#define INA_R 5
#define INB_R 4

//encoder
#define ENCODER_L 1 //interrup port 0, is the pin 2
#define ENCODER_R 0 //interrup port 1, is the pin 3
#define DIR_L 11
#define DIR_R 12

#define LMOTOR 0
#define RMOTOR 1

void setup()
{
  noInterrupts();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  interrupts();
}

void loop()
{
  // nada.
}


  int i_com = 0, i_msg = 0, opcode;
  char msg[100];
  char comando[100];
  float numero = 0;
  int end = 0;
void serialEvent()
{
	if (Serial.available() > 0)
	{
		/*
		while (Serial.available() != 5) // espera uma quantidade de char chegarem.
		{
		}
		for (i = 0; i < 10; i++)
		{
		 	msg[i] = 'a';
		}
		*/
		//i = 0;
		while (Serial.available())
		{
			comando[i_com] = Serial.read();
			
			/*
			if (comando[i_com] == ';')
			{
				Serial.println("subst comm ;...");
				comando[i_com] = '\0';
			}
			*/
			if (comando[i_com] == ':')
			{
				Serial.println("subst comm :...");
				i_com = -1;
			}

			i_com++;
		}

		
		if (!memcmp(comando, "msg.add", 7))
		{
			opcode = 1;
			i_com = 0;
			comando[i_com] = '\0';
		}
		if (!memcmp(msg, "msg.reset", 9))
		{
			opcode = 2;
			i_com = 0;
			comando[i_com] = '\0';
		}
		if (!memcmp(comando, "msg.show", 8))
		{
			Serial.println("msg: " + String(msg));
			i_com = 0;
			comando[i_com] = '\0';
		}
		/*
		if (!memcmp(comando, "float.add", 9))
		{
			opcode = 3;
		}
		if (!memcmp(msg, "float.reset", 11))
		{
			opcode = 4;
		}
		*/
		if (!memcmp(comando, "float", 5))
		{
			numero = String(msg).toFloat();
			Serial.println("float: " + String(numero));
			i_com = 0;
			comando[i_com] = '\0';
		}
		if (!memcmp(comando, "i", 1))
		{
			Serial.println("no: " + String(i_msg) + "...");
			i_com = 0;
			comando[i_com] = '\0';
		}

		switch (opcode)
		{
			case 1:
				do
				{
					if (Serial.available() > 0)
					{
						while (Serial.available())
						{
							msg[i_msg] = Serial.read();
							if (msg[i_msg] == ';')
							{
								Serial.println("subst msg ;...");
								msg[i_msg] = '\0';
								end = 1;
							}
							i_msg++;
						}
					}
				} while(!end);
				end = 0;
				opcode = 0;
			break;

			case 2:
				i_msg = 0;
				msg[i_msg] = '\0';
				opcode = 0;
			break;

			default:
			break;
		}

		//numero = String(msg).toFloat();
		//Serial.println("msg: " + String(msg));
		//Serial.println("float: " + String(numero));
		//Serial.println("no: " + String(i) + "...");
	}

}