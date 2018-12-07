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
#include <azure_c_shared_utility/uuid.h>

#include "iot_smarthome_callback.h"
#include "iot_smarthome_client.h"
#include "bos.h"
#include "iot_smarthome_client_sample.h"

// Should include serializer to operate shadow with device model.
#include "serializer.h"

#define         SPLIT               "--------------------------------------------------------------------------------------------"

// $puid
#define         DEVICE              "your_puid"

// $endpointName/$puid
#define         USERNAME            "your_endpoint_name/your_puid"

// if your device is a gateway, you can add subdevice puids here
#define         SUBDEVICE           "your_gateway_subdevice_puid"

static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
                            "you client cert\r\n"
                            "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
                           "your client key\r\n"
                           "-----END RSA PRIVATE KEY-----\r\n";

static bool isGateway;

// define your own parameters here
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

    if (isGateway == true) {
        Log("SubDevice:");
        Log(messageContext->subdevice);
    }

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
    if (isGateway == true) {
        Log("SubDevice:");
        Log(messageContext->subdevice);
    }
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
    if (isGateway == true) {
        Log("SubDevice:");
        Log(messageContext->subdevice);
    }
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
    if (isGateway == true) {
        Log("SubDevice:");
        Log(messageContext->subdevice);
    }
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
    int result = messageContext->subdevice == NULL
                 ? iot_smarthome_client_update_shadow(handle, messageContext->device, messageContext->requestId, 0, value, NULL)
                 : iot_smarthome_client_update_subdevice_shadow(handle, messageContext->device, messageContext->subdevice, messageContext->requestId, 0, value, NULL);
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

static char gatewayFirmwareVersion[64];

static char subdeviceFirmwareVersion[64];

static int InitOta(void *handle);

static void HandleOtaJob(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_OTA_JOB_INFO* otaJobInfo, void* callbackContext);

static void HandleOtaReportResult(const SHADOW_MESSAGE_CONTEXT* messageContext, void* callbackContext);

int iot_smarthome_client_run(bool isGatewayDevice)
{
    isGateway = isGatewayDevice;
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

    IOT_SH_CLIENT_HANDLE handle = iot_smarthome_client_init(isGatewayDevice);
    if (NULL == handle)
    {
        Log("iot_smarthome_client_init failed");
        return __FAILURE__;
    }

    iot_smarthome_client_register_delta(handle, HandleDelta, handle);
    iot_smarthome_client_register_get_accepted(handle, HandleGetAccepted, handle);
    iot_smarthome_client_register_get_rejected(handle, HandleGetRejected, handle);
    iot_smarthome_client_register_update_accepted(handle, HandleUpdateAccepted, handle);
    iot_smarthome_client_register_update_rejected(handle, HandleUpdateRejected, handle);
    iot_smarthome_client_register_update_documents(handle, HandleUpdateDocuments, handle);
    iot_smarthome_client_register_update_snapshot(handle, HandleUpdateSnapshot, handle);
    iot_smarthome_client_ota_register_job(handle, HandleOtaJob, handle);
    iot_smarthome_client_ota_register_report_result(handle, HandleOtaReportResult, handle);

    // do sha256 rsa signature and verification
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

    strncpy(gatewayFirmwareVersion, "0.1.0", 64);
    strncpy(subdeviceFirmwareVersion, "0.1.0", 64);

    if (0 != iot_smarthome_client_connect(handle, USERNAME, DEVICE, client_cert, client_key))
    {
        iot_smarthome_client_deinit(handle);
        Log("iot_smarthome_client_connect failed");
        return __FAILURE__;
    }

    // Subscribe the topics.
    iot_smarthome_client_dowork(handle);

    // Sample: get device shadow
    int result = isGateway ? iot_smarthome_client_get_subdevice_shadow(handle, DEVICE, SUBDEVICE, "123456789")
                           : iot_smarthome_client_get_shadow(handle, DEVICE, "123456789");
    if (0 == result)
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

        result = isGatewayDevice
                 ? iot_smarthome_client_update_subdevice_shadow_with_binary(handle, DEVICE, SUBDEVICE, "123456", 0, reportedString, NULL)
                : iot_smarthome_client_update_shadow_with_binary(handle, DEVICE, "123456", 0, reportedString, NULL);
        if (0 == result)
        {
            Log("Succeeded to update device shadow with binary");
        }
        else
        {
            Log("Failed to update device shadow with binary");
        }

        // Sample: update shadow with incorrect version, and receive error message at 'update/rejected'.
        result = isGateway ? iot_smarthome_client_update_subdevice_shadow_with_binary(handle, DEVICE, SUBDEVICE, "111111", 1, reportedString, NULL)
                           : iot_smarthome_client_update_shadow_with_binary(handle, DEVICE, "111111", 1, reportedString, NULL);
        if (0 == result)
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

    // Report firmware version and get OTA job
    iot_smarthome_client_ota_get_job((IOT_SH_CLIENT_HANDLE) handle, DEVICE, gatewayFirmwareVersion, "1234");
    if (isGateway)
    {
        iot_smarthome_client_ota_get_subdevice_job((IOT_SH_CLIENT_HANDLE) handle, DEVICE, SUBDEVICE,
                                                   subdeviceFirmwareVersion, "2345");
    }

    // Sample: subscribe the delta topic and update shadow with desired value.
    while (iot_smarthome_client_dowork(handle) >= 0)
    {
        ThreadAPI_Sleep(100);
    }

    iot_smarthome_client_deinit(handle);
    serializer_deinit();
    platform_deinit();

    return 0;

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

static char lastJobId[64];

static void HandleOtaJob(const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_OTA_JOB_INFO* otaJobInfo, void* callbackContext)
{
    const char *actualDevice = messageContext->subdevice == NULL ? messageContext->device : messageContext->subdevice;
    LogInfo("Received an OTA job for %s", actualDevice);
    if (0 == strcmp(lastJobId, otaJobInfo->jobId))
    {
        // Given AT_LEAST_ONCE QoS, we have to consider the situation that the ota job may be sent multiple times.
        return;
    }
    // Pull OTA file
    unsigned int status;
    BUFFER_HANDLE firmwareBuffer = BUFFER_new();
    BOS_RESULT result = BOS_Download_Presigned(otaJobInfo->firmwareUrl, NULL, NULL, &status, firmwareBuffer);
    if (result != BOS_OK)
    {
        LogError("Failed to download from BOS");
    }
    // Flash the firmware
    BUFFER_delete(firmwareBuffer);
    IOT_SH_CLIENT_HANDLE handle = (IOT_SH_CLIENT_HANDLE)callbackContext;
    // Report result
    if (messageContext->subdevice == NULL)
    {
        iot_smarthome_client_ota_report_start(handle, messageContext->device, otaJobInfo->jobId, "2345");
        strncpy(gatewayFirmwareVersion, otaJobInfo->firmwareVersion, 64);
        iot_smarthome_client_ota_report_result(handle, messageContext->device, otaJobInfo->jobId, true, "2345");
    }
    else
    {
        iot_smarthome_client_ota_report_subdevice_start(handle, messageContext->device, messageContext->subdevice, otaJobInfo->jobId, "2345");
        strncpy(subdeviceFirmwareVersion, otaJobInfo->firmwareVersion, 64);
        iot_smarthome_client_ota_report_subdevice_result(handle, messageContext->device, messageContext->subdevice, otaJobInfo->jobId, true, "2345");
    }
    strcpy(lastJobId, otaJobInfo->jobId);
}

static void HandleOtaReportResult(const SHADOW_MESSAGE_CONTEXT* messageContext, void* callbackContext)
{
    Log("OTA result reported");
}
