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

# OLED12864_四线SPI

## 1.代码

bsp_oled12864.h

```c
#ifndef _BSP_OLES12864_H_
#define _BSP_OLED12864_H_

#include "stm32f10x.h"
#include "self_type.h"

//硬件相关宏定义
#define OLED_D0_Pin     GPIO_Pin_13
#define OLED_D0_GPIO    GPIOD
#define OLED_D1_Pin     GPIO_Pin_15
#define OLED_D1_GPIO    GPIOD
#define OLED_RES_Pin    GPIO_Pin_3
#define OLED_RES_GPIO   GPIOG
#define OLED_DC_Pin     GPIO_Pin_5
#define OLED_DC_GPIO    GPIOG
#define OLED_CS_Pin     GPIO_Pin_7
#define OLED_CS_GPIO    GPIOG

//OLED模式
#ifndef OLED_IIC
    #define OLED_SCLK_Pin   OLED_D0_Pin
    #define OLED_SCLK_GPIO  OLED_D0_GPIO
    #define OLED_SDIN_Pin   OLED_D1_Pin
    #define OLED_SCIN_GPIO  OLED_D1_GPIO
#endif

//引脚编号
#define OLED_SCLK   0
#define OLED_SDIN   1
#define OLED_RES    2
#define OLED_DC     3
#define OLED_CS     4
//MCU引脚
typedef struct
{
    uint16_t Pin;
    GPIO_TypeDef*GPOP;
}Pin;
static Pin OLED_Pin[5] = {
    {OLED_SCLK_Pin,OLED_SCLK_GPIO},
    {OLED_SDIN_Pin,OLED_SCIN_GPIO},
    {OLED_RES_Pin,OLED_RES_GPIO},
    {OLED_DC_Pin,OLED_DC_GPIO},
    {OLED_CS_Pin,OLED_CS_GPIO}
};

#define OLED_CMD    0
#define OLED_DATA   1

//OLED12864缓存
static uint8_t OLED12864_Sbuffer[8][128];
//OLED_初始化指令
static uint8_t OLED12864_InitCmd[28] = {
    0xae,0x00,0x10,0x40,0x81,0xcf,
    0xa1,0xc8,0xa6,0xa8,0x3f,0xd3,
    0x00,0xd5,0x80,0xd9,0xf1,0xda,
    0x12,0xdb,0x40,0x20,0x02,0x8d,
    0x14,0xa4,0xa6,0xaf
};

void BSP_OLED12864_Init(void);
void OLED12864_GPIO_Init(void);
void OLED12864_Hard_Reset(void);

void OLED12864_Set_Bit(uint8_t bit);
void OLED12864_Reset_Bit(uint8_t bit);
void OLED12864_Write_Byte(uint8_t dat,uint8_t cmd);
void OLED12864_Write_NumByte(uint8_t*dat,uint8_t len,uint8_t cmd);

void OLED12864_Refresh(void);
void OLED12864_Draw_Point(uint8_t x,uint8_t y);
void OLED12864_Draw_Img(uint8_t x,uint8_t y,uint8_t len,uint8_t hight,uint8_t*img);
void OLED12864_Show_Char(uint8_t x,uint8_t y,uint8_t chr,uint8_t size);
void OLED12864_Show_String(uint8_t x,uint8_t y,uint8_t*str,uint8_t len,uint8_t size);
void OLED12864_Show_Num(uint8_t x,uint8_t y,int num,uint8_t size);
void OLED12864_Clear(void);
void OLED12864_Move(int xlen,int ylen);


#endif
```

bsp_oled12864.c

```c
#include "bsp_oled12864.h"
#include "soft_delay.h"

void BSP_OLED12864_Init(void)
{
    OLED12864_GPIO_Init();
    OLED12864_Hard_Reset();
}

void OLED12864_GPIO_Init(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    for(uint8_t temp=0;temp<5;temp++)
    {
        GPIO_InitStruct.GPIO_Pin = OLED_Pin[temp].Pin;
        GPIO_Init(OLED_Pin[temp].GPOP,&GPIO_InitStruct);
        GPIO_SetBits(OLED_Pin[temp].GPOP,OLED_Pin[temp].Pin);
    }
}

void OLED12864_Hard_Reset(void)
{
    OLED12864_Reset_Bit(OLED_RES);
    soft_delay_ms(500);
    OLED12864_Set_Bit(OLED_RES);
    
    OLED12864_Write_NumByte(OLED12864_InitCmd,28,OLED_CMD);
}

void OLED12864_Set_Bit(uint8_t bit)
{
    GPIO_SetBits(OLED_Pin[bit].GPOP,OLED_Pin[bit].Pin);
}

void OLED12864_Reset_Bit(uint8_t bit)
{
    GPIO_ResetBits(OLED_Pin[bit].GPOP,OLED_Pin[bit].Pin);
}

void OLED12864_Write_Byte(uint8_t dat,uint8_t cmd)
{
    if(cmd)
        OLED12864_Set_Bit(OLED_DC);
    else
        OLED12864_Set_Bit(OLED_DC);
    GPIO_SetBits(OLED_CS_GPIO,OLED_CS_Pin);
    for(uint8_t temp=0;temp<8;temp++)
    {
        OLED12864_Reset_Bit(OLED_SCLK);
        if(dat&0x80)
            OLED12864_Set_Bit(OLED_SDIN);
        else
            OLED12864_Reset_Bit(OLED_SDIN);
        OLED12864_Reset_Bit(OLED_SCLK);
        dat<<=1;
    }
    OLED12864_Set_Bit(OLED_CS);
    OLED12864_Set_Bit(OLED_DC);
}

void OLED12864_Write_NumByte(uint8_t*dat,uint8_t len,uint8_t cmd)
{
    for(uint8_t temp=0;temp<len;temp++)
    {
        OLED12864_Write_Byte(*dat,cmd);
        dat++;
    }
}

void OLED12864_Clear(void)
{
    uint8_t page,x;
    for(page=0;page<8;page++)
    {
        for(x=0;x<128;x++)
            OLED12864_Sbuffer[page][x] = 0xff;
    }
    OLED12864_Refresh();
}

void OLED12864_Refresh(void)
{
    uint8_t page;
    for(page=0;page<8;page++)
    {
        OLED12864_Write_Byte(0xb0+page,OLED_CMD);   //行设置
        OLED12864_Write_Byte(0x00,OLED_CMD);        //列低4位设置
        OLED12864_Write_Byte(0x10,OLED_CMD);        //列高4位设置
        OLED12864_Write_NumByte(OLED12864_Sbuffer[page],128,OLED_DATA);
    }
}
```

