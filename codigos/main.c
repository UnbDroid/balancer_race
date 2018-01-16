#include <stdio.h>
#include <wiringPi.h>
#include "jstick.c"

struct joystick js;

PI_THREAD(joystick)
{
	piHiPri(0);
	init_joystick(&js, devname);

    while(1)
    {
        update_joystick(&js);
        if(DEBUG_JOYSTICK && is_updated_js(&js))
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
	piThreadCreate(joystick);
	
	while(getchar() != '\n');

	return 0;
}