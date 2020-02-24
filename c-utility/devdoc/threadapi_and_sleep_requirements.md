# threadapi 和 sleep

## 简介

本文档定义了Baidu IOT SDK 中**sleep**以及**threadapi**适配器相关的接口以及行为。这两个适配器合在一起说是因为他们都是定义在同一个[threadapi.h](../../c-utility/inc/azure_c_shared_utility/threadapi.h)头文件中的。

之所以说 _sleep_ 以及 _threadapi_ 适配器是相关的，是应为他们共享了同一个头文件并且都是涉及处理线程相关的功能。但同时他们又是不同的适配器，因为在当前版本的SDK中，_sleep_ 适配器是必须的，而 _threadapi_ 适配器是可选的。

## 引用

[Baidu IoT porting guide](../../PortingGuide.md)<br/>
[threadapi.h](../../c-utility/inc/azure_c_shared_utility/threadapi.h)

## 暴露的 API

_threadapi_ 使用了如下的类型：
```
typedef void* THREAD_HANDLE;

typedef enum
{
    THREADAPI_OK,
    THREADAPI_INVALID_ARG,
    THREADAPI_NO_MEMORY,
    THREADAPI_ERROR,
} THREADAPI_RESULT;
```
##   sleep 适配器

###   ThreadAPI_Sleep
_sleep_ 适配器就暴露了唯一的一个函数，`ThreadAPI_Sleep`。这个函数对于当前版本的SDK来说是必须的。

```c
void ThreadAPI_Sleep(unsigned int milliseconds);
```

**SRS_THREADAPI_30_001: [** ThreadAPI_Sleep 应该挂起当前线程一定的时间。时间由`milliseconds`指定，单位是毫秒。 **]**  

## threadapi 适配器

###   ThreadAPI_Create

创建一个线程，入口函数由`func`参数指定。具体的`void* THREAD_HANDLE`类型由开发者自己指定。

```c
THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg);
```

**SRS_THREADAPI_30_010: [** 如果 **threadapi** 适配器没有被实现， `ThreadAPI_Create` 应该返回`THREADAPI_ERROR`。 **]**

**SRS_THREADAPI_30_011: [** 如果 `threadHandle` 为NULL `ThreadAPI_Create` 应该返回 `THREADAPI_INVALID_ARG`。 **]**

**SRS_THREADAPI_30_012: [** 如果 `func` 为NULL `ThreadAPI_Create` 应该返回 `THREADAPI_INVALID_ARG` **]**

**SRS_THREADAPI_30_013: [** 如果 `ThreadAPI_Create` 无法创建一个线程，他应该返回`THREADAPI_ERROR` 或 `THREADAPI_NO_MEMORY` **]**

**SRS_THREADAPI_30_014: [** 如果成功， `ThreadAPI_Create` 应该通过 `threadHandle` 返回成功创建的线程句柄。 **]**

**SRS_THREADAPI_30_015: [** 如果成功， `ThreadAPI_Create` 应该返回 `THREADAPI_OK`. **]**


###   ThreadAPI_Join

等待由 `threadHandle` 参数指定的线程完成。当 `threadHandle` 线程结束后，所有的线程资源都应该被释放，并且线程句柄也不再有效。

```c
THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* result);
```
**SRS_THREADAPI_30_020: [** 如果 **threadapi** 适配器没有被实现， `ThreadAPI_Join` 应该返回`THREADAPI_ERROR`。 **]**

**SRS_THREADAPI_30_021: [** 如果 `threadHandle` 为NULL `ThreadAPI_Join` 应该返回 `THREADAPI_INVALID_ARG`。 **]**

**SRS_THREADAPI_30_022: [** 如果 `ThreadAPI_Join` 失败了，它应该返回 `THREADAPI_ERROR`。 **]**

**SRS_THREADAPI_30_023: [** 如果成功，`ThreadAPI_Join` 应该等待直到`threadHandle`参数指定的线程完成。 **]**

**SRS_THREADAPI_30_024: [** 如果成功，所有的线程资源都应该被释放。 **]**

**SRS_THREADAPI_30_025: [** 如果成功，并且 `result` 不是NULL的话，`THREAD_START_FUNC` 的执行结果将会返回在 `result`中。 **]**

**SRS_THREADAPI_30_026: [** 如果成功，`ThreadAPI_Join` 应该返回 `THREADAPI_OK`. **]**

###   ThreadAPI_Exit

当 `THREAD_START_FUNC` 退出的时候，这个函数将会被调用。同时如果有其他线程之前调用了 `ThreadAPI_Join` ，那么那个线程将会从阻塞状态进入就绪状态。

threadapi 适配器的实现必须维护好通过`ThreadAPI_Join`传入的`THREAD_HANDLE` 线程以及当前运行的线程之间的关系。以便如果有多个 `THREAD_HANDLE`被创建的话，`ThreadAPI_Exit` 将正确的解锁某个等待线程。这种关系需要本地线程存储（或者其他类似的方式）。这个线程函数有时候在使用过程中是存在歧义的，因为我们无法保证是否多个线程正在使用同一套thead函数。


```c
void ThreadAPI_Exit(int result);
```
在`ThreadAPI_Exit`的说明里，所谓的“相关联的`ThreadAPI_Join`”，就是`ThreadAPI_Join` 和 `ThreadAPI_Exit` 使用的 `THREAD_HANDLE` 就是我们通过`ThreadAPI_Create`创建的那个，他们使用的是同一个 thread 句柄。

**SRS_THREADAPI_30_030: [** 如果 **threadapi** 适配器没有被实现，`ThreadAPI_Exit` 应该什么都不做。 **]**

**SRS_THREADAPI_30_031: [** 如果另外的一个线程调用了一个“相关联的`ThreadAPI_Join`”，则该线程会在`ThreadAPI_Exit`被调用的时候被解锁，成为就绪态。 **]**

**SRS_THREADAPI_30_032: [** `ThreadAPI_Exit` 中的 `result` 参数，也就是`ThreadAPI_Join`中指定的线程返回参数`result` **]**
