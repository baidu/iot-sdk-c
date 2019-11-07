xio 模块
================

## 简介

xio是一个实现IO接口的模块。对于上层接口提供了一个数据收发的抽象接口。

## Exposed API

```c
typedef struct XIO_INSTANCE_TAG* XIO_HANDLE;
typedef void* CONCRETE_IO_HANDLE;

typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;

typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;

typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);

typedef OPTIONHANDLER_HANDLE (*IO_RETRIEVEOPTIONS)(CONCRETE_IO_HANDLE concrete_io);
typedef CONCRETE_IO_HANDLE(*IO_CREATE)(void* io_create_parameters);
typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_OPEN)(CONCRETE_IO_HANDLE concrete_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
typedef int(*IO_CLOSE)(CONCRETE_IO_HANDLE concrete_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
typedef int(*IO_SEND)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
typedef void(*IO_DOWORK)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_SETOPTION)(CONCRETE_IO_HANDLE concrete_io, const char* optionName, const void* value);

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

extern XIO_HANDLE xio_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* io_create_parameters);
extern void xio_destroy(XIO_HANDLE xio);
extern int xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
extern int xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
extern int xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
extern void xio_dowork(XIO_HANDLE xio);
extern int xio_setoption(XIO_HANDLE xio, const char* optionName, const void* value);
```

### xio_create

```c
extern XIO_HANDLE xio_create(const IO_INTERFACE_DESCRIPTION* io_interface_description, const void* io_create_parameters);
```

**SRS_XIO_01_001: [** xio_create 会返回一个非NULL的句柄，指向新创建的IO接口。 **]**

**SRS_XIO_01_002: [** 为了实例化具体的IO接口，io_interface_description结构体中的concrete_xio_create将会被调用，传入xio_create_parameters作为参数。 **]**

**SRS_XIO_01_016: [** 如果底层的concrete_xio_create失败，xio_create将会返回NULL。 **]**

**SRS_XIO_01_003: [** 如果参数io_interface_description是NULL，xio_create将会返回NULL。 **]**

**SRS_XIO_01_004: [** 如果io_interface_description中的任何成员是NULL，xio_create将会返回NULL。 **]**

**SRS_XIO_01_017: [** 如果对于IO接口的内存分配失败，xio_create将会返回NULL。 **]**

### xio_destroy

```c
extern void xio_destroy(XIO_HANDLE xio);
```

**SRS_XIO_01_005: [** xio_destroy将会释放IO handle包含的所有资源。 **]**

**SRS_XIO_01_006: [** xio_destroy将会调用具体的concrete_xio_destroy函数进行IO接口的创建，concrete_xio_destroy是io_interface_description的成员之一，是在xio_create的时候传入的函数指针。 **]**

**SRS_XIO_01_007: [** 如果argument为NULL，xio_destroy将什么都不做。 **]**

### xio_open

```c
extern int xio_open(XIO_HANDLE xio, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
```

**SRS_XIO_01_019: [** xio_open会调用特定的 concrete_xio_open 函数，这个函数也是在xio_create中传入的。同时传入的还有以下几个回调函数：open completed, bytes received, and IO error。 **]**

**SRS_XIO_01_020: [** 成功，xio_open将会返回0。 **]**

**SRS_XIO_01_021: [** 如果handle为NULL，xio_open将会返回一个非0值。 **]**

**SRS_XIO_01_022: [** 如果底层的concrete_xio_open失败，xio_open将会返回一个非0值。 **]**

### xio_close

```c
extern int xio_close(XIO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_XIO_01_023: [** xio_close将会调用xio_create传入的特定的concrete_xio_close函数。 **]**

**SRS_XIO_01_024: [** 成功，xio_close会返回0。**]**

**SRS_XIO_01_025: [** 如果参数 xio 是NULL，xio_close将会返回一个非0值。 **]**

**SRS_XIO_01_026: [** 如果底层的concrete_xio_close失败了，xio_close将会返回一个非0值。**]**

### xio_send

```c
extern int xio_send(XIO_HANDLE xio, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_XIO_01_008: [** xio_send shall pass the sequence of bytes pointed to by buffer to the concrete IO implementation specified in xio_create, by calling the concrete_xio_send function while passing down the buffer and size arguments to it. **]**

**SRS_XIO_01_027: [** xio_send shall pass to the concrete_xio_send function the on_send_complete and callback_context arguments. **]**

**SRS_XIO_01_009: [** On success, xio_send shall return 0. **]**

**SRS_XIO_01_010: [** If the argument xio is NULL, xio_send shall return a non-zero value. **]**

**SRS_XIO_01_015: [** If the underlying concrete_xio_send fails, xio_send shall return a non-zero value. **]**

**SRS_XIO_01_011: [** No error check shall be performed on buffer and size. **]**

### xio_dowork

```c
extern void xio_dowork(XIO_HANDLE xio);
```

**SRS_XIO_01_012: [** xio_dowork shall call the concrete IO implementation specified in xio_create, by calling the concrete_xio_dowork function. **]**

**SRS_XIO_01_013: [** On success, xio_send shall return 0. **]**

**SRS_XIO_01_014: [** If the underlying concrete_xio_dowork fails, xio_dowork shall return a non-zero value. **]**

**SRS_XIO_01_018: [** When the io argument is NULL, xio_dowork shall do nothing. **]**

### xio_setoption

```c
extern int xio_setoption(XIO_HANDLE xio, const char* optionName, const void* value);
```

**SRS_XIO_03_028: [** xio_setoption shall pass the optionName and value to the concrete IO implementation specified in xio_create by invoking the concrete_xio_setoption function. **]**

**SRS_XIO_03_029: [** xio_setoption shall return 0 upon success. **]**

**SRS_XIO_03_030: [** If the xio argument or the optionName argument is NULL, xio_setoption shall return a non-zero value. **]**

**SRS_XIO_03_031: [** If the underlying concrete_xio_setoption fails, xio_setOption shall return a non-zero value. **]**

###  xio_retrieveoptions
```
OPTIONHANDLER_HANDLE xio_retrieveoptions(XIO_HANDLE xio)
```

**SRS_XIO_02_001: [** If argument `xio` is `NULL` then `xio_retrieveoptions` shall fail and return `NULL`. **]**

**SRS_XIO_02_002: [** `xio_retrieveoptions` shall create a `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create` passing `xio_setoption` as `setOption` argument and `xio_CloneOption` and `xio_DestroyOption` for `cloneOption` and `destroyOption`. **]**

**SRS_XIO_02_003: [** `xio_retrieveoptions` shall retrieve the concrete handle's options by a call to `concrete_io_retrieveoptions`. **]**

**SRS_XIO_02_004: [** `xio_retrieveoptions` shall add a hardcoded option named `concreteOptions` having the same content as the concrete handle's options. **]**

**SRS_XIO_02_005: [** If any operation fails, then `xio_retrieveoptions` shall fail and return NULL. **]**

**SRS_XIO_02_006: [** Otherwise, `xio_retrieveoptions` shall succeed and return a non-NULL handle. **]**
