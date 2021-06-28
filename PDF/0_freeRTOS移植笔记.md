

# **FreeRTOS学习笔记(使用层面)**

[TOC]

## 一.在stm32f10x上的移植(vet6)

------

### 	1.源文件的获取

​			官方网站:https://www.freertos.org/

---

### 2.源文件的拷贝和理解

**1. FreeRTOS系统层源代码**

   - /Source下的.c文件

     在工程目录下创建freeRTOS_core文件夹，用来存放/Suorce文件里的.c文件

   - /Source/include下的.h文件

     直接将整个文件夹放入freeRTOS_core文件夹里

**2. FreeRTOS接口层代码**

   - /Source/portable/RVDS

     在工程目录下创建freeRTOS_port文件夹整个文件夹放入工程目录下的freeRTOS_port文件夹中

**3. FreeRTOS内存管理方案** 

   - /Source/portable/MemMang

     heap_1~5.c，表示freeRTOS的5种内存管理方案，通常选择heap_4.c，整个文件夹考入port文件夹中

**4. FreeRTOS系统配置文件**

- FreeRTOSConfig.h

  需要自己创建或者从Demo中拷贝，主要用于配置系统相关功能

---

### 3.在startcode中重定向中断入口函数

​			port.c文件中，有3个freeRTOS的Exception handlers函数需要被设置为相应的中断入口

​			**1.xPortPendSVHandler**

​			**2.xPortSysTickHandler**

​			**3.vPortSVCHandler**

​			这三个函数是连接硬件设备的重要桥梁，需要在statrtcode将相应中断入口中重新定向至这三个函数

​			*****1和3是由内联汇编编写的函数，主要用于上下文切换，使用汇编能大大提高切换速率，减少切换过程中关闭中断的时间，增强系统的稳定性

---

### 4.freeRTOSConfig.h的编写或修改

​			添加Demo中的freeRTOSConfig.h后，编译会发现xTaskGetCurrentTaskHandle() 没有被定义，导致编译无法通过。这个函数的主要作用是获取当前任务的句柄，报错的地方出现在stream_buffer.c中。这时候转到定义，发现有一个条件宏

```c
#if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ) )

    TaskHandle_t xTaskGetCurrentTaskHandle( void )
    {
        TaskHandle_t xReturn;

        /* A critical section is not required as this is not called from
         * an interrupt and the current TCB will always be the same for any
         * individual execution thread. */
        xReturn = pxCurrentTCB;

        return xReturn;
    }

#endif 
```

原因是 INCLUDE_xTaskGetCurrentTaskHandle 和 configUSE_MUTEXES 都没有被定义导致函数没有定义，只要在freeRTOSConfig.h前加入相关宏定义即可编译通过

---

### 5.freeRTOS移植测试

​		做一个简单的bsp(板级支持包)，初始化一个led或者串口

```c
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_led.h"

static void LED_TestApp(void*ptr);		//声明任务函数
static TaskHandle_t LED_Handle = NULL;	//任务句柄

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	//任务分组
	BSP_LED_Init();									//LED初始化
    //创建一个led闪烁的任务
	xTaskCreate((TaskFunction_t )LED_TestApp,		
                (const char*    )"LED_Test",
                (uint16_t       )256 ,
                (void*          )NULL,
                (UBaseType_t    )1,
                (TaskHandle_t*  )&LED_Handle);        
    vTaskStartScheduler(); 		//开启任务调度
	while(1);
}

void LED_TestApp(void*ptr)				//移植成功验证任务
{
	while(1)
    {
        BSP_LED_CTR(0,LED_Off);
        vTaskDelay(500/portTICK_RATE_MS);
        BSP_LED_CTR(0,LED_On);
        vTaskDelay(500/portTICK_RATE_MS);
    }
}
```

​	led按照预定时间闪烁，freeRTOS移植成功

## 二.几个常用的API函数

### 1.xTaskCreate()

​	函数原型:

```c
BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,					//任务函数指针
                        const char * const pcName,					//任务名称
                        const configSTACK_DEPTH_TYPE usStackDepth,	//任务堆栈大小
                        void * const pvParameters,					//传入任务函数的参数
                        UBaseType_t uxPriority,						//任务优先级
                        TaskHandle_t * const pxCreatedTask )		//任务句柄
```

