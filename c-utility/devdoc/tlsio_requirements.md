# tlsio

## 简介

This specification defines generic behavior for tlsio adapters, which operate primarily through the `xio` 
interface and provide communication to remote systems over a TLS-conformant secure channel.该文档定义了tlsio适配器的通用行为。该适配器运行在`xio`接口之上，提供了和远端系统进行TLS数据传输的通道。

## 引用

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)

[xio.h](../../c-utility/inc/azure_c_shared_utility/xio.h)


## 暴露的 API

**SRS_TLSIO_30_001: [** tlsio适配器需要提供所有在IO_INTERFACE_DESCRIPTION（定义在`xio.h`中）中定义接口的具体实现。
```c
typedef OPTIONHANDLER_HANDLE (*IO_RETRIEVEOPTIONS)(CONCRETE_IO_HANDLE concrete_io);
typedef CONCRETE_IO_HANDLE(*IO_CREATE)(void* io_create_parameters);
typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_OPEN)(CONCRETE_IO_HANDLE concrete_io, ON_IO_OPEN_COMPLETE on_io_open_complete, 
    void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, 
    void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
typedef int(*IO_CLOSE)(CONCRETE_IO_HANDLE concrete_io, ON_IO_CLOSE_COMPLETE on_io_close_complete,
    void* callback_context);
typedef int(*IO_SEND)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, 
    ON_SEND_COMPLETE on_send_complete, void* callback_context);
typedef void(*IO_DOWORK)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_SETOPTION)(CONCRETE_IO_HANDLE concrete_io, const char* optionName, 
    const void* value);


typedef struct IO_INTERFACE_DESCRIPTION_TAG
{
    IO_RETRIEVEOPTIONS concrete_io_retrieveoptions;
    IO_CREATE concrete_io_create;
    IO_DESTROY concrete_io_destroy;
    IO_OPEN concrete_io_open;
    IO_CLOSE concrete_io_close;
    IO_SEND concrete_io_send;
    IO_DOWORK concrete_io_dowork;
    IO_SETOPTION concrete_io_setoption;
} IO_INTERFACE_DESCRIPTION;
```
**]**

以下类型（定义在`xio.h`中）在适配器中也会使用到。
```c

typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;

typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;

typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);

typedef struct TLSIO_CONFIG_TAG
{
    const char* hostname;
    int port;
    const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
    void* underlying_io_parameters;
} TLSIO_CONFIG;
```

## 外部状态
tlsio适配器的外部状态，由具体的接口函数调用以及相关的回调函数的调用的不同而不同。适配器的内部状态应该清晰地映射到它的外部状态，但是这种映射不是1：1的。外部状态的定义如下：

* TLSIO_STATE_EXT_CLOSED 该状态表示该模块要么刚刚被创建，通过`tlsio_create`接口，或者也有可能是接收到了`tlsio_close_async` 执行完成的回调，再或者`on_tlsio_open_complete` 回调函数返回了一个 `IO_OPEN_ERROR`。
* TLSIO_STATE_EXT_OPENING 该状态表示`tlsio_open_async` 的调用成功执行完成但是`on_tlsio_open_complete` 回调函数还没有被触发。
* TLSIO_STATE_EXT_OPEN 表示`on_tlsio_open_complete`回调返回了`IO_OPEN_OK`。
* TLSIO_STATE_EXT_CLOSING 表示，要么`tlsio_close_async`被成功执行完成，但是`on_tlsio_close_complete`回调还没有被触发。或者`on_tlsio_open_complete` 回调返回了一个 `IO_OPEN_CANCELLED`状态码。
* TLSIO_STATE_EXT_ERROR 表示  `tlsio_dowork`中调用了`on_io_error`回调接口。

## 状态转换
下面的表显示了各个函数调用对于状态转化的效果，`tlsio_setoption` 和 `tlsio_getoptions` 没有显示是因为他们对于状态切换不起作用。 `tlsio_send_async`也不会切换状态，并且仅当TLSIO_STATE_EXT_OPEN状态下才可以调用。

所有进入 TLSIO_STATE_EXT_CLOSED 状态的转换，其实之前都是经过了TLSIO_STATE_EXT_CLOSING状态的。


