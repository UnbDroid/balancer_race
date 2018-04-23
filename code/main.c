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

double EULER = 2.718281828459045;

// motor pequeno
float KP = 0.2;//125; //325;
float KD = 0.02;//2.5;
float KI = 0.007;//0.7;

float KPvel = 3.1;//1;//1;
float KDvel = 0.01;//0;//10;
float KIvel = 1.15;//1.7;//5;

float KPome = 1;
float KDome = 0.025;
float KIome = 2;

float teta = 0, teta_linha, teta_raw;
float gyroIntegrate = 0, old_gyroIntegrate = 0;
int pot = 0, dir;
float GK;
float dev_teta;
unsigned long long int temp = 0;
int flag;
float tetaIntegrat = 0;


// VARIAVEIS USADAS NO CONTROLE.
double speed = 0;

double vel_ref = 0;
double vel_erro = 0, vel_erro_old = 0, vel_erro_integrate = 0, vel_ref_integrate = 0, vel_erro_derivate = 0;
double vel_med = 0;
unsigned long long int vel_time, vel_time_old, vel_dt;

double req_tilt = 0, req_tilt_old = 0, req_tilt_linha = 0;
double tilt_erro = 0, tilt_erro_integrate = 0, tilt_erro_linha = 0;

double omega = 0, omega_integrate = 0, omega_ref_integrate = 0;
double omega_erro = 0, omega_erro_old = 0, omega_erro_integrate = 0, omega_erro_derivate = 0;
double speed_dir = 0;
double omega_ref = 0;

unsigned long long int ref_crono = 0, ref_crono_set = 0, ref_time = 0;
double diff = 0, ref = 0, atual = 0, ref_old = 0;
double time_k = 38.918203; // ln(1-%a/%a)/-ta // %a = porcentagem que deseja obter apos decorrido tempo ta. ta = tempo necessario para chegar na porcentagem desejada em segundos.
double ta = 0.12;
double scurve_extra_time = 0.06; // Tempo extra do scurve pra ficar próximo da referência.

unsigned long long int ref_crono_omega = 0, ref_crono_set_omega = 0, ref_time_omega = 0;
double diff_omega = 0, atual_omega = 0, omega_ref_old = 0;
double s_omega_ref = 0;
double time_k_omega = 382.926654; // ln(1-%a/%a)/-ta // %a = porcentagem que deseja obter apos decorrido tempo ta. ta = tempo necessario para chegar na porcentagem desejada em segundos.
double ta_omega = 0.012;
double scurve_extra_time_omega = 0.006; // Tempo extra do scurve pra ficar próximo da referência.

double lpf_vel_med[2];
double lpf_omega[2];

double LPFgain = 0.02;
double LPFgainOmega = 0.05;

