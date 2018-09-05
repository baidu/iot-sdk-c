本工程是针对 esp8266-nonos 版本的适配。相比于嵌入式linux 或 FreeRTOS,涉及到的库以及代码的修改会多一些。  
这也是由 esp8266-nonos SDK 的特性决定的：  
- 因为不带OS，并且是单线程，任何的sleep或者block操作（长时间占用CPU)都可能会引起watchdog reset或导致网络功能运行不正常，故所有的逻辑必须是基于`定时器`和`回调`的。
- esp8266 RAM总量为 160K，运行时 stack 和 heap 可用的空间大约50k左右，内存资源紧张，为了给 TLS 的正常运行留出足够的空间，需要对部分代码做优化。  
- esp8266 RAM 允许加载的代码量是有限制的，最大为32K，无法在运行时一次性装下edge sdk的所有代码。故需要在每个C函数前加上`ICACHE_FLASH_ATTR`属性，该属性表示将代码存放在Flash上，需要用到的时候再调入内存中运行。


鉴于以上限制，我们会将涉及到的edge sdk 代码拷贝一份，并做适当的修改，独立成一个小工程。  

以下是工程的安装和使用步骤。


## 安装esp8266 SDK 以及 toolchain  
可以参考[官方文档](https://github.com/pfalcon/esp-open-sdk)。  
很简单，照着一步步做就行，在此就不赘述。假定clone下来的esp-open-sdk的路径为`$OPEN_SDK`


## 编译 lwip 库  
官方已经提供了编译好的二进制 liblwip.a，我们可以自己从源代码编译 lwip 库，不用官方的.a。这样做的好处是：  
- 可以定制化tcp相关的一些参数。
- 可以方便调试，debug。
- 方便进一步裁剪，开发。

官方已经提供了移植好的lwip源代码，我们只需要编译一下就可以了。源代码的路径在`$OPEN_SDK/sdk/third_party/lwip`。
```sh
cd esp8266                                          # 进入工程目录
cp -R $OPEN_SDK/sdk/third_party/lwip esp8266_lwip/  # 拷贝sdk中的 lwip 源码
# 拷贝需要的头文件
cp -R $OPEN_SDK/sdk/third_party/include/lwipopts.h\
     $OPEN_SDK/sdk/third_party/include/lwip \
     $OPEN_SDK/sdk/third_party/include/arch \
     $OPEN_SDK/sdk/third_party/include/netif \
     $OPEN_SDK/sdk/third_party/include/user_config.h \
     esp8266_lwip/include/
cd esp8266_lwip
vim Makefile         # 修改 Makefile
```
Makefile的修改很简单，只要指定你的`$OPEN_SDK`路径就可以了。
```Makefile
OPEN_SDK   = xxx    # 设置你的`$OPEN_SDK`路径
CROSS_TOOL = $(OPEN_SDK)/xtensa-lx106-elf/bin/xtensa-lx106-elf-
SDK_BASE   = $(OPEN_SDK)/sdk

...
```
保存退出，开始编译
```
make
```
如果出现如下错误
```
lwip/core/sntp.c:343:1: error: unknown type name '__tzrule_type'
 __tzrule_type sntp__tzrule[2];
 ^
lwip/core/sntp.c: In function 'sntp_mktm_r':
lwip/core/sntp.c:371:6: error: dereferencing pointer to incomplete type
   res->tm_hour = (int) (rem / SECSPERHOUR);
```
修改`lwip/core/sntp.c`,添加头文件`#include "include/time.h"`

编译成功，在当前目录下生成了`liblwip.a`。



## 编译 mbedtls 库
mbedtls为我们提供标准的 tls 连接服务。相比于 openssl，mbedtls更小巧，更适用于嵌入式环境。在`mbedtls`子文件夹下，已经提供了裁剪好的`config_esp.h`，该文件通过定义一些宏开关，以开启/关闭/设置 mbedtls 库的特性。从而很好的控制代码大小来节省硬件资源。关于 mbedtls 的裁剪方法，后续小节将会说明。

### 准备源文件
首先，从github上下载 mbedtls：
```sh
git clone https://github.com/ARMmbed/mbedtls.git
cd mbedtls
git checkout mbedtls-2.2.1    # 切换到某个稳定 release 版本，在这里我们选用 2.2.1
cp -R include library /your_edge_sdk_dir/iot-edge-c-sdk/practise/nonos/esp8266/mbedtls/ # 拷贝 include library两个文件夹到我们的工程，我们编译只需要这两个文件夹
cd /your_edge_sdk_dir/iot-edge-c-sdk/practise/nonos/esp8266/mbedtls/
vim Makefile                  # 修改 Makefile
```
Makefile的修改也很简单，只需要修改`$(OPEN_SDK)`的路径就可以了。
```
OPEN_SDK   = xxx    # 设置你的`$OPEN_SDK`路径
BUILD_BASE	= build
SDK_BASE = $(OPEN_SDK)/sdk

...
```

### 适配 platform abstraction layer 代码
涉及到的文件有 `include/mbedtls/platform.h`  `platform/esp_hardware.c`等，需要针对 esp8266 平台，实现相关系统抽象层接口。比如内存分配，系统打印，随机数生成等等。以下，简要介绍以下 mbedtls 移植的方法。

mbedtls 采用模块化设计，代码编写时使用宏定义的方式将平台依赖代码进行提取，用户将 mbedtls 移植到新的平台运行时只需要修改宏定义，添加平台依赖相关的代码即可，除了标准的 C 库外，以下模块可能需要根据需求进行修改：
- 网络模块
- 时间模块
- 熵源模块
- 硬件加速
- 打印模块

#### 适配网络模块
网络模块 net.c 文件中对套接字接口进行了封装，用户可以将接口进行替换，替换后通过 mbedtls_ssl_set_bio() 接口来注册发送和接收接口，根据需要开启或关闭超时检测、阻塞/非阻塞功能。在这里，我们没有使用 mbedtls 的TCP/IP收发功能，因为是裸板程序，系统也未提供套接字接口供我们使用。我们只需要利用 mbedtls 库进行加解密，底层数据的收发是通过 `espconn` 接口。所以，网络模块的适配，我们暂时可以略过。

#### 适配时间模块
如果在系统中需要使用 DTLS 协议，则需要适配时间模块来提供相关接口。我们未使用 DTLS 协议，故时间模块可以暂时略过。

#### 适配熵源模块
在密码学中随机数是最基础的也是最重要的部分，mbedtls 中的熵池属于随机数模块部分，在 Linux 下可以使用 `/dev/urandom` 来提供熵源，在新的平台需要用户提供类似的熵源采集接口。可以通过定义 `MBEDTLS_ENTROPY_HARDWARE_ALT` 添加熵源采集接口。另外，如果平台无法提供 `/dev/urandom` 或 `Windows CryptoAPI` 的支持，则需要增加 `MBEDTLS_NO_PLATFORM_ENTROPY` 定义。  

在 esp8266 平台下熵源模块的适配文件`platform/esp_hardware.c`我们已经提供。

#### 适配硬件加速
如果硬件平台支持加密算法，用户可以通过定义 MBEDTLS_*_ALT 来添加自己的硬件加密接口，比如 MBEDTLS_AES_ALT 可以用来替换 AES 加密相关接口。在这里我们未使用。暂时略过。

#### 适配系统抽象层接口
比如在 esp8266 中的 os_malloc os_printf ets_snprintf 这些系统调用，怎么映射到 mbedtls 中响应到函数中去呢？  
首先，你得告诉 mbedtls ，你需要使用 平台抽象层代理函数，而不是用默认的 printf malloc snprintf等等这些标准C函数。  
只需要在`config_esp.h`中打开`MBEDTLS_PLATFORM_C`开关就可以了。如果不定义该宏，mbedtls就会使用标准C库的函数集。  
在 mbedtls 中的 平台抽象层代理函数 名字叫 mbedtls_calloc mbedtls_free mbedtls_printf 等等，这些“虚”函数，所以你现在需要做的就是定义一些映射关系。这些映射关系都定义在`include/mbedtls/platform.h`中：
```c
#ifndef MBEDTLS_PLATFORM_H
#define MBEDTLS_PLATFORM_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int ets_snprintf(char *buf, unsigned int size, const char *format, ...);
extern void *pvPortCalloc(unsigned int count, unsigned int size);
extern void vPortFree( void *pv );
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

//映射 esp8266 平台的 malloc/free 函数
#define mbedtls_free       vPortFree
#define mbedtls_calloc     pvPortCalloc

//映射 esp8266 平台的 fprintf 函数
#define mbedtls_fprintf    fprintf

//映射 esp8266 平台的 printf 函数
#define mbedtls_printf     os_printf

//映射 esp8266 平台的 snprintf 函数
#define mbedtls_snprintf   ets_snprintf

//映射 esp8266 平台的 exit 函数
#define mbedtls_exit   exit


#ifdef __cplusplus
}
#endif

#endif /* platform.h */
```

以上，系统抽象层接口就适配完了。

#### 其他重要参数设置
在 mbedtls 的适配中，还需要特别说明的一个参数是`MBEDTLS_SSL_MAX_CONTENT_LEN`，定义在config文件中。这个值表示在ssl通讯中，消息记录的长度的最大值，比如，在我们的应用场景下，最长的消息记录出现在 handshake 时候，server 发送证书给客户端，该条消息记录长达5K多，所以，在我们的应用场景下，我们选取一个稍稍大于5K的值，故该值设置为`0x1800`(十进制 6144)，mbedtls 在初始化时，就会分配这么大的两块 io_buffer,以便可以一次性装下整条消息记录。如果该值设置太小，mbedtls 则会在握手的过程中会返回`MBEDTLS_ERR_SSL_INVALID_RECORD`或者"bad message length"之类的错误。当然，该值也不必取过大，无端浪费内存空间。

### ESP8266 内存优化
完成系统抽象层的工作还是远远不够的。一些额外的因素还需要考虑。例如，esp8266 对加载到ram的代码容量是有限制的，最大为30k。显然，我们的库编译出的大小已经超过该值，故只能将代码存放到外部的 flash 上。在运行时，如果调用到库代码，cpu将读取flash中的代码片段，加载到内存中运行。为了实现代码存放在 flash上而不是加载到 ram,需要在每个 C 函数前加上 `ICACHE_FLASH_ATTR` 属性。比较繁琐。所以在这里直接在Makefile 中修改生成的目标文件中的段名，可以达到同样的效果。
```Makefile
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(CFLAGS) -c $$< -o $$@
    # 将`.text`段 放到 `.irom0.text`段中，后者会被放到 flash 上
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $$@

```

除了代码段的大小，还需要考虑的一个因素就是`内存`。在 mbedtls 库中，大量使用到了全局的数组，这会占用非常大的内存。导致程序运行时因为内存不够而崩溃。解决的方法是，将这些全局的数组也放到 flash 上，在需要的时候读flash。在这里就需要在所有的全局数组前加上`ICACHE_RODATA_ATTR`属性。

全局数组放入 flash 中也会引发另一个问题，就是非对齐访问。对 flash 的访问是需要严格遵守对齐要求的，否则，会引发系统异常，程序崩溃。对于我们的 flash 而言，是4字节对齐。比如：
```
const uint8 array[7] ICACHE_RODATA_ATTR = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
```
如果需要按字节读取 char 数组当中的元素，可从软件上进行处理，先按四字节读取，然后再按偏移取当中的一个字节。如果直接读取 array[0]，会导致 crash。故需要修改 mbedtls 中所有对全局数组访问的代码，按照4字节对齐访问。

鉴于这块内容对代码的修改比较繁琐，就不一一说明，我们提供了一个补丁`memory_opt.patch`，完成了上述内存优化的所有工作。
```
cd library
patch -p1 < ../memory_opt.patch
```

### 编译 mbedtls 库

```
cd ..
make
```
可能出现如下编译错误:
```
CC library/md.c
library/md.c:447:5: error: conflicting types for 'mbedtls_md_get_size'
 int mbedtls_md_get_size( const mbedtls_md_info_t *md_info )
     ^
In file included from library/md.c:34:0:
./include/mbedtls/md.h:185:15: note: previous declaration of 'mbedtls_md_get_size' was here
 unsigned char mbedtls_md_get_size( const mbedtls_md_info_t *md_info );
               ^
make: *** [build/library/md.o] Error 1
```
这应该是 mbedtls 的bug, 解决的办法就是修改下头文件中的声明：
```sh
vim ./include/mbedtls/md.h +185 # 将"unsigned char" 改成 "int"
```
重新编译，编译成功，在当前目录下生成`libmbedtls.a`。


## 编译 edge sdk 库以及 sample demo
用户主逻辑代码都存放在`user`子目录中。这部分代码包括 esp8266 系统启动时初始化代码，比如连 wifi，以及为我们的 iothub_demo 创建 task 等等。这块逻辑都在`user/user_main.c`中。另一部分就是`iot_edge_c_sdk`，所有相关代码存放在`user/src`目录下。其中包含了一个完整的 iothub mqtt client 实现。用户可以用来实现与百度天工 iot_hub 的连接。  

用户配置工程的步骤如下：
```sh
cd user
./setup.sh  #该脚本会从 iot_edge_c_sdk 中拷贝所需要的代码到 src 目录，然后打 patch
```

所有的代码以及库都准备 ok，现在可以编译整个工程了。  
```sh
cd ..           # 返回主工程目录
vim Makefile    # 修改 Makefile
```
Makefile的修改很简单，只要修改`OPEN_SDK`为你的你的`$OPEN_SDK`路径就可以了。修改完成，直接 make，如果一切正常，就会生成 user1.bin。可以用`ESP FLASH DOWNLOAD TOOL`工具进行烧写。镜像的排布顺序为：  

| 地址 | 镜像 |
| ------ | ------ |
| 0x0000 | boot_v1.7.bin |
| 0x1000 | user1.bin |
| 0x3fc000 | esp_init_data_default_v05.bin |
| 0x3fd000 | blank.bin |
| 0x3fe000 | blank.bin |

具体的烧写方法，可以参考官方文档。在此就不再赘述。



## 运行，测试
用户需要配置 esp8266 需要连接的 wifi ssid 以及 密码。这些字段的设置在`user/user_main.c`中。
```c
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
   // Wifi configuration 
   char ssid[32] = "Honor8";         //设置你自己的 wifi 名字以及密码
   char password[64] = "243587123"; 
   struct station_config stationConf; 
   
   os_memset(stationConf.ssid, 0, 32);
   os_memset(stationConf.password, 0, 64);
   //need not mac address
   stationConf.bssid_set = 0; 
   
   //Set ap settings 
   memcpy(&stationConf.ssid, ssid, 32); 
   memcpy(&stationConf.password, password, 64); 
   wifi_station_set_config(&stationConf); 

}
```

发现程序可以运行，但是无法连接baidu iothub，需要用户创建物接入实例，创建之后，百度云会提供认证信息（用户名，密码等）。关于如何创建`物接入`实例，参考[这里](https://cloud.baidu.com/doc/IOT/GettingStarted.html)。得到认证信息之后，需要用户同时修改  
`user/src/iothub_mqtt_client_sample.c`文件：
```

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"  //这里填入接入点名称

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"                     //这里填入用户名

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxx"                            //这里填入密码

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TLS"                           //这里选择 TLS —— 加密通讯方式，用户也可以选择 TCP 非加密通讯的方式（不推荐） 

```
修改完成后，重新编译运行，就可以连上 iothub 了。之后，可以利用mqtt测试工具测试数据收发通信是否正常。

## 其他问题
如果用户希望添加自己的逻辑，可以参考`user/src/iothub_mqtt_client_sample.c`中的`app_main()`函数。所有的用户逻辑代码可以添加到该函数中。
