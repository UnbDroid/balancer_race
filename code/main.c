#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "jstick.c"
#include "led.c"
#include "sensors.c"
#include "motor.c"
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
float KP = 0.2;//125; //325;
float KD = 0.02;//2.5;
float KI = 0.007;//0.7;

float KPvel = 1;
float KDvel = 10;
float KIvel = 5;

float teta = 0, teta_linha, teta_raw;
float gyroIntegrate = 0, old_gyroIntegrate = 0;
int pot = 0, dir;
float GK;
float dev_teta;
unsigned long long int temp = 0;
int flag;
float tetaIntegrat = 0;

double speed = 0;
double speedL = 0, speedR = 0;
double speed_refL = 0;
double speed_refR = 0;

float Lerro_vel = 0, Lderro_dt = 0, Lsoma_erro_vel = 0, Lerro_vel_old;
float Rerro_vel = 0, Rderro_dt = 0, Rsoma_erro_vel = 0, Rerro_vel_old;



double vel_ref = 0;
double vel_erro = 0, vel_erro_old = 0, vel_erro_integrate = 0, vel_ref_integrate = 0, vel_erro_derivate = 0;
double vel_med = 0;
unsigned long long int vel_time, vel_time_old, vel_dt;

double req_tilt = 0, req_tilt_old = 0, req_tilt_linha = 0;
double tilt_erro = 0, tilt_erro_integrate = 0, tilt_erro_linha = 0;



PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);

	
	delay(300);
	teta = RAD2DEG*atan2(imu.accel.filteredZ, imu.accel.filteredX);
	printf("%f\n", teta);
	//gyroIntegrate = teta;
	gyroIntegrate = teta - (-97.678610);
	

	while(keep_running)
	{
		//lendo o acell com filtro
		//teta = (RAD2DEG*atan2(imu.accel.filteredZ,imu.accel.filteredX)/*- (-95.416)*/);

	
		teta_linha = imu.gyro.treatedY - (-0.148855);//(-0.131567);

		if(temp != imu.last_update)
		{
			/*
			if(temp == 0)
			{
				//offset = (RAD2DEG*atan2(imu.accel.treatedZ,imu.accel.treatedX));
			}
			*/
			temp = imu.last_update;
			old_gyroIntegrate = gyroIntegrate;
			gyroIntegrate = gyroIntegrate+teta_linha*imu.dt;
			//teta = teta+teta_linha*imu.dt;
		}
		//teta = (RAD2DEG*atan2(imu.accel.filteredZ ,imu.accel.filteredX)) - (-97.045494);
		//teta = (RAD2DEG*atan2(imu.accel.treatedZ ,imu.accel.treatedX)) - (-95.916);
		//pot = (int)(teta*KP + teta_linha*KD);



		vel_ref = 0; // OBS: 0.01 JA EH UMA VELOCIDADE CONSIDERAVEL, CUIDADO!
		// PRIMEIRO CONTROLADOR. VEL -> TILT.
		vel_med = (left_motor.speed + right_motor.speed)/2;
		vel_time_old = vel_time;
		vel_time = micros();
		vel_dt = vel_time - vel_time_old;

		vel_erro_old = vel_erro;
		vel_erro = vel_ref - vel_med;
		vel_ref_integrate += vel_ref;
		vel_erro_integrate = vel_ref_integrate - (left_motor.displacement + right_motor.displacement)/2;
		vel_erro_derivate = (vel_erro - vel_erro_old)/vel_dt;
		
		req_tilt_old = req_tilt;
		req_tilt = -(vel_erro*KPvel + vel_erro_integrate*KIvel + vel_erro_derivate*KDvel);
	


		//tetaIntegrat = 0;

		/*
		Lerro_vel_old = Lerro_vel;
		Lerro_vel = left_motor.speed - speed_refL;
		Lsoma_erro_vel += Lerro_vel;
		Lderro_dt = (Lerro_vel - Lerro_vel_old)/left_motor.dt;

		Rerro_vel_old = Rerro_vel;
		Rerro_vel = right_motor.speed - speed_refR;
		Rsoma_erro_vel += Rerro_vel;
		Rderro_dt = (Rerro_vel - Rerro_vel_old)/right_motor.dt;
		*/


		//req_tilt = 5;
		//printf("%f\n", gyroIntegrate);

		// SEGUNDO CONTROLADOR. TILT -> PWM. 
		tilt_erro = req_tilt - gyroIntegrate;
		tilt_erro_integrate += tilt_erro;
		req_tilt_linha = (req_tilt - req_tilt_old)/vel_dt;
		tilt_erro_linha = req_tilt_linha - teta_linha;
	 	
	 	speed = (tilt_erro*KP + tilt_erro_linha*KD + tilt_erro_integrate*KI);



	 	//speedL = speed - (KPV*Lerro_vel + KIV*Lsoma_erro_vel + KDV*Lderro_dt);
	 	//speedR = speed - (KPV*Rerro_vel + KIV*Rsoma_erro_vel + KDV*Rderro_dt);

	 	//printf("%f\n", gyroIntegrate);
		setMotorSpeed(LMOTOR, speed);
		setMotorSpeed(RMOTOR, speed);
		write_motors();
	
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
		
		delay(5);
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
			plotvar[0] = right_motor.displacement;
			plotvar[1] = left_motor.displacement;
			//plotvar[0] = left_motor.raw_speed;
			//plotvar[1] = left_motor.filtered_speed;
			//plotvar[0] = left_motor.speed;
			//plotvar[1] = left_motor.set_speed;
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
        	//setMotorSpeed(LMOTOR, 0); // release motors
			//setMotorSpeed(RMOTOR, 0); // for safety purposes
			//write_motors();
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
			//update_ir();
			update_imu();
			//update_complementar();
			//update_kalman();
		} else {
			delayMicroseconds(100);
		}
	}
	sensors_finished = 1;
}

PI_THREAD(motors)
{
	motors_finished = 0;
	piHiPri(0);
	
	while(keep_running)
	{
		getValidData();
		storeValidData();
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
	int nao_terminou = 1;
	setMotorSpeed(LMOTOR, 0);
	setMotorSpeed(RMOTOR, 0);
	write_motors();
	set_color(RED, 255);
	light_rgb();
	while(!led_finished);
	set_color(RED, 255);
	light_rgb();
	do {
		nao_terminou = !(main_finished && joystick_finished && debug_finished && sensors_finished && supervisory_finished && plot_finished && motors_finished); 
		/*
		printf("%d...", main_finished);
		printf("%d...", joystick_finished);
		printf("%d...", debug_finished);
		printf("%d...", sensors_finished);
		printf("%d...", supervisory_finished);
		printf("%d...", plot_finished);
		printf("%d...", motors_finished);
		printf("%d...\n", nao_terminou);
		*/
	} while(nao_terminou);
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