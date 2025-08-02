#include "includes.h"
#include <time.h>
#include <math.h>

#define TASK_STK_SIZE 1024
#define TASK_STK_CNT 7
#define MSG_QUEUE_SIZE 10

#define INPUT_PRIO 30
#define PRINT_PRIO 20
#define CALCULATE_FAMILY_PRIO 10


OS_STK TaskStk[TASK_STK_CNT][TASK_STK_SIZE];
OS_EVENT* inputSem;
OS_EVENT* msgQueue;
void *msgQueueTbl[MSG_QUEUE_SIZE];

char err;
double speed = 0;
double accelaration = 0;
double gear_ratio[5] = { 0.0075, 0.015, 0.025, 0.04, 0.0575 };
short rpmLimit[10] = { 2000, 2000,2000,2000,2000,2000,2500,3000,3500,4000 };
short rpm = 600;
char break_pad = 0;
char accel_pad = 0;
char gear_in = 0;
char event_in = 0;
short gear_state = 'P';	// bit[7:0]: gearState(ASKII alphabet), auxilary gear state(number:1~5)

// task functions
void inputhandler(void* data);	// get accelater input and increase rpm information
void SUAhandler(void* data);
void gearhandler(void* data);	// get break input and decrease the speed information
void rpmhandler(void* data);	// adjust rpm information  
void speedhandler(void* data);	// adjust speed information
void eventhandler(void* data);	// if sudden unintended acceleration(SUA) is occured, it makes event signal
void monitorPrinter(void* data);	// it prints the state of the car

// kernel functions
void DispInit();
void DispClear();
void eventWriter(void* data);	// once get the event signal is occured, it writes the event state

int main()
{
	// initialize the OS
	OSInit();
	DispInit();
	DispClear();

	// OS Event initialize
	inputSem = OSSemCreate(1);  // 세마포어 생성 및 초기값 1로 설정
	msgQueue = OSQCreate(msgQueueTbl, MSG_QUEUE_SIZE); // 메시지 큐 생성

	// create the tasks
	OSTaskCreate(inputhandler, (void*)0, &TaskStk[0][TASK_STK_SIZE - 1], INPUT_PRIO);
	OSTaskCreate(SUAhandler, (void*)0, &TaskStk[1][TASK_STK_SIZE - 1], INPUT_PRIO - 1);
	OSTaskCreate(monitorPrinter, (void*)0, &TaskStk[2][TASK_STK_SIZE - 1], PRINT_PRIO);
	OSTaskCreate(eventhandler, (void*)0, &TaskStk[3][TASK_STK_SIZE - 1], PRINT_PRIO - 1);
	OSTaskCreate(rpmhandler, (void*)0, &TaskStk[4][TASK_STK_SIZE - 1], CALCULATE_FAMILY_PRIO);
	OSTaskCreate(gearhandler, (void*)0, &TaskStk[5][TASK_STK_SIZE - 1], CALCULATE_FAMILY_PRIO + 1);
	OSTaskCreate(speedhandler, (void*)0, &TaskStk[6][TASK_STK_SIZE - 1], CALCULATE_FAMILY_PRIO + 2);


	// start the OS
	OSStart();
	return 0;
}

