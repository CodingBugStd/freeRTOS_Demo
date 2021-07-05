#include "service_usart_play.h"

unsigned char*Get_Usart_bmp(void)
{
    if(bmp_end!=0)
        return Usart_Receive_bmp;
    else
        return (void*)0;
}


