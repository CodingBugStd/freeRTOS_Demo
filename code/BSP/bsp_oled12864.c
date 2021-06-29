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
