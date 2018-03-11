#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "./socket.h"
#include "./poll.h"

#define DEF_SUPERVISORY 0
#define DEF_MATLAB 1

struct debug_data {
	struct joystick js;
	struct motor left_motor;
	struct motor right_motor;
	struct infrared ir;
	struct imu imu;
	int led_state;
	int debug_flag;
};

void init_debug()
{
	printf("\e[2J\e[H");
	printf("                 ``****.....``     DEBUG MODE     ``.....****``                 \n");
	printf("             ```*`````````  ..`````````..`````````..  ````````*````             \n");
	printf("            .``````       `.`         \\ /          `.`       ``````.            \n");
	printf("          `.`              ``          *           ``              `.`          \n");
	printf("         .                   ``       / \\        ``                   .         \n");
	printf("        .       ****           ``````````````````         .-`-.        .        \n");
	printf("       `         .`.                                     .``*``.        `       \n");
	printf("      ``    ****`.*.`****       ....        ....     `````.`-`.`````    ``      \n");
	printf("      .           `             ..*.`      `. *.   `.` *`.     .`* `-    .      \n");
	printf("     .          ****             ``          ``     `....```-```....`     .     \n");
	printf("    ``                       .                           .`.*.`.          ``    \n");
	printf("    .                      .` `.                          .:.:.`           .    \n");
	printf("   .                       . * .                  ****                      .   \n");
	printf("  ``                   .````   ````.              .`.                       ``  \n");
	printf("  .                   `. *       * .`        ****`.*.`****                   .  \n");
	printf(" `                      ````   ````                `                          . \n");
	printf(" .                         . * .                  ****                        . \n");
	printf("``                          `.`                                                `\n");
	printf(".                                                                              .\n");
	printf(".                        ``````````````````````````````                        .\n");
	printf(".                     ``` Left Motor       Right Motor ```                     .\n");
	printf(".                  ``` A:* B:* PWM:****  A:* B:* PWM:**** ```                  .\n");
	printf(".                ``    Ticks: *********  Ticks: *********    ``                .\n");
	printf(".              ``      Pos: ***********  Pos: ***********      ``              .\n");
	printf(" .           ``        RawSpeed: ******  RawSpeed: ******        ``           . \n");
	printf("  ``       `` Left IR  FiltSpeed: *****  FiltSpeed: ***** Right IR ``       ``  \n");
	printf("    ```````     ***              LED: *********              ***     ```````    \n");
	printf("         Yaw       Pitch     Roll       1:                                      \n");
	printf("  Pos:   ******    ******    ******     2:                                      \n");
	printf("  Vel:   ******    ******    ******     3:                                      \n");
	printf("                                        4:                                      \n");
	printf("     Gyro        Accel       Magnet     5:                                      \n");
	printf("  X  *******  X  *******  X  *******    6:                                      \n");
	printf("  Y  *******  Y  *******  Y  *******    7:                                      \n");
	printf("  Z  *******  Z  *******  Z  *******    8:                                      \n");
	printf("  rawX ****** rawX ****** rawX ******   9:                                      \n");
	printf("  rawY ****** rawY ****** rawY ******  10:                                      \n");
	printf("  rawZ ****** rawZ ****** rawZ ******  11:                                      \n");
	printf("  IMU Time: ******us                   12:                                      \n");
}


