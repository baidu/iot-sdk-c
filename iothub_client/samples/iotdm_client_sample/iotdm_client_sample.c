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
#include <azure_c_shared_utility/buffer_.h>
#include <bos.h>

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

static void Log4Int(const int value)
{
    printf("%d\r\n", value);
}

static void LogCode(const int code)
{
    Log4Int(code);
}

static void LogVersion(const int version)
{
    Log4Int(version);
}

static void LogAcceptedMessage(const SHADOW_ACCEPTED* accepted)
{
    Log("ProfileVersion:");
    LogVersion(accepted->profileVersion);

    JSON_Value* value = json_object_get_wrapping_value(accepted->reported);
    char* encoded = json_serialize_to_string(value);
    Log("Reported:");
    Log(encoded);
    json_free_serialized_string(encoded);

    value = json_object_get_wrapping_value(accepted->desired);
    encoded = json_serialize_to_string(value);
    Log("Desired:");
    Log(encoded);
    json_free_serialized_string(encoded);

    value = json_object_get_wrapping_value(accepted->lastUpdateTime);
    encoded = json_serialize_to_string(value);
    Log("LastUpdateTime:");
    Log(encoded);
    json_free_serialized_string(encoded);
}

static void HandleAccepted(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ACCEPTED* accepted, void* callbackContext)
{
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);

    LogAcceptedMessage(accepted);

    if (NULL == callbackContext)
    {
        LogError("Failure: the callback context is NULL.");
    }
}

static void HandleRejected(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext)
{
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
}

static void HandleGetAccepted(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ACCEPTED* accepted, void* callbackContext)
{
    Log("Received a message for shadow get accepted.");
    HandleAccepted(messageContext, accepted, callbackContext);

    Log(SPLIT);
}

static void HandleGetRejected(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext)
{
    Log("Received a message for shadow get rejected.");
    HandleRejected(messageContext, error, callbackContext);

    Log(SPLIT);
}

static void HandleUpdateRejected(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext)
{
    Log("Received a message for shadow update rejected.");
    HandleRejected(messageContext, error, callbackContext);

    Log(SPLIT);
}

static void HandleUpdateAccepted(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ACCEPTED* accepted, void* callbackContext)
{
    Log("Received a message for shadow update accepted.");
    HandleAccepted(messageContext, accepted, callbackContext);

    Log(SPLIT);
}

static void HandleUpdateDocuments(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_DOCUMENTS* documents, void* callbackContext)
{
    Log("Received a message for shadow update documents.");
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);

    Log("ProfileVersion:");
    LogVersion(documents->profileVersion);

    JSON_Value* value = json_object_get_wrapping_value(documents->current);
    char* encoded = json_serialize_to_string(value);
    Log("Current:");
    Log(encoded);
    json_free_serialized_string(encoded);

    value = json_object_get_wrapping_value(documents->previous);
    encoded = json_serialize_to_string(value);
    Log("Previous:");
    Log(encoded);
    json_free_serialized_string(encoded);

    if (NULL == callbackContext)
    {
        LogError("Failure: the callback context is NULL.");
    }

    Log(SPLIT);
}

static void HandleUpdateSnapshot(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_SNAPSHOT* snapshot, void* callbackContext)
{
    Log("Received a message for shadow update snapshot.");
    Log("Request ID:");
    Log(messageContext->requestId);
    Log("Device:");
    Log(messageContext->device);

    Log("ProfileVersion:");
    LogVersion(snapshot->profileVersion);

    JSON_Value* value = json_object_get_wrapping_value(snapshot->reported);
    char* encoded = json_serialize_to_string(value);
    Log("Reported:");
    Log(encoded);
    json_free_serialized_string(encoded);

    value = json_object_get_wrapping_value(snapshot->lastUpdateTime);
    encoded = json_serialize_to_string(value);
    Log("LastUpdateTime:");
    Log(encoded);
    json_free_serialized_string(encoded);

    if (NULL == callbackContext)
    {
        LogError("Failure: the callback context is NULL.");
    }

    Log(SPLIT);
}

static void HandleDeleteAccepted(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ACCEPTED* accepted, void* callbackContext)
{
    Log("Received a message for shadow delete accepted.");
    HandleAccepted(messageContext, accepted, callbackContext);

    Log(SPLIT);
}

