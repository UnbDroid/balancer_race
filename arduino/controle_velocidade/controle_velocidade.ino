/*
 *Teste de controle de velocidade usando
 *o arduino nano. Tentando usar a adaptaço do
 *cdigo da leticia que fizemos para a open 2017
 */

#define DEBUG 1//1 - ligado /// 0 - desligado

#define RAIO 0.100 // 100mm

//driver
#define PWM_L 9
#define INA_L 4
#define INB_L 5

#define PWM_R 10
#define INA_R 6
#define INB_R 7

//encoder
#define ENCODER_L 0 //interrup port 0, is the pin 2
#define ENCODER_R 1 //interrup port 1, is the pin 3
#define DIR_L 12
#define  DIR_R 11


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

//Declaração das funções
void startDriver();
void startEncoder();
void interrupt_L();
void interrupt_R();
int UpdateVel(float refL,float refR);
void setpot(int potL, int potR);
void controle(float refL, float refR);



//funões principais
void setup()
{
  noInterrupts();
  startDriver();
  startEncoder();
  Serial.begin(2000000);
  interrupts();
}

#define MSG_SIZE 15

float lref = 0, rref = 0;
int pwmL, pwmR;
int flag = 0;
char msg[MSG_SIZE];
void loop()
{
	if(Serial.available())
	{
		//lref;rref;
		//+0.000;+0.000;
		Serial.readBytesUntil(';', msg, MSG_SIZE);
		lref = String(msg).toFloat();
		Serial.readBytesUntil(';', msg, MSG_SIZE);
		rref = String(msg).toFloat();
	}
	print_snd_msg();
	UpdateVel(0,rref);
}

#define KP 46.15
#define KI 1056.69
#define KD 0

float errL = 0, sum_errL = 0, old_errL, derrL;
float errR = 0, sum_errR = 0, old_errR, derrR;

void print_snd_msg()
{
	// ldisplacement;lspeed;rdisplacement;rspeed;
	// +0000.000;+0.000;+0000.000;+0.000;
	if(ldisplacement >= 0)
	{
		Serial.print("+" + String(ldisplacement) + ";");
	} else {
		Serial.print(String(ldisplacement) + ";");
	}

	if(velocidade_esquerda >= 0)
	{
		Serial.print("+" + String(velocidade_esquerda) + ";");
	} else {
		Serial.print(String(velocidade_esquerda) + ";");
	}

	if(rdisplacement >= 0)
	{
		Serial.print("+" + String(rdisplacement) + ";");
	} else {
		Serial.print(String(rdisplacement) + ";");
	}

	if(velocidade_direita >= 0)
	{
		Serial.print("+" + String(velocidade_direita) + ";");
	} else {
		Serial.print(String(velocidade_direita) + ";");
	}

	Serial.println();
}

void controle(float refL, float refR)
{
  old_errR = errR;
  errR = refR - velocidade_direita;
  derrR = (errR - old_errR)/dt;
  sum_errR += errR*dt;

  old_errL = errL;
  errL = refL - velocidade_esquerda;
  derrL = (errL - old_errL)/dt;
  sum_errL += errL*dt;

  if(refR < 0.2)
  {
    if(velocidade_direita < refR)
    {
      pwmR = 50;
    } else {
      pwmR = 0;
    }
  } else {
    pwmR = (int)(KP*errR + KI*sum_errR + KD*derrR);
  }

  pwmL = (int)(KP*errL + KI*sum_errL + KD*derrL);

  setpot(pwmL,pwmR);
}

void setpot(int potL, int potR)
{
  if(potL > 0)
  {
    if(potL > 255)
      potL = 255;

    digitalWrite(INA_L, LOW);
    digitalWrite(INB_L, HIGH);
    analogWrite(PWM_L, potL);
  }
  else if(potL < 0)
  {
    potL = -potL;
    if(potL > 255)
      potL = 255;

    digitalWrite(INA_L, HIGH);
    digitalWrite(INB_L, LOW);
    analogWrite(PWM_L, potL);
  }
  else //potL == 0
  {
    digitalWrite(INA_L, LOW);
    digitalWrite(INB_L, LOW);
    analogWrite(PWM_L, 0);
  }

  if(potR > 0)
  {
    if(potR > 255)
      potR = 255;

    digitalWrite(INA_R, LOW);
    digitalWrite(INB_R, HIGH);
    analogWrite(PWM_R, potR);
  }
  else if(potR < 0)
  {
    potR = -potR;
    if(potR > 255)
      potR = 255;

    digitalWrite(INA_R, HIGH);
    digitalWrite(INB_R, LOW);
    analogWrite(PWM_R, potR);
  }
  else //potL == 0
  {
    digitalWrite(INA_R, LOW);
    digitalWrite(INB_R, LOW);
    analogWrite(PWM_R, 0);
  }

}

//assinatura das funnções
int UpdateVel(float refL,float refR) {
   // To do:

   // Calcula as velocidades das rodas a cada 5 ms ---------------------------------
   unsigned long agora;
   agora = micros();
   if (agora - tempo >= 5000)
   {
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
      return 1;
  }
  // Chama a funcao que calcula a tensao de saida para os motores -----------------------
  //controleAdaptativoVelocidade();
  return 0;
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
