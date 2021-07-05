#include "stm32f10x.h"

#include <stdio.h>
#include "self_type.h"
#include "soft_delay.h"
#include "bsp_usart.h"
#include "bsp_oled12864.h"
#include "bsp_key.h"
#include "bsp_led.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "freeRTOS_bmp.h"
#include "earth_bmp.h"

#define System_Init_Task_Stack	64


void System_Init_Task(void*ptr);		//初始化
void OLED_Refresh_Task(void*ptr);		//OLED屏幕刷新
void Line_Move_Task(void*ptr);			//直线刷新
void Key_Scanf_Task(void*ptr);			//按键输入

TaskHandle_t SystemInit_TaskHandle = NULL;
TaskHandle_t OLED_Refresh_TaskHandle = NULL;
TaskHandle_t Line_Move_TaskHandle = NULL;
TaskHandle_t Key_Scanf_TaskHandle = NULL;

QueueHandle_t	Cmd_QueueHandle = NULL;

int main(void)
{
	xTaskCreate(
		System_Init_Task,
		"SystemInit",
		System_Init_Task_Stack,
		NULL,
		15,
		&SystemInit_TaskHandle
	);
	vTaskStartScheduler();

	while(1);
}

void System_Init_Task(void*ptr)
{
	//硬件初始化和中断分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();
	BSP_OLED12864_Init();
	BSP_Key_Init();
	BSP_LED_Init();

	OLED12864_Show_String(0,0,"BSP Load PASS",1);
	OLED12864_Refresh();
	OLED12864_Show_String(1,0,"freeRTOS",1);
	OLED12864_Show_String(2,0,"System Loading...",1);
	OLED12864_Refresh();
	soft_delay_ms(100);

	Cmd_QueueHandle = xQueueCreate(10,sizeof(User_Cmd));
	//创建任务
	xTaskCreate(
		OLED_Refresh_Task,
		"OLED",
		64,
		NULL,
		2,
		&OLED_Refresh_TaskHandle
	);
	xTaskCreate(
		Line_Move_Task,
		"Move",
		64,
		NULL,
		10,
		&Line_Move_TaskHandle
	);
	xTaskCreate(
		Key_Scanf_Task,
		"Key",
		128,
		NULL,
		12,
		&Key_Scanf_TaskHandle
	);

	if(OLED_Refresh_TaskHandle==NULL ||
		Line_Move_TaskHandle==NULL || 
		Key_Scanf_TaskHandle==NULL ||
		Cmd_QueueHandle == NULL)
	{
		printf("Erro\r\n");
		OLED12864_Clear();
		OLED12864_Show_String(0,0,"Init False",1);
		OLED12864_Refresh();
		while(1);
	}
	OLED12864_Draw_aImg(0,0,gImage_freeRTOS_img);
	OLED12864_Show_String(7,0,"SystemInit-PASS",1);
	OLED12864_Refresh();
	soft_delay_ms(300);
	OLED12864_Clear();
	OLED12864_Draw_aImg(63,0,gImage_earth);
	OLED12864_Draw_Rect(62,0,64,63);
	vTaskDelete(NULL);
	taskYIELD();
}

void OLED_Refresh_Task(void*ptr)
{
	static TickType_t time;
	time = xTaskGetTickCount();
	while(1)
	{
		OLED12864_Refresh();
		vTaskDelayUntil(&time,20/portTICK_RATE_MS);
	}
}

void Line_Move_Task(void*prt)
{
	static TickType_t time;
	time = xTaskGetTickCount();
	while(1)
	{
		static uint16_t st = 0;
		OLED12864_Show_String(7,0,"Time:",1);
		OLED12864_Show_Num(7,30,st,1);
		st++;
		vTaskDelayUntil(&time,100/portTICK_RATE_MS);
	}
}

void Key_Scanf_Task(void*ptr)
{
	while(1)
	{
		static TickType_t time;
		static uint8_t cycle_num;
		time = xTaskGetTickCount();
		OLED12864_Clear_PageBlock(0,0,62);
		OLED12864_Show_Num(0,0,Read_Key_Input(0),1);
		OLED12864_Show_Num(0,6,Read_Key_Input(1),1);
		cycle_num++;
		if(cycle_num == 10)
		{
			LED_Reversal(0);
			cycle_num = 0;
		}
		vTaskDelayUntil(&time,50/portTICK_RATE_MS);
	}
}
