#ifndef _MENU_H_
#define _MENU_H_

#define OptionUnit_Name_len   16

/************************************
 * 菜单管理库
 * 自带内存分配
 * 使用链表储存
 *       庞碧璋   2021/7/8
************************************/

#include <stdio.h>
#include "FreeRTOS.h"   //需要用到内存管理

//内存分配
#define Creat_OptionUnit()    pvPortMalloc(sizeof(Option_Unit))
#define Delete_OptionUnit(ptr)

typedef struct
{
    void*Farther;
    void*SonList_Head;
    void*Next_Option;
    unsigned char list_id;
    unsigned char id;
    unsigned char Name[OptionUnit_Name_len];
}Option_Unit;

Option_Unit*Creat_Menu(void);

#endif
