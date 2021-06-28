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

void System_Init_Task(void*ptr);
void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

TaskHandle_t Init_TaskHandel = NULL;
TaskHandle_t Usart_Send_TaskHandle = NULL;
TaskHandle_t Usart_Rx_TaskHandle = NULL;
SemaphoreHandle_t Usart_Rx_Flag = NULL;
QueueHandle_t Usart_Rx_Cmd = NULL;

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

	//创建串口接收信号量和串口指令队列
	Usart_Rx_Cmd = xQueueCreate(4,sizeof(Usart_Send_TaskDat));
	Usart_Rx_Flag = xSemaphoreCreateBinary();

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
	while(1)
	{
		uint8_t*dat;
		Usart_Send_TaskDat Cmd;
		xSemaphoreTake(Usart_Rx_Flag,portMAX_DELAY);
		dat = Usart_Read(1);
		uint8_t temp,n;
		Cmd.TaskDelayTime = 0;
		Cmd.TaskPriority = 0;
		for(temp=0;*(dat+temp+1)!=' ';temp++)
		{
			Cmd.TaskDelayTime*=10;
			Cmd.TaskDelayTime+=*(dat+temp+1)-0x30;
		}
		for(n=0;*(dat+temp+n+2)!='\n';n++)
		{
			Cmd.TaskPriority*=10;
			Cmd.TaskPriority+=*(dat+temp+n+2)-0x30;
		}
		USART_Clear(1);
		xQueueSend(Usart_Rx_Cmd,&Cmd,0);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		static Usart_Send_TaskDat In = {6,300};
		xQueueReceive(Usart_Rx_Cmd,&In,0);
		vTaskPrioritySet(NULL,In.TaskPriority);
		printf("%s\r\nDelay_Time:%d\r\nPriority:%d\r\n",(uint8_t*)ptr,In.TaskDelayTime,uxTaskPriorityGet(NULL));
		vTaskDelay(In.TaskDelayTime/portTICK_RATE_MS);
	}
}

