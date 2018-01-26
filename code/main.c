#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"
#include "motor.c"

struct joystick js;
int keep_running = 1;
int main_finished = 1, led_finished = 1, joystick_finished = 1, debug_finished = 1;
int shutdown = 0, reboot = 0, close_program=0;;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);
	while(keep_running)
	{
		if(js.B)
			set_color(RED);
		else if(js.A)
			set_color(GREEN);
		else if(js.X)
			set_color(BLUE);
		else if(js.Y)
			set_color(YELLOW);
		else if(js.start)
			set_color(WHITE);
		else
			set_color(OFF_COLOR);
		
		if(js.lanalog.up > 0)
			OnFwd(LMOTOR, js.lanalog.up);
		else if(js.lanalog.down > 0)
			OnRev(LMOTOR, js.lanalog.down);
		else
			Coast(LMOTOR);

		if(js.ranalog.up > 0)
			OnFwd(RMOTOR, js.ranalog.up);
		else if(js.ranalog.down > 0)
			OnRev(RMOTOR, js.ranalog.down);
		else
			Coast(RMOTOR);

		delay(100);
	}
	Coast(LMOTOR);
	Coast(RMOTOR);
	main_finished = 1;
}

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
		update_led();
		delay(LED_DELAY);
	}
	led_finished = 1;
}

PI_THREAD(debug)
{
	debug_finished = 0;
	if(0)
	{
		piHiPri(0);
		while(keep_running)
		{
				
		}
	}
	debug_finished = 1;
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

    piThreadCreate(debug);

    wiringPiSetupPhys();
	init_motors();
	piThreadCreate(main_thread);
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	
	while(keep_running) delay(100);
	set_color(RED);
	update_led();
	while(!(main_finished && joystick_finished && led_finished && debug_finished));
	set_color(WHITE);
	update_led();

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) system("sudo /home/pi/ccdir/watcher >> /home/pi/log/watcherlog.txt < /home/pi/log/input.txt");

	return 0;
}