#include "stm32f10x.h"

#include <stdio.h>
#include "self_type.h"
#include "bsp_usart.h"
#include "bsp_oled12864.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#define System_Init_Task_Stack	64
#define Usart_Send_Task_Stack	64
#define Usart_Rx_Task_Stack		64
#define Cmd_Respond_Task_Stack	64
#define OLED_Refresh_Task_Stack	24

void System_Init_Task(void*ptr);		//初始化
void Usart_Send_Task(void*ptr);			//串口发送任务
void Usart_Rx_Task(void*ptr);			//串口接收任务
void OLED_Refresh_Task(void*ptr);		//OLED屏幕刷新
void Cmd_Respond_Task(void*ptr);		//用户指令处理任务

TaskHandle_t Init_TaskHandel = NULL;
TaskHandle_t Usart_Send_TaskHandle = NULL;
TaskHandle_t Usart_Rx_TaskHandle = NULL;
TaskHandle_t OLED_Refresh_TaskHandle = NULL;
TaskHandle_t Cmd_Respond_TaskHandle = NULL;
SemaphoreHandle_t Usart_Rx_Flag = NULL;
QueueHandle_t Usart_Rx_Cmd = NULL;
QueueHandle_t User_Cmd_Queue = NULL;

int main(void)
{
	xTaskCreate(
		System_Init_Task,
		"SystemInit",
		System_Init_Task_Stack,
		NULL,
		15,
		&Init_TaskHandel
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

	OLED12864_Clear_PageBlock(1,8,8);
	OLED12864_Refresh();

	//创建串口接收信号量和串口指令队列
	Usart_Rx_Cmd = xQueueCreate(4,sizeof(Usart_Send_TaskDat));
	Usart_Rx_Flag = xSemaphoreCreateBinary();
	User_Cmd_Queue = xQueueCreate(12,sizeof(User_Cmd));

	//创建任务
	xTaskCreate(
		Usart_Send_Task,
		"UsartSend",
		Usart_Send_Task_Stack,
		"Usart_Send_Task",
		4,
		&Usart_Send_TaskHandle
	);
	xTaskCreate(
		Usart_Rx_Task,
		"UsartRx",
		Usart_Rx_Task_Stack,
		NULL,
		10,
		&Usart_Rx_TaskHandle
	);

	//检查是否初始化成功
	if(Usart_Rx_Flag!=NULL && Usart_Rx_Cmd!=NULL && Usart_Send_TaskHandle!=NULL && Usart_Rx_TaskHandle!=NULL)
	{
		printf("System_Init\r\n");
		vTaskDelete(NULL);
	}else
	{
		printf("SystemInit_Flase\r\n");
		while(1);
	}
}

void Usart_Rx_Task(void*ptr)
{
	while(1);
}

void Cmd_Respond_Task(void*ptr)
{
	while(1);
}

void OLED_Refresh_Task(void*ptr)
{
	TickType_t time;
	time = xTaskGetTickCount();
	while(1)
	{
		OLED12864_Refresh();
		xTaskDelayUntil(&time,16/portTICK_RATE_MS);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1);
}

