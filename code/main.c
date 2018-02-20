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

struct debug_data debug;	// struct containing all robot values
							// for debugging purposes

int keep_running = 1;	// end of program flag. it is controlled by the
						// joystick thread. if set to 0, all threads will
						// finish up and set their own flags to 1 so that
						// the main program can clean everything up and end.

int main_finished = 1, led_finished = 1, joystick_finished = 1;
int debug_finished = 1, sensors_finished = 1;
int matlab_finished = 1, supervisory_finished = 1;

int shutdown_flag = 0, reboot = 0, close_program=0;	// flags set by joystick
													// commands so that the
													// program knows what to
													// do when finishing up.

/*
This is the main thread. In it, we are supposed to put everything that doesn't
belong in the infrastructure threads below it. Generally, it is used to test
new features using the joystick controller.
*/
float KP = 32.7;
float KD = 0.9;
float teta, teta_linha;
int pot;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);
	while(keep_running)
	{		
		//brincando de controle
		teta_linha = imu.gyro.treatedY;
		if(imu.accel.freeze)
		{
			teta += teta_linha*dt;
		}
		else
		{
			teta = RAD2DEG*atan2(imu.accel.treatedZ,imu.accel.treatedX) - (-94.9215);
		}

		 
		pot = (int)(teta*KP + teta_linha*KD);
		//pot = 0;

		//printf("teta = %f  teta_linha = %f mag_Acc = %f\n", teta, teta_linha, imu.accel.magnitude);
		
		printf("%f\n", teta);

		//pot = 0;
		if(pot < 0)
		{
			pot = 150 + 0.853372*(-pot);//tirando a zona morta dos motores
			if(pot<=150)//levando em conta a saturação dos motores
				pot = 0;
			if(pot>1023)
				pot = 1023;	
			OnFwd(LMOTOR, pot);
			OnFwd(RMOTOR, pot);
		} else if(pot > 0)
		{
			pot = 150 + 0.853372*(pot);
			if(pot<=150)
				pot = 0;
			if(pot>1023)
				pot = 1023;	
			OnRev(LMOTOR, pot);
			OnRev(RMOTOR, pot);
		} else if(pot == 0)
		{
			Brake(RMOTOR);
			Brake(LMOTOR);
		}
		delay(10);
	}
	Coast(LMOTOR);
	Coast(RMOTOR);
	main_finished = 1;
}

/*
This is the joystick thread. It runs the code behind reading and interpreting
joystick commands. The joystick is the primary way of commanding the robot and
will be eventually used as safety trigger. Aside from that, it allows the
operator to control the robot without the need for opening a terminal.
All joystick support functions available in the jstick.c file.
*/
PI_THREAD(joystick)
{
    joystick_finished = 0;
	piHiPri(0);
    
    set_led_state(BLUETOOTH, ON);
    init_joystick(&js, devname);
    set_led_state(BLUETOOTH, OFF);

    while(!(js.select && js.start)) // START+SELECT finishes the program
    {
        if(js.disconnect)
        {
        	Coast(RMOTOR); // release motors
        	Coast(LMOTOR); // for safety purposes
        	set_led_state(BLUETOOTH, ON);
		    init_joystick(&js, devname);
		    set_led_state(BLUETOOTH, OFF);
		}
        update_joystick(&js);
	}
	// If a D-Pad key is pressed along with START+SELECT when finishing
	// the program, special finishing up routines are called inside the
	// clean_up() function. They are:
	
	// DOWN+START+SELECT: shuts the Raspberry Pi Zero W down
	if(js.dpad.down) shutdown_flag = 1;
	// UP+START+SELECT: reboots the Raspberry Pi Zero W
	if(js.dpad.up) reboot = 1;
	// LEFT+START+SELECT: finished the program instead of calling the watcher
	if(js.dpad.left) close_program = 1;

	keep_running = 0;
	joystick_finished = 1;
}

/*
This is the LED thread. It runs the LED above the robot, used for debugging
and for informing the operator about the robot's current status.
All LED support functions are available in the led.c file.
*/
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

/*
This is the sensors thread. It keeps the robot's sensors updated at a
(supposedly) steady rate.
*/
#define SENSORS_UPDATE_RATE 20 // defined in milliseconds
PI_THREAD(sensors)
{
	sensors_finished = 0;
	unsigned long int last_update, now_time;
	piHiPri(0);
	while(keep_running)
	{
		now_time = millis();
		if(now_time - last_update > SENSORS_UPDATE_RATE)
		{
			last_update = now_time;
			update_ir();
			update_imu();
			update_kalman();
		} else {
			delay(5);
		}
	}
	sensors_finished = 1;
}

/*
This is the debug thread. It runs the debug screen code and is only run if the
program is called with the -d or --debug parameter. For example:
pi@raspberrypi $ sudo ./main -d
*/
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

/*
This is the supervisory system support thread. It runs separately from the
debug thread although it shouldn't. The reason for that is that the socket
server functions halt the program if there is no supervisory client running.
*/
PI_THREAD(supervisory)
{
	piHiPri(0);
	while(keep_running)
	{
		init_supervisory();
		supervisory_finished = 0;
		do{
			delay(10);
			
			debug.js = js;
			debug.left_motor = left_motor;
			debug.right_motor = right_motor;
			debug.ir = ir;
			debug.imu = imu;
			debug.led_state = led_state;		
		}while(keep_running && (send_superv_message(&debug) != -1));
	supervisory_finished = 1;
	}	
}

PI_THREAD(matlab)
{
	piHiPri(0);
	init_matlab();
	matlab_finished = 0;
	while(keep_running)
	{
		debug.js = js;
		debug.left_motor = left_motor;
		debug.right_motor = right_motor;
		debug.ir = ir;
		debug.imu = imu;
		debug.led_state = led_state;
		
		send_matlab_message(&debug);
		delay(10);
	}
	matlab_finished = 1;
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
	while(!(main_finished && joystick_finished && debug_finished && sensors_finished && supervisory_finished));
	set_color(WHITE, 255);
	light_rgb();

	if(shutdown_flag) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program) 
	{
		if(debug.debug_flag) system("sudo /home/pi/ccdir/watcher -d");
		else system("sudo /home/pi/ccdir/watcher&");
	}
}

int main(int argc, char* argv[])
{
	debug.debug_flag = 0;
	if(argc > 1)
	{
		for(i = 1; i < argc; ++i)
		{
			if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0)
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
		printf("Restricted area. Super users only.\n");
		return 0;
	}

	system("gpio load i2c 350");	// Setting i2c frequency to 350kHz.
									// Raspberry Pi max is 150MHz
									// MPU9250 max is 400kHz
									// AK8963 max is 400kHz in fast-mode
									// PCA9685 max is 1MHz
    wiringPiSetupPhys();
	init_motors();
	init_sensors();

	piThreadCreate(main_thread);
	piThreadCreate(sensors);
	piThreadCreate(joystick);
	piThreadCreate(led);

	piThreadCreate(supervisory);
	piThreadCreate(matlab);
	
	while(keep_running) delay(100);
	clean_up();

	return 0;
}