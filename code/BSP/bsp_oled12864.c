#include "bsp_oled12864.h"
#include "font_lib.h"
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
    soft_delay_ms(500);
    OLED12864_Set_Bit(OLED_RES);
    
    OLED12864_Send_NumByte(OLED12864_InitCmd,28,OLED_CMD);
    OLED12864_Clear();
}

void OLED12864_Clear_Sbuffer(void)
{
    uint8_t page,x;
    for(page=0;page<8;page++)
    {
        for(x=0;x<128;x++)
            OLED12864_Sbuffer[page][x] = 0x00;
    }
}

void OLED12864_Clear(void)
{
    OLED12864_Clear_Sbuffer();
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
    OLED12864_Send_Byte(0xb0+page,OLED_CMD);
    OLED12864_Send_Byte(((0xf0&x)>>4)|0x10,OLED_CMD);
    OLED12864_Send_Byte(0x0f&x,OLED_CMD);
}

void OLED12864_Send_NumByte(uint8_t*dat,uint8_t len,uint8_t cmd)
{
    for(uint8_t temp=0;temp<len;temp++)
    {
        OLED12864_Send_Byte(*dat,cmd);
        dat++;
    }
}

void OLED12864_Send_Byte(uint8_t dat,uint8_t cmd)
{
    if(cmd)
        OLED12864_Set_Bit(OLED_DC);
    else
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

void OLED12864_Set_Bit(uint8_t bit)
{
    GPIO_SetBits(OLED_Pin[bit].GPIO,OLED_Pin[bit].Pin);
}

void OLED12864_Reset_Bit(uint8_t bit)
{
    GPIO_ResetBits(OLED_Pin[bit].GPIO,OLED_Pin[bit].Pin);
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
    OLED12864_Clear_PageBlock(page,0,127);
}

void OLED12864_Show_Char(uint8_t page,uint8_t x,uint8_t chr,uint8_t size)
{
    switch(size)
    {
        case 1:
            for(uint8_t temp=0;temp<6;temp++)
                OLED12864_Sbuffer[page][x+temp] = assic_0806[chr-0x20][temp];
            break;
        case 2:
            for(uint8_t page=0;page<2;page++)
            {
                for(uint8_t x=0;x<8;x++)
                    OLED12864_Sbuffer[page][x] = assic_1608[chr-0x20][page*8+x];
            }
            break;
        default:break;
    }
}

void OLED12864_Show_Num(uint8_t page,uint8_t x,uint16_t num,uint8_t size)
{
    uint16_t len;
    uint8_t chr;
    uint8_t lx;
    switch(size)
    {
        case 1:
            lx = 6;
            break;
        case 2:
            lx = 8;
            break;
        default:
            break;
    }
    switch(num)
    {
        case 0:
            OLED12864_Show_Char(page,x,'0',size);
            break;
        case 1:
            OLED12864_Show_Char(page,x,'1',size);
            break;
        case 2:
            OLED12864_Show_Char(page,x,'1',size);
            OLED12864_Show_Char(page,x+lx,'0',size);
            break;
        default:
            for(len=1;len<num;len*=10);
            for(len/=10;len!=0;len/=10)
            {
                chr = num/len+0x30;
                num%=len;
                OLED12864_Show_Char(page,x,chr,size);
                x+=lx;
            }
    }
}

void OLED12864_Show_String(uint8_t page,uint8_t x,uint8_t*str,uint8_t size)
{
    uint8_t sx = 0;
    while(*str!='\0')
    {
        OLED12864_Show_Char(page,x+sx,*str,size);
        switch(size)
        {
            case 1:
                sx+=6;
                break;
            case 2:
                sx+=8;
                break;
            default:
                break;
        }
        str++;
    }
}

//像素点相关操作
#if USE_POINT_CRT == 1

void OLED12864_Draw_Point(uint8_t x,uint8_t y,uint8_t bit)
{
    if(y > y_MAX-1 || x > x_MAX-1)
        return;
    uint8_t page = y/8;
    uint8_t col = y%8;
    if(bit)
        OLED12864_Sbuffer[page][x] |= (0x01<<col);
    else
        OLED12864_Sbuffer[page][x] &= ~(0x01<<col);
}

void OLED12864_Draw_Line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    double sx,sy;
    double k,k_1;   //斜率
    k = ((double)y2-y1) / ((double)x2-x1);
    k_1 = 1/k;
    sx = x1;
    sy = y1;
    for(;x1<=x2;x1++)
    {
        sy += k;
        OLED12864_Draw_Point(x1,(int)sy,1);
    }
    for(;y1<=y2;y1++)
    {
        sx += k_1;
        OLED12864_Draw_Point((int)sx,y1,1);
    }
}

void OLED12864_Draw_Rect(uint8_t x,uint8_t y,uint8_t len,uint8_t hight)
{
    for(uint8_t temp=0;temp<len;temp++)
    {
        OLED12864_Draw_Point(x+temp,y,1);
        OLED12864_Draw_Point(x+temp,y+hight,1);
    }
    for(uint8_t temp=0;temp<hight;temp++)
    {
        OLED12864_Draw_Point(x,y+temp,1);
        OLED12864_Draw_Point(x+len,y+temp,1);
    }
}

void OLED12864_Draw_Img(uint8_t x,uint8_t y,uint8_t len,uint8_t hight,uint8_t*img)
{
    uint8_t sx,sy;
    uint16_t dat_addr_pos;
    uint8_t page_pos;
    uint8_t bit_pos;
    for(sy=0;sy<hight;sy++)
    {
        page_pos = sy/8;
        bit_pos = sy%8;
        for(sx=0;sx<len;sx++)
        {
            dat_addr_pos = page_pos*len + sx;
            OLED12864_Draw_Point(sx+x,sy+y, *(img+dat_addr_pos) & ((0x80)>>bit_pos) );
        }
    }
}

void OLED12864_Draw_aImg(uint8_t x,uint8_t y,uint8_t*img)
{
    uint8_t len,hight;
    uint8_t sx,sy;
    uint16_t dat_addr_pos;
    uint8_t page_pos;
    uint8_t bit_pos;
    len = *(img+3) + *(img+2)*256;
    hight = *(img+5) + *(img+4)*256;
    for(sy=0;sy<hight;sy++)
    {
        page_pos = sy/8;
        bit_pos = sy%8;
        for(sx=0;sx<len;sx++)
        {
            dat_addr_pos = page_pos*len + sx + 6;
            OLED12864_Draw_Point(sx+x,sy+y, *(img+dat_addr_pos) & ((0x80)>>bit_pos) );
        }
    }
}

#endif  //USE_POINT_CRT
