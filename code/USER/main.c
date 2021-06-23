#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void Usart_Send_Task(void*ptr);

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
		Usart_Send_Task,
		"Task1",
		16,
		Send2,
		3,
		&Task2
	);
	vTaskStartScheduler();
	while(1);
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		printf("%s",(uint8_t*)ptr);
		vTaskDelay(200/portTICK_RATE_MS);
	}
}
