该工程完整包含了在`esp`系列芯片上运行百度物管理`iotdm` Demo代码，并且通过`DHT11`温湿度传感器实时采集温度和湿度上报`百度天工`云平台。后续可以进行数据存储`TSDB`以及数据呈现`物可视`等功能的接入。用户只需要少量修改配置即可烧录运行。


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
- 修改iot-edge-c-sdk的`component.mk`中加入```CFLAGS += -DESP8266_SOC_RTC```
- make flash monitor

# MQTT连接信息配置
修改`iotdm_client_sample.c`中如下字段。
```
#define         ADDRESS             "xxx://xxxxxxxxxxxxxxxxxxx.mqtt.iot.xx.baidubce.com:xxxx"

// The device name you created in device management service.
#define         DEVICE              "yyyyyyyyyyyyy"

// The username you can find on the device connection configuration web,
// and the format is like "xxxxxx/xxxxx"
#define         USERNAME            "xxxxxxxxxxxxxxxxxxx/yyyyyyyyyyyyy"

// The key (password) you can find on the device connection configuration web.
#define         PASSWORD            "xxxxxx"
```
首先，你必须在天工云平台创建`物影子`，详细请参考[官方文档](https://cloud.baidu.com/doc/IOTDM/GUIGettingStarted_New.html#.49.37.21.DE.67.ED.49.7B.B7.65.8F.5A.CE.DC.43.13)，创建完成之后平台就会生成上述连接信息，依次填写即可。
