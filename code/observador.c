#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"

struct joystick js;
int keep_running = 1;
int led_finished = 1, joystick_finished = 1, observador_finished = 1;
int shutdown = 0, reboot = 0, fechar=0;

PI_THREAD(joystick)
{
    joystick_finished = 0;
	piHiPri(0);
    init_joystick(&js, devname);
    while(!(js.select && js.start))
    {
        if(js.disconnect)
        	init_joystick(&js, devname);
        update_joystick(&js);
	}
	if(js.dpad.down) shutdown = 1;
	if(js.dpad.up) reboot = 1;
	if(js.dpad.left) fechar = 1;
	keep_running = 0;
	joystick_finished = 1;
}

PI_THREAD(led)
{
	led_finished = 0;
	piHiPri(0);
	init_led();
	while(keep_running)
	{
		light_color(led_color);
		delay(LED_DELAY);
	}
	led_finished = 1;
}

PI_THREAD(observador)
{
	observador_finished = 0;
	piHiPri(0);
	while(keep_running)
	{
		set_color(RED);
		delay(1000);
		set_color(YELLOW);
		delay(1000);
	}
	observador_finished = 1;
}

int main()
{
	char resposta;
    do
    {
    	printf("Are you su?(y/n)\n");
   		scanf("%c", &resposta);
    } while(resposta != 'y' && resposta != 'n');
    if(resposta!='y')
    	return 0;

    wiringPiSetupPhys();
	piThreadCreate(joystick);
	piThreadCreate(led);
	piThreadCreate(observador);
	
	while(keep_running) delay(100);
	light_color(RED);
	while(!(joystick_finished && led_finished));
	light_color(WHITE);

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!fechar) system("sudo /home/pi/ccdir/main >> /home/pi/log/mainlog.txt < /home/pi/log/input.txt");
	return 0;
}