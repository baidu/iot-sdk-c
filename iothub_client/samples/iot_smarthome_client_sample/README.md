# 设备管理平台设备端sdk使用文档 #

## 简介 ##
iot_smarthome_client是针对设备管理平台产品、运行于设备端的c语言sdk，适用于可直连设备管理平台云端的普通智能设备或者智能网关设备。建议用户在此sdk基础上进行二次开发满足自身需求。

开放环境配置等相关信息请参考根目录下的统一说明，本文档主要介绍iot_smarthome_client的使用方法。

## 使用说明 ##

每个设备会有一组属性，这组属性就是该设备所属产品的参数设置，每一个属性的值都有两个版本，desired和reported值，desired值代表期望值，而reported值代表实际值。

举一个简单例子来描述云端是如何和设备端交互以实现控制设备的。一个灯使用int型light_mode来标志亮度，当前的亮度为1（reported和desired值都是1），而希望通过云端将该灯的亮度调节为2，则由云端将该设备的light_mode的desired的值改为2，此时设备端会收到这条desired值的变化信息，然后将自身亮度调节为2，完成后上报reported值为2，完成后云端所记录的reported和desired的值也都同步变成了2。能够接受到desired值的变化，是因为设备端通过mqtt从云端sub(订阅)了相关的topic。能够上报新的reported值，是因为设备可以通过mqtt向云端pub(发布）了相应的消息。

iot_smarthome_client基于mqtt协议连接到设备管理平台云端。他可以运行于一个普通智能设备，直接pub/sub自己的topic与云端进行通信；也可以运行于一个网关设备，代理多个无法直连云端的本地子设备，网关以代理身份向云端pub子设备的信息，并sub一个通配符topic来监听多个子设备的云端指令。

动手开发前建议阅读和运行iot_smarthome_client_sample下的main.c及iot_smarthome_client_sample.c。该示例代码包含了如何新建一个smarthome_client并连上云端，如何监听并识别来自云端的指令信息，如何向云端发布自身信息等动作。


### 1. 准备工作 ###
设备可以成功连接到云端mqtt的前提条件包括：

1）设备可以连接公网

2）提供厂商endpointName，与厂商百度云账户唯一关联，一个百度云账户只有一个endpointName

3）提供设备唯一16位标志符PUID

4）提供设备唯一认证证书（含私钥），设备的证书和私钥需通过百度云天工http接口获取。

5）设备已经在云端授权激活。

设备的授权激活由厂商使用百度云认证信息调用百度云天工http接口完成，具体激活接口信息请参考文档https://cloud.baidu.com/doc/IOT/API.html（待更新）。

如果是网关代理子设备的场景，需要先激活网关，然后再使用子设备信息+已激活网关的信息去激活子设备后，网关方可启用对该子设备的代理功能。网关本身的激活同普通设备的激活。

### 2. 基于证书私钥计算数字签名
为了校验激活接口的触发者是设备证书的合法所有者，云端激活接口需要使用基于设备证书私钥进行RSA SHA256数字签名。出于安全考虑，建议将签名过程放在设备端执行，本sdk中提供了openssl、mbedtls、wolfssl三个版本的数字签名功能，使用示例如下。如果你不选择在设备端进行数字签名，可以跳过本节。
```
static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
        "you client cert\r\n"
        "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "your client key\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";
        
unsigned char* data = (unsigned char*)"123456";
const char* signature = computeSignature(data, client_key);
if (signature == NULL) {
    LogError("compute Signature failed");
    return __FAILURE__;
}

LogInfo("rsa sha256 signature of %s is %s", data, signature);

int sigRet = verifySignature(data, client_cert, signature);

if (sigRet == 0) {
    LogInfo("verify signature success");
}
else {
    LogError("verify signature fail");
    return __FAILURE__;
}
```

openssl，mbedtls,wolfssl的选择在iothub_client/CmakeLists.txt中指定，用户需根据自己平台支持情况进行选择。
```
option(openssl_enable "set use_openssl to ON if your platform supports openssl" ON)
#option(mbedtls_enable "set use mbedtls to ON if your platform supports mbedtls" ON)
#option(wolfssl_enable "set this if your platform supports wolfssl" ON)
```

### 3. 定义一个smarthome_client ###
完成准备工作以后，即可创建smarthome_client准备与云端连接。

创建client的代码如下，isGatewayDevice是一个布尔值，标志该设备是否为网关。
```
IOT_SH_CLIENT_HANDLE handle = iot_smarthome_client_init(isGatewayDevice);
    
#注册各类回调交口，当云端下发指令时，注册的回调接口将会被调用
iot_smarthome_client_register_delta(handle, HandleDelta, handle); //设备状态desired值变化时的回调
iot_smarthome_client_register_get_accepted(handle, HandleGetAccepted, handle); // 获取状态成功的回调
iot_smarthome_client_register_get_rejected(handle, HandleGetRejected, handle); // 获取状态失败的回调
iot_smarthome_client_register_update_accepted(handle, HandleUpdateAccepted, handle); // 更新状态成功后的回调
iot_smarthome_client_register_update_rejected(handle, HandleUpdateRejected, handle); // 更新状态失败后的回调
iot_smarthome_client_register_update_documents(handle, HandleUpdateDocuments, handle); // 设备状态reported值变化时的回调，收到发生变化的reported字段的当前值和更新值
iot_smarthome_client_register_update_snapshot(handle, HandleUpdateSnapshot, handle); // 设备状态reported值变化时的回调，收到设备的全部信息
```

回调函数会收到三个参数:

第一个参数messageContext包含了消息的基本信息，包括requestId(请求id), device(普通设备PUID或网关设备PUID), subdevice(网关代理场景下的子设备PUID)。

第二个参数各个回调函数不同，为回调的具体信息，例如delta事件为一个JSON_OBJECT信息。

第三个参数callbackContext一般是IOT_SH_CLIENT_HANDLE对象，在执行回调函数过程中，可以通过这个HANDLE对象拿到上下文信息或者向云端发起反馈信息。

以收到desired值的变化指令为例，HandleDelta被调用，一般需要设备执行指令，然后再将执行指令后的新状态反馈给云端。如下所示，HandleDelta回调函数收到云端desired状态变化的指令后，执行相应操作，然后再调用相应接口把执行操作后的设备新状态回馈给云端。
```
static void HandleDelta(const SHADOW_MESSAGE_CONTEXT* messageContext, const JSON_Object* desired, void* callbackContext)
{
    Log("Received a message for shadow delta");
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);
    if (isGateway == true) {
        Log("SubDevice:");
        Log(messageContext->subdevice);
    }
    JSON_Value* value = json_object_get_wrapping_value(desired);
    char* encoded = json_serialize_to_string(value);
    Log("Payload:");
    Log(encoded);
    json_free_serialized_string(encoded);

    if (NULL == callbackContext)
    {
        LogError("Failure: the callback context is NULL.");
    }

    // In the actual implementation, we should adjust the device status to match the control of device.
    // However, here we only sleep, and then update the device shadow (status in reported payload).

    ThreadAPI_Sleep(10);
    IOT_SH_CLIENT_HANDLE handle = (IOT_SH_CLIENT_HANDLE)callbackContext;
    int result = iot_smarthome_client_update_shadow(handle, messageContext->device, messageContext->requestId, 0, value, NULL);
    if (0 == result)
    {
        Log("Have done for the device controller request, and corresponding shadow is updated.");
    }
    else
    {
        LogError("Failure: failed to update device shadow.");
    }
}
```

### 4. 连接到云端 ###
DEVICE是设备PUID，USERNAME是endpointName/PUID, client_cert/client_key分别是设备的证书和私钥，通过这些信息可以成功与云端建立mqtt连接。

```
// $puid
#define         DEVICE              "your_puid"

// $endpointName/$puid
#define         USERNAME            "your_endpoint_name/your_puid"

static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
        "you client cert\r\n"
        "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "your client key\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";
        
iot_smarthome_client_connect(handle, USERNAME, DEVICE, client_cert, client_key);
```

### 5. 监听云端指令 ###
创建一个循环始终监听云端指令，当有信息下发时，步骤3中注册的相应回调函数将会被触发。
```
while (iot_smarthome_client_dowork(handle) >= 0)
{
    ThreadAPI_Sleep(100);
}
```

### 6. 向云端发布信息 ###
普通设备（包括网关设备发布自身信息而非代理子设备被信息的场景）向云端发布信息可使用如下接口：
```
// 获取设备云端状态信息
int iot_smarthome_client_get_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId) 
// 更新设备desired状态信息，以json格式
int iot_smarthome_client_update_desired(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* desired, JSON_Value* lastUpdatedTime)
// 更新设备reported状态信息，以json格式
int iot_smarthome_client_update_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* reported, JSON_Value* lastUpdatedTime)
// 更新设备desired状态信息，以序列化后的bytes格式
int iot_smarthome_client_update_desired_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* desired, const char* lastUpdatedTime)
// 更新设备reported状态信息，以序列化后的bytes格式
int iot_smarthome_client_update_shadow_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* reported, const char* lastUpdatedTime)
```
网关设备代理子设备向云端发布信息时需使用如下接口：
```
// 获取设备云端状态信息
int iot_smarthome_client_get_subdevice_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId)
// 更新设备desired状态信息，以json格式
int iot_smarthome_client_update_subdevice_desired(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, JSON_Value* desired, JSON_Value* lastUpdatedTime)
// 更新设备reported状态信息，以json格式
int iot_smarthome_client_update_subdevice_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, JSON_Value* reported, JSON_Value* lastUpdatedTime)
// 更新设备desired状态信息，以序列化后的bytes格式
int iot_smarthome_client_update_subdevice_desired_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, const char* desired, const char* lastUpdatedTime)
// 更新设备reported状态信息，以序列化后的bytes格式
int iot_smarthome_client_update_subdevice_shadow_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, const char* reported, const char* lastUpdatedTime)
```
如何执行序列化。如sample中所示，你需要先定义好设备的模型，模型中的字段名称和类型需要与在设备管理平台中所定义的一致，目前主要有int, float和ascii_char_ptr（即string)，然后将该model序列化为bytes数组上传。
```
BEGIN_NAMESPACE(BaiduIotDeviceSample);

DECLARE_MODEL(BaiduSamplePump,
WITH_DATA(int, FrequencyIn),
WITH_DATA(int, FrequencyOut),
WITH_DATA(int, Current),
WITH_DATA(int, Speed),
WITH_DATA(int, Torque),
WITH_DATA(int, Power),
WITH_DATA(int, DC_Bus_voltage),
WITH_DATA(int, Output_voltage),
WITH_DATA(int, Drive_temp)
);

END_NAMESPACE(BaiduIotDeviceSample);

// Sample: use 'serializer' to encode device model to binary and update the device shadow.
BaiduSamplePump* pump = CREATE_MODEL_INSTANCE(BaiduIotDeviceSample, BaiduSamplePump);
pump->FrequencyIn = 1;
pump->FrequencyOut = 2;
pump->Current = 3;
pump->Speed = 4;
pump->Torque = 5;
pump->Power = 6;
pump->DC_Bus_voltage = 7;
pump->Output_voltage = 8;
pump->Drive_temp = 9;

unsigned char* reported;
size_t reportedSize;
if (CODEFIRST_OK != SERIALIZE(&reported, &reportedSize, pump->FrequencyIn, pump->FrequencyOut, pump->Current))
{
    Log("Failed to serialize the reported binary");
}
else
{
    char* reportedString = malloc(sizeof(char) * (reportedSize + 1));
    reportedString[reportedSize] = '\0';
    for (size_t index = 0; index < reportedSize; ++index)
    {
        reportedString[index] = reported[index];
    }
    // reportedString就是对FrequencyIn = 1,FrequencyOut = 2,Current = 3的序列化值
    iot_smarthome_client_update_shadow_with_binary(handle, DEVICE, "123456", 0, reportedString, NULL) 
}    
```
注意version字段的值，如果填0则交由server端自动执行version自增长，如果不填0则必须大于当前version否则会报错。


## 附录 ##

### 1. 如何使用mbedtls进行mqtt双向认证 ###

sdk中默认使用openssl进行双向认证，如设备无法支持openssl需使用mbedtls，需要在c-utility目录中进行相应的修改，请参考iot_smarthome_client_sample/readme/resources/use_mbedtls_in_mqtt.patch文件在c-utility目录中进行相关配置。