#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "./socket.h"

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
	printf(" .           ``        Freq: **********  Freq: **********        ``           . \n");
	printf("  ``       `` Left IR  Speed: *********  Speed: ********* Right IR ``       ``  \n");
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
	printf("\033[%d;%dH%d\n", 22, 26, debug->left_motor.a_port);
	printf("\033[%d;%dH%d\n", 22, 30, debug->left_motor.b_port);
	printf("\033[%d;%dH%04d\n", 22, 36, debug->left_motor.pwm);
	printf("\033[%d;%dH%d\n", 22, 44, debug->right_motor.a_port);
	printf("\033[%d;%dH%d\n", 22, 48, debug->right_motor.b_port);
	printf("\033[%d;%dH%04d\n", 22, 54, debug->right_motor.pwm);
	printf("\033[%d;%dH%09lld\n", 23, 31, debug->left_motor.posCounter);
	printf("\033[%d;%dH%09lld\n", 23, 49, debug->right_motor.posCounter);
	printf("\033[%d;%dH%011.3f\n", 24, 29, debug->left_motor.displacement);
	printf("\033[%d;%dH%011.3f\n", 24, 47, debug->right_motor.displacement);
	printf("\033[%d;%dH%010ld\n", 25, 30, debug->left_motor.tickFreq);
	printf("\033[%d;%dH%010ld\n", 25, 48, debug->right_motor.tickFreq);
	printf("\033[%d;%dH%09.3f\n", 26, 31, debug->left_motor.speed);
	printf("\033[%d;%dH%09.3f\n", 26, 49, debug->right_motor.speed);
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
	
	printf("\033[%d;%dH%06ld\n", 39, 13, debug->imu.dt);
}

void print_message(char mess[], int num)
{
	if(num > 0 && num <= 12)
		printf("\033[%d;%dH%-36s\n", 27+num, 44, mess);
}

/* Supervisory system portion of the code */

#define PORT 9001 //Its Over 9000!!!

int server_fd, new_socket;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);

void init_supervisory()
{
	// Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

#define STRSIZE 103

void send_superv_message(struct debug_data* debug, int option)
{
	char mess[STRSIZE];
	char buffer[1024] = {0};
	int ret;

	if(option == DEF_SUPERVISORY)
	{
	// Format message to send data to the supervisory program
		
		snprintf(mess, STRSIZE,
			"%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f;%07.2f",
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			imu.yaw,
			imu.pitch,	// Zeroes to be replaced by Kalman Filter output when we implement it
			imu.roll);
		
	} else if(option == DEF_MATLAB) {
		
		snprintf(mess, STRSIZE,
			"%09f;%09f;%09f;%09f;%09f;%09f;%09f;%09f;%09f;%011d;",
			debug->imu.gyro.treatedX,
			debug->imu.gyro.treatedY,
			debug->imu.gyro.treatedZ,
			debug->imu.accel.treatedX,
			debug->imu.accel.treatedY,
			debug->imu.accel.treatedZ,
			debug->imu.magnet.treatedX,
			debug->imu.magnet.treatedY,
			debug->imu.magnet.treatedZ,
			debug->imu.last_update);
	} else { // in case the flag is set wrong, the function returns before sending the data
		return;
	}
	write(new_socket , mess , strlen(mess)); // Optimization: replace strlen call with STRSIZE constant
    ret = read(new_socket , buffer, 1024);

}