#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void Usart_Send_Task(void*ptr);
void Usart_Return_Task(void*ptr);

TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;

static uint8_t Send1[] = "Task1\r\n";
static uint8_t Send2[] = "Task2\r\n";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();

	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		16,
		Send1,
		3,
		&Task1
	);
	xTaskCreate(
		Usart_Return_Task,
		"Task2",
		32,
		NULL,
		6,
		&Task2
	);
	vTaskStartScheduler();
	while(1)
	{
	}
}

void Usart_Return_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		uint8_t Return_Dat[64];
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					for(uint8_t n=0;n<temp+1;n++)
						Return_Dat[n] = *(dat+n+1);
					while(Usart_Send(1,Return_Dat,temp+1));
					USART_Push(1,temp+1);
					break;
				}
			}
		}
		vTaskDelay(200/portTICK_RATE_MS);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		printf("%s",(uint8_t*)ptr);
		vTaskDelay(200/portTICK_RATE_MS);
	}
}