该函数的目的就是建立一个任务

- **pxTaskCode参数指向的函数必须是一个无返回值的任务函数**

  一般情况任务函数都是while(1)的死循环。若这个任务函数在整个过程中只执行一次，在执行完成后要删除自身来释放内存

- **pcName参数就是任务的名字或者是标识**

  用于调试时的标识

- **usStackDepth参数是该任务的堆栈深度**

  堆栈大小的单位不一定是Byte，而是StackType_t！

  xTaskCreate()中截取的堆栈内存分配的部分代码:

  ```c
  StackType_t * pxStack;
  pxStack = pvPortMalloc( ( ( ( size_t ) usStackDepth ) * sizeof( StackType_t ) ) ); 
  ```

  归根溯源，StackType_t的大小定义在portmacro.h中，为uint32_t。所以堆栈的单位为4byte。

- **pvParameters参数是传入任务函数的指针**

  很有用的一个参数，能提高代码的复用率。可以对同一个任务函数传入不同的参数创建多个功能相似的任务，而不用重新编写任务函数。

  ```c
  #include "stm32f10x.h"
  #include "FreeRTOS.h"
  #include "task.h"
  #include "inc.h"
  #include "bsp_led.h"
  
  static void LED_TestApp(void*ptr);
  
  //应该定义TaskHandle_t而不是TaskHandle_t*
  static TaskHandle_t LED0_Handle = NULL;	//任务句柄
  static TaskHandle_t LED1_Handle = NULL;	//任务句柄
  
  static LED_Data Test[2] = {{1000,100,0},{600,500,1}};
  
  int main(void)
  {
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  	BSP_LED_Init();
  	xTaskCreate((TaskFunction_t )LED_TestApp,
                  (const char*    )"LED0_Test",
                  (uint16_t       )256 ,
                  (void*          )(void*)&Test[0],
                  (UBaseType_t    )1,
                  (TaskHandle_t*  )&LED0_Handle);   
  	xTaskCreate((TaskFunction_t )LED_TestApp,
                  (const char*    )"LED1_Test",
                  (uint16_t       )256 ,
                  (void*          )(void*)&Test[1],
                  (UBaseType_t    )1,
                  (TaskHandle_t*  )&LED1_Handle);      
      vTaskStartScheduler(); 
  	while(1);
  }
  ```
  
  通过一个任务函数创建了两个控制LED闪烁的任务，而且周期和发光时长各不相同，避免了为两个LED分别写任务函数。
  
- **uxPriority参数用于设置任务优先级**

  表示任务的优先级，数值越大优先级越高。优先级数大小取决于freeRTOSConfig.h中的configMAX_PRIORITIES定义，0~configMAX_PRIORITIES-1，超过configMAX_PRIORITIES-1按照最大优先级给任务赋值

- **pxCreatedTask参数就是这个任务的句柄**

  freeRTOS就是通过对任务句柄的操作来实现任务的各种操作，包括 删除 重新配置优先级 等等

  若xTaskCreate()创建任务失败，则pxCreatedTask的值为NULL，一般情况下是内存不足导致的任务创建失败
  
  **注意:TaskHandle_t本身就是一个指向tskTaskControlBlock结构体的指针了，定义时不能用TaskHandle_t*。**

---

### 2.vTaskStartScheduler()

​		开启任务调度器，会默认创建优先级为0的prvIdleTask()任务(空闲任务)

vTaskStartScheduler()中创建空闲任务的代码片段

```c
xReturn = xTaskCreate( prvIdleTask,
                       configIDLE_TASK_NAME,
                       configMINIMAL_STACK_SIZE,
                       ( void * ) NULL,
                       portPRIVILEGE_BIT,
                        &xIdleTaskHandle ); 
```

vTaskStartScheduler()中还关闭了中断(内联汇编)，在portmacro.h中的vPortRaiseBASEPRI函数

freeRTOS中还有许多可以由用户定义的宏定义函数，vTaskStartScheduler()中就调用了以下的‘空’函数

```c
portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();
traceTASK_SWITCHED_IN();
```

FreeRTOS.h中的空宏定义

```c
#ifndef portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#endif
```

*<u>这些空的宏定义也是freeRTOS移植中非常重要的一环</u>*

### 3.vTaskDelete()

​	函数原型

```c
void vTaskDelete( TaskHandle_t xTaskToDelete );
```

