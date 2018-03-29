/*
 *Teste de controle de velocidade usando
 *o arduino nano. Tentando usar a adaptaço do
 *cdigo da leticia que fizemos para a open 2017
 */

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
void UpdateVel(float refL,float refR);
void setpot(int potL, int potR);
void controle(float refL, float refR);



//funões principais
void setup()
{
  noInterrupts();
  startDriver();
  startEncoder();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  interrupts();
}

#define MSG_SIZE 15

float lref = 0, rref = 0;
int pwmL, pwmR;
int flag = 0;
char msg1[MSG_SIZE], msg2[MSG_SIZE];
unsigned long last_send, last_recv;

int i = 0;

void loop()
{
  if(micros() - last_send > 5000)
  {
    last_send = micros();
    UpdateVel(lref,rref);
    //print_snd_msg();
  }
  // if(micros() - last_recv > 1000000)
  // {
  //   digitalWrite(LED_BUILTIN, HIGH);
  //   reset(); // comment out if using manual input
  // }
}
void serialEvent()
{
  if (Serial.available() > 0)
  {
    last_recv == micros();
    // lref;rref;
    // +0.000;+0.000;
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.readBytesUntil(';', msg1, MSG_SIZE);
    //lref = msg;//.toFloat();
    Serial.readBytesUntil(';', msg2, MSG_SIZE);
    //rref = msg;//.toFloat();
    if(!memcmp(msg2, "*chaVe_35", 9) && !memcmp(msg1, "foda-se", 7))
    {

    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      while(1);
    }
  }
  //limpando o buffer
  while(Serial.available() > 0 )
  {
    char t = Serial.read();
  }
  Serial.print("ok;:");
}

#define KP 46.15
#define KI 1056.69
#define KD 0

float errL = 0, sum_errL = 0, old_errL, derrL;
float errR = 0, sum_errR = 0, old_errR, derrR;
bool lbangbang = LOW, rbangbang = LOW; 

void print_snd_msg()
{
  // ldisplacement;lspeed;rdisplacement;rspeed;
  // +0000.00;+0.00;+0000.00;+0.00;\n
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
  Serial.print('\n');
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
        sum_errR = 0;
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

void reset()
{
  encoder_posL = 0;
  encoder_posR = 0;

  tempo_aux = 0;
  tempo = 0;
  voltas_esquerda = 0;
  voltas_direita = 0;
  ldisplacement = 0;
  rdisplacement = 0;
  velocidade_esquerda = 0;
  velocidade_direita = 0;
  voltas_esquerda_anterior = 0;
  voltas_direita_anterior = 0;
  dt = 0;
}
