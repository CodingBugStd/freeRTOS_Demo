#include "isr.h"

#include "bsp_usart.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/***************中断***********************/

/************************************************************
 * 在串口数据量过大和波特率过高时不建议使用二值信号量直接触发服务
 * 函数
 * 通常采用缓存式,中断产生信号量后,由信号量触发缓存函数。当达到某
 * 个特殊条件(超时,换行...)时在触发服务函数处理数据
extern SemaphoreHandle_t Usart_Rx_Flag;
void USART1_IRQHandler(void)
{
    BaseType_t temp = pdFALSE;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(1,USART_ReceiveData(USART1));
        xSemaphoreGiveFromISR(Usart_Rx_Flag,&temp);
        portEND_SWITCHING_ISR(&temp);
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}
************************************************************/

//extern SemaphoreHandle_t Usart_Rx_Flag;
void USART1_IRQHandler(void)
{
    BaseType_t temp = pdFALSE;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(1,USART_ReceiveData(USART1));
        if(*(Usart_Read(1) + *Usart_Read(1))=='\n')
        {
            //xSemaphoreGiveFromISR(Usart_Rx_Flag,&temp);
            portEND_SWITCHING_ISR(&temp);
        }
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(2,USART_ReceiveData(USART2));
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(3,USART_ReceiveData(USART3));
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC4) == SET)
    {
        Tx_Flag_Clear(1);
        TargetDMA_Channel[0]->CCR &= (uint16_t)(~DMA_CCR1_EN);
        DMA_ClearITPendingBit(DMA1_IT_TC4);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC7) == SET)
    {
        Tx_Flag_Clear(2);
        TargetDMA_Channel[1]->CCR &= (uint16_t)(~DMA_CCR1_EN);
        DMA_ClearITPendingBit(DMA1_IT_TC7);
    }
}

void DMA1_Channel2_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC2) == SET)
    {
        Tx_Flag_Clear(3);
        TargetDMA_Channel[2]->CCR &= (uint16_t)(~DMA_CCR1_EN);
        DMA_ClearITPendingBit(DMA1_IT_TC2);
    }
}