有创建任务的函数就有删除任务的函数，直接向vTaskDelete()里头丢任务的句柄，就能完成删除任务的操作，若删除的是调用这个函数的任务本身，可以直接传入NULL即可完成删除自身的操作。常用于开始任务。

**有些任务别瞎jb删！！**

### 4.vTaskDelay()

​	函数原型

```C
void vTaskDelay( const TickType_t xTicksToDelay );
```

延时函数，相比于普通的软件延时和裸机的定时器延时，vTaskDelay()有本质上的区别，它能使任务进入阻塞态，从而触发freeRTOS的任务调度器。可近似的简单理解为vTaskDelay()使任务阻塞一段时间，把cpu的使用权暂时交出去，阻塞的时间单位为系统心跳，系统心跳在freeRTOSConfig.h中定义

```c
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
```


阻塞的时间可以通过除以portTICK_RATE_MS来变成ms，portTICK_RATE_MS在FreeRTOS.h中有定义

### 5.vTaskDelayUntil()

​	函数原型

```c
BaseType_t xTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                                const TickType_t xTimeIncrement );
```

相比vTaskDelay()，vTaskDelayUntil()更能适用于周期性任务。vTaskDelay()能阻塞任务一段固定的时间，而vTaskDelayUntil()的计时时间长是从上一次从阻塞态回到就绪态时开始算起，所以能保证任务的周期性，屏蔽了任务运行本身所用的时长	

- **pxPreviousWakeTime参数**

  TickType_t等价于portTickType，这是一个用于计时的变量，用户无需考虑溢出，由系统管理，只需要将指针传入。

  通常任务刚刚创建时需要用xTaskGetTickCount()给portTickType变量赋值，获取当前的系统时间

- **xTimeIncrement参数**

  上一次从阻塞态回到就绪态到下一次从阻塞态回到就绪态的时间，在不复杂的任务中可以理解为任务周期。单位是系统心跳周期，可以通过portTICK_RATE_MS变成ms

- **返回值**

  pdFlase或pdTrue，分别表示失败和成功
  
  ```c
  void LED_TestApp(void*ptr)
  {
  	portTickType xLastWakeTime;
      uint16_t cycle,On_time;
      uint8_t Target_LED;
      cycle = ((LED_Data*)ptr)->Cycle;
      On_time = ((LED_Data*)ptr)->On_Time;
      Target_LED = ((LED_Data*)ptr)->Target_LED;
      xLastWakeTime = xTaskGetTickCount();
  	while(1)
  	{
  		BSP_LED_CTR( Target_LED, LED_ON);
          vTaskDelayUntil(&xLastWakeTime, On_time/portTICK_RATE_MS);
          BSP_LED_CTR( Target_LED, LED_OFF);
          vTaskDelayUntil(&xLastWakeTime, (cycle-On_time)/portTICK_RATE_MS );
  	}
  }
  ```

### 6.taskYIELD()

​	立刻唤醒调度器,不必等到其他能堵塞任务的情况出现。

​	**注意:taskYIELD()不能将cpu的使用权交给比该任务低的就绪态任务,taskYIELD()只是放弃该时间片,并没有使任务进入阻塞态!**

## 三,任务优先级API函数

### 1.vTaskPrioritySet()

​	可以在调度器运行之后改变任务的优先级。

函数原型

```c
void vTaskPrioritySet( TaskHandle_t xTask,
                           UBaseType_t uxNewPriority );
```

- **xTask 参数**

  要操作的任务的句柄，若为任务本身，可以使用NULL

- **uxNewPriority 参数**

  目标优先级

### 2.uxTaskPriorityGet()

​	查询任务优先级。

## 四,空闲任务

## 五,队列相关API函数

### 1.xQueueCreate()

​	队列创建函数

​	**在硬件中断中禁止使用这个函数!!**

原型

```c
#define xQueueCreate( uxQueueLength, uxItemSize )    xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
```

- **uxQueueLength参数**

  队列长度

- **uxItemSize参数**

  单个元素大小(单位:queueQUEUE_TYPE_BASE)

  queueQUEUE_TYPE_BASE在queue.h中定义,一般为unsigned char(一个字节)

- **返回值**

  xQueueHandle 类型

  若为NULL则表示分配失败，否则返回队列句柄。

