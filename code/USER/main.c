#include "stm32f10x.h"

#include <stdio.h>
#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

static TaskHandle_t Task1 = NULL;
static TaskHandle_t Task2 = NULL;

typedef struct
{
	uint8_t TaskPriority;
	uint16_t TaskDelayTime;
}xDat;

static QueueHandle_t Usart_Cmd = NULL;

static uint8_t Send1[] = "Task1";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();

	Usart_Cmd = xQueueCreate(4,sizeof(xDat));
	
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
		32,
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
		xDat Usart_Rx;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					uint8_t n,m;
					Usart_Rx.TaskPriority = 0;
					Usart_Rx.TaskDelayTime = 0;
					for(n=0;*(dat+n+1)!=' ';n++)
					{
						Usart_Rx.TaskPriority *= 10;
						Usart_Rx.TaskPriority += *(dat+n+1) - 0x30;
					}
					for(m=0;*(dat+n+m+2)!='\n';m++)
					{
						Usart_Rx.TaskDelayTime *= 10;
						Usart_Rx.TaskDelayTime += *(dat+n+m+2) - 0x30;
					}
					printf("%d %d\r\n",Usart_Rx.TaskPriority,Usart_Rx.TaskDelayTime);
					if(xQueueSend(Usart_Cmd,&Usart_Rx,0) == errQUEUE_FULL)
						printf("QueueSendErr\r\n");
					USART_Push(1,temp+1);
					break;
				}
			}
		}
		vTaskDelay(50/portTICK_RATE_MS);
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
