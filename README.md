## 百度云天工物联网平台 IoTCore SDK for C ##

此代码库包含以下组件：

- iotcore_client(IoTCore SDK for C) 帮助设备快速接入百度云天工IoTCore服务
- Serializer函数库帮助进行序列化和反序列化数据，来进行数据在设备上的存储或读取

## IoTCore SDK for C ##
- 代码使用ANSI C（C99）规范，从而使代码更方便移植到不同的平台中
- 请避免使用编译器扩展选项，防止在不同平台上编译的不同行为表现
- 在IoTCore SDK中，使用了一个平台抽象层，以隔离操作系统相关性（线程和互斥机制，通信协议，例如HTTP等）。


## 如何克隆资源库 ##
该代码库使用了一些第三方库作为子模块依赖关系。为了自动克隆这些子模块，您需要使用如下所述的--recursive选项：

    	git clone --recursive https://github.com/baidu/iot-sdk-c.git

## OS平台和硬件兼容性 ##
百度天工IoTCore SDK可用于广泛的操作系统平台和设备。对于设备的最低要求是：

- 能够建立IP连接：只有IP功能的设备可以直接与IoTCore进行通信。
- 支持TLS（可选）：推荐设备使用TLS来与IoTCore进行安全连接。 但这不是必需的。 IoTCore SDK也支持使用用户名/密码的方式进行非加密通讯
- 具有实时时钟或实现代码连接到NTP服务器（可选）：如果你使用TLS连接或使用安全令牌以进行身份​​验证，时钟同步是必需的。
- 具有至少64KB的RAM：SDK的具体内存占用取决于所使用的SDK文件、协议以及目标平台。我们尽可能将占用减到最低。

## SDK目录结构 ##


- /c-utility

	引用的git子模块，使用的第三方工具库。请注意，其中可能包含嵌套子模块。

- /umqtt

	引用的git子模块，使用的第三方MQTT客户端。请注意，其中可能包含嵌套子模块。

- /parson

	引用的git子模块，使用的第三方的JSON库。请注意，其中可能包含嵌套子模块

- /certs

	包含与 IoT Core 进行通信所需的证书。

- /build_all

	包含客户端库和相关组件的针对指定平台的编译脚本。

