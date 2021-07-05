#ifndef _SERVICE_MENU_H_
#define _SERVICE_MENU_H_

#define MenuUnit_Name_len   16

typedef struct
{
    menu_unit*Father;
    menu_unit*Son;
    menu_unit*Next;
    menu_unit*Previous;
    unsigned char ID;
    unsigned char*Name[MenuUnit_Name_len];
}menu_unit;

static menu_unit*Head = 0;

void Create_Menu(menu_unit*head);

#endif
