#include "bsp_led.h"

void BSP_LED_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    for(uint8_t temp=0;temp<2;temp++)
    {
        GPIO_InitStruct.GPIO_Pin = LED_Pin[temp].Pin;
        GPIO_Init(LED_Pin[temp].GPIO,&GPIO_InitStruct);
        LED_CTR(temp,LED_OFF);
    }
}

void LED_CTR(uint8_t num,uint8_t cmd)
{
    if(cmd)
        LED_Pin[num].GPIO->BSRR |= LED_Pin[num].Pin;
    else
        LED_Pin[num].GPIO->BRR |= LED_Pin[num].Pin;
}

void LED_Reversal(uint8_t num)
{
    if( (LED_Pin[num].GPIO->ODR & LED_Pin[num].Pin) != 0 )
        LED_Pin[num].GPIO->BRR |= LED_Pin[num].Pin;
    else
        LED_Pin[num].GPIO->BSRR |= LED_Pin[num].Pin;
}
