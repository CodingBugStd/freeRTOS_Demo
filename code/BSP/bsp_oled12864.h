#ifndef _BSP_OLES12864_H_
#define _BSP_OLED12864_H_

#include "stm32f10x.h"
#include "self_type.h"

#define OLED12864_4Pin_SPI   1

/*************************************
 * OLED12864 7_Pin SPI BSP
 * stm32f103zet6
 * 注意缓存定义的上下与硬件上下相反
 *                  2021/7/1 庞碧璋
*************************************/

//硬件相关宏定义
#define OLED_D0_Pin     GPIO_Pin_9
#define OLED_D0_GPIO    GPIOD
#define OLED_D1_Pin     GPIO_Pin_11
#define OLED_D1_GPIO    GPIOD
#define OLED_RES_Pin    GPIO_Pin_13
#define OLED_RES_GPIO   GPIOD
#define OLED_DC_Pin     GPIO_Pin_15
#define OLED_DC_GPIO    GPIOD
#define OLED_CS_Pin     GPIO_Pin_3
#define OLED_CS_GPIO    GPIOG

//OLED模式
#if OLED12864_4Pin_SPI == 1
    #define OLED_SCLK_Pin   OLED_D0_Pin
    #define OLED_SCLK_GPIO  OLED_D0_GPIO
    #define OLED_SDIN_Pin   OLED_D1_Pin
    #define OLED_SCIN_GPIO  OLED_D1_GPIO
#elif OLED12864_3Pin_SPI == 1
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
/************************************************
 * [paeg][x]
 * 一个元素保函8像素点信息
 * 在屏幕上,Bit0~Bit7自上向下排列 高位在下
 * 在缓存中,Bit0~Bit7自下向上(方便处理) 高位在上
************************************************/
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
//因为 定义的缓存高低位 与 硬件高低位 相反
//所以 将命令与数据的发送函数分离,方便操作
//void OLED12864_Send_Byte(uint8_t dat,uint8_t cmd)
void OLED12864_Send_Cmd(uint8_t dat);
void OLED12864_Send_Data(uint8_t dat);
void OLED12864_Send_NumByte(uint8_t*dat,uint8_t len,uint8_t cmd);

void OLED12864_Refresh(void);
void OLED12864_Set_Position(uint8_t page,uint8_t x);
void OLED12864_Clear(void);

void OLED12864_Draw_Point(uint8_t x,uint8_t y);
void OLED12864_Draw_Block(uint8_t x,uint8_t y,uint8_t len,uint8_t hight,uint8_t*dat);
void OLED12864_Clear_Block(uint8_t x,uint8_t y,uint8_t len,uint8_t hight);

#endif