<table>
  <tr>From state <b>TLSIO_STATE_EXT_CLOSED</b></tr>
  <tr>
    <td>tlsio_destroy</td>
    <td>ok (destroyed)</td>
  </tr>
  <tr>
    <td>tlsio_open_async</td>
    <td>ok, enter TLSIO_STATE_EXT_OPENING</td>
  </tr>
  <tr>
    <td>tlsio_close_async</td>
    <td>ok, remain in TLSIO_STATE_EXT_CLOSED</td>
  </tr>
  <tr>
    <td>tlsio_dowork</td>
    <td>ok (does nothing), remain in TLSIO_STATE_EXT_CLOSED</td>
  </tr>
</table>

<table>
  <tr>From state <b>TLSIO_STATE_EXT_OPENING</b></tr>
  <tr>
    <td>tlsio_destroy</td>
    <td>log error, force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>tlsio_open_async</td>
    <td>fail, log error, remain in TLSIO_STATE_EXT_OPENING (usage error)</td>
  </tr>
  <tr>
    <td>tlsio_close_async</td>
    <td>ok, force immediate close, enter TLSIO_STATE_EXT_CLOSED</td>
  </tr>
  <tr>
    <td>tlsio_dowork</td>
    <td>ok (continue opening), remain in TLSIO_STATE_EXT_OPENING or enter TLSIO_STATE_EXT_OPEN</td>
  </tr>
</table>

<table>
  <tr>From state <b>TLSIO_STATE_EXT_OPEN</b></tr>
  <tr>
    <td>tlsio_destroy</td>
    <td>(a possible normal SDK behavior) force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>tlsio_open_async</td>
    <td>fail, log error, remain in TLSIO_STATE_EXT_OPEN (usage error)</td>
  </tr>
  <tr>
    <td>tlsio_close_async</td>
    <td>adapters with internal async close: ok, enter TLSIO_STATE_EXT_CLOSING<br/>adatpers without internal async close: ok, enter TLSIO_STATE_EXT_CLOSING, then  <br/>immediately enter TLSIO_STATE_EXT_CLOSED</td>
  </tr>
  <tr>
    <td>tlsio_dowork</td>
    <td>ok (send and receive as necessary), remain in TLSIO_STATE_EXT_OPEN</td>
  </tr>
</table>

<table>
  <tr>From state <b>TLSIO_STATE_EXT_CLOSING</b></tr>
  <tr>
    <td>tlsio_destroy</td>
    <td>log error, force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>tlsio_open_async</td>
    <td>fail, log error, remain in TLSIO_STATE_EXT_CLOSING (usage error)</td>
  </tr>
  <tr>
    <td>tlsio_close_async</td>
    <td>ok, force immediate close, enter TLSIO_STATE_EXT_CLOSED</td>
  </tr>
  <tr>
    <td>tlsio_dowork</td>
    <td>ok (continue graceful closing) , remain in TLSIO_STATE_EXT_CLOSING or <br/>enter TLSIO_STATE_EXT_CLOSED</td>
  </tr>
</table>

<table>
  <tr>From state <b>TLSIO_STATE_EXT_ERROR</b></tr>
  <tr>
    <td>tlsio_destroy</td>
    <td>log error, force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>tlsio_open_async</td>
    <td>fail, log error, remain in TLSIO_STATE_EXT_ERROR (usage error)</td>
  </tr>
  <tr>
    <td>tlsio_close_async</td>
    <td>ok, force immediate close, enter TLSIO_STATE_EXT_CLOSED</td>
  </tr>
  <tr>
    <td>tlsio_dowork</td>
    <td>ok (does nothing), remain in TLSIO_STATE_EXT_ERROR</td>
  </tr>
</table>

![State transition diagram](img/tlsio_state_diagram.png)

## 名词解释 

#### 显示的状态转换

在这篇文档中，我们的状态切换只会通过显示的状态指定而产生。如果没有状态切换发生，默认情况下就是什么都不做。

#### 指定的状态切换

这篇文档中会使用“进入 TLSIO_STATE_EXT_XXXX状态”这种短句来标识某种行为，以下定义了这类状态切换语句：

