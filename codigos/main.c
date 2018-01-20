#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"
#include "motor.c"

struct joystick js;
int led_color = UNDEFINED_COLOR;

PI_THREAD(main_thread)
{
	piHiPri(0);
	while(1)
	{
		if(js.B)
			led_color = RED;
		else if(js.A)
			led_color = GREEN;
		else if(js.X)
			led_color = BLUE;
		else if(js.Y)
			led_color = YELLOW;
		else
			led_color = WHITE;
	}
}

PI_THREAD(joystick)
{
	piHiPri(0);
    init_joystick(&js, devname);
    while(1)
    {
        if(js.disconnect)
        	init_joystick(&js, devname);
        update_joystick(&js);
	}
}

PI_THREAD(led)
{
	piHiPri(0);
	init_led();
	while(1)
	{
		light_color(led_color);
		delay(LED_DELAY);
	}
}

PI_THREAD(debug)
{
	if(DEBUG_MOTORS || DEBUG_JOYSTICK)
	{
		piHiPri(0);
		if(DEBUG_MOTORS)
			print_debug_encoders();
		else if(DEBUG_JOYSTICK)
		    update_print_js(js);	
	}
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

	return 0;
}