#ifndef _ISR_H_
#define _ISR_H_

#include "stm32f10x.h"

//中断
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);

#endif
