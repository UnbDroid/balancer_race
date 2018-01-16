#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"
#include "led.c"

struct joystick js;
int led_color;

PI_THREAD(joystick)
{
	piHiPri(0);
	init_joystick(&js, devname);
    while(1)
    {
        if(disconnect)
        	
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
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	while(getchar() != 'q');

	return 0;
}