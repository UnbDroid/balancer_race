#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include "jstick.c"
#include "led.c"
#include "motor.c"
//#include "debug.c"

//struct debug_data debug;

int keep_running = 1;
int main_finished = 1, led_finished = 1, joystick_finished = 1, debug_finished = 1;
int shutdown = 0, reboot = 0, close_program=0;;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);
	while(keep_running)
	{		
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
    
    set_led_state(BLUETOOTH, ON);
    init_joystick(&js, devname);
    set_led_state(BLUETOOTH, OFF);

    while(!(js.select && js.start))
    {
        if(js.disconnect)
        {
        	set_led_state(BLUETOOTH, ON);
		    init_joystick(&js, devname);
		    set_led_state(BLUETOOTH, OFF);
		}
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
		delay(50);
	}
	led_finished = 1;
}

PI_THREAD(debug_thread)
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

int am_i_su()
{
    if(geteuid())
    	return 0;
    return 1;
}

void clean_up()
{
	set_color(RED, 255);
	light_rgb();
	while(!led_finished);
	set_color(RED, 255);
	light_rgb();
	while(!(main_finished && joystick_finished && debug_finished));
	set_color(WHITE, 255);
	light_rgb();

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) system("sudo /home/pi/ccdir/watcher");
}

int main(int argc, char* argv)
{
	printf("main\n");
	if(!am_i_su()) 
	{
		printf("Restricted area. Super users only.\n");
		return 0;
	}

    piThreadCreate(debug_thread);

    wiringPiSetupPhys();
	init_motors();
	piThreadCreate(main_thread);
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	while(keep_running) delay(100);
	clean_up();

	return 0;
}