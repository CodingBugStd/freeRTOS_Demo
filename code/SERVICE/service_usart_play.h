#ifndef _SERVICE_USART_PLAY_H_
#define _SERVICE_USART_PLAY_H_

static unsigned char Usart_Receive_bmp[1024];
static unsigned char bmp_end = 0;

unsigned char*Get_Usart_bmp(void);

#endif
