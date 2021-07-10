#ifndef _BSP_LED_H_
#define _BSP_LED_H_

#include "stm32f10x.h"
#include "self_type.h"

static Pin LED_Pin[2] = 
{
    {GPIO_Pin_5,GPIOB},
    {GPIO_Pin_5,GPIOE}
};

#define LED_ON  0
#define LED_OFF 1

void BSP_LED_Init(void);
void LED_CTR(uint8_t num,uint8_t cmd);
void LED_Reversal(uint8_t num);
void KED_Breath(uint8_t num,uint8_t Fre);

#endif