##### "进入 TLSIO_STATE_EXT_ERROR 状态"
**SRS_TLSIO_30_005: [** "进入 TLSIO_STATE_EXT_ERROR状态"意味着适配器应该调用 `on_io_error` 并且将`tlsio_open_async`获取到的ctx信息传入 `on_io_error_context`  **]**

##### "进入 TLSIO_STATE_EXT_CLOSING 状态"
**SRS_TLSIO_30_009: [** "进入 TLSIO_STATE_EXT_CLOSING 状态" 意味着适配器将会遍历未发送消息队列，并且在调用完 `on_send_complete` 之后删除所有的消息。在调用 `on_send_complete` 时，需要带上`callback_context` 以及 `IO_SEND_CANCELLED`状态。 **]**

##### "进入 TLSIO_STATE_EXT_CLOSED 状态"
**SRS_TLSIO_30_006: [** "进入 TLSIO_STATE_EXT_CLOSED状态"意味着适配器将会强制关闭所有现有的连接，同时调用`on_io_close_complete`回调，同时传入由`tlsio_close_async`传入的 `on_io_close_complete_context`参数 **]**

##### "进入 TLSIO_STATE_EXT_OPENING 状态"
"进入 TLSIO_STATE_EXT_OPENING 状态"意味着适配器将在下一次 `tlsio_dowork` 调用的时候，开启和主机的TLS连接。

##### "进入 TLSIO_STATE_EXT_OPEN 状态"
**SRS_TLSIO_30_007: [** "进入 TLSIO_STATE_EXT_OPEN 状态"意味着适配器将会调用`on_io_open_complete`，同时传入IO_OPEN_OK状态标志以及由`tlsio_open_async`传入的`on_io_open_complete_context` **]**

##### "销毁失败的消息"
**SRS_TLSIO_30_002: [** "销毁失败的消息"意味着适配器将会调用 `on_send_complete` 回调函数，传入`IO_SEND_ERROR`回调标志以及`callback_context`。之后会从消息队列中销毁该消息。 **]**




## API 调用

###   tlsio_get_interface_description
```c
const IO_INTERFACE_DESCRIPTION* tlsio_get_interface_description(void);
```

**SRS_TLSIO_30_008: [** tlsio_get_interface_description 将会返回 IO_INTERFACE_DESCRIPTION类型的一个结构体对象。 **]**


###   tlsio_create
`concrete_io_create`的实现. 
```c
CONCRETE_IO_HANDLE tlsio_create(void* io_create_parameters);
```

##### _直接的_ VS _串联的_ tlsio 适配器

