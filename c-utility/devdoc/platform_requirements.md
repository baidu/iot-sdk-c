# platform


## 简介

该文档将解释说明Baidu IoT C SDK中的 **platform** 适配器。 _platform_ 适配器的作用就是在SDK启动或者退出的时候，进行平台必要的全局的“初始化”或“反初始化”，比如对于windwos socket编程接口中的`WSAStartup` 和 `WSACleanup`。它也会通过`platform_get_default_tlsio`函数提供合适的TLSIO适配器。

尽管platform适配器提供了一种全局的初始化，反初始化的机制。但是开发者有时候需要在SDK之外进行初始化，这个时候`platform_init` 和 `platform_deinit`可以什么都不做，设置成空函数即可。

### 引用 
[Baidu IoT porting guide](../../PortingGuide.md)<br/>
[platform.h](../../c-utility/inc/azure_c_shared_utility/platform.h)<br/>
[xio.h](../../c-utility/inc/azure_c_shared_utility/xio.h)


###   暴露的 API
适配器必须实现[platform.h](../../c-utility/inc/azure_c_shared_utility/platform.h)中的几个接口:
`platform_init`, `platform_deinit`, 以及 `platform_get_default_tlsio`. 第四个接口`platform_get_platform_info`不被SDK使用到，可以忽略。

###   platform_init

`platform_init` 可以对于特定的平台进行所需的全局初始化工作。

```c
int platform_init();
```

**SRS_PLATFORM_30_000: [** `platform_init` 可以对于特定的平台进行所需的全局初始化工作。成功返回0。 **]**

**SRS_PLATFORM_30_001: [** 失败返回非0值。 **]**


###   platform_deinit

`platform_deinit` 可以对于特定的平台进行所需的全局逆初始化工作。

```c
void platform_deinit();
```

**SRS_PLATFORM_30_010: [** `platform_deinit` 可以对于特定的平台进行所需的全局逆初始化工作。 **]**


###   platform_get_default_tlsio

该调用返回`IO_INTERFACE_DESCRIPTION*` 类型的tlsio接口，该接口的详细定义在
[xio.h](../../c-utility/inc/azure_c_shared_utility/xio.h).



```c
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void);
```

**SRS_PLATFORM_30_020: [** 该调用返回`IO_INTERFACE_DESCRIPTION*` 类型的tlsio接口。 **]**
