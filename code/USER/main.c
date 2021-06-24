#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void vApplicationIdleHook( void );
void Usart_Send_Task(void*ptr);
void Usart_Return_Task(void*ptr);

static TaskHandle_t Task1 = NULL;
static TaskHandle_t Task2 = NULL;

static uint8_t Send1[] = "Task1\r\n";

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
		16,
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
		uint8_t Target;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					Target = 0;
					for(uint8_t n=0;n<temp+1;n++)
					{
						Target*=10;
						Target+=*(dat+temp+1)-0x30;
					}
					USART_Push(1,temp+2);
					//printf("Input:%d\r\n",Target);
					//vTaskPrioritySet(Task1,Target);
					break;
				}
			}
		}
		printf("Task2\r\n");
		vTaskDelay(200/portTICK_RATE_MS);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		printf("%s",(uint8_t*)ptr);
		printf("优先级:%d\r\n",uxTaskPriorityGet(NULL));
		vTaskDelay(200/portTICK_RATE_MS);
	}
}

void vApplicationIdleHook(void)
{
	
}
