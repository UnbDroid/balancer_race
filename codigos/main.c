#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"

struct joystick js;
int led_color;

PI_THREAD(main)
{
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
    while(1)
    {
        if(disconnect)
        	init_joystick(&js, devname);
        
        disconnect = update_joystick(&js);
        
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
		delay(100);
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
	piThreadCreate(main);
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	while(getchar() != 'q');

	return 0;
}