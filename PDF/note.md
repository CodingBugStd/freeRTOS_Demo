# 用于练习FreeRTOS的项目,基于STM32F103ZET6

## 一，串口板级支持包

```c
#ifndef _BSP_USART_H_
#define _BSP_USART_H_

#include "stm32f10x.h"
#include <stdio.h>

#define Rx_SbufferSize  64
#define Tx_SbufferSize  64

//每行第一个元素表示该缓存区存放的数据个数
static uint8_t USART_Rx_Sbuffer[3][Rx_SbufferSize] = {{0},{0},{0}};
static uint8_t USART_Tx_Sbuffer[3][Tx_SbufferSize] = {{0},{0},{0}};

//初始化
void BSP_Usart_Init(void);
void USART_GPIO_Init(void);
void USART_Config(void);
void USART_NVIC_Config(void);

//发送&接收
uint8_t*Read_Usart_Sbuffer(uint8_t USARTx);
void Usart_Send(uint8_t USARTx,uint8_t *dat,uint8_t len);
void USART_Clear(uint8_t USARTx);
void USART_Push(uint8_t USARTx,uint8_t len);

//内部函数
void Rx_SbufferInput(uint8_t USARTx,uint8_t dat);   //将dat载入对应缓冲区

//中断
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

#endif

```

2021/5/16 1:33