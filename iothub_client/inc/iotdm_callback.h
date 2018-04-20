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

#ifndef IOTDM_CALLBACK_H
#define IOTDM_CALLBACK_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif // __cplusplus

#include "parson.h"

typedef enum SHADOW_CALLBACK_TYPE_TAG
{
    SHADOW_CALLBACK_TYPE_DELTA,
    SHADOW_CALLBACK_TYPE_GET_ACCEPTED,
    SHADOW_CALLBACK_TYPE_GET_REJECTED,
    SHADOW_CALLBACK_TYPE_UPDATE_ACCEPTED,
    SHADOW_CALLBACK_TYPE_UPDATE_REJECTED,
    SHADOW_CALLBACK_TYPE_UPDATE_DOCUMENTS,
    SHADOW_CALLBACK_TYPE_UPDATE_SNAPSHOT,
    SHADOW_CALLBACK_TYPE_DELETE_ACCEPTED,
    SHADOW_CALLBACK_TYPE_DELETE_REJECTED,
    SHADOW_CALLBACK_TYPE_METHOD_RESP,
    SHADOW_CALLBACK_TYPE_METHOD_REQ,
} SHADOW_CALLBACK_TYPE;

typedef struct SHADOW_ERROR_TAG
{
    int code;
    const char* message;
} SHADOW_ERROR;

typedef struct SHADOW_MESSAGE_CONTEXT_TAG
{
    const char* requestId;
    char* device;
} SHADOW_MESSAGE_CONTEXT;

typedef struct SHADOW_ACCEPTED_TAG
{
    int profileVersion;
    const JSON_Object* reported;
    const JSON_Object* desired;
    const JSON_Object* lastUpdateTime;
} SHADOW_ACCEPTED;

typedef struct SHADOW_DOCUMENTS_TAG
{
    int profileVersion;
    const JSON_Object* current;
    const JSON_Object* previous;
} SHADOW_DOCUMENTS;

typedef struct SHADOW_SNAPSHOT_TAG
{
    int profileVersion;
    const JSON_Object* reported;
    const JSON_Object* lastUpdateTime;
} SHADOW_SNAPSHOT;

typedef struct SHADOW_OTA_JOB_INFO_TAG
{
    const char *jobId;
    const char *firmwareUrl;
    const char *firmwareVersion;
} SHADOW_OTA_JOB_INFO;

typedef void (*SHADOW_DELTA_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const JSON_Object* desired, void* callbackContext);
typedef void (*SHADOW_ERROR_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ERROR* error, void* callbackContext);
typedef void (*SHADOW_ACCEPTED_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_ACCEPTED* accepted, void* callbackContext);
typedef void (*SHADOW_DOCUMENTS_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_DOCUMENTS* documents, void* callbackContext);
typedef void (*SHADOW_SNAPSHOT_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_SNAPSHOT* snapshot, void* callbackContext);

typedef void (*SHADOW_OTA_JOB_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const SHADOW_OTA_JOB_INFO* otaJobInfo, void* callbackContext);
typedef void (*SHADOW_OTA_REPORT_RESULT_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, void* callbackContext);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // IOTDM_CALLBACK_H