static void HandleDeleteRejected(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext)
{
    Log("Received a message for shadow delete rejected.");
    HandleRejected(messageContext, error, callbackContext);

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
    // However, here we only sleep, and then update the device shadow (status in reported payload).

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

static char firmwareVersion[64];

static void HandleOtaJob(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_OTA_JOB_INFO* otaJobInfo, void* callbackContext);

static void HandleOtaReportResult(const SHADOW_MESSAGE_CONTEXT* messageContext, void* callbackContext);

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
    iotdm_client_register_get_accepted(handle, HandleGetAccepted, handle);
    iotdm_client_register_get_rejected(handle, HandleGetRejected, handle);
    iotdm_client_register_update_accepted(handle, HandleUpdateAccepted, handle);
    iotdm_client_register_update_rejected(handle, HandleUpdateRejected, handle);
    iotdm_client_register_update_documents(handle, HandleUpdateDocuments, handle);
    iotdm_client_register_update_snapshot(handle, HandleUpdateSnapshot, handle);
    iotdm_client_register_delete_accepted(handle, HandleDeleteAccepted, handle);
    iotdm_client_register_delete_rejected(handle, HandleDeleteRejected, handle);

    iotdm_client_ota_register_job(handle, HandleOtaJob, handle);
    iotdm_client_ota_register_report_result(handle, HandleOtaReportResult, handle);

    IOTDM_CLIENT_OPTIONS options;
    options.cleanSession = true;
    options.clientId = DEVICE;
    options.username = USERNAME;
    options.password = PASSWORD;
    options.keepAliveInterval = 5;
    options.retryTimeoutInSeconds = 300;
    options.enableOta = true;

    if (0 != iotdm_client_connect(handle, &options))
    {
        iotdm_client_deinit(handle);
        Log("iotdm_client_connect failed");
        return __FAILURE__;
    }

    strncpy(firmwareVersion, "0.1.0", 64);

    // Subscribe the topics.
    iotdm_client_dowork(handle);
    iotdm_client_ota_get_job(handle, firmwareVersion, "1234");

    // Sample: get device shadow
    if (0 == iotdm_client_get_shadow(handle, DEVICE, "123456789"))
    {
        Log("Succeeded to get device shadow");
    }
    else
    {
        Log("Failed to get device shadow");
    }

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
            Log("Succeeded to update device shadow with binary");
        }
        else
        {
            Log("Failed to update device shadow with binary");
        }

        // Sample: update shadow with incorrect version, and receive error message at 'update/rejected'.
        if (0 == iotdm_client_update_shadow_with_binary(handle, DEVICE, "111111", 1, reportedString, NULL))
        {
            Log("Succeeded to send message for updating device shadow with binary");
        }
        else
        {
            Log("Failed to send message for updating device shadow with binary");
        }

        free(reportedString);
        free(reported);
    }

    DESTROY_MODEL_INSTANCE(pump);

    // Sample: delete the shadow
    if (0 == iotdm_client_delete_shadow(handle, DEVICE, "222222"))
    {
        Log("Succeeded to get device shadow");
    }
    else
    {
        Log("Failed to get device shadow");
    }

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

static void HandleOtaJob(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_OTA_JOB_INFO* otaJobInfo, void* callbackContext)
{
    LogInfo("Received an OTA job for %s", messageContext->device);
    // Pull OTA file
    unsigned int status;
    IOTDM_CLIENT_HANDLE handle = (IOTDM_CLIENT_HANDLE)callbackContext;

    if (0 == strcmp(firmwareVersion, otaJobInfo->firmwareVersion))
    {
        // No need to update.
        iotdm_client_ota_report_result(handle, otaJobInfo->jobId, true, "2345");
    }
    else
    {
        BUFFER_HANDLE firmwareBuffer = BUFFER_new();
        BOS_RESULT result = BOS_Download_Presigned(otaJobInfo->firmwareUrl, NULL, NULL, &status, firmwareBuffer);
        if (result != BOS_OK)
        {
            LogError("Failed to download from BOS");
        }
        // Flash the firmware
        BUFFER_delete(firmwareBuffer);

        // Report result

        iotdm_client_ota_report_start(handle, otaJobInfo->jobId, "1234");
        strncpy(firmwareVersion, otaJobInfo->firmwareVersion, 64);
        iotdm_client_ota_report_result(handle, otaJobInfo->jobId, true, "3456");
    }
}

static void HandleOtaReportResult(const SHADOW_MESSAGE_CONTEXT* messageContext, void* callbackContext)
{
    Log("OTA result reported");
}