PI_THREAD(main_thread)
{
	main_finished = 0;
	piHiPri(0);


	delay(1000);
	teta = RAD2DEG*atan2(imu.accel.filteredZ, imu.accel.filteredX);
	printf("%f\n", teta);
	//gyroIntegrate = teta;
	gyroIntegrate = teta - (-97.567947);
	ref_crono_set = micros();

	lpf_vel_med[0] = 0;
	lpf_vel_med[1] = 0;


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

		//---------------------------------------------------------------------------------------------------------------------
		// COMANDO VELOCIDADE POR JOYSTICK.
		if(js.lanalog.up > 0)
		{
			ref = 0.0015640274*js.lanalog.up;//0.0000127077*js.lanalog.up;
		}
		else if (js.lanalog.down > 0)
		{
			ref = -0.0015640274*js.lanalog.down;//-0.0000127077*js.lanalog.down;
		}
		else 
		{
			ref = 0;
		}

		ref_crono = micros();
		ref_time = ref_crono - ref_crono_set;
		
		if (ref != ref_old){
			diff = ref - vel_ref;
			atual = vel_ref;
			ref_crono_set = micros();
			ref_time = 0;

			ref_old = ref;
		}

		if (((double)ref_time)/1000000 < (2*ta + scurve_extra_time)){
			vel_ref = atual + diff*(1/(1 + pow(EULER, -time_k*(-ta + ((double)ref_time)/1000000))));
		} else {
			vel_ref = ref;
		}
		//vel_ref = 1/(1 + pow(EULER, -time_k*(-0.2 + ((double)ref_time)/1000000))); // S-CURVE FUNCIONANDO.
		//vel_ref = ref;

		//---------------------------------------------------------------------------------------------------------------------
		// PRIMEIRO CONTROLADOR. VEL -> TILT.
		vel_med = (left_motor.speed + right_motor.speed)/2;
		lpf_vel_med[0] = vel_med*LPFgain + lpf_vel_med[1]*(1-LPFgain);
		lpf_vel_med[1] = lpf_vel_med[0];
		vel_time_old = vel_time;
		vel_time = micros();
		vel_dt = vel_time - vel_time_old;

		vel_erro_old = vel_erro;
		vel_erro = vel_ref - lpf_vel_med[0];
		//vel_ref_integrate += vel_ref;
		vel_erro_integrate += vel_erro*((double)vel_dt)/1000000;
		//vel_erro_integrate = vel_ref_integrate - (left_motor.displacement + right_motor.displacement)/2;
		vel_erro_derivate = (vel_erro - vel_erro_old)/vel_dt;
		
		req_tilt_old = req_tilt;
		
		req_tilt = -(vel_erro*KPvel + vel_erro_integrate*KIvel + vel_erro_derivate*KDvel);
		
		//---------------------------------------------------------------------------------------------------------------------	
		// SEGUNDO CONTROLADOR. TILT -> PWM. 
		tilt_erro = req_tilt - gyroIntegrate;
		tilt_erro_integrate += tilt_erro;
		req_tilt_linha = (req_tilt - req_tilt_old)/vel_dt;
		tilt_erro_linha = req_tilt_linha - teta_linha;
	 	
	 	speed = (tilt_erro*KP + tilt_erro_linha*KD + tilt_erro_integrate*KI);

	 	//---------------------------------------------------------------------------------------------------------------------
	 	// COMANDO ROTACAO POR JOYSTICK.
	 	if(js.ranalog.left > 0)
		{
			omega_ref = 0.0016617791*js.ranalog.left;
		}
		else if (js.ranalog.right > 0)
		{
			omega_ref = -0.0016617791*js.ranalog.right;
		}
		else 
		{
			omega_ref = 0;
		}
	 	//omega_ref = 5;

		ref_crono_omega = micros();
		ref_time_omega = ref_crono_omega - ref_crono_set_omega;
		
		if (omega_ref != omega_ref_old){
			diff_omega = omega_ref - s_omega_ref;
			atual_omega = s_omega_ref;
			ref_crono_set_omega = micros();
			ref_time_omega = 0;

			omega_ref_old = omega_ref;
		}

		if (((double)ref_time_omega)/1000000 < (2*ta_omega + scurve_extra_time_omega)){
			s_omega_ref = atual_omega + diff_omega*(1/(1 + pow(EULER, -time_k_omega*(-ta_omega + ((double)ref_time_omega)/1000000))));
		} else {
			s_omega_ref = omega_ref;
		}

	 	//---------------------------------------------------------------------------------------------------------------------
	 	// TERCEIRO CONTROLADOR. DIRECAO.
	 	omega = right_motor.speed - left_motor.speed;
	 	lpf_omega[0] = omega*LPFgainOmega + lpf_omega[1]*(1-LPFgainOmega);
		lpf_omega[1] = lpf_omega[0];
	 	omega_integrate = right_motor.displacement - left_motor.displacement;
	 	omega_ref_integrate += s_omega_ref*((double)vel_dt)/1000000;

	 	omega_erro_old = omega_erro;
	 	omega_erro = s_omega_ref - lpf_omega[0];
	 	omega_erro_integrate = omega_ref_integrate - omega_integrate;
	 	omega_erro_derivate = (omega_erro - omega_erro_old)/(((double)vel_dt)/1000000);

	 	speed_dir = omega_erro*KPome + omega_erro_integrate*KIome + omega_erro_derivate*KDome;

	 	//---------------------------------------------------------------------------------------------------------------------
	 	// COMANDO PARA O ARDUINO.
	 	setMotorSpeed(LMOTOR, speed - speed_dir);
		setMotorSpeed(RMOTOR, speed + speed_dir);
		write_motors();

		delay(5);
	}
	main_finished = 1;
}

#define SAMPLE_TIME 5
#define NPLOTVARS 15
double plotvar[NPLOTVARS] = {};
PI_THREAD(plot)
{
	plot_finished = 0;
	FILE *fp;
	unsigned long long int now, last_fprintf = 0;
	int i = 0;
	char fname[40];
	while(keep_running)
	{
		while(!js.B && keep_running) delay(20);
		while(js.B && keep_running) delay(20);
		
		i = 0;
		if (keep_running)
		{
			do {
				snprintf(fname, 40, "/home/pi/datalog/plot_data_%03d", i);
				++i;
			} while(exists(fname));
			fp = fopen(fname, "w");
			
			set_led_state(PLOT, ON);
			while(!js.B && keep_running)
			{
				now = micros();
				if(now - last_fprintf > SAMPLE_TIME*1000)
				{
					last_fprintf = now;

					plotvar[0] = lpf_vel_med[0];
					plotvar[1] = vel_ref;
					plotvar[2] = lpf_omega[0];
					plotvar[3] = vel_erro_integrate;

					fprintf(fp, "%lld ", now);
					for(i = 0; (i < NPLOTVARS-1 && plotvar[i+1] == plotvar[i+1]); ++i)
					{
						fprintf(fp, "% f ", plotvar[i]);
					}
					if(plotvar[i] == plotvar[i])
					{
						fprintf(fp, "% f;\n", plotvar[i]);
					}
				}
			}
			fclose(fp);
			printf("Saved data to file %s\n", fname);
			set_led_state(PLOT, OFF);
			while(js.B && keep_running) delay(20);
		}
	}
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
	else if (!close_program)
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
	piThreadCreate(joystick);
	piThreadCreate(led);

	piThreadCreate(supervisory);
	piThreadCreate(matlab);

	while(keep_running) delay(100);
	clean_up();

	return 0;
}