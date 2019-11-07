AgentTime
================

## 简介

AgentTime 实现了一套与平台无关的时间相关的函数。它是一个平台抽象层，针对不同的平台必须单独实现。大多数情况下，这些函数都可以简单调用C标准库中的相应的time函数实现。

大多数对于 C `time()`函数的实现，都是返回自从1970/1/1 00:00 开始到现在的秒数。

###### 头文件
- [agenttime.h](../../c-utility/inc/azure_c_shared_utility/agenttime.h)<br/>


## 暴露的API
**SRS_AGENT_TIME_99_001: [** AGENT_TIME 需要如下接口 **]**
```c
/* 和C库的 time() 函数实现相同的功能 */
time_t get_time(time_t* p);

/* 和C库的 difftime() 函数实现相同的功能 */
extern double get_difftime(time_t stopTime, time_t startTime);
```

**SRS_AGENT_TIME_30_002: [** 这个接口中`time_t`的值应该是自从1970/1/1 00:00 开始到现在的秒数。 **]**

**SRS_AGENT_TIME_30_003: [** `get_gmtime`,  `get_mktime`, 以及  `get_ctime` 已经废弃，不应该被使用了。 **]**