### 2.xQueueSend()

​	向队列的队尾发送数据

原型

```c
#define xQueueSend( xQueue, pvItemToQueue, xTicksToWait ) \
    xQueueGenericSend( ( xQueue ), ( pvItemToQueue ), ( xTicksToWait ), queueSEND_TO_BACK )
```

- **xQueue参数**

  要读取的队列的句柄

- **pvItemToQueue参数**

  要发送的数据的指针(注意大小!)

- **xTicksToWait参数**

  若队列中数据满载,则任务进入阻塞态,等待队列有空余位置,xTicksToWait为最大等待时间。

  如果这个参数设置为portMAX_DELAY,而且定义了INCLUDE_vTaskSuspend为1,则等待没有最大时间限制

- **返回值**

  1. **pdPASS**

     表示成功

  2. **errQUEUE_FULL**

     队列满载

### 3.xQueueReceive()

​	读取队列数据

​	**在硬件中断中禁止使用这个函数!!**

原型

```c
BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,
                          TickType_t xTicksToWait )
```

- **xQueue 参数**

  队列句柄

- **pvBuffer 参数**

  读取到的数据要放入的地址

  *该指针要指向一个静态变量!! static*

- **xTicksToWait 参数**

  若队列为空,任务进入阻塞态,等待队列中有数据进入,xTicksToWait为最大等待时间。

  如果这个参数设置为portMAX_DELAY,而且定义了INCLUDE_vTaskSuspend为1,则等待没有最大时间限制

- **返回值**

  1. **pdPASS**

     成功读取到了数据

  2. **errQUEUE_FULL**

     队列为空

## 五,二值信号量(常用于ISR函数中)

### 1.**vSemaphoreCreateBinary() & xSemaphoreCreateBinary()**

vSemaphoreCreateBinary()已经弃用

```c
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define vSemaphoreCreateBinary( xSemaphore )                                                                                     \
    {                                                                                                                                \
        ( xSemaphore ) = xQueueGenericCreate( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE ); \
        if( ( xSemaphore ) != NULL )                                                                                                 \
        {                                                                                                                            \
            ( void ) xSemaphoreGive( ( xSemaphore ) );                                                                               \
        }                                                                                                                            \
    }
#endif
```

vSemaphoreCreateBinary()采用宏定义的方式,中间使用了xSemaphoreGive()产生了一个二值信号

xSemaphoreCreateBinary()

```c
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    #define xSemaphoreCreateBinary()    xQueueGenericCreate( ( UBaseType_t ) 1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE )
#endif
```

相比vSemaphoreCreateBinary()没有在创建时产生二值信号

**vSemaphoreCreateBinary()直接返回xSemaphoreHandle_t，xSemaphoreCreateBinary()需要传入xSemaphoreHandle_t(不是地址,变量本身就是一个指向结构体的指针)**

### 2.xSemaphoreGive() & xSemaphoreGiveFromISR()

​	发送一个信号量

**xSemaphoreGive原型**

```c
#define xSemaphoreGive( xSemaphore )    xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), NULL, semGIVE_BLOCK_TIME, queueSEND_TO_BACK )
```

- **xSemaphore参数**

  要发送的信号量

- **返回值**

  - **pdPASS**
  - **pdFALSE**

**xSemaphoreGiveFromISR()原型**

```c
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )    xQueueGiveFromISR( ( QueueHandle_t ) ( xSemaphore ), ( pxHigherPriorityTaskWoken ) )
```

- **xSemophore参数**

  要发送的信号量

- **pxHigherPriorityTaskWoken参数**

  用于判断是否有优先级更高的任务在等待这个信号量

  - 若被xSemaphoreGiveFromISR()设置成pdTRUE

    有优先级更高的任务解除了阻塞,需要在中断中进行一次上下文切换 - ***taskYIELD()***

- **返回值**

  - **pdPASS**

    调用成功

  - **pdFASLE**

    已有该信号量

### 3.xSemaphoreTake() & xSemaphoreGiveFromISR()

​	任务进入阻塞态,等待(释放)一个信号量

**xSemaphoreTake()原型**

```c
#define xSemaphoreTake( xSemaphore, xBlockTime )    xQueueSemaphoreTake( ( xSemaphore ), ( xBlockTime ) )
```

- **xSemophore参数**

  要等待的信号量句柄

