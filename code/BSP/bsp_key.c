#include "bsp_key.h"

void BSP_Key_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    
    for(uint8_t temp=0;temp<2;temp++)
    {
        GPIO_InitStruct.GPIO_Pin = Key_Pin[temp].Pin;
        GPIO_Init(Key_Pin[temp].GPIO,&GPIO_InitStruct);
    }
}

uint8_t Read_Key_Input(uint8_t num)
{
    uint8_t re;
    if((Key_Pin[num].GPIO->IDR & Key_Pin[num].Pin) != 0)
        re = 0;
    else
        re = 1;
    return re;
}
