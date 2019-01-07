我们暂时只提供`iothub_sample`和`smarthome_sample`这两个例子：

- `iothub_sample`  
    提供接入 baidu iothub的例子。

- `smarthome_sample`  
    提供接入 DuHome 的例子。


## 关于openssl在linux下的编译安装  



### 对于非嵌入式环境（电脑）  
在电脑上编译安装 openssl 库是十分方便的。可以使用包管理工具，比如在ubuntu下
```
sudo apt-get install libcurl4-openssl-dev
```
这里讲一下源代码安装的方式。  

- 从网上下载最新的源码。**注意：0.9.8, 1.0.0 和 1.0.1版本官方已经不再支持，请避免使用！**
```
wget https://www.openssl.org/source/openssl-1.1.0i.tar.gz
```
- 解压，编译，安装。
```
tar -xzf openssl-1.1.0i.tar.gz
cd openssl-1.1.0i
mkdir install
./config shared --prefix=`pwd`/install       # shared选项表示生成动态库，--prefix 参数为欲安装之目录
make
make install
```
编译完成之后，在`install`目录下的`include` `lib`就是我们需要链接的头文件和库文件目录。

### 对于嵌入式linux环境
一般情况下，我们需要把openssl应用到不同的环境中，这样需要我们进行交叉编译。

以下交叉编译的步骤：

- 配置
```
./config no-asm shared --prefix=`pwd`/install
```

- 修改makefile


```
CC= arm-linux-gcc             #改为你的 toolchain 前缀，这里以`arm-linux`为例

CFLAG= -fPIC -DOPENSSL_PIC -DOPENSSL_THREADS -D_REENTRANT -DDSO_DLFCN -DHAVE_DLFCN_H -DL_ENDIAN -O3 -Wall 

DEPFLAG= -DOPENSSL_NO_EC_NISTP_64_GCC_128 -DOPENSSL_NO_GMP -DOPENSSL_NO_JPAKE -DOPENSSL_NO_MD2 -DOPENSSL_NO_RC5 -DOPENSSL_NO_RFC3779 -DOPENSSL_NO_SCTP -DOPENSSL_NO_STORE -DOPENSSL_NO_UNIT_TEST

PEX_LIBS= 

EX_LIBS= -ldl

EXE_EXT= 

ARFLAGS= 

AR= arm-linux-ar $(ARFLAGS) r   # 改为你的 toolchain 前缀

RANLIB= arm-linux-ranlib        # 改为你的 toolchain 前缀

NM= arm-linux-nm                # 改为你的 toolchain 前缀

PERL= /usr/bin/perl

TAR= tar

TARFLAGS= --no-recursion --record-size=10240

MAKEDEPPROG= gcc

LIBDIR=lib
```
修改完成保存。


- 编译，安装
```
make
make install
```
即可看到在`install`目录下有动态库`libcrypto.so` `libcrypto.so.1.0.0` `libssl.so` `libssl.so.1.0.0`和静态库`libcrypto.a` `libssl.a`。


### 动态链接 or 静态链接  

一般是采用`动态链接`的方式，这样可以有效节省空间。把编译出来的.so文件拷贝到开发板的`/usr/lib`目录下。如果是电脑运行，直接拷贝到系统查找目录下。  


如果是`静态链接`，链接的时候需要链接静态库。应用程序需要连接外部库的情况下，Linux默认对库的连接是使用动态库，在找不到动态库的情况下再选择静态库。
所以只要将lib文件夹中的.so文件删除，系统在编译的时候就会使用静态库编译。

用静态链接，程序执行的效率会更高一些，但缺点是可执行程序太大，占用空间。

编译程序的时，用`-L` `-l`选项链接相应动态库就行了。注意引用库的顺序为：`-lssl -lcrypto`，如果为`-lcrypto –lssl`，编译时会出现错误。



## 示例工程的编译
我们提供了两个例子，`iothub_sample`和`smarthome_sample`，以前者为例 

- 进入子目录`iothub_sample`，修改Makefile

```
CFLAGS = -g -O2 -std=gnu99
DFLAGS = -DUSE_OPENSSL
# modify openssl lib dir path on your system
LDFLAGS =  -L/path/to/your/openssl/install/lib -lssl -lcrypto  # 修改这一路径为你编译出来的.so库文件的路径
LDFLAGS +=  -lm -lpthread -lrt -ldl 
TARGET := iothub_sample_test

# modify to your cross-compiler name if your are cross-compiling
CROSS_COMPILE = arm-linux-             # 如果你是交叉编译，则修改这一名称为你交叉编译工具的名称前缀，否则，设置为空值
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

# modify openssl include dir path on your system
INC = -I/path/to/your/openssl/install/include \   # 修改这一路径为你编译安装后的头文件所在目录
-I$(EDGE_SDK_DIR)/c-utility/inc  \
-I$(EDGE_SDK_DIR)/iothub_client/inc \
......
```
- 编译
```
make
```
如果编译成功，则可执行文件`iothub_sample_test`会生成在`build`目录下。  

- 运行，测试

如果是嵌入式平台，把`iothub_sample_test`拷贝到开发板上运行，同时拷贝交叉编译openssl库生成的.so文件到开发板的`/usr/lib`目录下。  
如果是PC平台，拷贝编译openssl库生成的.so文件到系统库查找路径下。然后直接运行。
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




