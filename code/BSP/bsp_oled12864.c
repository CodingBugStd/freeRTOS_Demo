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
        GPIO_Init(OLED_Pin[temp].GPIO,&GPIO_InitStruct);
        GPIO_SetBits(OLED_Pin[temp].GPIO,OLED_Pin[temp].Pin);
    }
}

void OLED12864_Hard_Reset(void)
{
    OLED12864_Reset_Bit(OLED_RES);
    soft_delay_ms(300);
    OLED12864_Set_Bit(OLED_RES);

    OLED12864_Send_NumByte(OLED12864_InitCmd,28,OLED_CMD);
    OLED12864_Clear();
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
        OLED12864_Set_Position(page,0);
        OLED12864_Send_NumByte(OLED12864_Sbuffer[page],128,OLED_DATA);
    }
}

void OLED12864_Set_Position(uint8_t page,uint8_t x)
{
    OLED12864_Send_Cmd(0xb0+page);
    OLED12864_Send_Cmd( ((0xf0&x)>>4)|0x10);
    OLED12864_Send_Cmd(0x0f&x);
}

void OLED12864_Send_NumByte(uint8_t*dat,uint8_t len,uint8_t cmd)
{
    void (*FunPtr)(uint8_t dat);
    if(cmd)
        FunPtr = OLED12864_Send_Data;
    else
        FunPtr = OLED12864_Send_Cmd;
    for(uint8_t temp=0;temp<len;temp++)
    {
        FunPtr(*dat);
        dat++;
    }
}

void OLED12864_Send_Cmd(uint8_t dat)
{
    OLED12864_Reset_Bit(OLED_DC);
    OLED12864_Reset_Bit(OLED_CS);
    for(uint8_t temp=0;temp<8;temp++)
    {
        OLED12864_Reset_Bit(OLED_SCLK);
        if(dat&0x80)
            OLED12864_Set_Bit(OLED_SDIN);
        else
            OLED12864_Reset_Bit(OLED_SDIN);
        dat<<=1;
        OLED12864_Set_Bit(OLED_SCLK);
    }
    OLED12864_Set_Bit(OLED_DC);
    OLED12864_Set_Bit(OLED_CS);
}

void OLED12864_Send_Data(uint8_t dat)
{
    OLED12864_Set_Bit(OLED_DC);
    OLED12864_Reset_Bit(OLED_CS);
    for(uint8_t temp=0;temp<8;temp++)
    {
        OLED12864_Reset_Bit(OLED_SCLK);
        if(dat&0x01)
            OLED12864_Set_Bit(OLED_SDIN);
        else
            OLED12864_Reset_Bit(OLED_SDIN);
        dat>>=1;
        OLED12864_Set_Bit(OLED_SCLK);
    }
    OLED12864_Set_Bit(OLED_DC);
    OLED12864_Set_Bit(OLED_CS);
}

void OLED12864_Set_Bit(uint8_t bit)
{
    GPIO_SetBits(OLED_Pin[bit].GPIO,OLED_Pin[bit].Pin);
}

void OLED12864_Reset_Bit(uint8_t bit)
{
    GPIO_ResetBits(OLED_Pin[bit].GPIO,OLED_Pin[bit].Pin);
}

void OLED12864_Draw_Point(uint8_t x,uint8_t y,uint8_t bit)
{
    if(y > y_MAX-1 || x > x_MAX-1)
        return;
    uint8_t page = y/8;
    uint8_t col = y%8;
    if(bit)
        OLED12864_Sbuffer[page][x] |= (0x80>>col);
    else
        OLED12864_Sbuffer[page][x] &= ~(0x80>>col);
}

void OLED12864_Clear_PageBlock(uint8_t page,uint8_t x,uint8_t len)
{
    uint8_t sx = x+len;
    if(sx > x_MAX-1 || page > page_MAX-1)
        return;
    for(uint8_t temp=0;temp<len;temp++)
        OLED12864_Sbuffer[page][temp+x] = 0x00;
}

void OLED12864_Clear_Page(uint8_t page)
{
    OLED12864_Clear_PageBlock(page,0,128);
}

void OLED12864_Draw_PageBlock(uint8_t page,uint8_t x,uint8_t len,uint8_t*dat)
{
    for(uint8_t temp=0;temp<len;temp++)
    {
        OLED12864_Sbuffer[page][x+temp] = *dat;
        dat++;
    }
}
