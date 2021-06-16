#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOS.h"
#include "task.h"

void Usart_Test_Task(void*ptr);

TaskHandle_t* Usart_Test = NULL;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();

	xTaskCreate(
		Usart_Test_Task,
		"Test",
		64,
		NULL,
		5,
		Usart_Test
	);

	while(1)
	{
		
	}
}

void Usart_Test_Task(void*ptr)
{
	while(1)
	{
		
	}
}
