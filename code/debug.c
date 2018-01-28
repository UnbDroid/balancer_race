#include <stdio.h>
//#include "led.c"
//#include "motor.c"
//#include "jstick.c"
//#include "sensors.c"

struct debug_data {
	struct joystick js;
	struct motor left_motor;
	struct motor right_motor;
	struct infrared ir;
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
}