#ifndef _BSP_KEY_H_
#define _BSP_KEY_H_

#include "stm32f10x.h"
#include "self_type.h"

static Pin Key_Pin[2] = 
{
    {GPIO_Pin_3,GPIOE},
    {GPIO_Pin_4,GPIOE}
};

void BSP_Key_Init(void);
uint8_t Read_Key_Input(uint8_t num);

#endif
