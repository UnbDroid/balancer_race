#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <string.h>
#include "jstick.c"
#include "led.c"
#include "motor.c"
#include "sensors.c"
#include "debug.c"

struct debug_data debug;

int keep_running = 1;
int main_finished = 1, led_finished = 1, joystick_finished = 1, debug_finished = 1, sensors_finished = 1;
int shutdown = 0, reboot = 0, close_program=0;;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);
	while(keep_running)
	{		
		if(js.lanalog.up > 0)
		{
			OnFwd(LMOTOR, js.lanalog.up);
		} else if (js.lanalog.down > 0) {
			OnRev(LMOTOR, js.lanalog.down);
		} else {
			Brake(LMOTOR);
		}

		if(js.ranalog.up > 0)
		{
			OnFwd(RMOTOR, js.ranalog.up);
		} else if (js.ranalog.down > 0) {
			OnRev(RMOTOR, js.ranalog.down);
		} else {
			Brake(RMOTOR);
		}
		char mess[36];
		
		snprintf(mess, 36, "Accel X Tan: %07.3f", (double)imu.accel.rawZ/(double)imu.accel.rawY);
		print_message(mess, 1);

		snprintf(mess, 36, "Accel X atan(Tan) rad: %07.3f", atan((double)imu.accel.rawZ/(double)imu.accel.rawY));
		print_message(mess, 2);

		snprintf(mess, 36, "Accel X atan2(Tan) rad: %07.3f", atan2((double)imu.accel.rawZ, (double)imu.accel.rawY));
		print_message(mess, 3);
		
		snprintf(mess, 36, "Accel X atan(Tan) deg: %07.3f", RAD2DEG*atan((double)imu.accel.rawZ/(double)imu.accel.rawY));
		//print_message(mess, 4);

		snprintf(mess, 36, "Accel Y Tan: %07.3f", (double)imu.accel.rawX/(double)imu.accel.rawZ);
		print_message(mess, 5);

		snprintf(mess, 36, "Accel Y atan(Tan) rad: %07.3f", atan((double)imu.accel.rawX/(double)imu.accel.rawZ));
		print_message(mess, 6);

		snprintf(mess, 36, "Accel Y atan2(Tan) rad: %07.3f", atan2((double)imu.accel.rawX, (double)imu.accel.rawZ));
		print_message(mess, 7);
		
		snprintf(mess, 36, "Accel Y atan(Tan) deg: %07.3f", RAD2DEG*atan((double)imu.accel.rawX/(double)imu.accel.rawZ));
		//print_message(mess, 8);

		snprintf(mess, 36, "Accel Z Tan: %07.3f", (double)imu.accel.rawY/(double)imu.accel.rawX);
		print_message(mess, 9);

		snprintf(mess, 36, "Accel Z atan(Tan) rad: %07.3f", atan((double)imu.accel.rawY/(double)imu.accel.rawX));
		print_message(mess, 10);

		snprintf(mess, 36, "Accel Z atan2(Tan) rad: %07.3f", atan2((double)imu.accel.rawY, (double)imu.accel.rawX));
		print_message(mess, 11);
		
		snprintf(mess, 36, "Accel Z atan(Tan) deg: %07.3f", RAD2DEG*atan((double)imu.accel.rawY/(double)imu.accel.rawX));
		//print_message(mess, 12);
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
        	Coast(RMOTOR);
        	Coast(LMOTOR);
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

PI_THREAD(sensors)
{
	sensors_finished = 0;
	//piHiPri(0); //escolher prioridade
	while(keep_running)
	{
		update_ir();
		update_imu();
		delay(10);
	}
	sensors_finished = 1;
}

PI_THREAD(debug_thread)
{
	debug_finished = 0;
	piHiPri(0);
	init_debug();
	while(keep_running)
	{
		update_motors();
		debug.js = js;
		debug.left_motor = left_motor;
		debug.right_motor = right_motor;
		debug.ir = ir;
		debug.imu = imu;
		debug.led_state = led_state;
		update_debug(&debug);
		delay(100);
	}
	printf("\033[%d;%dH\n", 43, 1);
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
	while(!(main_finished && joystick_finished && debug_finished && sensors_finished));
	set_color(WHITE, 255);
	light_rgb();

	if(shutdown) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) system("sudo /home/pi/ccdir/watcher&");
}

int main(int argc, char* argv[])
{
	debug.debug_flag = 0;
	if(argc > 1)
	{
		for(i = 1; i < argc; ++i)
		{
			if(	strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0 )
			{
				debug.debug_flag = 1;
			}
		}
		if(debug.debug_flag)
		{
			piThreadCreate(debug_thread);
		}
	}

	if(!am_i_su()) 
	{
		if(debug.debug_flag) printf("Restricted area. Super users only.\n");
		return 0;
	}

    wiringPiSetupPhys();
	init_motors();
	init_sensors();
	piThreadCreate(main_thread);
	piThreadCreate(sensors);
	piThreadCreate(joystick);
	piThreadCreate(led);
	
	while(keep_running) delay(100);
	clean_up();

	return 0;
}