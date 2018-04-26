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

#ifndef IOT_SH_CLIENT_H
#define IOT_SH_CLIENT_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif // __cplusplus

#include "iot_smarthome_callback.h"
#include "parson.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct IOT_SH_CLIENT_TAG* IOT_SH_CLIENT_HANDLE;

typedef struct IOT_SH_CLIENT_OPTIONS_TAG
{
    // Indicate whether receiving deliveried message when reconnection.
    bool cleanSession;

    char* clientId;
    char* username;
    char* password;

    // Device keep alive interval with seconds.
    uint16_t keepAliveInterval;

    // Timeout for publishing device request in seconds.
    size_t retryTimeoutInSeconds;

    char* client_cert;
    char* client_key;

} IOT_SH_CLIENT_OPTIONS; 

MOCKABLE_FUNCTION(, IOT_SH_CLIENT_HANDLE, iot_smarthome_client_init, bool, isGatewayDevice);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_deinit, IOT_SH_CLIENT_HANDLE, handle);

MOCKABLE_FUNCTION(, int, iot_smarthome_client_connect, IOT_SH_CLIENT_HANDLE, handle, const char*, username, const char*, deviceId,
                    const char*, client_cert, const char*, client_key);

MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_delta, IOT_SH_CLIENT_HANDLE, handle, SHADOW_DELTA_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_get_accepted, IOT_SH_CLIENT_HANDLE, handle, SHADOW_ACCEPTED_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_get_rejected, IOT_SH_CLIENT_HANDLE, handle, SHADOW_ERROR_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_update_accepted, IOT_SH_CLIENT_HANDLE, handle, SHADOW_ACCEPTED_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_update_rejected, IOT_SH_CLIENT_HANDLE, handle, SHADOW_ERROR_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_update_documents, IOT_SH_CLIENT_HANDLE, handle, SHADOW_DOCUMENTS_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_register_update_snapshot, IOT_SH_CLIENT_HANDLE, handle, SHADOW_SNAPSHOT_CALLBACK, callback, void*, callbackContext);

MOCKABLE_FUNCTION(, int, iot_smarthome_client_get_shadow, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_desired, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, JSON_Value*, desired, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_shadow, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, JSON_Value*, reported, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_desired_with_binary, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, const char*, desired, const char*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_shadow_with_binary, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, const char*, reported, const char*, lastUpdatedTime);

// Pub actions for gateway on behalf on subdevices
MOCKABLE_FUNCTION(, int, iot_smarthome_client_get_subdevice_shadow, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_subdevice_desired, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, requestId, uint32_t, version, JSON_Value*, desired, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_subdevice_shadow, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, requestId, uint32_t, version, JSON_Value*, reported, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_subdevice_desired_with_binary, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, requestId, uint32_t, version, const char*, desired, const char*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_update_subdevice_shadow_with_binary, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, requestId, uint32_t, version, const char*, reported, const char*, lastUpdatedTime);


MOCKABLE_FUNCTION(, int, iot_smarthome_client_dowork, const IOT_SH_CLIENT_HANDLE, handle);

// OTA functions
MOCKABLE_FUNCTION(, void, iot_smarthome_client_ota_register_job, const IOT_SH_CLIENT_HANDLE, handle, SHADOW_OTA_JOB_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iot_smarthome_client_ota_register_report_result, const IOT_SH_CLIENT_HANDLE, handle, SHADOW_OTA_REPORT_RESULT_CALLBACK, callback, void*, callbackContext);

MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_get_job, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, firmwareVersion, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_report_start, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, jobId, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_report_result, const IOT_SH_CLIENT_HANDLE, handle, const char*, device, const char*, jobId, bool, isSuccess, const char*, requestId);

MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_get_subdevice_job, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, firmwareVersion, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_report_subdevice_start, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, jobId, const char*, requestId);
MOCKABLE_FUNCTION(, int, iot_smarthome_client_ota_report_subdevice_result, const IOT_SH_CLIENT_HANDLE, handle, const char*, gateway, const char*, subdevice, const char*, jobId, bool, isSuccess, const char*, requestId);

// RSA signature functions
MOCKABLE_FUNCTION(, const char *, computeSignature, unsigned char*, data, const char *, clientKey);
MOCKABLE_FUNCTION(, int, verifySignature, unsigned char*, data, const char*, clientCert, const char*, base64Signature);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // IOT_SH_CLIENT_H