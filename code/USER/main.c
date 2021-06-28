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


void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;
SemaphoreHandle_t Usart_Rx_Flag = NULL;
QueueHandle_t Usart_Cmd = NULL;
uint8_t Send1[] = "Task1";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();
	Usart_Cmd = xQueueCreate(4,sizeof(xDat));
	Usart_Rx_Flag = xSemaphoreCreateBinary();
	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		64,
		Send1,
		6,
		&Task1
	);
	xTaskCreate(
		Usart_Rx_Task,
		"Task2",
		64,
		NULL,
		6,
		&Task2
	);
	if(Usart_Cmd == NULL)
		printf("QueueFlase\r\n");
	else
		vTaskStartScheduler();
	while(1);
}

void Usart_Rx_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		xDat Cmd;
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
		xQueueSend(Usart_Cmd,&Cmd,0);
		printf("Task2\r\n");
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		static xDat In = {6,300};
		xQueueReceive(Usart_Cmd,&In,0);
		vTaskPrioritySet(NULL,In.TaskPriority);
		printf("%s\r\nDelay_Time:%d\r\nPriority:%d\r\n",(uint8_t*)ptr,In.TaskDelayTime,uxTaskPriorityGet(NULL));
		vTaskDelay(In.TaskDelayTime/portTICK_RATE_MS);
	}
}

