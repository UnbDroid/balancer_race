#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"
#include "motor.c"

struct joystick js;
int led_color = OFF_COLOR;
int keep_running = 1;
int main_finished = 1, led_finished = 1, joystick_finished = 1, debug_finished = 1;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);
	while(keep_running)
	{
		if(js.B)
			led_color = RED;
		else if(js.A)
			led_color = GREEN;
		else if(js.X)
			led_color = BLUE;
		else if(js.Y)
			led_color = YELLOW;
		else if(js.start)
			led_color = WHITE;
		else
			led_color = OFF_COLOR;
		
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
    while(keep_running)
    {
        if(js.disconnect)
        	init_joystick(&js, devname);
        update_joystick(&js);
	}
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

PI_THREAD(debug)
{
	debug_finished = 0;
	if(DEBUG_MOTORS || DEBUG_JOYSTICK)
	{
		piHiPri(0);
		if(DEBUG_MOTORS)
			print_debug_encoders();
		else if(DEBUG_JOYSTICK)
		    update_print_js(js);	
	}
	debug_finished = 1;
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
	init_motors();
	
	piThreadCreate(main_thread);
	piThreadCreate(joystick);
	piThreadCreate(led);
	piThreadCreate(debug);
	
	while(getchar() != 'q');

	keep_running = 0;
	light_color(RED);
	while(!(main_finished && joystick_finished && led_finished && debug_finished));
	light_off();

	return 0;
}