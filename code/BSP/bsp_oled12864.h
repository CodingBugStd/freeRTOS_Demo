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