void update_debug(struct debug_data* debug)
{
	printf("\033[%d;%dH%04d\n", 1, 20, debug->js.LT);
	printf("\033[%d;%dH%04d\n", 1, 58, debug->js.RT);
	printf("\033[%d;%dH%d\n", 2, 17, debug->js.LB);
	printf("\033[%d;%dH%d\n", 2, 63, debug->js.RB);
	printf("\033[%d;%dH%d\n", 4, 40, debug->js.home);
	printf("\033[%d;%dH%04d\n", 6, 17, debug->js.lanalog.up);
	printf("\033[%d;%dH%d\n", 7, 61, debug->js.Y);
	printf("\033[%d;%dH%04d\n", 8, 13, debug->js.lanalog.left);
	printf("\033[%d;%dH%d\n", 8, 19, debug->js.lanalog.press);
	printf("\033[%d;%dH%04d\n", 8, 22, debug->js.lanalog.right);
	printf("\033[%d;%dH%d\n", 9, 35, debug->js.select);
	printf("\033[%d;%dH%d\n", 9, 47, debug->js.start);
	printf("\033[%d;%dH%d\n", 9, 56, debug->js.X);
	printf("\033[%d;%dH%d\n", 9, 66, debug->js.B);
	printf("\033[%d;%dH%04d\n", 10, 17, debug->js.lanalog.down);
	printf("\033[%d;%dH%d\n", 11, 61, debug->js.A);
	printf("\033[%d;%dH%d\n", 13, 30, debug->js.dpad.up);
	printf("\033[%d;%dH%04d\n", 13, 51, debug->js.ranalog.up);
	printf("\033[%d;%dH%d\n", 15, 26, debug->js.dpad.left);
	printf("\033[%d;%dH%d\n", 15, 34, debug->js.dpad.right);
	printf("\033[%d;%dH%04d\n", 15, 46, debug->js.ranalog.left);
	printf("\033[%d;%dH%d\n", 15, 52, debug->js.ranalog.press);
	printf("\033[%d;%dH%04d\n", 15, 55, debug->js.ranalog.right);
	printf("\033[%d;%dH%d\n", 17, 30, debug->js.dpad.down);
	printf("\033[%d;%dH%04d\n", 17, 51, debug->js.ranalog.down);
	//printf("\033[%d;%dH%d\n", 22, 26, debug->left_motor.a_port);
	//printf("\033[%d;%dH%d\n", 22, 30, debug->left_motor.b_port);
	//printf("\033[%d;%dH%04d\n", 22, 36, debug->left_motor.pwm);
	//printf("\033[%d;%dH%d\n", 22, 44, debug->right_motor.a_port);
	//printf("\033[%d;%dH%d\n", 22, 48, debug->right_motor.b_port);
	//printf("\033[%d;%dH%04d\n", 22, 54, debug->right_motor.pwm);
	//printf("\033[%d;%dH%09lld\n", 23, 31, debug->left_motor.posCounter);
	//printf("\033[%d;%dH%09lld\n", 23, 49, debug->right_motor.posCounter);
	//printf("\033[%d;%dH%011.3f\n", 24, 29, debug->left_motor.displacement);
	//printf("\033[%d;%dH%011.3f\n", 24, 47, debug->right_motor.displacement);
	//printf("\033[%d;%dH%+06.3f\n", 25, 30, debug->left_motor.raw_speed);
	//printf("\033[%d;%dH%+06.3f\n", 25, 48, debug->right_motor.raw_speed);
	//printf("\033[%d;%dH%+05.2f\n", 26, 31, debug->left_motor.filtered_speed);
	//printf("\033[%d;%dH%+05.2f\n", 26, 49, debug->right_motor.filtered_speed);
	if(debug->ir.left)
		printf("\033[%d;%dHOn \n", 27, 17);
	else
		printf("\033[%d;%dHOff\n", 27, 17);
	switch(debug->led_state)
	{
		case BLUETOOTH:
			printf("\033[%d;%dHBluetooth\n", 27, 39);
			break;
		case STANDBY:
			printf("\033[%d;%dHStandby  \n", 27, 39);
			break;
		default:
			printf("\033[%d;%dHOff      \n", 27, 39);
			break;
	}
	if(debug->ir.right)
		printf("\033[%d;%dHOn \n", 27, 62);
	else
		printf("\033[%d;%dHOff\n", 27, 62);
	
	// Code to print Yaw-Pitch-Roll values here
	
	printf("\033[%d;%dH%07.2f\n", 29, 10, debug->imu.yaw);
	printf("\033[%d;%dH%07.2f\n", 29, 20, debug->imu.pitch);
	printf("\033[%d;%dH%07.2f\n", 29, 30, debug->imu.roll);

	printf("\033[%d;%dH%07.2f\n", 30, 10, debug->imu.gyro.treatedZ);
	printf("\033[%d;%dH%07.2f\n", 30, 20, debug->imu.gyro.treatedY);
	printf("\033[%d;%dH%07.2f\n", 30, 30, debug->imu.gyro.treatedX);

	printf("\033[%d;%dH%07.2f\n", 33, 6, debug->imu.gyro.treatedX);
	printf("\033[%d;%dH%07.2f\n", 33, 18, debug->imu.accel.treatedX);
	printf("\033[%d;%dH%07.2f\n", 33, 30, debug->imu.magnet.treatedX);
	
	printf("\033[%d;%dH%07.2f\n", 34, 6, debug->imu.gyro.treatedY);
	printf("\033[%d;%dH%07.2f\n", 34, 18, debug->imu.accel.treatedY);
	printf("\033[%d;%dH%07.2f\n", 34, 30, debug->imu.magnet.treatedY);

	printf("\033[%d;%dH%07.2f\n", 35, 6, debug->imu.gyro.treatedZ);
	printf("\033[%d;%dH%07.2f\n", 35, 18, debug->imu.accel.treatedZ);
	printf("\033[%d;%dH%07.2f\n", 35, 30, debug->imu.magnet.treatedZ);

	printf("\033[%d;%dH%06d\n", 36, 8, debug->imu.gyro.rawX);
	printf("\033[%d;%dH%06d\n", 36, 20, debug->imu.accel.rawX);
	printf("\033[%d;%dH%06d\n", 36, 32, debug->imu.magnet.rawX);

	printf("\033[%d;%dH%06d\n", 37, 8, debug->imu.gyro.rawY);
	printf("\033[%d;%dH%06d\n", 37, 20, debug->imu.accel.rawY);
	printf("\033[%d;%dH%06d\n", 37, 32, debug->imu.magnet.rawY);

	printf("\033[%d;%dH%06d\n", 38, 8, debug->imu.gyro.rawZ);
	printf("\033[%d;%dH%06d\n", 38, 20, debug->imu.accel.rawZ);
	printf("\033[%d;%dH%06d\n", 38, 32, debug->imu.magnet.rawZ);
	
	printf("\033[%d;%dH%08.6f\n", 39, 13, debug->imu.dt);
}

