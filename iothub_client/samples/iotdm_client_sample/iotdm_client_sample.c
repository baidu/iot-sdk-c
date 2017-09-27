/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>

#include "iotdm_callback.h"
#include "iotdm_client.h"
#include "iotdm_client_sample.h"

// Should include serializer to operate shadow with device model.
#include "serializer.h"

#define         SPLIT               "--------------------------------------------------------------------------------------------"


// Please set the device client data and security which are shown as follow.
// The endpoint address your device should cnnect, which is like
// 1. "tcp://xxxxxx.mqtt.iot.xx.baidubce.com:1883" or
// 2. "ssl://xxxxxx.mqtt.iot.xx.baidubce.com:1884"
#define         ADDRESS             "tcp://xxxxxx.mqtt.iot.xx.baidubce.com:1883"

// The device name you created in device management service.
#define         DEVICE              "xxxxxx"

// The username you can find on the device connection configuration web,
// and the format is like "xxxxxx/xxxxx"
#define         USERNAME            "xxxxxx/xxxxxx"

// The key (password) you can find on the device connection configuration web.
#define         PASSWORD            "xxxxxx"

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

static void Log(const char* message)
{
    printf("%s\r\n", message);
}

static void LogCode(const int code)
{
    printf("%d\r\n", code);
}

static void HandleUpdateRejected(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext)
{
    Log("Received a message for shadow update rejected.");
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);
    Log("Code:");
    LogCode(error->code);
    Log("Message:");
    Log(error->message);

    if (NULL == callbackContext)
    {
        LogError("Failure: the callback context is NULL.");
    }

    Log(SPLIT);
}

static void HandleDelta(const SHADOW_MESSAGE_CONTEXT* messageContext, const JSON_Object* desired, void* callbackContext)
{
    Log("Received a message for shadow delta");
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);

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
    // However, here we only sleep, and then update the device shadow (status in reproted payload).

    ThreadAPI_Sleep(10);
    IOTDM_CLIENT_HANDLE handle = (IOTDM_CLIENT_HANDLE)callbackContext;
    int result = iotdm_client_update_shadow(handle, messageContext->device, messageContext->requestId, 0, value, NULL);
    if (0 == result)
    {
        Log("Have done for the device controller request, and corresponding shadow is updated.");
    }
    else
    {
        LogError("Failure: failed to update device shadow.");
    }

    Log(SPLIT);
}

int iotdm_client_run(void)
{
    Log("The device management edge simulator is starting ...");
    if (0 != platform_init())
    {
        Log("platform_init failed");
        return __FAILURE__;
    }

    if (SERIALIZER_OK != serializer_init(NULL))
    {
        Log("serializer_init failed");
        return __FAILURE__;
    }

    IOTDM_CLIENT_HANDLE handle = iotdm_client_init(ADDRESS, DEVICE);
    if (NULL == handle)
    {
        Log("iotdm_client_init failed");
        return __FAILURE__;
    }

    iotdm_client_register_delta(handle, HandleDelta, handle);
    iotdm_client_register_update_rejected(handle, HandleUpdateRejected, handle);

    IOTDM_CLIENT_OPTIONS options;
    options.cleanSession = true;
    options.clientId = DEVICE;
    options.username = USERNAME;
    options.password = PASSWORD;
    options.keepAliveInterval = 5;
    options.retryTimeoutInSeconds = 300;

    if (0 != iotdm_client_connect(handle, &options))
    {
        iotdm_client_deinit(handle);
        Log("iotdm_client_connect failed");
        return __FAILURE__;
    }

    // Subscribe the topics.
    iotdm_client_dowork(handle);

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

        if (0 == iotdm_client_update_shadow_with_binary(handle, DEVICE, "123456", 0, reportedString, NULL))
        {
            Log("Succeeded to update device shadow with bianry");
        }
        else
        {
            Log("Failed to update device shadow with bianry");
        }

        // Sample: update shadow with incorrect version, and receive error message at 'update/rejected'.
        if (0 == iotdm_client_update_shadow_with_binary(handle, DEVICE, "111111", 1, reportedString, NULL))
        {
            Log("Succeeded to send message for updating device shadow with bianry");
        }
        else
        {
            Log("Failed to send message for updating device shadow with bianry");
        }

        free(reportedString);
        free(reported);
    }

    DESTROY_MODEL_INSTANCE(pump);

    // Sample: subscribe the delta topic and update shadow with desired value.
    while (iotdm_client_dowork(handle) >= 0)
    {
        ThreadAPI_Sleep(100);
    }

    iotdm_client_deinit(handle);
    serializer_deinit();
    platform_deinit();

    return 0;

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

