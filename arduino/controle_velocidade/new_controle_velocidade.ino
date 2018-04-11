//driver
#define PWM_L 10
#define INA_L 7
#define INB_L 6

#define PWM_R 9
#define INA_R 5
#define INB_R 4

#define KP 46.15
#define KI 1056.69
#define KD 0

#define RAIO 0.100 // 100mm

//encoder
#define ENCODER_L 1 //interrup port 0, is the pin 2
#define ENCODER_R 0 //interrup port 1, is the pin 3
#define DIR_L 11
#define DIR_R 12

#define LMOTOR 0
#define RMOTOR 1

#define MSG_MAX 30
#define SEND_PRECISION 3
#define BAUDRATE 2000000

// Variaveis globais.
char msg[MSG_MAX];
boolean newMsg = false;

float lref = 0, rref = 0;
float old_lref = 0, old_rref = 0;

//variaveis
volatile long encoder_posL = 0;
volatile long encoder_posR = 0;

unsigned long tempo_aux;
unsigned long tempo;
float voltas_esquerda;
float voltas_direita;
float ldisplacement, rdisplacement;
float velocidade_esquerda;
float velocidade_direita;
float voltas_esquerda_anterior;
float voltas_direita_anterior;
float dt;

float errL = 0, sum_errL = 0, old_errL, derrL;
float errR = 0, sum_errR = 0, old_errR, derrR;
bool lbangbang = LOW, rbangbang = LOW; 

int pwmL, pwmR;

unsigned long last_control;

void setup()
{
	int ok = 0;

	noInterrupts();					// Nao permite interrupcoes.

	startDriver();
	startEncoder();
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
	if(micros() - last_control > 5000 )
	{
		last_control = micros();
		UpdateVel(lref,rref);
		send_msg();
		// Serial.print("lref = ");
		// Serial.print(lref);
		// Serial.print(" rref = ");
		// Serial.println(rref);
	}
}

void serialEvent()	// Obs: ocorre apenas depois de loop() ser executado.
{
	getValidData();			// Obtem msg sem parar o codigo, importante que se tenha uma boa frequencia de execucao de loop().
	storeValidData();		// Armazena msg caso tenha tido uma msg completa recebida.
}

int lenght = 0;

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
	if(newMsg)
	{
		if(msg[lenght] == 'l')
		{
			lref = String(msg).toFloat();
		} else if(msg[lenght] == 'r') {
			rref = String(msg).toFloat();
		}
		//valor = String(msg).toFloat();		// Transforma float, 0 para entradas invalidos.
		//Serial.println(msg);				// Apenas para testes.
		//Serial.println(valor, 5);			// Apenas para testes.
		newMsg = false;
	}
}

void send_msg()
{
	Serial.print(":");
	Serial.print(ldisplacement, SEND_PRECISION);
	Serial.print("dl;:");

	//Serial.print(":");
	Serial.print(velocidade_esquerda, SEND_PRECISION);
	Serial.print("sl;:");
	
	//Serial.print(":");
	Serial.print(rdisplacement, SEND_PRECISION);
	Serial.print("dr;:");
	
	//Serial.print(":");
	Serial.print(velocidade_direita, SEND_PRECISION);
	Serial.print("sr;");
}

void controle(float refL, float refR)
{
  old_errR = errR;
  errR = refR - velocidade_direita;
  derrR = (errR - old_errR)/dt;
  if((errR > 0 && pwmR < 255) || (errR < 0 && pwmR > -255))
  {
  	sum_errR += errR*dt;
  }

  old_errL = errL;
  errL = refL - velocidade_esquerda;
  derrL = (errL - old_errL)/dt;
  sum_errL += errL*dt;
  if((errL > 0 && pwmL < 255) || (errL < 0 && pwmL > -255))
  {
  	sum_errL += errL*dt;
  }

  //controle motor direita
  if(refR >= 0 && refR < 0.2)
  {
    //bang bang
    rbangbang = HIGH;
    if(velocidade_direita < refR)
    {
      pwmR = 50;
    } else {
      pwmR = 0;
    }
  } else if(refR <= 0 && refR > -0.2) {
    rbangbang = HIGH;
    if(velocidade_direita > refR)
    {
      pwmR = -50;
    } else {
      pwmR = 0;
    }
  } else {
      //PID
      if(rbangbang)
      {
        sum_errR = 0;
        rbangbang = LOW;
      }
      pwmR = (int)(KP*errR + KI*sum_errR + KD*derrR);
  }

  //controle motor esquerda
  if(refL >= 0 && refL < 0.2)
  {
    //bang bang
    lbangbang = HIGH;
    if(velocidade_esquerda < refL)
    {
      pwmL = 50;
    } else {
      pwmL = 0;
    }
  } else if(refL <= 0 && refL > -0.2) {
    lbangbang = HIGH;
    if(velocidade_esquerda > refL)
    {
      pwmL = -50;
    } else {
      pwmL = 0;
    }
  } else {
      //PID
      if(lbangbang)
      {
        sum_errL = 0;
        lbangbang = LOW;
      }
      pwmL = (int)(KP*errL + KI*sum_errL + KD*derrL);
  }

  if(lref == 0)
  {
    brake(LMOTOR);
  } else {
    setpot(LMOTOR, pwmL);
  }

  if(rref == 0)
  {
    brake(RMOTOR);
  } else {
    setpot(RMOTOR, pwmR);
  }
}