Tlsio适配器有两种不同的实现方式：_直接的_, 以及 _串联的_。一个 _直接的_ tlsio适配器可以直接和服务器直接通过TCP端口进行通讯。一个 _串联的_ tlsio接口自己不拥有一个TCP连接，而是拥有另一个xio适配器 -- 典型的，一般是一个`socketio`适配器，从[这里](../../PortingGuide.md#socketio-adapter-overview)可以得到更多信息。该socketio适配器负责和远程服务器进行TCP通讯。

在本文档中，_拥有的xio资源_ 表示一个xio适配器，该适配器用于和远程服务器进行通讯。它的生命周期由tlsio适配器管理。伴随着`tlsio_create` 和 `tlsio_destroy`的调用，其相应的资源会被创建或回收。

对于 _串联的_ tlsio适配器，它所 _拥有的xio资源_ 由`io_create_parameters`中的`underlying_io_interface` 成员变量指定。如果没有指定的话，那tlsio适配器需要通过提供的`hostname` 和 `port`创建一个`socketio`（定义在[socketio.h](../../c-utility/inc/azure_c_shared_utility/socketio.h)中）。


**SRS_TLSIO_30_010: [** `tlsio_create` 应该分配并且初始化必要的资源，并且返回一个 `tlsio`的实例，处于TLSIO_STATE_EXT_CLOSED状态中。 **]**

**SRS_TLSIO_30_011: [** 如果任何的资源分配失败, `tlsio_create` 将返回 NULL. **]**

**SRS_TLSIO_30_012: [** `tlsio_create` 将会从`io_create_parameters`中获取连接配置信息。 **]**

**SRS_TLSIO_30_013: [** 如果`io_create_parameters` 是 NULL, `tlsio_create` 将记录一个错误并且返回 NULL。 **]**

**SRS_TLSIO_30_014: [** 如果`io_create_parameters`的成员`hostname`是 NULL, `tlsio_create`  将记录一个错误并且返回 NULL。 **]**

**SRS_TLSIO_30_015: [** 如果`io_create_parameters`的成员`port` 小于 0 或者大于 0xffff, `tlsio_create` 将记录一个错误并且返回 NULL。 **]**

**SRS_TLSIO_30_016: [** `tlsio_create` 将会对`io_create_parameters`的成员`hostname`制作一份拷贝，并且允许 `hostname` 在该操作后立刻被删除. 这份拷贝会传递给底层的 `xio`. **]**

**SRS_TLSIO_30_017: [** 对于 _直接_ 模式, 如果`io_create_parameters`中的成员`underlying_io_interface` 或 `underlying_io_parameters`不是NULL, `tlsio_create` 将记录一个错误并且返回 NULL。 **]**

**SRS_TLSIO_30_018: [** 对于 _串联_ 模式，如果`io_create_parameters`中的成员`underlying_io_interface`不是NULL，`tlsio_create`将会使用`underlying_io_interface`来创建一个 _拥有的xio资源_ 来进行远程的TCP通讯。 **]**

**SRS_TLSIO_30_019: [** 对于 _串联_ 模式，`underlying_io_interface`参数是非NULL的，`tlsio_create`将会传入`underlying_io_parameters`给`underlying_io_interface->xio_create`来创建 _拥有的xio资源_  **]**


###   tlsio_destroy
`concrete_io_destroy`的实现。
```c
void tlsio_destroy(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_30_020: [** 如果 `tlsio_handle`是NULL，`tlsio_destroy`将会什么都不做。 **]**

**SRS_TLSIO_30_021: [** `tlsio_destroy`将会释放所有分配的资源，之后释放`tlsio_handle`句柄。 **]**

**SRS_TLSIO_30_022: [** 如果在调用 `tlsio_destroy` 时，adapter处于非 TLSIO_STATE_EXT_CLOSED 的任意一个状态，adapter应该[进入 TLSIO_STATE_EXT_CLOSING 状态](#enter-TLSIO_STATE_EXT_CLOSING "遍历未发送的消息队列，调用该消息的 `on_send_complete`方法（传入相应的`callback_context` 和 `IO_SEND_CANCELLED`状态）之后删除该消息。")然后在完成destroy操作之前，[进入 TLSIO_STATE_EXT_CLOSED 状态](#enter-TLSIO_STATE_EXT_CLOSED "强制关闭任何现存的连接，然后调用 `on_io_close_complete` 函数（其中的`on_io_close_complete_context` 参数就是 `tlsio_close_async`函数传进来的）")  **]**


###   tlsio_open_async
`concrete_io_open` 的实现

```c
int tlsio_open_async(
    CONCRETE_IO_HANDLE tlsio_handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete,
    void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context);
```

**SRS_TLSIO_30_030: [** 如果 `tlsio_handle` 参数是 NULL，`tlsio_open_async` 应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_031: [** 如果参数on_io_open_complete是NULL，`tlsio_open_async` 应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_032: [** 如果参数 on_bytes_received 是NULL，`tlsio_open_async` 应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_033: [** 如果参数 on_io_error 是NULL，`tlsio_open_async` 应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_037: [** 如果在调用 `tlsio_open_async` 时 adapter 处于除 TLSIO_STATE_EXT_CLOSED 外任意一种状态，`tlsio_open_async` 应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_038: [** 如果 `tlsio_open_async` [进入 TLSIO_STATE_EXT_OPENING 状态](#enter-TLSIO_STATE_EXT_OPENING "在下一次`tlsio_dowork`调用的过程中继续处理和远程服务器的TLS握手连接过程")失败，它应该返回`_FAILURE_`。  **]**

**SRS_TLSIO_30_039: [** 失败，`tlsio_open_async`不应该调用`on_io_open_complete`  **]**

**SRS_TLSIO_30_034: [** 成功，`tlsio_open_async`应该保存`on_bytes_received`,  `on_bytes_received_context`, `on_io_error`, `on_io_error_context`, `on_io_open_complete`,  以及 `on_io_open_complete_context`这些参数。这些参数会在后续的处理工程中使用到。 **]**

**SRS_TLSIO_30_035: [** 成功，`tlsio_open_async` 应该导致adapter[进入 TLSIO_STATE_EXT_OPENING 状态](#enter-TLSIO_STATE_EXT_OPENING "在下一次`tlsio_dowork`调用的过程中继续处理和远程服务器的TLS握手连接过程")并且返回0。 **]**


###   tlsio_close_async
`concrete_io_close` 的实现
```c
int tlsio_close_async(CONCRETE_IO_HANDLE tlsio_handle, 
    ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_TLSIO_30_050: [** 如果 `tlsio_handle` 参数是NULL, `tlsio_close_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_055: [** 如果 `on_io_close_complete` 参数是NULL, `tlsio_close_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_054: [** 如果失败，该适配器不应该调用`on_io_close_complete`。 **]**

**SRS_TLSIO_30_053: [** 如果适配器处于任何非 TLSIO_STATE_EXT_OPEN 或 TLSIO_STATE_EXT_ERROR 的状态，则`tlsio_close_async`应该记录`tlsio_close_async`已经被调用了，继续之后的操作。 **]**

**SRS_TLSIO_30_057: [** 如果成功，并且如果adapter处于 TLSIO_STATE_EXT_OPENING 状态，它应该调用 `on_io_open_complete`函数（其中的`on_io_open_complete_context`参数是 `tlsio_open_async`传入的，状态设置为 `IO_OPEN_CANCELLED`）。该回调应该在改变adaptor内部状态之前被调用。 **]**

**SRS_TLSIO_30_056: [** 如果成功，adapter应该[进入 TLSIO_STATE_EXT_CLOSING 状态](#enter-TLSIO_STATE_EXT_CLOSING "遍历未发送的消息队列，调用该消息的 `on_send_complete`方法（传入相应的`callback_context` 和 `IO_SEND_CANCELLED`状态）之后删除该消息。") **]**

**SRS_TLSIO_30_051: [** 如果成功，并且如果底层的TLS接口不支持异步的 close 或者适配器不在 TLSIO_STATE_EXT_OPEN 状态，则适配器应该在进入 TLSIO_STATE_EXT_CLOSING 状态之后立刻[进入 TLSIO_STATE_EXT_CLOSED 状态](#enter-TLSIO_STATE_EXT_CLOSED "强制关闭任何现存的连接，然后调用 `on_io_close_complete` 函数（其中的`on_io_close_complete_context` 参数就是 `tlsio_close_async`函数传进来的）") **]**

**SRS_TLSIO_30_052: [** 如果成功，`tlsio_close_async`应该返回0。 **]**


###   tlsio_send_async
`concrete_io_send` 的实现
```c
int tlsio_send_async(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, 
    size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_TLSIO_30_060: [** 如果 `tlsio_handle` 参数是NULL, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_061: [** 如果 `buffer` 参数是NULL, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_062: [** 如果 `on_send_complete` 参数是NULL, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_067: [** 如果 `size` 参数是0, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_065: [** 如果 adapter 当前状态不是 TLSIO_STATE_EXT_OPEN, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_064: [** 如果提供的message无法加入发送队列, `tlsio_send_async`应该记录一个错误并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_066: [** 如果失败了，`on_send_complete`不应该被调用。 **]**

**SRS_TLSIO_30_063: [** 如果成功， `tlsio_send_async`会把数据加入发送队列，同时返回0. **]**


###   tlsio_dowork
`concrete_io_dowork`的实现
```c
void tlsio_dowork(CONCRETE_IO_HANDLE tlsio_handle);
```
`tlsio_dowork`调用会处理tlsio异步的工作，包括建立TLS连接，发送数据，检测可读状态。

**SRS_TLSIO_30_070: [** 如果`tlsio_handle`参数是NULL，`tlsio_dowork`将什么都不做，只会记录一个错误。 **]**


#### 行为选择

**SRS_TLSIO_30_071: [** 如果adapter处于 TLSIO_STATE_EXT_ERROR 状态，那么 `tlsio_dowork` 应该什么都不做。 **]**

**SRS_TLSIO_30_075: [** 如果adapter处于 TLSIO_STATE_EXT_CLOSED 状态，那么 `tlsio_dowork` 应该什么都不做。 **]**

**SRS_TLSIO_30_077: [** 如果adapter处于 TLSIO_STATE_EXT_OPENING 状态，那么 `tlsio_dowork` 应该只进行[TLSIO_STATE_EXT_OPENING 操作](#TLSIO_STATE_EXT_OPENING-behaviors) **]**

**SRS_TLSIO_30_077: [** 如果adapter处于 TLSIO_STATE_EXT_OPEN 状态，那么 `tlsio_dowork` 应该只进行[数据发送操作](#data-transmission-behaviors) 以及 [数据接收操作](#data-reception-behaviors)。 **]**

**SRS_TLSIO_30_078: [** 如果adapter处于 TLSIO_STATE_EXT_CLOSING 状态，那么 `tlsio_dowork` 应该只进行[TLSIO_STATE_EXT_CLOSING 操作](#TLSIO_STATE_EXT_CLOSING-behaviors) **]**

#### TLSIO_STATE_EXT_OPENING 行为

从 TLSIO_STATE_EXT_OPENING 到 TLSIO_STATE_EXT_OPEN 的状态转换，可能涉及到多个`tlsio_dowork`操作。具体的操作个数不确定。

**SRS_TLSIO_30_080: [** `tlsio_dowork`会更具`tlsio_open_async`中提供 `hostName` 和 `port` 进行TLS连接。 **]**

**SRS_TLSIO_30_082: [** 如果连接过程因为任何原因失败了。`tlsio_dowork`应该记录一条错误，并且调用 `on_io_open_complete`（带上`tlsio_open_async`中的`on_io_open_complete_context`参数以及`IO_OPEN_ERROR`状态，同时[进入 TLSIO_STATE_EXT_CLOSED状态](#enter-TLSIO_STATE_EXT_CLOSED "强制关闭任何现存的连接，然后调用 `on_io_close_complete` 函数（其中的`on_io_close_complete_context` 参数就是 `tlsio_close_async`函数传进来的）")）  **]**

**SRS_TLSIO_30_083: [** 如果`tlsio_dowork`成功打开了TLS连接，apapter将会[进入 TLSIO_STATE_EXT_OPEN状态](#enter-TLSIO_STATE_EXT_OPEN "调用 `on_io_open_complete`函数，并且传入 IO_OPEN_OK 状态标志")。 **]**

#### 数据发送行为


**SRS_TLSIO_30_091: [** 如果 `tlsio_dowork` 可以一次性发送完一条message中的所有字节数据，他将首先从消息队列中删除该消息，然后调用该消息的 `on_send_complete` 回调方法，该方法中还需带上 `IO_SEND_OK`状态标志位。 **]**<br/>
在调用该回调函数前需要从消息队列中dequeue该消息，这是因为该回调函数可能会触发一次可重入的`tlsio_close`操作，该操作需要队列的一致性。

**SRS_TLSIO_30_093: [** 如果TLS连接接口无法一次性发送完一个message中所有的数据，后续的 `tlsio_dowork`调用会继续这个发送操作，知道把剩余的字节发送完毕。 **]**

**SRS_TLSIO_30_095: [** 如果进程在发送message中数据时遇到错误导致发送失败，`tlsio_dowork`会[销毁失败的消息](#destroy-the-failed-message "从消息队列中移除该消息，并且调用该消息的`on_send_complete` 方法，填入相对应的`callback_context`参数 以及 `IO_SEND_ERROR`状态标志") 同时 [进入 TLSIO_STATE_EXT_ERROR 状态](#enter-TLSIO_STATE_EXT_ERROR "调用 `on_io_error`回调函数，同时传入`tlsio_open_async`提供的 `on_io_error_context`参数") **]**

**SRS_TLSIO_30_096: [** 如果消息队列中没有消息，`tlsio_dowork`将会什么都不做。 **]**

#### 数据接收行为

**SRS_TLSIO_30_100: [** 只要TLS连接一直有接收数据，`tlsio_dowork` 将会持续地读取这些数据，同时调用`on_bytes_received`回调通知上层。该回调函数中会带上接收数据buffer的地址，以及接收到的字节个数，还有相应的`on_bytes_received_context`参数。 **]**

**SRS_TLSIO_30_102: [** If the TLS connection receives no data then `tlsio_dowork` shall not call the  `on_bytes_received` callback.如果TLS连接当前没有数据接收，`tlsio_dowork`不会调用`on_bytes_received`回调。 **]**

#### TLSIO_STATE_EXT_CLOSING 行为

对于没有异步特性的'closing'状态的TLS连接，适配器将不会有一个显示的TLSIO_STATE_EXT_CLOSING状态，同时他们的 `tlsio_dowork` 也不会执行这些动作。

**SRS_TLSIO_30_106: [** 如果'closing'的操作失败，`tlsio_dowork`会记录一个error并[进入 TLSIO_STATE_EXT_CLOSED 状态](#enter-TLSIO_STATE_EXT_CLOSED "强制关闭任何现存的连接，然后调用 `on_io_close_complete` 函数（其中的`on_io_close_complete_context` 参数就是 `tlsio_close_async`函数传进来的）") **]**

**SRS_TLSIO_30_107: [** 如果'closing'的操作成功，`tlsio_dowork`将会[进入 TLSIO_STATE_EXT_CLOSED 状态](#enter-TLSIO_STATE_EXT_CLOSED "强制关闭任何现存的连接，然后调用 `on_io_close_complete` 函数（其中的`on_io_close_complete_context` 参数就是 `tlsio_close_async`函数传进来的）") **]**

###   tlsio_setoption
`concrete_io_setoption`的实现. 特定的tlsio实现可能会需要定义相关`tlsio_setoption`的行为。

options是`tlsio_create`过程中创建的一部分，这些option会持久化直到对象被销毁。
```c
int tlsio_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value);
```
**SRS_TLSIO_30_120: [** 如果`tlsio_handle`参数是NULL，`tlsio_setoption`不会做任何事，除了记录一条日志并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_121: [** 如果`optionName`参数是NULL，`tlsio_setoption`不会做任何事，除了记录一条日志并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_122: [** 如果`value`参数是NULL，`tlsio_setoption`不会做任何事，除了记录一条日志并且返回 `_FAILURE_`。 **]**

**SRS_TLSIO_30_124 [** 实现了`options`接口的适配器需要保存这些`option`信息直到`tlsio_destroy`被调用。 **]**


###   tlsio_retrieveoptions
`concrete_io_retrieveoptions`的实现。特定的tlsio实现可能会需要定义相关`tlsio_retrieveoptions`的行为。

```c
OPTIONHANDLER_HANDLE tlsio_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_30_160: [** 如果`tlsio_handle`参数是NULL，`tlsio_retrieveoptions`不会做任何事，除了记录一条日志并且返回 `_FAILURE_`。 **]**

### 错误恢复测试
tlsio适配器的错误恢复需要上层模块的支持。也有大量可选的流程用于错误恢复。罗列所有的retry逻辑流程已经超出了本文档的范围。但是这里提供的两个测试用例展示了一套最小的重连流程。上层的模块可以参考。

本节中的测试场景特意没有说明，而是留给了实现者。单元测试中的注释应该足够用于相关的文档注解。

在本节中提到的"上层重连流程"表示:
  1. 在程序运行中某个点发生了一个错误。
  2. `tlsio_close_async`被调用，并且接受到了`on_io_close_complete` 回调。
  3. `tlsio_open_async` 已经成功被调用。
  4. `tlsio_dowork` 已经被足够地调用以确保事件处理的正常进行.
  5. 接收到`on_io_open_complete`回调，并且状态标志为`IO_OPEN_OK`.

需要注意的是，本节中提到的测试需求都存在于相应的单元测试用例文件中，但是不会出现在代码实现中。