- **xBlockTime参数**

  最长等待时间,设置为portMAX_DELAY,并且INCLUDE_vTaskSuspend定义为1,则无最长等待时间。

- **返回值**

  - **pdPASS**

    等待到信号量

  - **pdFALSE**

    未能等待到信号量

## 五,常见问题

### 1.任务莫名其妙被饿死

​	可能是任务堆栈溢出!给相关任务多分配一写内存即可。

## 六,示例

### 	1.通过一个任务函数创建多个任务(使用任务参数)

```c
#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void Usart_Send_Task(void*ptr);

TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;

static uint8_t Send1[] = "Task1\r\n";
static uint8_t Send2[] = "Task2\r\n";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();
    
	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		16,
		Send1,
		3,
		&Task1
	);
	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		16,
		Send2,
		3,
		&Task2
	);
	vTaskStartScheduler();
	while(1);
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		printf("%s",(uint8_t*)ptr);
		vTaskDelay(200/portTICK_RATE_MS);
	}
}
```

**注意:任务参数必须定义为静态变量,通过指针传入任务函数。**

### 2.更改任务优先级

```c
#include "stm32f10x.h"

#include "bsp_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void vApplicationIdleHook( void );
void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

static TaskHandle_t Task1 = NULL;
static TaskHandle_t Task2 = NULL;

static uint8_t Send1[] = "Task1\r\n";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();

	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		32,
		Send1,
		6,
		&Task1
	);
	
	xTaskCreate(
		Usart_Rx_Task,
		"Task2",
		32,
		NULL,
		6,
		&Task2
	);
	vTaskStartScheduler();
	while(1);
}

void Usart_Rx_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		uint8_t Target;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					Target = 0;
					for(uint8_t n=0;n<temp;n++)
					{
						Target*=10;
						Target+=*(dat+n+1) - 0x30;
					}
					USART_Push(1,temp+2);
					printf("Input:%d\r\n",Target);
					vTaskPrioritySet(Task1,Target);
					break;
				}
			}
		}
		vTaskDelay(200/portTICK_RATE_MS);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		printf("%s",(uint8_t*)ptr);
		printf("优先级:%d\r\n",uxTaskPriorityGet(NULL));
		vTaskDelay(200/portTICK_RATE_MS);
	}
}
```

### 3.串口指令截取('\n'为指令结束符,基于自写的bsp_usart)

```c
void Usart_Rx_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		uint8_t cmd;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					cmd = 0;
					for(uint8_t n=0;n<temp;n++)
					{
						cmd*=10;
						cmd+=*(dat+n+1) - 0x30;
					}
					USART_Push(1,temp+2);
					printf("Input:%d\r\n",Target);
					break;
				}
			}
		}
		vTaskDelay(200/portTICK_RATE_MS);
	}
}
```

### 4.队列发送结构体

```c
#include "stm32f10x.h"
#include <stdio.h>
#include "bsp_usart.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void Usart_Send_Task(void*ptr);
void Usart_Rx_Task(void*ptr);

static TaskHandle_t Task1 = NULL;
static TaskHandle_t Task2 = NULL;

typedef struct
{
	uint8_t TaskPriority;
	uint16_t TaskDelayTime;
}xDat;

static QueueHandle_t Usart_Cmd = NULL;

static uint8_t Send1[] = "Task1";

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	BSP_Usart_Init();
	Usart_Cmd = xQueueCreate(4,sizeof(xDat));
	xTaskCreate(
		Usart_Send_Task,
		"Task1",
		64,
		Send1,
		6,
		&Task1
	);
	xTaskCreate(
		Usart_Rx_Task,
		"Task2",
		32,
		NULL,
		6,
		&Task2
	);

	if(Usart_Cmd == NULL)
		printf("QueueFlase\r\n");
	else
		vTaskStartScheduler();
	while(1);
}

void Usart_Rx_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		xDat Usart_Rx;
		dat = Usart_Read(1);
		if(*dat != 0)
		{
			for(uint8_t temp=0;temp<*dat;temp++)
			{
				if(*(dat+temp+1) == '\n')
				{
					uint8_t n,m;
					Usart_Rx.TaskPriority = 0;
					Usart_Rx.TaskDelayTime = 0;
					for(n=0;*(dat+n+1)!=' ';n++)
					{
						Usart_Rx.TaskPriority *= 10;
						Usart_Rx.TaskPriority += *(dat+n+1) - 0x30;
					}
					for(m=0;*(dat+n+m+2)!='\n';m++)
					{
						Usart_Rx.TaskDelayTime *= 10;
						Usart_Rx.TaskDelayTime += *(dat+n+m+2) - 0x30;
					}
					printf("%d %d\r\n",Usart_Rx.TaskPriority,Usart_Rx.TaskDelayTime);
					if(xQueueSend(Usart_Cmd,&Usart_Rx,0) == errQUEUE_FULL)
						printf("QueueSendErr\r\n");
					USART_Push(1,temp+1);
					break;
				}
			}
		}
		vTaskDelay(50/portTICK_RATE_MS);
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		static xDat In = {6,300};
		xQueueReceive(Usart_Cmd,&In,0);
		vTaskPrioritySet(NULL,In.TaskPriority);
		printf("%s\r\nDelay_Time:%d\r\nPriority:%d\r\n",(uint8_t*)ptr,In.TaskDelayTime,uxTaskPriorityGet(NULL));
		vTaskDelay(In.TaskDelayTime/portTICK_RATE_MS);
	}
}
```

