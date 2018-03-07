/*
 *Teste de controle de velocidade usando
 *o arduino nano. Tentando usar a adaptaço do
 *cdigo da leticia que fizemos para a open 2017
 */

//driver
#define PWM_L 9
#define INA_L 4
#define INB_L 5

#define PWM_R 10
#define INA_R 6
#define INB_R 7

//encoder
#define encoderL 2
#define encoderR 3
#define dirL 11
#define	dirR 12


volatile long encoder_posL = 0; 
volatile long encoder_posR = 0;

void startDriver() {
  pinMode(INA_L, OUTPUT);
  pinMode(INA_R, OUTPUT);
  pinMode(INB_R, OUTPUT);
  pinMode(INB_L, OUTPUT);
  pinMode(PWM_R, OUTPUT);
  pinMode(PWM_L, OUTPUT);
}

void startEncoder () {
  pinMode(encoderL, INPUT);
  pinMode(encoderR, INPUT);

  attachInterrupt(0, interrupt_L, CHANGE);
  attachInterrupt(1, interrupt_R, CHANGE);
}


void interrupt_L() {
  noInterrupts();
  if(digitalRead(dirL))
  	encoder_posL++;
  else
  	encoder_posL--;
  interrupts();
}

void interrupt_R() {
  noInterrupts();
  if(digitalRead(dirR))
  	encoder_posR++;
  else
  	encoder_posR--;
  interrupts();
}

void setup()
{
      	noInterrupts();
      	
      	startDriver();
      	startEncoder();
      	Serial.begin(9600);
      	
      	interrupts();
}

void loop()
{
    digitalWrite(INA_L, HIGH);
    digitalWrite(INB_L, LOW);
    digitalWrite(PWM_L, 100);
    digitalWrite(INA_R, HIGH);
    digitalWrite(INB_R, LOW);
    digitalWrite(PWM_R, 100);
    
    Serial.print(encoder_posL);
    Serial.print("\t");
    Serial.println(encoder_posR);
    
    delay(100);
 
}
