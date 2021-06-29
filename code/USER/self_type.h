#ifndef _SELF_TYPE_H_
#define _SELF_TYPE_H_

#include "stm32f10x.h"

//串口发送任务信息结构体
typedef struct 
{
	uint8_t TaskPriority;
	uint16_t TaskDelayTime;
}Usart_Send_TaskDat;

//MCU引脚
typedef struct
{
    uint16_t Pin;
    GPIO_TypeDef*GPOP;
}Pin;

#endif
