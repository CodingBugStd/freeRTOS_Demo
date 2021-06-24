#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

static TaskHandle_t Task1 = NULL;
static TaskHandle_t Task2 = NULL;

static QueueHandle_t Usart_Cmd = NULL;

static uint8_t Send1[] = "Task1";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();

	Usart_Cmd = xQueueCreate(4,1);
	
	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		32,
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
		uint8_t Target;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					Target = 0;
					for(uint8_t n=0;n<temp;n++)
					{
						Target*=10;
						Target+=*(dat+n+1) - 0x30;
					}
					USART_Push(1,temp+2);
					printf("Input:%d\r\n",Target);
					if(xQueueSend(Usart_Cmd,&Target,0) == pdPASS)
						printf("Dat In Queue\r\n");
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
		static uint8_t delay_time = 200;
		xQueueReceive(Usart_Cmd,&delay_time,0);
		printf("%s Delay_Time:%d\r\n",(uint8_t*)ptr,delay_time);
		vTaskDelay(delay_time/portTICK_RATE_MS);
	}
}
