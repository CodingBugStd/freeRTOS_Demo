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
		72,
		NULL,
		5,
		&Usart_Test
	);

	if(Usart_Test != NULL)
		vTaskStartScheduler();
	else
		printf("Erro\r\n");
	while(1)
	{
	}
}

void Usart_Test_Task(void*ptr)
{
	uint8_t*dat;
	while(1)
	{
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			while(Usart_BusyCheck(1));
			Usart_Send(1,dat+1,*dat);
			while(Usart_BusyCheck(1));
			USART_Clear(1);
		}
		vTaskDelay(50/portTICK_RATE_MS);
	}
}