void brake(int motor)
{
  int a, b, pwm;

  if(motor == LMOTOR)
  {
    a = INA_L;
    b = INB_L;
    pwm = PWM_L;
  } else if (motor == RMOTOR) {
    a = INA_R;
    b = INB_R;
    pwm = PWM_R;
  }

  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  analogWrite(pwm, 0);
}

void setpot(int motor, int pot)
{
  int a, b, pwm;

  if(motor == LMOTOR)
  {
    a = INA_L;
    b = INB_L;
    pwm = PWM_L;
  } else if (motor == RMOTOR) {
    a = INA_R;
    b = INB_R;
    pwm = PWM_R;
  }

  if(pot > 0)
  {
    if(pot > 255)
      pot = 255;

    digitalWrite(a, LOW);
    digitalWrite(b, HIGH);
    analogWrite(pwm, pot);
  }
  else if(pot < 0)
  {
    pot = -pot;
    if(pot > 255)
      pot = 255;

    digitalWrite(a, HIGH);
    digitalWrite(b, LOW);
    analogWrite(pwm, pot);
  }
  else //potL == 0
  {
    digitalWrite(a, LOW);
    digitalWrite(b, LOW);
    analogWrite(pwm, 0);
  }
}

//assinatura das funnções
void UpdateVel(float refL,float refR) {
    // To do:

    // Calcula as velocidades das rodas a cada 5 ms ---------------------------------
    unsigned long agora;
    agora = micros();

    tempo_aux = (agora - tempo);
    tempo = agora;

    dt = ((float)tempo_aux)/1000000.0;

    voltas_esquerda = encoder_posL / (800.0); // 400 pontos por volta só que pegando a subida e a descida
    voltas_direita = encoder_posR / (800.0);

    ldisplacement = voltas_esquerda*2*PI*RAIO;
    rdisplacement = voltas_direita*2*PI*RAIO;

    //velocidade angular em revoluções/segundo
    velocidade_esquerda = 2*PI*RAIO * 1000000 * (voltas_esquerda - voltas_esquerda_anterior) / (tempo_aux);//o *1000 é para corrigir a unidade de tempo
    velocidade_direita  = 2*PI*RAIO * 1000000 * (voltas_direita - voltas_direita_anterior) / (tempo_aux);

    voltas_esquerda_anterior = voltas_esquerda;
    voltas_direita_anterior = voltas_direita;
    // if(DEBUG)
    //   Serial.println(tempo_aux);

    controle(refL,refR);
    //setpot(refL,refR);
    //setpot(0,(int)refR);

    // Chama a funcao que calcula a tensao de saida para os motores -----------------------
    //controleAdaptativoVelocidade();
}

void startDriver() {
  pinMode(INA_L, OUTPUT);
  pinMode(INA_R, OUTPUT);
  pinMode(INB_R, OUTPUT);
  pinMode(INB_L, OUTPUT);
  pinMode(PWM_R, OUTPUT);
  pinMode(PWM_L, OUTPUT);
}

void startEncoder () {
  pinMode(ENCODER_L, INPUT);
  pinMode(ENCODER_R, INPUT);

  attachInterrupt(ENCODER_L, interrupt_L, CHANGE);
  attachInterrupt(ENCODER_R, interrupt_R, CHANGE);
}

void interrupt_L() {
  noInterrupts();
  if(digitalRead(DIR_L))
    encoder_posL++;
  else
    encoder_posL--;
  interrupts();
}

void interrupt_R() {
  noInterrupts();
  if(digitalRead(DIR_R))
    encoder_posR--;
  else
    encoder_posR++;
  interrupts();
}