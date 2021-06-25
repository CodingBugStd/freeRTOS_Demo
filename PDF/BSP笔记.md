[TOC]

# 串口DMA发送

## 1.串口DMA发送注意事项

1. DMA_Cmd()函数会直接开启DMA,所以在初始化串口DMA时不需要调用DMA_Cmd()
2. 需要配置DMA中断
   - 用与清除发送完成中断硬件标志位
   - 用于清除发送在忙软件标志位
3. 配置Usart_DMA(应该算是Usart硬件的一部分,用于告诉DMA是发送还是接收
4. DMA需要配置中断事件 DMA_ITConfig()，否则无法进入中断

## 2.代码

### 	DMA初始化

```c
void USART_DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);

    DMA_InitStruct.DMA_BufferSize = 0;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;

    for(uint8_t temp=0;temp<3;temp++)
    {
        DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&Target_Usart[temp]->DR;
        DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&USART_Tx_Sbuffer[temp][1];

        DMA_Init(TargetDMA_Channel[temp],&DMA_InitStruct);
        USART_DMACmd(Target_Usart[temp],USART_DMAReq_Tx,ENABLE);
        DMA_ClearFlag(DMA1_FLAG_TC1);
    }

    NVIC_InitTypeDef    NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;

    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_Init(&NVIC_InitStruct);
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_Init(&NVIC_InitStruct);

    for(uint8_t temp=0;temp<3;temp++)
        DMA_ITConfig(TargetDMA_Channel[temp],DMA_IT_TC,ENABLE);
}
```

### 	DMA中断

```c
void DMA1_Channel4_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC4) == SET)
    {
        Tx_Flag_Clear(1);
        TargetDMA_Channel[0]->CCR &= (uint16_t)(~DMA_CCR1_EN);
        DMA_ClearITPendingBit(DMA1_IT_TC4);
    }
}
```

### 	DMA发送

```c
uint8_t Usart_Send(uint8_t USARTx,uint8_t *dat,uint8_t len)
{
    if(USART_Tx_Sbuffer[USARTx-1][0]==0)
    {
        uint8_t temp;
        USART_Tx_Sbuffer[USARTx-1][0] = len;
        for(temp=0;temp<len;temp++)
            USART_Tx_Sbuffer[USARTx-1][temp+1] = *(dat+temp);
        TargetDMA_Channel[USARTx-1]->CNDTR = len;
        TargetDMA_Channel[USARTx-1]->CCR |= DMA_CCR1_EN;
        return 0;
    }
    return 1;
}
```

### 	DMA软件在忙清除

```c
void USART_Clear(uint8_t USARTx)
{
    USART_Rx_Sbuffer[USARTx-1][0] = 0;
}
```

### 	头文件相关变量

```C
//每行第一个元素表示该缓存区存放的数据个数
static uint8_t USART_Rx_Sbuffer[3][Rx_SbufferSize] = {{0},{0},{0}};
static uint8_t USART_Tx_Sbuffer[3][Tx_SbufferSize] = {{0},{0},{0}};

static USART_TypeDef* Target_Usart[3] = {USART1,USART2,USART3};
static DMA_Channel_TypeDef* TargetDMA_Channel[3] = {DMA1_Channel4,DMA1_Channel7,DMA1_Channel2};
```

## 2.Debug日志

### 	1.printf()和Usart_Send()冲突

​			原因:Usart_Send()使用DMA发送,printf()使用串口直接发送,在Usart_Send()还没发送完时调用了printf(),导致DMA错误,Usart_Send()发送终止

​			printf()重定向时增加DMA在忙标准判断

```c
int fputc (int c, FILE *fp)
{
    while(Usart_BusyCheck(1));
	USART_SendData(USART1,c);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	return c;
}
```

### 2.printf()陷入死循环(未解决)

​	原因:可能是fputc()重入出现了问题,在多个任务调用printf()时会堵塞任务

```x
int fputc (int c, FILE *fp)
{
    while(Usart_BusyCheck(1));
	USART_SendData(USART1,c);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	return c;
}
```

找到问题了,任务堆栈溢出!!!!!!