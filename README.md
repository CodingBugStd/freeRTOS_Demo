# freeRTOS学习DEMO

2021/6/4

串口收发通过

freeRTOS接口函数还没对接

2021/6/16

freeRTOS环境下串口直接发送和DMA发送没问题

freeRTOS环境下串口接收的缓存区长度记录没问题

2021/6/17

freeRTOS环境下,用串口DMA发送串口接收成功

2021/6/22

freeRTOS通过任务参数使任务共用代码段

2021/6/24

优化了工程模板,将中断函数纳入了isr.c中

Queue和Semaphore测试通过,包括ISR中的情况

2021/6/26

优化了Usart接收ISR函数

缓存式,使用'\n'为信号量发送标志

2021/6/28

优化整理了所有代码

添加了系统初始胡任务

2021/6/29

加入了OLED12864的板级支持包

但是点亮失败

2021/6/29

OLED12864点亮成功

完成单个像素点的控制

完成整页块擦除

2021/7/1

新增了LIB分组,包含图片头文件

OLED12864 bmp显示Pass

2021/7/2

OLED assic 0806

2021/7/3

OLED numShow

service group

bsp add led key

2021/7/5