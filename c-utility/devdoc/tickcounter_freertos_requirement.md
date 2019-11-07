tickcounter_freertos
=========

## 简介

tickcounter_freertos实现了Baidu IoT C Shared Utility库中的tickcounter适配器接口。该文件可以用在运行FreeRTOS的系统中。

FreeRTOS tick计数功能是通过FreeRTOS系统调用`xTaskGetTickCount`来实现的。因此，需要在此之前已经调用过了`vTaskStartScheduler`。`vTaskStartScheduler` 调用一般发生在FreeRTOS系统初始化的时候。所以在tickcounter_freertos适配器中无需再次调用该函数进行系统初始化了。

FreeRTOS可以通过设置`configUSE_16_BIT_TICKS`变量为0，来配置16位的时钟tick。但是在某些情况下容易导致时钟tick溢出，造成程序异常，所以请谨慎使用这样的配置。  


## 引用
[Baidu IoT C Shared Utility tickcounter 适配器](../../PortingGuide.md#tickcounter适配器)  
[tickcounter.h](../../c-utility/inc/azure_c_shared_utility/tickcounter.h)



## 暴露的 API

**SRS_TICKCOUNTER_FREERTOS_30_001: [** tickcounter_freertos 适配器需要使用以下定义在tickcounter.h的数据类型：
```c
// uint_fast32_t is a 32 bit uint
typedef uint_fast32_t tickcounter_ms_t;
// TICK_COUNTER_INSTANCE_TAG* is an opaque handle type
typedef struct TICK_COUNTER_INSTANCE_TAG* TICK_COUNTER_HANDLE;
```
 **]**  

**SRS_TICKCOUNTER_FREERTOS_30_002: [** The tickcounter_freertos 适配器需要实现以下定义在tickcounter.h中的API接口：
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```
 **]**  


###   tickcounter_create
`tickcounter_create`函数用来分配、初始化一个内部的`TICK_COUNTER_INSTANCE`结构体，并且返回它的指针对象`TICK_COUNTER_HANDLE`。通过判断该指针是否为`NULL`来判断是否创建成功。
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
```


###   tickcounter_destroy
`tickcounter_destroy`函数会释放`tickcounter_create`创建的所有资源。如果`tick_counter`为NULL，该函数将什么都不做。
```c
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
```

###   tickcounter_get_current_ms
`tickcounter_get_current_ms`会返回自`tickcounter_create`调用以来经过的时钟毫秒数。    
如果`tick_counter``current_ms`任一为NULL，该函数将返回一个非零值代表出错。如果正常调用，该函数会设置`*current_ms`为自`tickcounter_create`调用以来经过的时钟毫秒数。同时返回 0 表示成功。
```c
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```

