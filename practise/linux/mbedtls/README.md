## 关于 mbedtls  
mbedtls 是一款开源软件加密库，使用 c 语言编写，采用模块化设计使得模块之间松散耦合。  
mbedtls 很适合应用于嵌入式系统中，可以作为 openssl 的替代者，相比 openssl, mbedtls更小巧，  
代码更加简洁，api更加直观和易于理解。如果在嵌入式平台使用`mbedtls`库，主要是两块的工作：`移植`和`裁剪`。  
前者涉及`网络模块` `时间模块` `熵模块` 等模块的适配工作，需要修改源代码。mbedtls 库已经默认支持了linux系统，  
就免去了`移植`的工作。故本文重点只介绍如何裁剪。

## 关于例子工程
我们暂时只提供`iothub_sample`和`smarthome_sample`这两个例子：

- `iothub_sample`  
    提供接入 baidu iothub的例子。

- `smarthome_sample`  
    提供接入 DuHome 的例子。


## 关于 mbedtls 在 linux 下的编译安装  



### 对于非嵌入式环境（电脑）  
在电脑上编译安装 mbedtls 库是十分方便的。具体步骤如下:

- clone 源代码
```
git clone -b mbedtls-2.12.0 https://github.com/ARMmbed/mbedtls.git
```
- 编译，安装。
```
mkdir install                           # 创建安装目录
make SHARED=1                           # SHARED=1 表示生成动态库
make install DESTDIR=`pwd`/install
```
编译完成之后，在`install`目录下的`include` `lib`就是我们需要链接的头文件和库文件目录。

注意，对于某些32位系统，执行`make SHARED=1`之后可能会出现如下编译错误：
```
  CC    aes.c
  CC    aesni.c
  CC    arc4.c
  CC    asn1parse.c
  CC    asn1write.c
  CC    base64.c
  CC    bignum.c
bignum.c: In function ‘mpi_mul_hlp’:
bignum.c:1139:9: error: PIC register clobbered by ‘ebx’ in ‘asm’
bignum.c:1154:9: error: PIC register clobbered by ‘ebx’ in ‘asm’
bignum.c:1165:9: error: PIC register clobbered by ‘ebx’ in ‘asm’
make[1]: *** [bignum.o] Error 1
make: *** [lib] Error 2
```
临时的解决方法是，先执行`make`生成静态库，再执行`make SHARED=1`生成动态库。

### 对于嵌入式linux环境
因为嵌入式环境硬件资源的限制，需要针对特定应用场景对`mbedtls`库进行裁剪，减小生成的库文件大小。  

以下为交叉编译的步骤：

- clone 源代码
```
git clone https://github.com/ARMmbed/mbedtls.git
```
- 裁剪
为节省ROM和RAM，修改配置文件`include/mbedtls/config.h`,注释如下宏定义

```
MBEDTLS_SELF_TEST         # self test
MBEDTLS_VERSION_FEATURES  # disable run-time checking and save ROM space

# 去掉未使用的加密方式
MBEDTLS_BLOWFISH_C
MBEDTLS_CAMELLIA_C
```

- 编译，安装。
```
mkdir install                           # 创建安装目录
make CC=arm-linux-gcc SHARED=1          # SHARED=1 表示生成动态库 CC为你交叉编译工具的名字
make install DESTDIR=`pwd`/install
```
编译完成之后，在`install`目录下的`include` `lib`就是我们需要链接的头文件和库文件目录。

### 动态链接 or 静态链接  

一般是采用`动态链接`的方式，这样可以有效节省空间。把编译出来的.so文件拷贝到开发板的`/usr/lib`目录下。如果是电脑运行，直接拷贝到系统查找目录下。  
如果不想拷贝文件，也可以设置`LD_LIBRARY_PATH`环境变量为你so文件的目录，这样系统就可以在指定的目录下找到库文件了。


如果是`静态链接`，链接的时候需要链接静态库。应用程序需要连接外部库的情况下，Linux默认对库的连接是使用动态库，在找不到动态库的情况下再选择静态库。
所以只要将lib文件夹中的.so文件删除，系统在编译的时候就会使用静态库编译。

用静态链接，程序执行的效率会更高一些，但缺点是可执行程序太大，占用空间。


## 示例工程的编译
我们提供了两个例子，`iothub_sample`和`smarthome_sample`，以前者为例 

- 进入子目录`iothub_sample`，修改Makefile

```
CFLAGS = -g -O2 -std=gnu99
DFLAGS = -DUSE_MBED_TLS
# modify mbedtls lib dir path on your system
LDFLAGS =  -L/path/to/your/mbedtls/install/lib -lmbedtls -lmbedcrypto -lmbedx509     # 修改这一路径为你编译出来的.so库文件的路径
LDFLAGS +=  -lm -lpthread -lrt -ldl 
TARGET := iothub_sample_test

# modify to your cross-compiler name if your are cross-compiling
CROSS_COMPILE = arm-linux-                      # 如果你是交叉编译，则修改这一名称为你交叉编译工具的名称前缀，否则，设置为空值
CC = gcc

BUILD_BASE = ./build
OBJ_BASE = $(BUILD_BASE)/objs
EDGE_SDK_DIR=../../../..


V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

# modify mbedtls include dir path on your system
INC = -I/path/to/your/mbedtls/install/include \    # 修改这一路径为你编译安装后的头文件所在目录
-I$(EDGE_SDK_DIR)/c-utility/inc  \
-I$(EDGE_SDK_DIR)/c-utility/inc/azure_c_shared_utility \
-I$(EDGE_SDK_DIR)/iothub_client/inc \
......
```
- 编译
```
make
```
如果编译成功，则可执行文件`iothub_sample_test`会生成在`build`目录下。  

- 运行，测试（如果是动态链接，请确认so文件已经拷贝到系统查找目录，否则会报错`cannot open shared object file: No such file or directory`）
如果是嵌入式平台，拷贝`iothub_sample_test`到开发板上运行。  
如果是PC平台，可以直接运行。
```
cd build && ./iothub_sample_test
```
发现程序可以运行，但是无法连接baidu iothub，需要用户创建物接入实例，创建之后，百度云会提供认证信息（用户名，密码等）。  
关于如何创建`物接入`实例，参考[这里](https://cloud.baidu.com/doc/IOT/GettingStarted.html)。得到认证信息之后，需要用户同时修改  
`iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.c`文件：
```

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/utf8_checker.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iothub_mqtt_client.h"
#include "iothub_mqtt_client_sample.h"

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"  //这里填入接入点名称

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"                     //这里填入用户名

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxx"                            //这里填入密码

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TLS"                            //这里选择 TLS —— 加密通讯方式，用户也可以选择 TCP 非加密通讯方式（不推荐） 

```
修改完成后，重新编译运行，就可以连上 iothub 了。之后，可以利用mqtt测试工具测试数据收发通信是否正常。




