该工程遵守[微软开源行为准则](https://opensource.microsoft.com/codeofconduct/)。更多信息，请参考[行为准则 FAQ](https://opensource.microsoft.com/codeofconduct/faq/)或者联系[opencode@microsoft.com](mailto:opencode@microsoft.com)。

# Azure C Shared Utility

azure-c-shared-utility是一个C库，用来给程序提供基础的功能模块（比如string，列表操作，IO等等）。

## 依赖

azure-c-shared-utility提供了三个tlsio适配器的实现:
- tlsio_schannel - 只运行在Windows系统
- tlsio_openssl - 依赖系统安装的OpenSSL库
- tlsio_wolfssl - 依赖系统安装的WolfSSL库

对于Linux系统azure-c-shared-utility中的HTTPAPI模块还依赖curl库。

azure-c-shared-utility使用cmake来配置以及编译工程。

## 使用步骤

1. 使用Git clone **azure-c-shared-utility** 的时候加上`recursive`参数:

```
git clone --recursive https://github.com/Azure/azure-c-shared-utility.git
```

2. 在*azure-c-shared-utility*中创建一个 *cmake* 文件夹

3. 进入 *cmake* 文件夹运行
```
cmake ..
```

4. 编译

```
cmake --build .
```

### 安装 和 使用
可选地，你可能会想在自己的机器上安装 azure-c-shared-utility:

1. 进入*cmake* 文件夹并运行
    ```
    cmake -Duse_installed_dependencies=ON ../
    ```
    ```
    cmake --build . --target install
    ```

    或者对于不同的平台使用如下的命令进行安装:

    对于Linux系统:
    ```
    sudo make install
    ```

    对于Windows系统:
    ```
    msbuild /m INSTALL.vcxproj
    ```

2. 在你的工程中使用该库（如果你已经安装了的话）
    ```
    find_package(azure_c_shared_utility REQUIRED CONFIG)
    target_link_library(yourlib aziotsharedutil)
    ```

_如果要运行测试程序的话，需要在您的机器上通过CMake安装umock-c, azure-ctest, and azure-c-testrunnerswitcher等库。_

### 编译测试程序

通过如下命令编译测试程序:

```
cmake .. -Drun_unittests:bool=ON
```

## 配置选项

使用如下 CMAKE 选项来开/关 tlsio 适配器的实现:

* `-Duse_schannel:bool={ON/OFF}` - 开/关SChannel的支持。
* `-Duse_openssl:bool={ON/OFF}` - 开/关 OpenSSL 的支持。如果使用该选项，`OpenSSLDir`环境变量需要设置为指向系统安装的OpenSSL库目录。
* `-Duse_wolfssl:bool={ON/OFF}` - 开/关 WolfSSL 支持。如果使用该选项，`WolfSSLDir`环境变量需要设置为指向系统安装的OpenSSL库目录。
* `-Duse_http:bool={ON/OFF}` - 开/关 HTTP API 的支持。
* `-Duse_installed_dependencies:bool={ON/OFF}` - 开/关是否在编译的时候使用已经安装的依赖。该包只有在该flag为ON的时候才会被安装。
* `-Drun_unittests:bool={ON/OFF}` - 是否编译单元测试。默认是 OFF。


## 移植到新的设备

将Baidu IoT C SDK移植到新的设备上的方法，可以参考
[这里](../PortingGuide.md).