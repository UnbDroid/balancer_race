#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"
#include "encoder.c"
#include "motor.c"

struct joystick js;
int led_color;

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
        
        if(DEBUG_JOYSTICK && is_updated_js(&js))
		    update_print_js(js);
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

PI_THREAD(encoder)
{
	piHiPri(0);
	init_encoders();
	if(DEBUG_ENCODERS)
		print_debug_encoders();
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
	piThreadCreate(encoder);
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	while(getchar() != 'q');

	return 0;
}