void print_message(char mess[], int num)
{
	if(num > 0 && num <= 12)
		printf("\033[%d;%dH%-36s\n", 27+num, 44, mess);
}

/* Supervisory system portion of the code */

#define PORT_SUPERV 9001 //Its Over 9000!!!
#define PORT_MATLAB 9002 //Its Over 9000!!!

int server_fd_s, new_socket_s;
int server_fd_m, new_socket_m;
struct sockaddr_in address_m;
struct sockaddr_in address_s;
int opt = 1;
int addrlen_m = sizeof(address_m);
int addrlen_s = sizeof(address_s);

int init_matlab()
{
	// Creating socket file descriptor
    if ((server_fd_m = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        return -1;//tratar erro depois
    }
    // Forcefully attaching socket to the port 9001
    if (setsockopt(server_fd_m, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        return -1;//tratar erro depois
    }
    address_m.sin_family = AF_INET;
    address_m.sin_addr.s_addr = INADDR_ANY;
    address_m.sin_port = htons( PORT_MATLAB );

	if (bind(server_fd_m, (struct sockaddr *)&address_m, sizeof(address_m))<0)
    {
        return -1;//tratar erro depois
    }
    
    if (listen(server_fd_m, 3) < 0)
    {
        return -1;//tratar erro depois
    }

    if ((new_socket_m = accept(server_fd_m, (struct sockaddr *)&address_m, (socklen_t*)&addrlen_m))<0)
    {
        return -1;//tratar erro depois
    }
}

int init_supervisory()
{
	// Creating socket file descriptor
    //printf("0.1\n");
    if ((server_fd_s = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        //printf("0.1.1\n");
        return -1;//tratar erro depois
    }
    // Forcefully attaching socket to the port 9001
    
   // printf("0.2\n");
    if (setsockopt(server_fd_s, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        //printf("0.2.1\n");
        return -1;//tratar erro depois
    }
    address_s.sin_family = AF_INET;
    address_s.sin_addr.s_addr = INADDR_ANY;
    address_s.sin_port = htons( PORT_SUPERV );

	//printf("0.3\n");
	if (bind(server_fd_s, (struct sockaddr *)&address_s, sizeof(address_s))<0)
    {
        //printf("0.3.1\n");
        return -1;//tratar erro depois
    }
    
   // printf("0.4\n");
    if (listen(server_fd_s, 3) < 0)
    {
        //printf("0.4.1\n");
        return -1;//tratar erro depois
    }

    //printf("0.5\n");
    if ((new_socket_s = accept(server_fd_s, (struct sockaddr *)&address_s, (socklen_t*)&addrlen_s))<0)
    {
        //printf("0.5.1\n");
        return -1;//tratar erro depois
    }
    //printf("0.6\n");
}

#define STRSIZE_MATLAB 72
#define STRSIZE_SUPERV 41//corrigir valores do supervisório depois
void send_matlab_message(struct debug_data* debug)
{
	char mess[STRSIZE_MATLAB];
	char buffer[1024] = {0};
	int ret;

	snprintf(mess, STRSIZE_MATLAB,
			"%09f;%09f;%09f;%09f;%09f;%09f;%010d;",
			debug->imu.gyro.treatedX,
			debug->imu.gyro.treatedY,
			debug->imu.gyro.treatedZ,
			debug->imu.yaw,
			debug->imu.pitch,
			debug->imu.roll,
			debug->imu.last_update);

	write(new_socket_m , mess , strlen(mess)); // Optimization: replace strlen call with STRSIZE constant
    ret = read(new_socket_m , buffer, 1024);
}

int send_superv_message(struct debug_data* debug)
{
	char mess[STRSIZE_SUPERV];
	char buffer[1024] = {0};
	int ret;

	//printf("1\n");

	struct pollfd fd;
	int rv;
	
	//printf("2\n");

	snprintf(mess, STRSIZE_SUPERV,
		"%09f;%09f;%09f;%9lf;",
		kalman.roll,
		kalman.pitch,	// Zeroes to be replaced by Kalman Filter output when we implement it
		kalman.yaw,
		imu.dt);

	//printf("3\n");

	write(new_socket_s , mess , strlen(mess)); // Optimization: replace strlen call with STRSIZE constant
    
	//printf("4\n");

	fd.fd = new_socket_s;
	fd.events = POLLIN;

	//printf("5\n");

	rv = poll(&fd, 1, 1000); // 1 second for timeout

  	//printf("6\n");

  	

  	//printf("7 - %d\n", rv);

  	if(rv == -1)
		return -1; /* an error accured */
	else if(rv == 0)
		return -1;  /* a timeout occured */

	//printf("8\n");

	ret = read(new_socket_s , buffer, 1024); /* there was data to read */
    
    //printf("%s\n", buffer);

    //printf("9\n");

    return 0;
}