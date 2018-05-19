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

#ifndef IOTDM_CLIENT_H
#define IOTDM_CLIENT_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif // __cplusplus

#include "iotdm_callback.h"
#include "parson.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct IOTDM_CLIENT_TAG* IOTDM_CLIENT_HANDLE;

typedef struct IOTDM_CLIENT_OPTIONS_TAG
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

    // If set to true, will get OTA job after each reconnect in iotdm_client_dowork.
    bool enableOta;

} IOTDM_CLIENT_OPTIONS; 

MOCKABLE_FUNCTION(, IOTDM_CLIENT_HANDLE, iotdm_client_init, char*, broker, char*, name);
MOCKABLE_FUNCTION(, void, iotdm_client_deinit, IOTDM_CLIENT_HANDLE, handle);

MOCKABLE_FUNCTION(, int, iotdm_client_connect, IOTDM_CLIENT_HANDLE, handle, const IOTDM_CLIENT_OPTIONS*, options);

MOCKABLE_FUNCTION(, void, iotdm_client_register_delta, IOTDM_CLIENT_HANDLE, handle, SHADOW_DELTA_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_get_accepted, IOTDM_CLIENT_HANDLE, handle, SHADOW_ACCEPTED_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_get_rejected, IOTDM_CLIENT_HANDLE, handle, SHADOW_ERROR_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_update_accepted, IOTDM_CLIENT_HANDLE, handle, SHADOW_ACCEPTED_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_update_rejected, IOTDM_CLIENT_HANDLE, handle, SHADOW_ERROR_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_update_documents, IOTDM_CLIENT_HANDLE, handle, SHADOW_DOCUMENTS_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_update_snapshot, IOTDM_CLIENT_HANDLE, handle, SHADOW_SNAPSHOT_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_delete_accepted, IOTDM_CLIENT_HANDLE, handle, SHADOW_ACCEPTED_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_register_delete_rejected, IOTDM_CLIENT_HANDLE, handle, SHADOW_ERROR_CALLBACK, callback, void*, callbackContext);

MOCKABLE_FUNCTION(, int, iotdm_client_get_shadow, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId);
MOCKABLE_FUNCTION(, int, iotdm_client_delete_shadow, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId);
MOCKABLE_FUNCTION(, int, iotdm_client_update_desired, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, JSON_Value*, desired, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iotdm_client_update_shadow, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, JSON_Value*, reported, JSON_Value*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iotdm_client_update_desired_with_binary, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, const char*, desired, const char*, lastUpdatedTime);
MOCKABLE_FUNCTION(, int, iotdm_client_update_shadow_with_binary, const IOTDM_CLIENT_HANDLE, handle, const char*, device, const char*, requestId, uint32_t, version, const char*, reported, const char*, lastUpdatedTime);


MOCKABLE_FUNCTION(, int, iotdm_client_dowork, const IOTDM_CLIENT_HANDLE, handle);

// OTA functions
MOCKABLE_FUNCTION(, void, iotdm_client_ota_register_job, const IOTDM_CLIENT_HANDLE, handle, SHADOW_OTA_JOB_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_ota_register_report_start, const IOTDM_CLIENT_HANDLE, handle, SHADOW_OTA_REPORT_RESULT_CALLBACK, callback, void*, callbackContext);
MOCKABLE_FUNCTION(, void, iotdm_client_ota_register_report_result, const IOTDM_CLIENT_HANDLE, handle, SHADOW_OTA_REPORT_RESULT_CALLBACK, callback, void*, callbackContext);

MOCKABLE_FUNCTION(, int, iotdm_client_ota_get_job, const IOTDM_CLIENT_HANDLE, handle, const char*, firmwareVersion, const char*, requestId);
MOCKABLE_FUNCTION(, int, iotdm_client_ota_report_start, const IOTDM_CLIENT_HANDLE, handle, const char*, jobId, const char*, requestId);
MOCKABLE_FUNCTION(, int, iotdm_client_ota_report_result, const IOTDM_CLIENT_HANDLE, handle, const char*, jobId, bool, isSuccess, const char*, requestId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // IOTDM_CLIENT_H