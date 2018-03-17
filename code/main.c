#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
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
int debug_finished = 1, sensors_finished = 1, motors_finished = 1;
int matlab_finished = 1, supervisory_finished = 1, plot_finished = 1;

int plot_flag = 0;
long plot_time = 5;

int shutdown_flag = 0, reboot = 0, close_program=0;	// flags set by joystick
													// commands so that the
													// program knows what to
													// do when finishing up.

/*
This is the main thread. In it, we are supposed to put everything that doesn't
belong in the infrastructure threads below it. Generally, it is used to test
new features using the joystick controller.
*/

/*
//Butterworth Filter - A manteiga que vale
#define A0 1.000000000000000
#define A1 -1.968427786938518
#define A2 1.735860709208886
#define A3 -0.724470829507362
#define A4 0.120389599896245
#define B0 0.010209480791203
#define B1 0.040837923164813
#define B2 0.061256884747219
#define B3 0.040837923164813
#define B4 0.010209480791203

double A[5] = {A0, A1, A2, A3, A4};
double B[5] = {B0, B1, B2, B3, B4};
double bw_filtered[4];
double bw_raw[5];
*/

#define DEV_ACC_Z_OVER_X 97.1256
// motor grande
//float KP = 385; //325;
//float KD = 15; //10;
//float KI = 20;

// motor pequeno
float KP = 0.3;//125; //325;
float KD = 0*0.01;//2.5;
float KI = 0*0.0055;//0.7;

float teta = 0, teta_linha, teta_raw;
float gyroIntegrate = 0, old_gyroIntegrate = 0;
int pot = 0, dir;
float GK;
float dev_teta;
unsigned long long int temp = 0;
int flag;
float tetaIntegrat = 0;

double speed = 0;

