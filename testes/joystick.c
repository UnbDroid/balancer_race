#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define DEBUG 1

#define	DEAD_ZONE 3500
#define ZERO_VAL 32768

#define B_KEY 305
#define Y_KEY 307
#define X_KEY 306
#define A_KEY 304
#define LB_KEY 308
#define RB_KEY 309
#define LT_KEY 10
#define RT_KEY 9
#define DPAD_UD 17
#define DPAD_LR 16
#define LANALOG_UD 1
#define LANALOG_LR 0
#define LANALOG_PRESS 312
#define RANALOG_UD 5
#define RANALOG_LR 2
#define RANALOG_PRESS 313
#define START 311
#define SELECT 310
#define HOME 172

/*
struct input_event {
struct timeval time;
__u16 type; 
__u16 code;
__s32 value;
};
*/

char devname[] = "/dev/input/event0";

struct analog {
	int up, down, left, right;
	int press;
};

struct dpad {
	int up, down, left, right;
};

struct joystick {
	int device;
	int B, Y, X, A;
	int LB, RB, LT, RT;
	struct dpad dpad;
	struct analog lanalog;
	struct analog ranalog;
	int start, select;
	int home;
	int new_data;
	struct input_event last_event;
};

int exists(const char *fname)
{
	FILE *file;
	if (file = fopen(fname, "r"))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

void print_error()
{
	printf("\e[2J\e[H");
	//printf("       ________  _______   _______    ______   _______   __               \n      |        \\|       \\ |       \\  /      \\ |       \\ |  \\              \n      | $$$$$$$$| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\| $$$$$$$\\| $$              \n      | $$__    | $$__| $$| $$__| $$| $$  | $$| $$__| $$| $$              \n      | $$  \\   | $$    $$| $$    $$| $$  | $$| $$    $$| $$              \n      | $$$$$   | $$$$$$$\\| $$$$$$$\\| $$  | $$| $$$$$$$\\ \\$$              \n      | $$_____ | $$  | $$| $$  | $$| $$__/ $$| $$  | $$ __               \n      | $$     \\| $$  | $$| $$  | $$ \\$$    $$| $$  | $$|  \\              \n       \\$$$$$$$$ \\$$   \\$$ \\$$   \\$$  \\$$$$$$  \\$$   \\$$ \\$$              \n                                                                          \n                                                                          \n                                                                          \n  ______                                                       __         \n /      \\                                                     |  \\        \n|  $$$$$$\\  ______   _______   _______    ______    _______  _| $$_       \n| $$   \\$$ /      \\ |       \\ |       \\  /      \\  /       \\|   $$ \\      \n| $$      |  $$$$$$\\| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\|  $$$$$$$ \\$$$$$$      \n| $$   __ | $$  | $$| $$  | $$| $$  | $$| $$    $$| $$        | $$ __     \n| $$__/  \\| $$__/ $$| $$  | $$| $$  | $$| $$$$$$$$| $$_____   | $$|  \\    \n \\$$    $$ \\$$    $$| $$  | $$| $$  | $$ \\$$     \\ \\$$     \\   \\$$  $$    \n  \\$$$$$$   \\$$$$$$  \\$$   \\$$ \\$$   \\$$  \\$$$$$$$  \\$$$$$$$    \\$$$$     \n                                                                          \n                                                                          \n                                                                          \n    _____                                  __      __            __       \n   |     \\                                |  \\    |  \\          |  \\      \n    \\$$$$$  ______   __    __   _______  _| $$_    \\$$  _______ | $$   __ \n      | $$ /      \\ |  \\  |  \\ /       \\|   $$ \\  |  \\ /       \\| $$  /  \\\n __   | $$|  $$$$$$\\| $$  | $$|  $$$$$$$ \\$$$$$$  | $$|  $$$$$$$| $$_/  $$\n|  \\  | $$| $$  | $$| $$  | $$ \\$$    \\   | $$ __ | $$| $$      | $$   $$ \n| $$__| $$| $$__/ $$| $$__/ $$ _\\$$$$$$\\  | $$|  \\| $$| $$_____ | $$$$$$\\ \n \\$$    $$ \\$$    $$ \\$$    $$|       $$   \\$$  $$| $$ \\$$     \\| $$  \\$$\\\n  \\$$$$$$   \\$$$$$$  _\\$$$$$$$ \\$$$$$$$     \\$$$$  \\$$  \\$$$$$$$ \\$$   \\$$\n                    |  \\__| $$                                            \n                     \\$$    $$                                            \n                      \\$$$$$$                                             ");
	printf("       ________  _______   _______    ______   _______   __               \n      |        \\|       \\ |       \\  /      \\ |       \\ |  \\              \n      | $$$$$$$$| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\| $$$$$$$\\| $$              \n      | $$__    | $$__| $$| $$__| $$| $$  | $$| $$__| $$| $$              \n      | $$  \\   | $$    $$| $$    $$| $$  | $$| $$    $$| $$              \n      | $$$$$   | $$$$$$$\\| $$$$$$$\\| $$  | $$| $$$$$$$\\ \\$$              \n      | $$_____ | $$  | $$| $$  | $$| $$__/ $$| $$  | $$ __               \n      | $$     \\| $$  | $$| $$  | $$ \\$$    $$| $$  | $$|  \\              \n       \\$$$$$$$$ \\$$   \\$$ \\$$   \\$$  \\$$$$$$  \\$$   \\$$ \\$$              \n                                                                          \n                                                                          \n                                                                          \n  ______                                                       __         \n /      \\                                                     |  \\        \n|  $$$$$$\\  ______   _______   _______    ______    _______  _| $$_       \n| $$   \\$$ /      \\ |       \\ |       \\  /      \\  /       \\|   $$ \\      \n| $$      |  $$$$$$\\| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\|  $$$$$$$ \\$$$$$$      \n| $$   __ | $$  | $$| $$  | $$| $$  | $$| $$    $$| $$        | $$ __     \n| $$__/  \\| $$__/ $$| $$  | $$| $$  | $$| $$$$$$$$| $$_____   | $$|  \\    \n \\$$    $$ \\$$    $$| $$  | $$| $$  | $$ \\$$     \\ \\$$     \\   \\$$  $$    \n  \\$$$$$$   \\$$$$$$  \\$$   \\$$ \\$$   \\$$  \\$$$$$$$  \\$$$$$$$    \\$$$$     \n                                                                          \n                                                                          \n                                                                          \n    _____                                  __      __            __       \n   |     \\                                |  \\    |  \\          |  \\      \n    \\$$$$$  ______   __    __   _______  _| $$_    \\$$  _______ | $$   __ \n      | $$ /      \\ |  \\  |  \\ /       \\|   $$ \\  |  \\ /       \\| $$  /  \\\n __   | $$|  $$$$$$\\| $$  | $$|  $$$$$$$ \\$$$$$$  | $$|  $$$$$$$| $$_/  $$\n|  \\  | $$| $$  | $$| $$  | $$ \\$$    \\   | $$ __ | $$| $$      | $$   $$ \n| $$__| $$| $$__/ $$| $$__/ $$ _\\$$$$$$\\  | $$|  \\| $$| $$_____ | $$$$$$\\ \n \\$$    $$ \\$$    $$ \\$$    $$|       $$   \\$$  $$| $$ \\$$     \\| $$  \\$$\\\n  \\$$$$$$   \\$$$$$$  _\\$$$$$$$ \\$$$$$$$     \\$$$$  \\$$  \\$$$$$$$ \\$$   \\$$\n                    |  \\__| $$                                            \n                     \\$$    $$                                            \n                      \\$$$$$$\n");
	//printf("\033[%d;%dH%04d\n", 2, 8, 6);
	//printf("\033[%d;%dH%s\n", 0, 0, " ");
}

void update_print_js(struct joystick js)
{
	printf("\033[%d;%dH%d\n", 9, 66, js.B);
	printf("\033[%d;%dH%d\n", 7, 61, js.Y);
	printf("\033[%d;%dH%d\n", 9, 56, js.X);
	printf("\033[%d;%dH%d\n", 11, 61, js.A);
	printf("\033[%d;%dH%d\n", 2, 17, js.LB);
	printf("\033[%d;%dH%d\n", 2, 63, js.RB);
	printf("\033[%d;%dH%04d\n", 1, 20, js.LT);
	printf("\033[%d;%dH%04d\n", 1, 58, js.RT);
	printf("\033[%d;%dH%d\n", 9, 47, js.start);
	printf("\033[%d;%dH%d\n", 9, 35, js.select);
	printf("\033[%d;%dH%d\n", 4, 40, js.home);
	printf("\033[%d;%dH%d\n", 8, 19, js.lanalog.press);
	printf("\033[%d;%dH%d\n", 15, 52, js.ranalog.press);
	printf("\033[%d;%dH%d\n", 13, 30, js.dpad.up);
	printf("\033[%d;%dH%d\n", 17, 30, js.dpad.down);
	printf("\033[%d;%dH%d\n", 15, 26, js.dpad.left);
	printf("\033[%d;%dH%d\n", 15, 34, js.dpad.right);
	printf("\033[%d;%dH%04d\n", 6, 17, js.lanalog.up);
	printf("\033[%d;%dH%04d\n", 10, 17, js.lanalog.down);
	printf("\033[%d;%dH%04d\n", 8, 13, js.lanalog.left);
	printf("\033[%d;%dH%04d\n", 8, 22, js.lanalog.right);
	printf("\033[%d;%dH%04d\n", 13, 50, js.ranalog.up);
	printf("\033[%d;%dH%04d\n", 17, 50, js.ranalog.down);
	printf("\033[%d;%dH%04d\n", 15, 46, js.ranalog.left);
	printf("\033[%d;%dH%04d\n", 15, 55, js.ranalog.right);
	printf("\033[%d;%dHType:%4i Code:%4i Value:%5i Time_sec:%12li Time_usec:%7li\n", 28, 1,js.last_event.type, js.last_event.code, js.last_event.value, js.last_event.time.tv_sec, js.last_event.time.tv_usec);
}

void print_joystick(struct joystick js)
{
	printf("\e[2J\e[H");
	printf("                 ``%04d.....``                    ``.....%04d``                 \n", js.LT, js.RT);
	printf("             ```%d`````````  ..`````````..`````````..  ````````%d````             \n", js.LB, js.RB);
	printf("            .``````       `.`         \\ /          `.`       ``````.            \n");
	printf("          `.`              ``          %d           ``              `.`          \n", js.home);
	printf("         .                   ``       / \\        ``                   .         \n");
	printf("        .       %04d           ``````````````````         .-`-.        .        \n", js.lanalog.up);
	printf("       `         .`.                                     .``%d``.        `       \n",js.Y);
	printf("      ``    %04d`.%d.`%04d       ....        ....     `````.`-`.`````    ``      \n", js.lanalog.left, js.lanalog.press, js.lanalog.right);
	printf("      .           `             ..%d.`      `. %d.   `.` %d`.     .`%d `-    .      \n", js.select, js.start, js.X, js.B);
	printf("     .          %04d             ``          ``     `....```-```....`     .     \n", js.lanalog.down);
	printf("    ``                       .                           .`.%d.`.          ``    \n",js.A);
	printf("    .                      .` `.                          .:.:.`           .    \n");
	printf("   .                       . %d .                 %04d                       .   \n", js.dpad.up, js.ranalog.up);
	printf("  ``                   .````   ````.              .`.                       ``  \n");
	printf("  .                   `. %d       %d .`        %04d`.%d.`%04d                   .  \n", js.dpad.left, js.dpad.right, js.ranalog.left, js.ranalog.press, js.ranalog.right);
	printf(" `                      ````   ````                `                          . \n");
	printf(" .                         . %d .                 %04d                         . \n", js.dpad.down, js.ranalog.down);
	printf("``                          `.`                                                `\n");
	printf(".                                                                              .\n");
	printf(".                                                                              .\n");
	printf(".                     ````````````````````````````````````                     .\n");
	printf(".                  ```                                    ```                  .\n");
	printf(".                ``                                          ``                .\n");
	printf(".              ``                                              ``              .\n");
	printf(" .           ``                                                  ``           . \n");
	printf("  ``       ``                                                      ``       ``  \n");
	printf("    ```````                                                          ```````    \n");
	printf("Type:%4i Code:%4i Value:%5i Time_sec:%12li Time_usec:%7li\n", js.last_event.type, js.last_event.code, js.last_event.value, js.last_event.time.tv_sec, js.last_event.time.tv_usec);
}

void init_joystick(struct joystick* js, char devname[])
{
	if(!exists(devname))
	{
		print_error();
	}
	while(!exists(devname));
	{
		sleep(0.5);
	}
	js->device = open(devname, O_RDONLY);
	js->B = 0;
	js->Y = 0;
	js->X = 0;
	js->A = 0;
	js->LB = 0;
	js->RB = 0;
	js->LT = 0;
	js->RT = 0;
	js->dpad.up = 0;
	js->dpad.down = 0;
	js->dpad.left = 0;
	js->dpad.right = 0;
	js->lanalog.up = 0;
	js->lanalog.down = 0;
	js->lanalog.left = 0;
	js->lanalog.right = 0;
	js->lanalog.press = 0;
	js->ranalog.up = 0;
	js->ranalog.down = 0;
	js->ranalog.left = 0;
	js->ranalog.right = 0;
	js->ranalog.press = 0;
	js->start = 0;
	js->select = 0;
	js->home = 0;
	js->new_data = 0;
	print_joystick(*js);
}

int is_updated_js(struct joystick* js)
{
	if(!js->new_data)
		return 0;
	js->new_data = 0;
	return 1;
}

int analog_map(int v_max, int v_min, int value)
{
	if(v_max > v_min)
	{
		v_min += DEAD_ZONE;		
		if(value < v_min)
			return 0; 
	}
	else
	{
		v_min -= DEAD_ZONE;
		if(value > v_min)
			return 0;
	}
	return ((1023*(value - v_min))/(v_max - v_min)); 
}

void update_joystick(struct joystick* js)
{
	struct input_event ev;
	int temp;
	
	if(read(js->device, &ev, sizeof(ev)) != -1)
	{
		if(ev.type != 0)
		{
			js->last_event = ev;
			js->new_data = 1;
			switch(ev.code)
			{
				case B_KEY:
					js->B = ev.value;
					break;
				case Y_KEY:
					js->Y = ev.value;
					break;
				case X_KEY:
					js->X = ev.value;
					break;
				case A_KEY:
					js->A = ev.value;
					break;
				case LB_KEY:
					js->LB = ev.value;
					break;
				case RB_KEY:
					js->RB = ev.value;
					break;
				case LT_KEY:
					js->LT = ev.value;
					break;
				case RT_KEY:
					js->RT = ev.value;
					break;
				case START:
					js->start = ev.value;
					break;
				case SELECT:
					js->select = ev.value;
					break;
				case HOME:
					js->home = ev.value;
					break;
				case LANALOG_PRESS:
					js->lanalog.press = ev.value;
					break;
				case RANALOG_PRESS:
					js->ranalog.press = ev.value;
					break;
				case DPAD_UD:
					if(ev.value == 1)
					{
						js->dpad.up = 0;
						js->dpad.down = 1;
					} else if(ev.value == -1) {
						js->dpad.up = 1;
						js->dpad.down = 0;
					} else {
						js->dpad.up = 0;
						js->dpad.down = 0;
					}
					break;
				case DPAD_LR:
					if(ev.value == 1)
					{
						js->dpad.right = 1;
						js->dpad.left = 0;
					} else if(ev.value == -1) {
						js->dpad.right = 0;
						js->dpad.left = 1;
					} else {
						js->dpad.left = 0;
						js->dpad.right = 0;
					}
					break;
				case LANALOG_UD:
					if(ev.value < ZERO_VAL)
					{
						temp = analog_map(0, ZERO_VAL, ev.value);
						js->lanalog.down = 0;
						if(temp != js->lanalog.up)
							js->lanalog.up = temp;
						else
							js->new_data = 0;	
					} else {
						temp = analog_map(65535, ZERO_VAL, ev.value);
						js->lanalog.up = 0;
						if(temp != js->lanalog.down)
							js->lanalog.down = temp;
						else
							js->new_data = 0;
					}
					break;
				case LANALOG_LR:
					if(ev.value < ZERO_VAL)
					{
						temp = analog_map(0, ZERO_VAL, ev.value);
						js->lanalog.right = 0;
						if(temp != js->lanalog.left)
							js->lanalog.left = temp;
						else
							js->new_data = 0;
					} else {
						temp = analog_map(65535, ZERO_VAL, ev.value);
						js->lanalog.left = 0;
						if(temp != js->lanalog.right)
							js->lanalog.right = temp;
						else
							js->new_data = 0;
					}
					break;
				case RANALOG_UD:
					if(ev.value < ZERO_VAL)
					{
						temp = analog_map(0, ZERO_VAL, ev.value);
						js->ranalog.down = 0;
						if(temp != js->ranalog.up)
							js->ranalog.up = temp;
						else
							js->new_data = 0;
					} else {
						temp = analog_map(65535, ZERO_VAL, ev.value);
						js->ranalog.up = 0;
						if(temp != js->ranalog.down)
							js->ranalog.down = temp;
						else
							js->new_data = 0;
					}
					break;
				case RANALOG_LR:
					if(ev.value < ZERO_VAL)
					{
						temp = analog_map(0, ZERO_VAL, ev.value);
						js->ranalog.right = 0;
						if(temp != js->ranalog.left)
							js->ranalog.left = temp;
						else
							js->new_data = 0;
					} else {
						temp = analog_map(65535, ZERO_VAL, ev.value);
						js->ranalog.left = 0;
						if(temp != js->ranalog.right)
							js->ranalog.right = temp;
						else
							js->new_data = 0;
					}
					break;
			}
		}
	} else { //erro de leitura, provavelmente controle desconectou do rasp {
		close(js->device);
		init_joystick(js, devname);
	}
}

int mymillis(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

int main()
{
    struct joystick js;


 	init_joystick(&js, devname);
 	//js_old = js;

    while(1)
    {
        update_joystick(&js);
        if(is_updated_js(&js))
        {
	        if(DEBUG)
	        	update_print_js(js);
	        	//print_joystick(js);
	        	//printf("Type:%i Code:%i Value:%i Time_sec:%li Time_usec:%li\n", js.last_event.type, js.last_event.code, js.last_event.value, js.last_event.time.tv_sec, js.last_event.time.tv_usec);
		}
	}
	return 0;
}