void inputhandler(void* data)
{
	short key;
	for (;1;)
	{
		OSSemPend(inputSem, 0, &err);  // 세마포어 획득
		// get the input charctors from the keyboard
		if (PC_GetKey(&key))
		{
			switch ((char)key)
			{
			case'R':
			case'r':
				if (break_pad < 9) break_pad++;
				break;
			case'F':
			case'f':
				if (break_pad > 0) break_pad--;
				PC_DispStr(5, 23, "                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				break;
			case'T':
			case't':
				if (accel_pad < 9) accel_pad++;
				break;
			case'G':
			case'g':
				if (accel_pad > 0) accel_pad--;
				break;
			case'7':
				gear_in = 'P';
				break;
			case'Y':
			case'y':
				gear_in = 'R';
				break;
			case'H':
			case'h':
				gear_in = 'N';
				break;
			case'N':
			case'n':
				gear_in = 'D';
				break;
			case'J':
			case'j':
				gear_in = '+';
				break;
			case'm':
			case'M':
				gear_in = '-';
				break;
			case'1':
				event_in = 1;
				break;
			case 0x1B:
				exit(0);
			default:
				break;
			}
		}
		OSSemPost(inputSem);  // 세마포어 반환
		OSTimeDly(1);
	}
}
void SUAhandler(void* data)
{
	short key;
	for (;;)
	{
		if (PC_GetKey(&key))
		{
			if ((char)key == '1')
			{
				OSSemPend(inputSem, 0, &err);  // 세마포어 획득

				while ((char)key != '2')
				{
					PC_GetKey(&key);
					PC_DispStr(5, 22, "unintended acceleration", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					gear_state = 'D';
					rpm += 150;
					OSTimeDly(1);
				}
				OSSemPost(inputSem);  // 세마포어 반환
			}
		}
		OSTimeDly(7);
	}
}

void rpmhandler(void* data)
{
	// adjust rpm info with input signal(accelator) and speed, gear state
	for (;;)
	{
		if (rpm > 7000) rpm = 7000;
		rpm += (5 - (gear_state >> 8)) * accel_pad * 3;
		if (rpm < 600) rpm = 600;
		rpm -= (short)round(exp((double)rpm / 1000)) + gear_ratio[gear_state >> 8];
		OSTimeDly(10);	// suspend to see other messages
	}
}

void gearhandler(void* data)
{
	// adjust gear info with input signal(gear) and speed, gear state
	for (;;)
	{
		switch (gear_in)
		{
		case'P':
		case'R':
		case'N':
			gear_state = 0;
		case'D':
		case'M':
			gear_state = gear_in | gear_state & 0xFF00;
			gear_in = 0;
			break;
		case'+':
		case'-':
			if((gear_state & 0xFF) == 'D')
				gear_state = 'M' | gear_state & 0xFF00;
		default:
			break;
		}
		switch (gear_state&0xFF)
		{
		case'M':
			speed = rpm * gear_ratio[gear_state >> 8];
			if (gear_in == '+' && (gear_state >> 8) < 4 && speed / gear_ratio[(gear_state >> 8) + 1] > 600)
			{
				gear_state += 1 << 8;
				gear_in = 0;
			}
			if (gear_in == '-' && (gear_state >> 8) > 0 && speed / gear_ratio[(gear_state >> 8) - 1] < 5000)
			{
				gear_state -= 1 << 8;;
				gear_in = 0;
			}
			break;
		case'D':
			speed = rpm * gear_ratio[gear_state >> 8];
			if ((gear_state >> 8) < 4 && rpm > rpmLimit[accel_pad])
			{
				gear_state += 1 << 8;
				//rpm = speed / gear_ratio[gear_state >> 8];
			}
			if ((gear_state >> 8) > 0 && rpm < 1000)
			{
				gear_state -= 1 << 8;
				//rpm = speed / gear_ratio[gear_state >> 8];
			}
		case'N':
			break;
		case'R':
			speed = rpm * gear_ratio[0];
			break;
		case'P':
			speed = 0;
		default:
			break;
		}
		OSTimeDly(10);	// suspend to see other messages
	}
}

void speedhandler(void* data)
{
	// adjust speed info with input signal(break) and speed, gear state
	for (;;)
	{
		speed -= break_pad;
		if (speed < 0) speed = 0;
		if ((gear_state & 0xFF) == 'R' || (gear_state & 0xFF) == 'D' || (gear_state & 0xFF) == 'M')
			rpm = speed / gear_ratio[gear_state >> 8];	// synchronize rpm with the speed
		if (rpm < 600) rpm = 600;

		OSTimeDly(10);	// suspend to see other messages
	}
}

void eventhandler(void* data)
{
	char speedhistory[3] = { 0, };
	int speedPrev = 0;
	// make event signal if SUA is occured
	for (;;) 
	{
		speedhistory[2] = speedhistory[1];
		speedhistory[1] = speedhistory[0];
		if (accel_pad == 0 && (int)speed > speedPrev) 
		{
			speedhistory[0] = 1;
		}
		else
		{
			speedhistory[0] = 0;
		}
		if (speedhistory[2] == 1 && speedhistory[1] == 1 && speedhistory[0] == 1)
		{
			INT8U msg = 1;
			eventWriter(NULL);
			OSQPost(msgQueue, (void*)&msg); // 메시지 큐에 급발진 상황임을 알리는 메시지 전송
			OSTimeDly(5); // 급발진 상황이 일정 시간 지속되도록 딜레이 추가
		}
		speedPrev = speed;
		OSTimeDly(8);
	}
}

void eventWriter(void* data)
{
	// if an event is occured, writes now state at external storage(at this program case, it's file output)
	break_pad = 9; // 브레이크 패드 값을 최대로 설정하여 급제동
}

void monitorPrinter(void* data)
{
	char msg[5] = { 0, };
	INT8U* msgData;

	for (;1;)
	{
		int i, cnt;
		char askiiAlphabet[2] = { 0, };
		INT8U msg[40];
		// print the states, gear, rpm, speed, accelator and break etc.
		DispClear();
		msg[0] = ((int)speed / 1000) + '0';
		msg[1] = (((int)speed % 1000) / 100) + '0';
		msg[2] = (((int)speed % 100) / 10) + '0';
		msg[3] = ((int)speed % 10) + '0';
		msg[4] = 0;
		PC_DispStr(10, 2, msg, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		msg[0] = (rpm / 1000) + '0';
		msg[1] = ((rpm % 1000) / 100) + '0';
		msg[2] = ((rpm % 100) / 10) + '0';
		msg[3] = (rpm % 10) + '0';
		msg[4] = 0;
		PC_DispStr(16, 8, msg, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		// display speed
		cnt = ((int)speed / 10) * 3 + (((int)speed % 10) + 2) / 3;
		if (cnt > 35) cnt = 35;
		for (i = 0; i < cnt; i++)
			PC_DispStr(4 + (i << 1), 5, "■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		// display rpm
		cnt = (rpm + 199) / 200;
		if (cnt > 35) cnt = 35;
		for (i = 0; i < cnt; i++)
			PC_DispStr(4 + (i << 1), 11, "■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		// display break pad
		for (i = 0; i < break_pad; i++)
			PC_DispStr(66, 22 - i, "■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		// display accel pad
		for (i = 0; i < accel_pad; i++)
			PC_DispStr(70, 22 - i, "■", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		// display gearState
		askiiAlphabet[0] = (char)(gear_state & 0xFF);
		//askiiAlphabet[0] = gear_in;
		PC_DispStr(74, 20, askiiAlphabet, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		if ((gear_state & 0xFF) == 'M' || (gear_state & 0xFF) == 'D')
		{
			askiiAlphabet[0] = (char)(((gear_state >> 8) & 0xFF) + '0');
			PC_DispStr(74, 21, askiiAlphabet, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}
		PC_DispStr(79, 24, " ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		msgData = OSQPend(msgQueue, 1, &err);
		if (err == OS_NO_ERR && *(INT8U*)msgData == 1) {
			PC_DispStr(5, 23, "Emergency break opened!", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		}

		OSTimeDly(5);
	}
}

//초기화면 그리기
void DispInit()
{
	PC_DispStr(0,  0, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  1, "                           car auxilary unit simulator                          ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  2, "    speed                                                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  3, "    0     10    20    30    40    50    60    70    80    90   100   110        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  4, "    |=====|=====|=====|=====|=====|=====|=====|=====|=====|=====|=====|====     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  6, "    =======================================================================     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  8, "    rpm (x1000)                                                                 ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0,  9, "    0         1         2         3         4         5         6         7     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 10, "    |=========|=========|=========|=========|=========|=========|=========|     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 12, "    =======================================================================     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 13, "                                                                  ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 14, "     esc: exit                   ---GEAR---                       ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 15, "     R : break+(B)               7:Parking                        ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 16, "     F : break-(B)               Y:Reverse                        ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 17, "     T : accel+(A)               H:Neutral                        ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 18, "     G : accel-(A)               N:Drive                          ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 19, "     1 : unintended accel.       J:+(Manual)                      ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 20, "     2 : reset unintented acc.   M:-(Manual)                      ■  ■  D     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 21, "                                                                  ■  ■  1     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 22, "                                                                  ■  ■        ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 23, "                                                                  B   A   G     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 24, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
}

void DispClear()
{
	PC_DispStr(0, 2, "    speed                                                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 8, "    rpm (x1000)                                                                 ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 13, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 14, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 15, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 16, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 17, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 18, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 19, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 20, "         ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 21, "         ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr(66, 22, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 5, 22, "                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	//PC_DispStr( 5, 23, "                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
}