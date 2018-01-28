#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

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
	int disconnect;
	struct input_event last_event;
};

struct joystick js;

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
	printf("       ________  _______   _______    ______   _______   __               \n      |        \\|       \\ |       \\  /      \\ |       \\ |  \\              \n      | $$$$$$$$| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\| $$$$$$$\\| $$              \n      | $$__    | $$__| $$| $$__| $$| $$  | $$| $$__| $$| $$              \n      | $$  \\   | $$    $$| $$    $$| $$  | $$| $$    $$| $$              \n      | $$$$$   | $$$$$$$\\| $$$$$$$\\| $$  | $$| $$$$$$$\\ \\$$              \n      | $$_____ | $$  | $$| $$  | $$| $$__/ $$| $$  | $$ __               \n      | $$     \\| $$  | $$| $$  | $$ \\$$    $$| $$  | $$|  \\              \n       \\$$$$$$$$ \\$$   \\$$ \\$$   \\$$  \\$$$$$$  \\$$   \\$$ \\$$              \n                                                                          \n                                                                          \n                                                                          \n  ______                                                       __         \n /      \\                                                     |  \\        \n|  $$$$$$\\  ______   _______   _______    ______    _______  _| $$_       \n| $$   \\$$ /      \\ |       \\ |       \\  /      \\  /       \\|   $$ \\      \n| $$      |  $$$$$$\\| $$$$$$$\\| $$$$$$$\\|  $$$$$$\\|  $$$$$$$ \\$$$$$$      \n| $$   __ | $$  | $$| $$  | $$| $$  | $$| $$    $$| $$        | $$ __     \n| $$__/  \\| $$__/ $$| $$  | $$| $$  | $$| $$$$$$$$| $$_____   | $$|  \\    \n \\$$    $$ \\$$    $$| $$  | $$| $$  | $$ \\$$     \\ \\$$     \\   \\$$  $$    \n  \\$$$$$$   \\$$$$$$  \\$$   \\$$ \\$$   \\$$  \\$$$$$$$  \\$$$$$$$    \\$$$$     \n                                                                          \n                                                                          \n                                                                          \n    _____                                  __      __            __       \n   |     \\                                |  \\    |  \\          |  \\      \n    \\$$$$$  ______   __    __   _______  _| $$_    \\$$  _______ | $$   __ \n      | $$ /      \\ |  \\  |  \\ /       \\|   $$ \\  |  \\ /       \\| $$  /  \\\n __   | $$|  $$$$$$\\| $$  | $$|  $$$$$$$ \\$$$$$$  | $$|  $$$$$$$| $$_/  $$\n|  \\  | $$| $$  | $$| $$  | $$ \\$$    \\   | $$ __ | $$| $$      | $$   $$ \n| $$__| $$| $$__/ $$| $$__/ $$ _\\$$$$$$\\  | $$|  \\| $$| $$_____ | $$$$$$\\ \n \\$$    $$ \\$$    $$ \\$$    $$|       $$   \\$$  $$| $$ \\$$     \\| $$  \\$$\\\n  \\$$$$$$   \\$$$$$$  _\\$$$$$$$ \\$$$$$$$     \\$$$$  \\$$  \\$$$$$$$ \\$$   \\$$\n                    |  \\__| $$                                            \n                     \\$$    $$                                            \n                      \\$$$$$$\n");
}

void init_joystick(struct joystick* js, char devname[])
{
	while(!exists(devname)) sleep(0.5);
	
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
	js->disconnect = 0;
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
		js->disconnect = 1;
	}
}