- /iotcore_client

	包含IoTCore客户端组件，连接IoTCore服务，并发送和接受消息。有关如何使用它的信息，请参阅[https://cloud.baidu.com/doc/IoTCore/index.html](https://cloud.baidu.com/doc/IoTCore/index.html "物联网核心套件 IoTCore")

- /serializer

	包含在原始消息库之上提供存储和JSON序列化功能的库。这些库便于上传结构化数据以及用于 IoT Core 服务的命令和控制。



## 准备开发环境 ##

这篇文章介绍如何准备开发环境来使用百度云天工的c语言版本的IoTCore SDK。这里主要介绍如何配置windows和Linux下的开发环境。

### 关于openssl ###

目前openssl LTS的版本有两个：`1.0.2x`和`1.1.x` ，两者的区别在于，后者在前者的基础上优化了API接口，一些数据结构已经对开发者透明，也就是说，如果在`1.0.2x`可以编译成功的软件，在`1.1.x`版本的openssl下就无法编译成功了。正是因为这个原因，openssl组织仍需要维护之前老接口的openssl版本（因为大量软件使用的仍旧是老接口），也就是`1.0.2x`，但这两个版本在安全性上是等价的。本sdk使用的是`1.0.2x`版本。请务必[下载](https://www.openssl.org/source/)最新版本的openssl！如果是用git clone openssl仓库，也务必使用`-b`参数注明版本！
```
git clone https://github.com/openssl/openssl.git -b OpenSSL_1_0_2-stable
```

### 配置Windows的开发环境 ###

- 安装Visual Studio 2015（或其他版本Visual Studio，针对其他版本VS，需使用相应版本的VS命令替换以下与版本号相关的命令内容），你可以使用Visual Studio Community免费版本通过遵循license许可

- 安装Visual C++ 和 NuGet安装包管理工具

- 安装git

	确认git是否在你的PATH环境变量目录列表，你可以使用git version来检查git的版本

    		git version

- 安装CMake

	确认CMake在你的PATH环境变量目录列表，你可以使用cmake –version来测试安装是否正常并且检测版本。可以使用CMake来创建Visual Studio的项目，还可以编译libraries和样例。

	CMake官方下载地址：https://cmake.org/download

- 编译C语言SDK

如果你想在本地编译开发和测试SDK的话，可以通过执行下面的步骤来生成项目文件:

- 打开CMD命令行

- 在respository的根目录运行以下命令：

			mkdir build
			cd build
			cmake -G "Visual Studio 14 2015" .. 

	如果想编译64位程序，修改cmake参数：

			cmake .. -G "Visual Studio 14 2015 Win64"
	
	如果

	如果项目文件成功生成的话，你应该可以看到一个Visual Studio的工程文件.sln在cmakefolder下面，可以通过下面的步骤来编译SDK

- 在visual studio里面打开cmake\iotcore_c_sdk.sln，或者运行下面的命令来生成项目文件：

		cmake --build . -- /m /p:Configuration=Release 

- 你也可以使用MSBuild argument编译用于调试的文件：
 
		cmake --build . -- /m /p:Configuration=Debug



### 设置Linux开发环境 ###

这一节会介绍如何设置C SDK在ubuntu下面的开发环境。可以使用CMake来创建makefiles，执行命令make调用gcc来将他们编译成为C语言版本的SDK

- 安装IDE开发工具，你可以下载Clion工具，链接地址：https://www.jetbrains.com/clion/, 可以直接导入现有项目，不要覆盖当前的CMake项目

- 在编译SDK之前确认所有的依赖库都已经安装好，例如ubuntu平台，你可以执行apt-get这个命令区安装对应的安装包

			sudo apt-get update sudo apt-get install -y git cmake build-essential curl libcurl4-openssl-dev libssl-dev uuid-dev 

- 验证CMake是不是最低允许的版本2.8.12

			cmake --version
 
	关于如何在ubuntu 14.04上将Cmake升级到3.x，可以阅读 [How to install CMake 3.2 on Ubuntu 14.04](http://askubuntu.com/questions/610291/how-to-install-cmake-3-2-on-ubuntu-14-04 "How to install CMake 3.2 on Ubuntu 14.04")
- 验证gcc是不是最低允许的版本呢4.4.7

			gcc  --version

- 关于如何在ubuntu 14.04上将gcc升级的信息可以阅读 [How do I use the latest GCC 4.9 on Ubuntu 14.04](http://askubuntu.com/questions/466651/how-do-i-use-the-latest-gcc-4-9-on-ubuntu-14-04 ).

- 编译C版本的SDK

	执行下面命令编译SDK：

		mkdir build
		cd build 
		cmake .. 
		cmake --build .  # append '-- -j <n>' to run <n> jobs in parallel 

	如果要调试可以编译binaries的话，可以通过传递下面的参数个CMake，这样就可以生成可以调试的binaries了

		cmake -DCMAKE_BUILD_TYPE=Debug .. 


### 设置macOS的开发环境 ###

这一节介绍如何设置C语言SDK在macOS上的开发环境。CMake可以生成makefiles，make使用makefiles可以将他们编译成为c语言SDK利用clang，clang默认会包含在XCode里面，我们已经测试过c语言版本的SDK在Sierra上，对应XCode版本是8.
- 安装IDE开发工具，你可以下载Clion工具，链接地址：https://www.jetbrains.com/clion/, 可以直接导入现有项目，不要覆盖当前的CMake项目

- 在编译SDK之前，确保所有的dependencies都安装好。对于macOS系统，你可以使用Homebrew来安装正确的packages
		brew update brew install git cmake pkgconfig openssl ossp-uuid 

- 验证CMake是不是允许最低版本2.8.12

		cmake --version 

- 编译C语言SDK

		mkdir build
		cd build 
		cmake -DOPENSSL_ROOT_DIR:PATH=/usr/local/opt/openssl .. 
		cmake --build .  # append '-- -j <n>' to run <n> jobs in parallel 

- 如果需要编译可以调试的binaries的话，可以添加一个参数到CMake

		cmake -DOPENSSL_ROOT_DIR:PATH=/usr/local/opt/openssl -DCMAKE_BUILD_TYPE=Debug .. 

***
以上是百度IoTCore SDK的C语言版本介绍，若在其他平台上使用，请参考[如何移植百度天工IOT Core C语言SDK到其他平台](https://github.com/baidu/iot-sdk-c/blob/master/PortingGuide.md).
