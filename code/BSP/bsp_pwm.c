#include "bsp_pwm.h"

void BSP_PWM_Init(void)
{
    PWM_GPIO_Init();
    
}

void PWM_GPIO_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed =  GPIO_Speed_50MHz;

    GPIO_Init(GPIOA,&GPIO_InitStruct);
}

void PWM_TIM_Init(void)
{

}
