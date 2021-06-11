#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();
	static uint8_t test[15] = "HelloWorld!\r\n";
	Usart_Send(1,test,15);
	while(1)
	{
	}
}