PI_THREAD(main_thread)
{
	int i ;

	main_finished = 0;
	piHiPri(0);

	dev_teta = 0;

	//filtro do geovanny
	//inicializando os valores do vetor de leituras
	/*for(i = 0; i<4; i++)
	{
		bw_raw[i] = 0.0;
		bw_filtered[i] = 0.0;
	}
	bw_raw[4] = 0;*/

	delay(30);
	teta = RAD2DEG*atan2(imu.accel.filteredZ, imu.accel.filteredX);
	//printf("%f\n", teta);
	//gyroIntegrate = teta;
	gyroIntegrate = 0;
	while(keep_running)
	{
		//lendo o acell com filtro
		//teta = (RAD2DEG*atan2(imu.accel.filteredZ,imu.accel.filteredX)/*- (-95.416)*/);

		//filtro do geovanny
		/*teta_raw = (RAD2DEG*atan2(imu.accel.treatedZ,imu.accel.treatedX)/*- (-95.416)//);
		teta = 0;
		for (i = 0; i < 4; ++i)
			teta += -(A[i]*bw_filtered[i+1])+(B[i]*bw_raw[i]);
		teta += (B[i]*bw_raw[i]);
		for (i = 0; i < 3; ++i)
		{
			bw_raw[i+1] = bw_raw[i];
			bw_filtered[i+1] = bw_filtered[i];
		}
		bw_raw[i+1] = bw_raw[i];
		bw_raw[0] = teta_raw;
		bw_filtered[0] = teta;
		printf("%f\n", teta);
		//lendo o gyro
		*/

	/*
		teta_linha = imu.gyro.treatedY - (-0.131567);

		if(temp != imu.last_update)
		{
			if(temp == 0)
			{
				//offset = (RAD2DEG*atan2(imu.accel.treatedZ,imu.accel.treatedX));
			}
			temp = imu.last_update;
			old_gyroIntegrate = gyroIntegrate;
			gyroIntegrate = gyroIntegrate+teta_linha*imu.dt;
			teta = teta+teta_linha*imu.dt;
		}
		//teta = (RAD2DEG*atan2(imu.accel.filteredZ ,imu.accel.filteredX)) - (-97.045494);
		//teta = (RAD2DEG*atan2(imu.accel.treatedZ ,imu.accel.treatedX)) - (-95.916);
		//pot = (int)(teta*KP + teta_linha*KD);


		//tetaIntegrat = 0;
		tetaIntegrat += gyroIntegrate;
	 	speed = -(gyroIntegrate*KP + teta_linha*KD + tetaIntegrat*KI);


		setMotorSpeed(LMOTOR, speed);
		setMotorSpeed(RMOTOR, speed);
	*/
		//pot = 0;
		//int dz = 25;
		int dz = 230;
/*
		if(pot < 0)
		{
			dir = -1;
			pot = dz + ((1023.0-dz)/1023.0)*(-pot);//tirando a zona morta dos motores
			if(pot<=dz)//levando em conta a saturação dos motores
				pot = 0;
			OnFwd(LMOTOR, pot);
			OnFwd(RMOTOR, pot);
		} else if(pot > 0)
		{
			dir = 1;
			pot = dz + ((1023.0-dz)/1023.0)*(pot);
			if(pot<=dz)
				pot = 0;
			OnRev(LMOTOR, pot);
			OnRev(RMOTOR, pot);
		} else if(pot == 0)
		{
			Brake(RMOTOR);
			Brake(LMOTOR);
		}
		//printf("%f   |   %d\n", teta, pot);
		//printf("%f\n", imu.dt);
		//printf("%f\n", gyroIntegrate);
*/
		//delay(1);

/*
		if(js.lanalog.up)
		{
			OnFwd(LMOTOR, js.lanalog.up);
		} else if (js.lanalog.down) {
			OnRev(LMOTOR, js.lanalog.down);
		} else {
			Brake(LMOTOR);
		}

		if(js.ranalog.up)
		{
			OnFwd(RMOTOR, js.ranalog.up);
		} else if (js.ranalog.down) {
			OnRev(RMOTOR, js.ranalog.down);
		} else {
			Brake(RMOTOR);
		}
		delay(20);
*/
/*
		if(js.LB)
		{
			++pot;
		} else if (js.RB) {
			--pot;
		}
		OnFwd(LMOTOR, pot);
		OnFwd(RMOTOR, pot);
		delay(200);
*/
		/*
		if(speed > 2)
		{
			flag = 0;
		} else if(speed < 0) {
			flag = 1;
		}
		if(flag)
		{
			speed += 0.01;
		} else {
			speed -= 0.01;
		}
		*/
		speed = 1;
		setMotorSpeed(LMOTOR, speed);
		setMotorSpeed(RMOTOR, speed);
		delay(100);
		print_message(motor_sent_message, 1);
		print_message(motor_received_message, 2);
/*
		if(pot > 1023)
		{
			flag = 0;
		} else if(pot < -1023) {
			flag = 1;
		}
		if(flag)
		{
			pot += 4;
		} else {
			pot -= 4;
		}
		OnFwd(LMOTOR, pot);
		OnFwd(RMOTOR, pot);
		delay(1);
*/
/*
		speed = 1;
		setMotorSpeed(LMOTOR, speed);
		setMotorSpeed(RMOTOR, speed);
		delay(1);
*/
	}
	main_finished = 1;
}

