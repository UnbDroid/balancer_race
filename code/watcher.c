#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
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
	while(!(joystick_finished));
	set_color(WHITE, 255);
	light_rgb();

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) system("sudo /home/pi/ccdir/main");
}

int main()
{
	printf("watcher\n");
	if(!am_i_su()) 
	{
		printf("Restricted area. Super users only.\n");
		return 0;
	}

    wiringPiSetupPhys();
	piThreadCreate(joystick);
	piThreadCreate(led);
	set_led_state(STANDBY, ON);
	while(keep_running) delay(100);
	clean_up();

	return 0;
}