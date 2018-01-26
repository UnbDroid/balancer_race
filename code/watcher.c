#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"

struct joystick js;
int keep_running = 1;
int led_finished = 1, joystick_finished = 1, watch_finished = 1;
int shutdown = 0, reboot = 0, close_program=0;

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
	if(js.dpad.left) close_program = 1;
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
		update_led(led_color);
		delay(LED_DELAY);
	}
	led_finished = 1;
}

PI_THREAD(watch)
{
	watch_finished = 0;
	piHiPri(0);
	while(keep_running)
	{
		set_color(RED);
		delay(1000);
		set_color(YELLOW);
		delay(1000);
	}
	watch_finished = 1;
}

int main()
{
	char ans;
    do
    {
    	printf("Are you su?(y/n)\n");
   		scanf("%c", &ans);
    } while(ans != 'y' && ans != 'n');
    if(ans!='y')
    	return 0;

    wiringPiSetupPhys();
	piThreadCreate(joystick);
	piThreadCreate(led);
	piThreadCreate(watch);
	
	while(keep_running) delay(100);
	set_color(RED);
	update_led();
	while(!(joystick_finished && led_finished && watch_finished));
	set_color(WHITE);
	update_led();

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) system("sudo /home/pi/ccdir/main >> /home/pi/log/mainlog.txt < /home/pi/log/input.txt");
	return 0;
}