#define PLOT_RATE 5
#define NPLOTVARS 15
double plotvar[NPLOTVARS] = {};
PI_THREAD(plot)
{
	plot_finished = 0;
	FILE *fp;
	unsigned long long int last_fprintf = 0;
	int i = 0;
	char fname[40];
	do {
		snprintf(fname, 40, "/home/pi/datalog/plot_data_%03d", i);
		++i;
	} while(exists(fname));
	fp = fopen(fname, "w");
	if(imu.last_update > plot_time*1000000) keep_running = 0;
	while(keep_running)
	{
		if(imu.last_update != last_fprintf)
		{
			last_fprintf = imu.last_update;
			//plotvar[0] = gyroIntegrate;
			plotvar[0] = left_motor.displacement;
			//plotvar[0] = left_motor.raw_speed;
			//plotvar[1] = left_motor.filtered_speed;
			//plotvar[2] = right_motor.filtered_speed;
			//plotvar[2] = right_motor.displacement;
			fprintf(fp, "%lld ", imu.last_update);
			for(i = 0; (i < NPLOTVARS-1 && plotvar[i+1] == plotvar[i+1]); ++i)
			{
				fprintf(fp, "% f ", plotvar[i]);
			}
			if(plotvar[i] == plotvar[i])
			{
				fprintf(fp, "% f;\n", plotvar[i]);
			}
		}
		if(imu.last_update > plot_time*1000000) keep_running = 0;
	}
	fclose(fp);
	printf("Saved data to file %s\n", fname);
	plot_finished = 1;
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

    while((!(js.select && js.start)) && (keep_running)) // START+SELECT finishes the program
    {
        if(js.disconnect)
        {
        	setMotorSpeed(LMOTOR, 0); // release motors
			setMotorSpeed(RMOTOR, 0); // for safety purposes
			write_motors();
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
	// LEFT+START+SELECT: closes the program, not calling the watcher
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
		delay(10);
	}
	led_finished = 1;
}

/*
This is the sensors thread. It keeps the robot's sensors updated at a
(supposedly) steady rate.
*/
#define SENSORS_UPDATE_RATE 10 // defined in milliseconds
PI_THREAD(sensors)
{
	sensors_finished = 0;
	unsigned long long int last_update, now_time;
	piHiPri(0);
	while(keep_running)
	{
		now_time = micros();
		if(now_time - last_update > 1000*SENSORS_UPDATE_RATE)
		{
			last_update = now_time;
			update_ir();
			update_imu();
			update_kalman();
		} else {
			delayMicroseconds(100);
		}
	}
	sensors_finished = 1;
}

PI_THREAD(motors)
{
	motors_finished = 0;
	piHiPri(99);
	while(keep_running)
	{
		read_motors();
		write_motors();
		delayMicroseconds(100);
	}
	motors_finished = 1;
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
		} while(keep_running && (send_superv_message(&debug) != -1));
	}
	supervisory_finished = 1;
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
	setMotorSpeed(LMOTOR, 0);
	setMotorSpeed(RMOTOR, 0);
	write_motors();
	set_color(RED, 255);
	light_rgb();
	while(!led_finished);
	set_color(RED, 255);
	light_rgb();
	while(!(main_finished && joystick_finished && debug_finished && sensors_finished && supervisory_finished && plot_finished && motors_finished));
	set_color(WHITE, 255);
	light_rgb();

	if(shutdown_flag) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
	else if (!close_program && !plot_flag)
	{
		if(debug.debug_flag) system("sudo /home/pi/ccdir/watcher -d&");
		else system("sudo /home/pi/ccdir/watcher&");
	}
}

int main(int argc, char* argv[])
{
	if(!am_i_su())
	{
		printf("Restricted area. Super users only.\n");
		return 0;
	}

	wiringPiSetupPhys();

	debug.debug_flag = 0;
	if(argc > 1)
	{
		for(i = 1; i < argc; ++i)
		{
			if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0)
			{
				debug.debug_flag = 1;
			}
			if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--plot") == 0)
			{
				long temp;
				if(i + 1 < argc && (temp = atol(argv[i + 1])))
				{
					plot_time = temp;
				}
				plot_flag = 1;
			}
		}
		if(debug.debug_flag)
		{
			piThreadCreate(debug_thread);
		}
		if(plot_flag)
		{
			for(i = 0; i < NPLOTVARS; ++i)
			{
				plotvar[i] = sqrt(-1);
			}
			piThreadCreate(plot);
		}
	}

	init_motors();
	init_sensors();

	piThreadCreate(main_thread);
	piThreadCreate(motors);
	piThreadCreate(sensors);
	if(!plot_flag) piThreadCreate(joystick);
	piThreadCreate(led);

	piThreadCreate(supervisory);
	piThreadCreate(matlab);

	while(keep_running) delay(100);
	clean_up();

	return 0;
}