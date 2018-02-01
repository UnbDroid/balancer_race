#include <stdio.h>

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
	printf("     Gyro         Accel       Magnet     1:                                     \n");
	printf("  X   *******  X   *******  X   *******  2:                                     \n");
	printf("  Y   *******  Y   *******  Y   *******  3:                                     \n");
	printf("  Z   *******  Z   *******  Z   *******  4:                                     \n");
	printf("  X'  *******  X'  *******  X'  *******  5:                                     \n");
	printf("  Y'  *******  Y'  *******  Y'  *******                                         \n");
	printf("  Z'  *******  Z'  *******  Z'  *******                                         \n");
	printf("  X'' *******  X'' *******  X'' *******                                         \n");
	printf("  Y'' *******  Y'' *******  Y'' *******                                         \n");
	printf("  Z'' *******  Z'' *******  Z'' *******                                         \n");
	printf("  rawX ******  rawX ******  rawX ******                                         \n");
	printf("  rawY ******  rawY ******  rawY ******                                         \n");
	printf("  rawZ ******  rawZ ******  rawZ ******                                         \n");
	printf("  IMU Time: ******us                                                            \n");
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
	printf("\033[%d;%dH%07.2f\n", 29, 7, debug->imu.gyro.posX);
	if(debug->imu.accel.posX < 9999 && debug->imu.accel.posX > -9999) printf("\033[%d;%dH%07.2f\n", 29, 20, debug->imu.accel.posX);
	printf("\033[%d;%dH%07.2f\n", 29, 33, debug->imu.magnet.posX);
	
	printf("\033[%d;%dH%07.2f\n", 30, 7, debug->imu.gyro.posY);
	if(debug->imu.accel.posY < 9999 && debug->imu.accel.posY > -9999) printf("\033[%d;%dH%07.2f\n", 30, 20, debug->imu.accel.posY);
	printf("\033[%d;%dH%07.2f\n", 30, 33, debug->imu.magnet.posY);

	printf("\033[%d;%dH%07.2f\n", 31, 7, debug->imu.gyro.posZ);
	if(debug->imu.accel.posZ < 9999 && debug->imu.accel.posZ > -9999) printf("\033[%d;%dH%07.2f\n", 31, 20, debug->imu.accel.posZ);
	printf("\033[%d;%dH%07.2f\n", 31, 33, debug->imu.magnet.posZ);

	printf("\033[%d;%dH%07.2f\n", 32, 7, debug->imu.gyro.velX);
	printf("\033[%d;%dH%07.2f\n", 32, 20, debug->imu.accel.velX);
	printf("\033[%d;%dH%07.2f\n", 32, 33, debug->imu.magnet.velX);

	printf("\033[%d;%dH%07.2f\n", 33, 7, debug->imu.gyro.velY);
	printf("\033[%d;%dH%07.2f\n", 33, 20, debug->imu.accel.velY);
	printf("\033[%d;%dH%07.2f\n", 33, 33, debug->imu.magnet.velY);

	printf("\033[%d;%dH%07.2f\n", 34, 7, debug->imu.gyro.velZ);
	printf("\033[%d;%dH%07.2f\n", 34, 20, debug->imu.accel.velZ);
	printf("\033[%d;%dH%07.2f\n", 34, 33, debug->imu.magnet.velZ);

	printf("\033[%d;%dH%07.2f\n", 35, 7, debug->imu.gyro.accX);
	printf("\033[%d;%dH%07.2f\n", 35, 20, debug->imu.accel.accX);
	printf("\033[%d;%dH%07.2f\n", 35, 33, debug->imu.magnet.accX);

	printf("\033[%d;%dH%07.2f\n", 36, 7, debug->imu.gyro.accY);
	printf("\033[%d;%dH%07.2f\n", 36, 20, debug->imu.accel.accY);
	printf("\033[%d;%dH%07.2f\n", 36, 33, debug->imu.magnet.accY);

	printf("\033[%d;%dH%07.2f\n", 37, 7, debug->imu.gyro.accZ);
	printf("\033[%d;%dH%07.2f\n", 37, 20, debug->imu.accel.accZ);
	printf("\033[%d;%dH%07.2f\n", 37, 33, debug->imu.magnet.accZ);

	printf("\033[%d;%dH%06d\n", 38, 8, debug->imu.gyro.rawX);
	printf("\033[%d;%dH%06d\n", 38, 21, debug->imu.accel.rawX);
	printf("\033[%d;%dH%06d\n", 38, 34, debug->imu.magnet.rawX);

	printf("\033[%d;%dH%06d\n", 39, 8, debug->imu.gyro.rawY);
	printf("\033[%d;%dH%06d\n", 39, 21, debug->imu.accel.rawY);
	printf("\033[%d;%dH%06d\n", 39, 34, debug->imu.magnet.rawY);

	printf("\033[%d;%dH%06d\n", 40, 8, debug->imu.gyro.rawZ);
	printf("\033[%d;%dH%06d\n", 40, 21, debug->imu.accel.rawZ);
	printf("\033[%d;%dH%06d\n", 40, 34, debug->imu.magnet.rawZ);

	printf("\033[%d;%dH%06ld\n", 41, 13, debug->imu.dt);
}

void print_message(char mess[], int num)
{
	switch(num)
	{
		case 1:
			//printf("\033[%d;%dH                                    \n", 28, 45);
			printf("\033[%d;%dH%-36s\n", 28, 45, mess);
			break;
		case 2:
			//printf("\033[%d;%dH                                    \n", 29, 45);
			printf("\033[%d;%dH%-36s\n", 29, 45, mess);
			break;
		case 3:
			//printf("\033[%d;%dH                                    \n", 30, 45);
			printf("\033[%d;%dH%-36s\n", 30, 45, mess);
			break;
		case 4:
			//printf("\033[%d;%dH                                    \n", 31, 45);
			printf("\033[%d;%dH%-36s\n", 31, 45, mess);
			break;
		case 5:
			//printf("\033[%d;%dH                                    \n", 32, 45);
			printf("\033[%d;%dH%-36s\n", 32, 45, mess);
			break;
	}
}