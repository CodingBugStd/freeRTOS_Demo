#ifndef _EAT_SNAKE_APP_H_
#define _EAT_SNAKE_APP_H_

#include <stdio.h>

#define map_sx      0
#define map_sy      56
#define map_width   112
#define map_hight   56

extern void OLED12864_Draw_Rect(unsigned char,unsigned char,unsigned char,unsigned char);
extern void OLED12864_Clear_Sbuffer(void);

#define Map_Clear() OLED12864_Clear_Sbuffer()
#define Map_Draw()  OLED12864_Draw_Rect(0,56,112,56)

static unsigned char  Head_Pos[2] = {0,0};
static unsigned char  Food_Pos[2] = {0,0};

static unsigned char Grade;
static float Speed;
static unsigned char Dir;
static unsigned int Point;

void Snake_App_Init(void);

#endif
