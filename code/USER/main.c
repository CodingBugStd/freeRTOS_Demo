#include "stm32f10x.h"

#include <stdio.h>
#include "self_type.h"
#include "soft_delay.h"
#include "bsp_usart.h"
#include "bsp_oled12864.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "Asoul_bmp.h"
#include "freeRTOS_bmp.h"
#include "asoul_jl_bmp.h"

#define System_Init_Task_Stack	64
#define Usart_Send_Task_Stack	64
#define Usart_Rx_Task_Stack		64
#define Cmd_Respond_Task_Stack	64
#define OLED_Refresh_Task_Stack	24

void System_Init_Task(void*ptr);		//初始化
void OLED_Refresh_Task(void*ptr);		//OLED屏幕刷新
void Line_Move_Task(void*ptr);			//直线刷新

TaskHandle_t SystemInit_TaskHandle = NULL;
TaskHandle_t OLED_Refresh_TaskHandle = NULL;
TaskHandle_t Line_Move_TaskHandle = NULL;

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

	OLED12864_Show_String(0,0,"SystemInit...",1);
	OLED12864_Show_String(7,64,"author:PBZ",1);
	OLED12864_Refresh();

	soft_delay_ms(700);

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
		128,
		NULL,
		10,
		&Line_Move_TaskHandle
	);

	if(OLED_Refresh_TaskHandle==NULL || Line_Move_TaskHandle==0)
	{
		printf("Erro\r\n");
		while(1);
	}
	OLED12864_Clear_Page(0);
	OLED12864_Show_String(0,0,"SystemInit-PASS",1);
	OLED12864_Refresh();
	soft_delay_ms(300);
	OLED12864_Draw_aImg(0,0,gImage_freeRTOS_img);
	OLED12864_Refresh();
	vTaskDelete(NULL);
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
		
		vTaskDelayUntil(&time,1000/portTICK_RATE_MS);
	}
}
