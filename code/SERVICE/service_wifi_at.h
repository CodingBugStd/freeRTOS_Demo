#ifndef _SERVICE_WIFI_AT_H_
#define _SERVICE_WIFI_AT_H_

/*********对外接口***********/
//dat:要发送的数据指针   len:数据长度   返回值:0(发送成功) 1(在忙)
#define AT_Send(dat,len)    Usart_Send(2,dat,len)
//返回值:0(空闲) 1(在忙)
#define AT_BusyCheck()      Usart_BusyCheck(2)
//返回值:unsigned char*类型 首地址表示接收到的数据个数,往后为接收到的数据
#define Read_AT_Receive()   Usart_Read(2)

void Wifi_AT_Init(void);
void Wifi_Mode_Set(unsigned char mode);
void Wifi_AT_Decode(unsigned char*dat,unsigned char len);

#endif
