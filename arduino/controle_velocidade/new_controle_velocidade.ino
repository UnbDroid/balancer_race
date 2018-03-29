

void setup()
{
  noInterrupts();         // Nao permite interrupcoes.

  pinMode(LED_BUILTIN, OUTPUT); // LED imbutido no arduino. 
  Serial.begin(2000000);     // Inicia comunicacao serial com baud rate 115200bps.
  
  interrupts();         // Volta a permitir interrupcoes.
}

char msg[100];
int i = 0;
void loop()
{
  // Por enquanto nada.
  while(!i)
  {
    if (Serial.available() > 0)
    {
      
      msg[0] = Serial.read();
      if(msg[0] == 'b')
      {
        Serial.print("a");
        i = 1;
      }
    }
    else 
    {
      Serial.print("c");
    }
    delay(1000);
  }
}

// Variaveis usadas em serialEvent().

int i_msg = 0, flag = 0;
float velocidade = 0;
void serialEvent()
{
  if (Serial.available() > 0)
  {
    // Obtem msg[]. Obs: pode levar mais do que um if para receber tudo.
    do
    {
      msg[i_msg] = Serial.read();
      if (msg[i_msg] == ':')      // Reseta a escrita para o inicio de msg[].
      {
        i_msg = -1;
      }
      i_msg++;
    } while(Serial.available());

    // Checa por comandos. Obs: o que importa sao os primeiros caracteres de msg[].
    if(!memcmp(msg, "set", 3))      // Seta o valor de velocidade desejado.
    {
      
      // Apaga o comando de msg[] para evitar repetir comando.
      msg[0] = 'a';
      msg[1] = 'a';
      msg[2] = 'a';

      while(Serial.available() > 0)
      {
        msg[0] = Serial.read();
      }

      // Sinaliza para o Raspberry que recebeu comando e esta pronto.
      Serial.print(":ok;");

      // Obtem msg[].
      i_msg = 0;
      do{
        while (Serial.available())
        {
          msg[i_msg] = Serial.read();
          if (msg[i_msg] == ':')      // Reseta a escrita para o inicio de msg[].
          {
            i_msg = -1;
          }
          if (msg[i_msg] == ';')      // Indica fim de String.
          {
            msg[i_msg] = '\0';
            flag = 1;
          }
          i_msg++;
        }
      } while(!flag);
      flag = 0;

      velocidade = String(msg).toFloat();   // Obs: Caso msg[] seja invalido o valor de velocidade sera 0.
      digitalWrite(LED_BUILTIN, HIGH);
      
      /* IDE arduino.
      Serial.print("Valor recebido: ");
      Serial.print(velocidade, 5);
      Serial.println("...");
      */
    }

    if(!memcmp(msg, "get", 3))      // Fornece valores encoder.
    {
      // Apaga o comando de msg[] para evitar repetir comando.
      msg[0] = 'a';
      msg[1] = 'a';
      msg[2] = 'a';

      Serial.print(":");
      Serial.print(velocidade, 5);
      Serial.print(";");
      /* IDE arduino.
      Serial.print("Velocidade: ");
      Serial.print(velocidade, 5);
      Serial.println("...");
      */
    }
  }
}