### 5.ISR中发送信号量

任务函数

```c
SemaphoreHandle_t Usart_Rx_Flag = NULL;
QueueHandle_t Usart_Cmd = NULL;

void Usart_Rx_Task(void*ptr)
{
	while(1)
	{
		uint8_t*dat;
		xDat Cmd;
		xSemaphoreTake(Usart_Rx_Flag,portMAX_DELAY);
		dat = Usart_Read(1);
		if(*(dat+*dat) == '\n')
		{
			uint8_t temp,n;
			Cmd.TaskPriority = 0;
			Cmd.TaskDelayTime = 0;
			for(temp=0;*(dat+temp+1)!=' ';temp++)
			{
				Cmd.TaskPriority*=10;
				Cmd.TaskPriority+=*(dat+temp+1) - 0x30;
			}
			for(n=0;*(dat+temp+n+2)!='\n';n++)
			{
				Cmd.TaskDelayTime*=10;
				Cmd.TaskDelayTime+=*(dat+temp+n+2) - 0x30;
			}
			USART_Clear(1);
			xQueueSend(Usart_Cmd,&Cmd,0);
		}
	}
}

void Usart_Send_Task(void*ptr)
{
	while(1)
	{
		static xDat In = {6,300};
		xQueueReceive(Usart_Cmd,&In,0);
		vTaskPrioritySet(NULL,In.TaskPriority);
		printf("%s\r\nDelay_Time:%d\r\nPriority:%d\r\n",(uint8_t*)ptr,In.TaskDelayTime,uxTaskPriorityGet(NULL));
		vTaskDelay(In.TaskDelayTime/portTICK_RATE_MS);
	}
}
```

中断

```c
extern SemaphoreHandle_t Usart_Rx_Flag;

void USART1_IRQHandler(void)
{
    BaseType_t temp = pdFALSE;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(1,USART_ReceiveData(USART1));
        xSemaphoreGive(Usart_Rx_Flag);
        xSemaphoreGiveFromISR(Usart_Rx_Flag,&temp);
        portEND_SWITCHING_ISR(&temp);
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}
```

**注意:在其他文件里定义的Usart_Rx_Flag不能是静态变量,否则无法通过exturn声明该变量!**

### 6.在缓存式串口ISR方式二值信号量

 \* 在串口数据量过大和波特率过高时不建议使用二值信号量直接触发服务

 \* 函数

 \* 通常采用缓存式,中断产生信号量后,由信号量触发缓存函数。当达到某

 \* 个特殊条件(超时,换行...)时在触发服务函数处理数据

```c
extern SemaphoreHandle_t Usart_Rx_Flag;
void USART1_IRQHandler(void)
{
    BaseType_t temp = pdFALSE;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        Rx_SbufferInput(1,USART_ReceiveData(USART1));
        if(USART_Rx_Sbuffer[0][USART_Rx_Sbuffer[0][0]+1] == '\n')
        {
            xSemaphoreGiveFromISR(Usart_Rx_Flag,&temp);
            portEND_SWITCHING_ISR(&temp);
        }
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}
```

