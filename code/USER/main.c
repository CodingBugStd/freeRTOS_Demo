#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOS.h"
#include "task.h"

void Usart_Test_Task(void*ptr);

TaskHandle_t Usart_Test = NULL;

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
		&Usart_Test
	);

	if(Usart_Test != NULL)
		vTaskStartScheduler();
	else
		printf("Erro\r\n");

	while(1);
}

void Usart_Test_Task(void*ptr)
{
	uint8_t Test[5] = "123\r\n";
	while(1)
	{
		Usart_Send(1,Test,5);
		vTaskDelay(500/portTICK_RATE_MS);
	}
}
