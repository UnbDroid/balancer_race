//arduino code
/*
int ledPin = 13;   // LED that flashes when a key is pressed

void setup() {                      // called once on start up
   // A baud rate of 115200 (8-bit with No parity and 1 stop bit)
   Serial.begin(115200, SERIAL_8N1);
   pinMode(ledPin, OUTPUT);         // the LED is an output
}

void loop() {                       // Loops forever!
   byte charIn;
   digitalWrite(ledPin, LOW);       // set the LED to be off
   if(Serial.available()){          // a byte has been received
      charIn = Serial.read();       // read the character in from the RPi
      Serial.write(charIn);         // send the character back to the RPi
      digitalWrite(ledPin, HIGH);   // light the LED
      delay(100);                   // delay so the LED is visible
   }
}
*/

//C code for Rasp

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>   // using the termios.h library

int main(){
   int file, count;
   if ((file = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY))<0){
   //if ((file = open("/dev/ttyUSB0", O_RDWR))<0){ //teste que deu errado também 
      perror("UART: Failed to open the file.\n");
      return -1;
   }
   struct termios options;       // the termios structure is vital
   tcgetattr(file, &options);    // sets the parameters for the file

   // Set up the communications options:
   // 115200 baud, 8-bit, enable receiver, no modem control lines
   options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;   // ignore partity errors
   tcflush(file, TCIFLUSH);            // discard file information
   tcsetattr(file, TCSANOW, &options); // changes occur immmediately
   unsigned char transmit[25] = "23.5;to te testando";  // send string
   if ((count = write(file, &transmit, 20))<0){         // transmit
      perror("Failed to write to the output\n");
      return -1;
   }
   //usleep(3000000);             // give the Arduino a chance to respond
   unsigned char receive[100] = "abc"; //declare a buffer for receiving data
   unsigned char message[100] = "abc";
   int end_message = 1;
   do
   {
      if ((count = read(file, (void*)receive, 20)) < 0){   //receive data
         //perror("Erro: Failed to read from the input lol\n");
         //return -1;
      } 
      else if (count == 0)
      {
         //usleep(500);
      } 
      else 
      {
         int d = 0;
         int c;
         for (c = 0; c < count; c++)
         {
            if (receive[c] == ';')
            {
               end_message = 0;
               break;
            } else
            {
               message[d] = receive[c];
               d++;
            }
         }
      }
   }while (end_message);
   if (count==0) printf("There was no data available to read!\n");
   else printf("The following was read in [%d]: %s\n",count,message);
   close(file);
   return 0;
}