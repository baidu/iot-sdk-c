该工程完整包含了在`esp`系列芯片上运行百度`SmartHome` Demo代码，用户只需要少量修改配置即可烧录运行。


# ESP32 上
- export IDF_PATH=你的 esp-idf 路径 (基于 https://github.com/espressif/esp-idf release/v3.0 分支)
- export PATH=加入esp32 toolchain 路径
- ```make menuconfig```配置串口和 wifi 信息
- make flash monitor

# ESP8266 上
- export IDF_PATH=你的 ESP8266_RTOS_SDK 路径(基于 https://github.com/espressif/ESP8266_RTOS_SDK master 分支)
- export PATH=加入esp8266 toolchain 路径
- ```make menuconfig```配置串口和 wifi 信息,同时配置`CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN`，其路径为```-> Component config-> SSL-> mbedTLS```设置其值为`6144`，保存退出。
- 默认情况下，esp8266不支持浮点数打印为了节省栈空间，但是在`iot-edge-c-sdk`中的JSON解析库`parson`中会使用对浮点数的计算和打印输出(例如，`strtod``sprintf`)，为此，从ESP8266_RTOS_SDK 3.0开始，可以配置 newlib 为 normal 模式从而支持浮点。其路径为```-> Component config-> Newlib -> [*] Enable newlib -> newlib level (nano)```这里改成`normal`即可。
- 修改`component.mk`中加入```CFLAGS += -DESP8266_SOC_RTC```
- make flash monitor

# 证书信息配置
以上，配置后即可运行，但是还无法连接服务器，还需要从百度云获取设备`双向认证`所需要的证书信息。填写`iot_smarthome_client_sample.c`中如下字段。
```
// $puid
#define         DEVICE              "xxxxx"

// $endpointName/$puid
#define         USERNAME            "xxxxx/xxxxx"

// if your device is a gateway, you can add subdevice puids here
#define         SUBDEVICE           "xxxxxxxxxxxx"

static char * client_cert = "xxxxxxxxxxxxxxxxxxxx";

static char * client_key = "xxxxxxxxxxxxxxxxxxxx";
```
详细请参考[官方文档](https://cloud.baidu.com/doc/SHC/GettingStarted.html#.E6.8E.A5.E5.85.A5.E6.96.B9.E6.A1.88)
