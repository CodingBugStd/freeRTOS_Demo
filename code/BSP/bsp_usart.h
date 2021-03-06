#ifndef _BSP_USART_H_
#define _BSP_USART_H_

#include "stm32f10x.h"
#include <stdio.h>

#define Rx_SbufferSize  64
#define Tx_SbufferSize  64

//每行第一个元素表示该缓存区存放的数据个数
static uint8_t USART_Rx_Sbuffer[3][Rx_SbufferSize] = {{0},{0},{0}};
static uint8_t USART_Tx_Sbuffer[3][Tx_SbufferSize] = {{0},{0},{0}};

static USART_TypeDef* Target_Usart[3] = {USART1,USART2,USART3};
static DMA_Channel_TypeDef* TargetDMA_Channel[3] = {DMA1_Channel4,DMA1_Channel7,DMA1_Channel2};

//初始化
void BSP_Usart_Init(void);
void USART_GPIO_Init(void);
void USART_Config(void);
void USART_NVIC_Config(void);
void USART_DMA_Config(void);

//发送&接收
uint8_t*Usart_Read(uint8_t USARTx);
uint8_t Usart_Send(uint8_t USARTx,uint8_t *dat,uint8_t len);
uint8_t Usart_BusyCheck(uint8_t USARTx);
void USART_Clear(uint8_t USARTx);
void USART_Push(uint8_t USARTx,uint8_t len);

//内部函数
void Rx_SbufferInput(uint8_t USARTx,uint8_t dat);   //将dat载入对应缓冲区
void Tx_Flag_Clear(uint8_t USARTx);

//printf()重定向
int fputc (int c, FILE *fp);

#endif
