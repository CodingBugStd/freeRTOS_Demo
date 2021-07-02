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

	OLED12864_Draw_Img(0,0,128,49,gImage_freeRTOS_img);
	OLED12864_Refresh();

	soft_delay_ms(1000);

	//创建任务
	xTaskCreate(
		OLED_Refresh_Task,
		"OLED",
		32,
		NULL,
		15,
		&OLED_Refresh_TaskHandle
	);
	xTaskCreate(
		Line_Move_Task,
		"Move",
		32,
		NULL,
		10,
		&Line_Move_TaskHandle
	);

	if(OLED_Refresh_TaskHandle==NULL || Line_Move_TaskHandle==0)
	{
		printf("Erro\r\n");
		while(1);
	}
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
		static uint8_t sx = 0;
		static uint8_t sy = 0;
		OLED12864_Clear_Sbuffer();
		OLED12864_Draw_Line(sx,0,sx,63);
		OLED12864_Draw_Line(0,sy,127,sy);
		sx+=2;
		sy++;
		if(sx>=127)
			sx = 0;
		if(sy>=63
		)
			sy = 0;
		vTaskDelayUntil(&time,50/portTICK_RATE_MS);
	}
}
