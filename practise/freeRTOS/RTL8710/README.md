本文简要介绍如何在`RTL8710`wifi模组上快速适配`iot-edge-c-sdk`，以及适配过程中的一些注意事项。

## 关于mbedtls
这里采用`mbedtls`作为tls库，而对于如何移植`mbedtls`，本文不做详细讲解，可以参考相关文档。这里只提供经过裁剪之后的`config.h`。供读者参考。

## 准备代码
1. 从github clone `iot-edge-c-sdk` 至你的工程目录。

2. 在用户的 Makefile 中 include `module.mk`。该文件中摘取了运行一个最简单的`SmartHome` Demo所需要的源文件，以及所需要的编译参数。
3. 修改`module.mk`中的`BAIDU_SRC`为`iot-edge-c-sdk`的目录路径。

4. 拷贝`platform`文件夹到`iot-edge-c-sdk`工程目录下。我们把客户需要适配的文件单独抽取出来放在该文件夹下。该文件夹中，除了需要适配的文件，还有一些针对该平台定制化的一些模块（比如，去掉一些用不到的功能，减少code size）。


以上，代码准备工作就OK了。

## 开始适配
需要适配的文件/函数，在下面已经列出。在代码中均有标记(标记"TODO")，请读者根据自己系统需求，替换成本平台的相关调用。
```bash
platform/agenttime.c                --      `get_time` `get_time`
platform/platform_freertos.c        --      `platform_init`
platform/socketio_berkeley.c        --      所有socket操作相关函数/宏，带有`xxx_`前缀
platform/threadapi_freertos.c       --      `ThreadAPI_Sleep`
platform/tickcounter_freertos.c     --      `tickcounter_create` `tickcounter_get_current_ms`
```


## 编译，测试
以上适配结束，就可以编译测试了，在用户的主逻辑中，直接调用测试代码
```
...
iot_smarthome_client_run(false);
...
```
就可以测试一个最基本的`SmartHome` Demo了。