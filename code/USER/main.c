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

#include "eat_snake_app.h"

#define Edition	"Learn V0.1"

#define Supper_Stack	128
#define Large_Stack		64
#define Big_Stack		48
#define Medium_Stack	32
#define Small_Stack		16
#define Mini_Stack		8

TaskHandle_t System_Init_TaskHandle = NULL;
TaskHandle_t LED_FeedBack_TaskHandle = NULL;
TaskHandle_t OLED_Refresh_TaskHandle = NULL;

SemaphoreHandle_t	OLED_Refresh_flag = NULL;

void System_Init_Task(void*ptr);
void LED_Shine_Task(void*ptr);
void OLED_Refresh_Task(void*ptr);

//任务相关参数
LED_Shine_Data	FeedBack = {0,100,300};
uint8_t Refresh_Cycle = 17;

int main(void)
{
	xTaskCreate(
		System_Init_Task,
		"SystemInit",
		Supper_Stack,
		NULL,
		15,
		&System_Init_TaskHandle
	);
	vTaskStartScheduler();
	while(1);
}

void System_Init_Task(void*ptr)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_LED_Init();
	BSP_Usart_Init();
	BSP_Key_Init();
	BSP_OLED12864_Init();

	OLED12864_Draw_aImg(0,0,gImage_freeRTOS_img);
	OLED12864_Show_String(7,0,"BSP_Init_PASS",1);
	OLED12864_Refresh();
	printf("BSP_Init_PASS\r\n");

	OLED_Refresh_flag = xSemaphoreCreateBinary();

	xTaskCreate(
		LED_Shine_Task,
		"FeedBack",
		Small_Stack,
		&FeedBack,
		1,
		&LED_FeedBack_TaskHandle
	);
	xTaskCreate(
		OLED_Refresh_Task,
		"Refresh",
		Large_Stack,
		&Refresh_Cycle,
		14,
		&OLED_Refresh_TaskHandle
	);

	if(LED_FeedBack_TaskHandle == NULL ||
		OLED_Refresh_flag == NULL)
	{
		printf("System_Init_False\r\n");
		OLED12864_Clear();
		OLED12864_Show_String(0,0,"System_Init_Flase",1);
		OLED12864_Refresh();
		while(1);
	}else
	{
		printf("System_Init_PASS\r\n");
		OLED12864_Clear();
		OLED12864_Draw_aImg(0,0,gImage_earth);
		vTaskDelete(NULL);
	}
}

void LED_Shine_Task(void*ptr)
{
	LED_Shine_Data	Data;
	Data = *(LED_Shine_Data*)ptr;
	while(1)
	{
		static TickType_t time;
		time = xTaskGetTickCount();
		LED_CTR(Data.num,LED_ON);
		xTaskDelayUntil(&time,Data.on_time/portTICK_RATE_MS);
		LED_CTR(Data.num,LED_OFF);
		xTaskDelayUntil(&time,(Data.cycle-Data.on_time)/portTICK_RATE_MS);
	}
}

void OLED_Refresh_Task(void*ptr)
{
	uint8_t cycle;
	cycle = *(uint8_t*)ptr;
	while(1)
	{
		static TickType_t time;
		xSemaphoreTake(OLED_Refresh_flag,1000/portTICK_RATE_MS);
		OLED12864_Refresh();
		time = xTaskGetTickCount();
		xTaskDelayUntil(&time,cycle/portTICK_RATE_MS);
	}
}
