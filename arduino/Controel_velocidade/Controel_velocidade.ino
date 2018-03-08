/*
 *Teste de controle de velocidade usando
 *o arduino nano. Tentando usar a adaptaço do
 *cdigo da leticia que fizemos para a open 2017
 */

#define DEBUG 1//1 - ligado /// 0 - desligado

#define PI 3.14159265359
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
        if(DEBUG)
         Serial.begin(9600);
        
        interrupts();
}

float rref = 0, ref = 3;

int flag = 0;
unsigned long int tempo_loop = 0;
void loop()
{
  if(millis() - tempo_loop > 1000)
  {
    tempo_loop = millis();
    if(rref == ref)
      rref = -ref ;
    else
      rref = ref ;
  }
  
 
    UpdateVel(0,rref);

  if(DEBUG)
  {  
     Serial.print(rref);
     Serial.print("\t");
     Serial.print(velocidade_direita);
     Serial.print("\t");
     Serial.println(0);
  }
  //delay(1);
}

#define KP 60
#define KI 200
#define KD 700

float errL = 0, sum_errL = 0, old_errL, derrL;
float errR = 0, sum_errR = 0, old_errR, derrR; 

int pwmL, pwmR;

void controle(float refL, float refR)
{
  old_errL = errL;
  old_errR = errR;
  errL = refL - velocidade_esquerda;
  errR = refR - velocidade_direita;

  derrR = (errR - old_errR)*dt;
  derrL = (errL - old_errL)*dt;

  sum_errR += errR*dt;
  sum_errL += errL*dt;

  pwmR = (int)(KP*errR + KI*sum_errR + KD*derrR);
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
    digitalWrite(INA_L, HIGH);
    digitalWrite(INB_L, HIGH);
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
    digitalWrite(INA_R, HIGH);
    digitalWrite(INB_R, HIGH);
    analogWrite(PWM_R, 0);
  }

}

//assinatura das funnções
void UpdateVel(float refL,float refR) {
   // Calcula as velocidades das rodas a cada 20 ms ---------------------------------
   if (millis() - tempo > 20) 
   {      
      tempo_aux = (millis() - tempo);
      tempo = millis();

      dt = ((float)tempo_aux)/1000.0;

      voltas_esquerda = encoder_posL / (800.0); // 400 pontos por volta só que pegando a subida e a descida
      voltas_direita = encoder_posR / (800.0); 

      //velocidade angular em revoluções/segundo
      velocidade_esquerda = 2*PI*RAIO * 1000 * (voltas_esquerda - voltas_esquerda_anterior) / (tempo_aux);//o *1000 é para corrigir a unidade de tempo
      velocidade_direita  = 2*PI*RAIO * 1000 * (voltas_direita - voltas_direita_anterior) / (tempo_aux);
      
      voltas_esquerda_anterior = voltas_esquerda;
      voltas_direita_anterior = voltas_direita;
      // if(DEBUG)
      //   Serial.println(tempo_aux);
      controle(refL,refR);
      //setpot(0,(int)refR);
  }
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
    encoder_posL--;
  else
    encoder_posL++;
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
