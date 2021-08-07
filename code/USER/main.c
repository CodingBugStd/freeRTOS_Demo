#include "stm32f10x.h"

#include "self_type.h"
#include "soft_delay.h"

#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_oled12864.h"

int main(void)
{
	BSP_OLED12864_Init();
	BSP_LED_Init();
	OLED12864_Show_Char(0,0,'A',2);
	OLED12864_Show_Num(0,8,12,2);
	OLED12864_Show_String(2,0,"Test",2);
	OLED12864_Refresh();
	while(1);
}

