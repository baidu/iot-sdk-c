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

#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/uuid.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iotdm_client.h"
#include "iothub_mqtt_client.h"

#define     SUB_TOPIC_SIZE                  9
#define     SUB_METHOD_TOPIC_SIZE           2

#define     SLASH                           '/'

#define     TOPIC_PREFIX                    "$baidu/iot/shadow/"

#define     TOPIC_SUFFIX_DELTA              "delta"
#define     TOPIC_SUFFIX_GET_ACCEPTED       "get/accepted"
#define     TOPIC_SUFFIX_GET_REJECTED       "get/rejected"
#define     TOPIC_SUFFIX_UPDATE_ACCETPED    "update/accepted"
#define     TOPIC_SUFFIX_UPDATE_REJECTED    "update/rejected"
#define     TOPIC_SUFFIX_UPDATE_DOCUMENTS   "update/documents"
#define     TOPIC_SUFFIX_UPDATE_SNAPSHOT    "update/snapshot"
#define     TOPIC_SUFFIX_DELETE_ACCEPTED    "delete/accepted"
#define     TOPIC_SUFFIX_DELETE_REJECTED    "delete/rejected"
#define     TOPIC_SUFFIX_METHOD_DEVICE_REQ  "method/device/req"
#define     TOPIC_SUFFIX_METHOD_CLOUD_RESP  "method/cloud/resp"

#define     PUB_GET                         "$baidu/iot/shadow/%s/get"
#define     PUB_UPDATE                      "$baidu/iot/shadow/%s/update"
#define     PUB_DELETE                      "$baidu/iot/shadow/%s/delete"
#define     PUB_METHOD_CLOUD_REQ            "$baidu/iot/shadow/%s/method/cloud/req"
#define     PUB_METHOD_DEVICE_RESP          "$baidu/iot/shadow/%s/method/device/resp"


#define     SUB_DELTA                       "$baidu/iot/shadow/%s/delta"
#define     SUB_GET_ACCEPTED                "$baidu/iot/shadow/%s/get/accepted"
#define     SUB_GET_REJECTED                "$baidu/iot/shadow/%s/get/rejected"
#define     SUB_UPDATE_ACCEPTED             "$baidu/iot/shadow/%s/update/accepted"
#define     SUB_UPDATE_REJECTED             "$baidu/iot/shadow/%s/update/rejected"
#define     SUB_UPDATE_DOCUMENTS            "$baidu/iot/shadow/%s/update/documents"
#define     SUB_UPDATE_SNAPSHOT             "$baidu/iot/shadow/%s/update/snapshot"
#define     SUB_DELETE_ACCEPTED             "$baidu/iot/shadow/%s/delete/accepted"
#define     SUB_DELETE_REJECTED             "$baidu/iot/shadow/%s/delete/rejected"
#define     SUB_METHOD_RESP                 "$baidu/iot/shadow/%s/method/cloud/resp"
#define     SUB_METHOD_REQ                  "$baidu/iot/shadow/%s/method/device/req"

#define     KEY_CODE                        "code"
#define     KEY_DESIRED                     "desired"
#define     KEY_LASTUPDATEDTIME             "lastUpdatedTime"
#define     KEY_MESSAGE                     "message"
#define     KEY_REPORTED                    "reported"
#define     KEY_REQUEST_ID                  "requestId"
#define     KEY_VERSION                     "profileVersion"
#define     KEY_CURRENT                     "current"
#define     KEY_PREVIOUS                    "previous"

#define     KEY_METHOD_NAME                 "methodName"
#define     KEY_STATUS                      "status"
#define     KEY_PAYLOAD                     "payload"
#define     KEY_JOB_ID                      "jobId"
#define     KEY_FIRMWARE_VERSION            "firmwareVersion"
#define     KEY_FIRMWARE_URL                "firmwareUrl"
#define     KEY_RESULT                      "result"

#define     VALUE_RESULT_SUCCESS            "success"
#define     VALUE_RESULT_FAILURE            "failure"

#define     METHOD_GET_FIRMWARE             "getFirmware"
#define     METHOD_DO_FIRMWARE_UPDATE       "doFirmwareUpdate"
#define     METHOD_REPORT_FIRMWARE_UPDATE_START     "reportFirmwareUpdateStart"
#define     METHOD_REPORT_FIRMWARE_UPDATE_RESULT    "reportFirmwareUpdateResult"

typedef struct SHADOW_CALLBACK_TAG
{
    SHADOW_DELTA_CALLBACK delta;
    SHADOW_ACCEPTED_CALLBACK getAccepted;
    SHADOW_ERROR_CALLBACK getRejected;
    SHADOW_ERROR_CALLBACK updateRejected;
    SHADOW_ACCEPTED_CALLBACK updateAccepted;
    SHADOW_DOCUMENTS_CALLBACK updateDocuments;
    SHADOW_SNAPSHOT_CALLBACK updateSnapshot;
    SHADOW_ACCEPTED_CALLBACK deleteAccepted;
    SHADOW_ERROR_CALLBACK deleteRejected;
    SHADOW_OTA_JOB_CALLBACK otaJob;
    SHADOW_OTA_REPORT_RESULT_CALLBACK otaReportStart;
    SHADOW_OTA_REPORT_RESULT_CALLBACK otaReportResult;
} SHADOW_CALLBACK;

typedef struct SHADOW_CALLBACK_CONTEXT_TAG
{
    void* delta;
    void* getAccepted;
    void* getRejected;
    void* updateRejected;
    void* updateAccepted;
    void* updateDocuments;
    void* updateSnapshot;
    void* deleteAccepted;
    void* deleteRejected;
    void* otaJob;
    void* otaReportStart;
    void* otaReportResult;
} SHADOW_CALLBACK_CONTEXT;

typedef struct IOTDM_CLIENT_TAG
{
    bool subscribed;
    time_t subscribeSentTimestamp;
    char* endpoint;
    char* name;
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;
    MQTT_CONNECTION_TYPE mqttConnType;
    SHADOW_CALLBACK callback;
    SHADOW_CALLBACK_CONTEXT context;
    bool enableOta;
} IOTDM_CLIENT;

/* Send a Method response to cloud */
static int SendMethodResp(IOTDM_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT *msgContext, const char *methodName, JSON_Value *payload) ;

static size_t StringLength(const char* string)
{
    if (NULL == string) return 0;
    
    size_t index = 0;
    while ('\0' != string[index++]) {}

    return index - 1;
}

static bool StringCmp(const char* src, const char* des, size_t head, size_t end)
{
    if (NULL == src && NULL == des) return true;
    if (NULL == src || NULL == des) return false;

    size_t length = end - head;
    for (size_t index = 0; index < length; ++index)
    {
        if (src[index] != des[head + index]) return false;
    }

    return true;
}

static void StringCpy(char* des, const char* src, size_t head, size_t end)
{
    for (size_t index = head; index < end; ++index)
    {
        des[index - head] = src[index];
    }
}

static char* GenerateTopic(const char* formate, const char* device)
{
    if (NULL == formate || NULL == device)
    {
        LogError("Failure: both formate and device should not be NULL.");
        return NULL;
    }
    size_t size = StringLength(formate) + StringLength(device) + 1;
    char* topic = malloc(sizeof(char) * size);
    if (NULL == topic) return NULL;
    if (sprintf_s(topic, size, formate, device) < 0)
    {
        free(topic);
        topic = NULL;
    }

    return topic;
}

static char* GetBrokerEndpoint(char* broker, MQTT_CONNECTION_TYPE* mqttConnType)
{
    if ('t' == broker[0] && 'c' == broker[1] && 'p' == broker[2])
    {
        *mqttConnType = MQTT_CONNECTION_TCP;
    }
    else if ('s' == broker[0] && 's' == broker[1] && 'l' == broker[2])
    {
        *mqttConnType = MQTT_CONNECTION_TLS;
    }
    else
    {
        LogError("Failure: the prefix of the broker address is incorrect.");
        return NULL;
    }

    size_t head = 0;
    size_t end = 0;
    for (size_t pos = 0; '\0' != broker[pos]; ++pos)
    {
        if (':' == broker[pos])
        {
            end = head == 0 ? end : pos;
            // For head, should skip the substring "//".
            head = head == 0 ? pos + 3 : head;
        }
    }
    if (head == 0 || end <= head)
    {
        LogError("Failure: cannot get the endpoint from broker address.");
        return NULL;
    }

    size_t length = end - head + 1;
    char* endpoint = malloc(sizeof(char) * length);
    if (NULL == endpoint)
    {
        LogError("Failure: cannot init the memory for endpoint.");
        return NULL;
    }

    endpoint[length - 1] = '\0';
    for (size_t pos = 0; pos < length - 1; ++pos)
    {
        endpoint[pos] = broker[head + pos];
    }

    return endpoint;
}

static char* GetDeviceFromTopic(const char* topic, SHADOW_CALLBACK_TYPE* type)
{
    char* prefix = TOPIC_PREFIX;

    size_t prefixLength = StringLength(prefix);
    if (false == StringCmp(prefix, topic, 0, prefixLength))
    {
        LogError("Failure: the topic prefix is illegal.");
        return NULL;
    }

    size_t head = prefixLength;
    size_t end = head;
    for (size_t index = head; '\0' != topic[index]; ++index)
    {
        if (SLASH == topic[index])
        {
            end = index;
            break;
        }
    }
    if (end <= head)
    {
        LogError("Failure: the topic is illegal.");
        return NULL;
    }

    size_t tmp;
    size_t topicLength = strlen(topic);
    // TODO: support more topics for subscription handle.
    if (StringCmp(TOPIC_SUFFIX_DELTA, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_DELTA;
    }
    else if (StringCmp(TOPIC_SUFFIX_GET_ACCEPTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_GET_ACCEPTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_GET_REJECTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_GET_REJECTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_ACCETPED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_ACCEPTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_REJECTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_REJECTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_DOCUMENTS, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_DOCUMENTS;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_SNAPSHOT, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_SNAPSHOT;
    }
    else if (StringCmp(TOPIC_SUFFIX_DELETE_ACCEPTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_DELETE_ACCEPTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_DELETE_REJECTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_DELETE_REJECTED;
    }
    else if (StringCmp(TOPIC_SUFFIX_METHOD_DEVICE_REQ, topic, tmp = topicLength - strlen(TOPIC_SUFFIX_METHOD_DEVICE_REQ), topicLength + 1)) {
        *type = SHADOW_CALLBACK_TYPE_METHOD_REQ;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_METHOD_CLOUD_RESP, topic, tmp = topicLength - strlen(TOPIC_SUFFIX_METHOD_CLOUD_RESP), topicLength + 1)) {
        *type = SHADOW_CALLBACK_TYPE_METHOD_RESP;
        end = tmp - 1;
    }
    else
    {
        LogError("Failure: the subscribe handle is not supported.");
        return NULL;
    }

    char* device = malloc(sizeof(char) * (end - head + 1));
    if (NULL == device)
    {
        LogError("Failure: failed to allocate memory for device.");
        return NULL;
    }

    device[end - head] = '\0';
    StringCpy(device, topic, head, end);
    return device;
}

static void ResetSubscription(char** subscribe, size_t length)
{
    if (NULL == subscribe) return;
    for (size_t index = 0; index < length; ++index)
    {
        subscribe[index] = NULL;
    }
}

static void ReleaseSubscription(char** subscribe, size_t length)
{
    if (NULL == subscribe) return;
    for (size_t index = 0; index < length; ++index)
    {
        if (NULL != subscribe[index])
        {
            free(subscribe[index]);
            subscribe[index] = NULL;
        }
    }
}

static int GetSubscription(IOTDM_CLIENT_HANDLE handle, char** subscribe, size_t length)
{
    if (NULL == subscribe) return 0;

    size_t index = 0;
    ResetSubscription(subscribe, length);
    if (NULL != handle->callback.delta)
    {
        subscribe[index] = GenerateTopic(SUB_DELTA, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'delta'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.getAccepted)
    {
        subscribe[index] = GenerateTopic(SUB_GET_ACCEPTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'get/accepted'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.getRejected)
    {
        subscribe[index] = GenerateTopic(SUB_GET_REJECTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'get/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateAccepted)
    {
        subscribe[index] = GenerateTopic(SUB_UPDATE_ACCEPTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'update/accepted'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateRejected)
    {
        subscribe[index] = GenerateTopic(SUB_UPDATE_REJECTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'update/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateDocuments)
    {
        subscribe[index] = GenerateTopic(SUB_UPDATE_DOCUMENTS, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'update/documents'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateSnapshot)
    {
        subscribe[index] = GenerateTopic(SUB_UPDATE_SNAPSHOT, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'update/snapshot'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.deleteAccepted)
    {
        subscribe[index] = GenerateTopic(SUB_DELETE_ACCEPTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'delete/accepted'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.deleteRejected)
    {
        subscribe[index] = GenerateTopic(SUB_DELETE_REJECTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'delete/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (handle->enableOta)
    {
        subscribe[index] = GenerateTopic(SUB_METHOD_REQ, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'method/req'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
        subscribe[index] = GenerateTopic(SUB_METHOD_RESP, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Failure: failed to generate the sub topic 'method/resp'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }

    return index;
}

static void OnRecvCallbackForDelta(const IOTDM_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
{
    const JSON_Object* desired = json_object_get_object(root, KEY_DESIRED);
    if (NULL == desired)
    {
        LogError("Failure: cannot find desired object in the delta message.");
        return;
    }

    (*(handle->callback.delta))(msgContext, desired, handle->context.delta);
}

static void OnRecvCallbackForAccepted(const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root, const SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    SHADOW_ACCEPTED shadow_accepted;
    shadow_accepted.reported = json_object_get_object(root, KEY_REPORTED);
    shadow_accepted.desired = json_object_get_object(root, KEY_DESIRED);
    shadow_accepted.lastUpdateTime = json_object_get_object(root, KEY_LASTUPDATEDTIME);
    shadow_accepted.profileVersion = (int)json_object_get_number(root, KEY_VERSION);

    (*callback)(msgContext, &shadow_accepted, callbackContext);
}

static void OnRecvCallbackForDocuments(const IOTDM_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
{
    SHADOW_DOCUMENTS shadow_documents;
    shadow_documents.profileVersion = (int)json_object_get_number(root, KEY_VERSION);
    shadow_documents.current = json_object_get_object(root, KEY_CURRENT);
    shadow_documents.previous = json_object_get_object(root, KEY_PREVIOUS);

    (*(handle->callback.updateDocuments))(msgContext, &shadow_documents, handle->context.updateDocuments);
}

static void OnRecvCallbackForSnapshot(const IOTDM_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
{
    SHADOW_SNAPSHOT shadow_snapshot;
    shadow_snapshot.profileVersion = (int)json_object_get_number(root, KEY_VERSION);
    shadow_snapshot.reported = json_object_get_object(root, KEY_REPORTED);
    shadow_snapshot.lastUpdateTime = json_object_get_object(root, KEY_LASTUPDATEDTIME);

    (*(handle->callback.updateSnapshot))(msgContext, &shadow_snapshot, handle->context.updateSnapshot);
}

static void OnRecvCallbackForError(const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root, const SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    SHADOW_ERROR error;
    error.code = (int)json_object_get_number(root, KEY_CODE);
    error.message = json_object_get_string(root, KEY_MESSAGE);

    (*callback)(msgContext, &error, callbackContext);
}

static void OnRecvCallbackForMethodReq(const IOTDM_CLIENT_HANDLE handle, const char *topic,
                                       const SHADOW_MESSAGE_CONTEXT *msgContext, const JSON_Object *root,
                                       const APP_PAYLOAD* payload)
{
    STRING_HANDLE message = STRING_from_byte_array(payload->message, payload->length);
    if (message != NULL) {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Method request:\n%s\n%s", topic, STRING_c_str(message));
        STRING_delete(message);
    }
    else
    {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Method request:\n%s", topic);
    }
    const char* methodName = json_object_get_string(root, KEY_METHOD_NAME);

    if (NULL == methodName) {
        LogError("Failure: methodName should not be NULL");
    } else {
        double status = json_object_get_number(root, KEY_STATUS);
        JSON_Object* payload = json_object_get_object(root, KEY_PAYLOAD);
        if (strcmp(methodName, METHOD_DO_FIRMWARE_UPDATE) == 0)
        {
            // Handle response for get firmware
            if (NULL != handle->callback.otaJob)
            {
                SHADOW_OTA_JOB_INFO otaJobInfo;
                otaJobInfo.jobId = json_object_get_string(payload, KEY_JOB_ID);
                otaJobInfo.firmwareUrl = json_object_get_string(payload, KEY_FIRMWARE_URL);
                otaJobInfo.firmwareVersion = json_object_get_string(payload, KEY_FIRMWARE_VERSION);
                (*(handle->callback.otaJob))(msgContext, &otaJobInfo, handle->context.otaJob);
                SendMethodResp(handle, msgContext, methodName, NULL);
            }
        }
        else
        {
            LogError("Failure: cannot handle unknown method %s.", methodName);
        }
    }
}

static void OnRecvCallbackForMethodResp(const IOTDM_CLIENT_HANDLE handle, const char *topic,
                                        const SHADOW_MESSAGE_CONTEXT *msgContext, const JSON_Object *root,
                                        const APP_PAYLOAD* payload)
{
    STRING_HANDLE message = STRING_from_byte_array(payload->message, payload->length);
    if (message != NULL) {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Method response:\n%s\n%s", topic, STRING_c_str(message));
        STRING_delete(message);
    }
    else
    {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Received Method response:\n%s", topic);
    }
    const char* methodName = json_object_get_string(root, KEY_METHOD_NAME);

    if (NULL == methodName) {
        LogError("Failure: methodName should not be NULL");
    } else {
        double status = json_object_get_number(root, KEY_STATUS);
        JSON_Object* payload = json_object_get_object(root, KEY_PAYLOAD);
        if (strcmp(methodName, METHOD_GET_FIRMWARE) == 0)
        {
            // Handle response for get firmware
            if (status == 404)
            {
                // No job
            }
            else if (status == 200)
            {
                if (NULL != handle->callback.otaJob)
                {
                    SHADOW_OTA_JOB_INFO otaJobInfo;
                    otaJobInfo.jobId = json_object_get_string(payload, KEY_JOB_ID);
                    otaJobInfo.firmwareUrl = json_object_get_string(payload, KEY_FIRMWARE_URL);
                    otaJobInfo.firmwareVersion = json_object_get_string(payload, KEY_FIRMWARE_VERSION);
                    (*(handle->callback.otaJob))(msgContext, &otaJobInfo, handle->context.otaJob);
                }
            }
            else
            {
                LogError("Unexpected response. %d", status);
            }
        }
        else if (strcmp(methodName, METHOD_REPORT_FIRMWARE_UPDATE_START) == 0)
        {
            if (status >= 200 && status < 300)
            {
                if (NULL != handle->callback.otaReportStart)
                {
                    (*(handle->callback.otaReportStart))(msgContext, handle->context.otaReportStart);
                }
            }
            else
            {
                LogError("Unexpected response. %d", status);
            }
        }
        else if (strcmp(methodName, METHOD_REPORT_FIRMWARE_UPDATE_RESULT) == 0)
        {
            if (status >= 200 && status < 300)
            {
                if (NULL != handle->callback.otaReportResult)
                {
                    (*(handle->callback.otaReportResult))(msgContext, handle->context.otaReportResult);
                }
            }
            else
            {
                LogError("Unexpected response. %d", status);
            }
        }
        else
        {
            LogError("Failure: cannot handle unknown method response %s.", methodName);
        }
    }
}

static void OnRecvCallback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const char* topic = mqttmessage_getTopicName(msgHandle);
    const APP_PAYLOAD* payload = mqttmessage_getApplicationMsg(msgHandle);
    if (NULL == topic || NULL == payload)
    {
        LogError("Failure: cannot find topic or payload in the message received.");
        return;
    }

    SHADOW_CALLBACK_TYPE type;
    SHADOW_MESSAGE_CONTEXT msgContext;
    msgContext.device = GetDeviceFromTopic(topic, &type);
    if (NULL == msgContext.device)
    {
        LogError("Failure: cannot get the device name from the subscribed topic.");
        return;
    }

    IOTHUB_MQTT_CLIENT_HANDLE mqttClient = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    IOTDM_CLIENT_HANDLE handle = (IOTDM_CLIENT_HANDLE)mqttClient->callbackContext;

    STRING_HANDLE jsonData = STRING_from_byte_array(payload->message, payload->length);
    if (jsonData == NULL)
    {
        LogError("Failure: failed to copy MQTT payload.");
        free(msgContext.device);
        return ;
    }

    JSON_Value* data = json_parse_string(STRING_c_str(jsonData));
    if (NULL == data)
    {
        LogError("Failure: failed to deserialize the payload to json.");
        free(msgContext.device);
        STRING_delete(jsonData);
        return;
    }

    JSON_Object* root = json_object(data);
    msgContext.requestId = json_object_get_string(root, KEY_REQUEST_ID);
    if (NULL == msgContext.requestId)
    {
        LogError("Failure: cannot find the request ID in the received message.");
    }
    else
    {
        switch (type)
        {
            case SHADOW_CALLBACK_TYPE_DELTA:
                OnRecvCallbackForDelta(handle, &msgContext, root);
                break;

            case SHADOW_CALLBACK_TYPE_GET_ACCEPTED:
                OnRecvCallbackForAccepted(&msgContext, root, handle->callback.getAccepted, handle->context.getAccepted);
                break;

            case SHADOW_CALLBACK_TYPE_GET_REJECTED:
                OnRecvCallbackForError(&msgContext, root, handle->callback.getRejected, handle->context.getRejected);
                break;

            case SHADOW_CALLBACK_TYPE_UPDATE_ACCEPTED:
                OnRecvCallbackForAccepted(&msgContext, root, handle->callback.updateAccepted, handle->context.updateAccepted);
                break;

            case SHADOW_CALLBACK_TYPE_UPDATE_REJECTED:
                OnRecvCallbackForError(&msgContext, root, handle->callback.updateRejected, handle->context.updateRejected);
                break;

            case SHADOW_CALLBACK_TYPE_UPDATE_DOCUMENTS:
                OnRecvCallbackForDocuments(handle, &msgContext, root);
                break;

            case SHADOW_CALLBACK_TYPE_UPDATE_SNAPSHOT:
                OnRecvCallbackForSnapshot(handle, &msgContext, root);
                break;

            case SHADOW_CALLBACK_TYPE_DELETE_ACCEPTED:
                OnRecvCallbackForAccepted(&msgContext, root, handle->callback.deleteAccepted, handle->context.deleteAccepted);
                break;

            case SHADOW_CALLBACK_TYPE_DELETE_REJECTED:
                OnRecvCallbackForError(&msgContext, root, handle->callback.deleteRejected, handle->context.deleteRejected);
                break;

            case SHADOW_CALLBACK_TYPE_METHOD_REQ:
                OnRecvCallbackForMethodReq(handle, topic, &msgContext, root, payload);
                break;

            case SHADOW_CALLBACK_TYPE_METHOD_RESP:
                OnRecvCallbackForMethodResp(handle, topic, &msgContext, root, payload);
                break;

            default:
                LogError("Failure: the shadow callback type is not supported.");
        }
    }

    STRING_delete(jsonData);
    free(msgContext.device);
    json_value_free(data);
}

static void InitIotHubClient(IOTDM_CLIENT_HANDLE handle, const IOTDM_CLIENT_OPTIONS* options) {
    MQTT_CLIENT_OPTIONS mqttClientOptions;
    mqttClientOptions.clientId = options->clientId == NULL ? handle->name : options->clientId;
    mqttClientOptions.willTopic = NULL;
    mqttClientOptions.willMessage = NULL;
    mqttClientOptions.username = options->username;
    mqttClientOptions.password = options->password;
    mqttClientOptions.keepAliveInterval = options->keepAliveInterval > 0 ? options->keepAliveInterval : 10;
    mqttClientOptions.messageRetain = false;
    mqttClientOptions.useCleanSession = options->cleanSession;
    mqttClientOptions.qualityOfServiceValue = DELIVER_AT_LEAST_ONCE;

    handle->mqttClient = initialize_mqtt_client_handle(
        &mqttClientOptions,
        handle->endpoint,
        handle->mqttConnType,
        OnRecvCallback,
        IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER,
        options->retryTimeoutInSeconds < 300 ? 300 : options->retryTimeoutInSeconds);
}

static void ResetIotDmClient(IOTDM_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;

        handle->endpoint = NULL;
        handle->name = NULL;
        handle->mqttClient = NULL;

        handle->callback.delta = NULL;
        handle->callback.getAccepted = NULL;
        handle->callback.getRejected = NULL;
        handle->callback.updateAccepted = NULL;
        handle->callback.updateRejected = NULL;
        handle->callback.updateDocuments = NULL;
        handle->callback.updateSnapshot = NULL;
        handle->callback.deleteAccepted = NULL;
        handle->callback.deleteRejected = NULL;
        handle->callback.otaJob = NULL;
        handle->callback.otaReportStart = NULL;
        handle->callback.otaReportResult = NULL;

        handle->context.delta = NULL;
        handle->context.getAccepted = NULL;
        handle->context.getRejected = NULL;
        handle->context.updateAccepted = NULL;
        handle->context.updateRejected = NULL;
        handle->context.updateSnapshot = NULL;
        handle->context.updateDocuments = NULL;
        handle->context.deleteAccepted = NULL;
        handle->context.deleteRejected = NULL;
        handle->context.otaJob = NULL;
        handle->context.otaReportStart = NULL;
        handle->context.otaReportResult = NULL;
    }
}

static int SendRequest(const IOTDM_CLIENT_HANDLE handle, char* topic, JSON_Value* request)
{
    int result = 0;
    if (NULL == handle || NULL == topic || NULL == request)
    {
        LogError("Failure: handle, topic and payload should not be NULL.");
        result = __FAILURE__;
    }
    else
    {
        char* encoded = json_serialize_to_string(request);
        if (NULL == encoded)
        {
            LogError("Failue: failed to encode the json.");
            result = __FAILURE__;
        }
        else
        {
            result = publish_mqtt_message(handle->mqttClient, topic, DELIVER_AT_LEAST_ONCE, (uint8_t*)encoded, strlen(encoded), NULL, NULL);
            if (result != 0)
            {
                LogError("Failed to publish method message");
            }
            json_free_serialized_string(encoded);
        }
    }

    if (NULL != topic) free(topic);
    if (NULL != request) json_value_free(request);

    return result;
}

int UpdateShadow(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* key, const char* requestId, uint32_t version, JSON_Value* shadow, JSON_Value* lastUpdatedTime)
{
    if (NULL == requestId)
    {
        LogError("Failure: request id should not be NULL.");
        return __FAILURE__;
    }
    if (NULL == shadow)
    {
        LogError("Failure: the shadow properties should not be NULL.");
        return __FAILURE__;
    }

    // Consider the exceptions in the json operations.
    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_REQUEST_ID, requestId);
    json_object_set_value(root, key, json_value_deep_copy(shadow));
    json_object_set_number(root, KEY_VERSION, version);
    if (NULL != lastUpdatedTime)
    {
        JSON_Value* lastUpdatedTimeForReported = json_value_deep_copy(lastUpdatedTime);
        lastUpdatedTime = json_value_init_object();
        json_object_set_value(json_object(lastUpdatedTime), key, lastUpdatedTimeForReported);
        json_object_set_value(root, KEY_LASTUPDATEDTIME, lastUpdatedTime);
    }

    char* topic = GenerateTopic(PUB_UPDATE, device);
    return SendRequest(handle, topic, request);
}

int UpdateShadowWithBinary(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* key, const char* requestId, uint32_t version, const char* shadow, const char* lastUpdatedTime)
{
    JSON_Value* json_shadow = NULL;
    JSON_Value* json_lastUpdatedTime = NULL;

    if (NULL != shadow)
    {
        json_shadow = json_parse_string(shadow);
        if (NULL == json_shadow)
        {
            LogError("Failure: cannot parse the shadow binary.");
            return __FAILURE__;
        }
    }

    if (NULL != lastUpdatedTime)
    {
        json_lastUpdatedTime = json_parse_string(lastUpdatedTime);
        if (NULL == json_lastUpdatedTime)
        {
            json_value_free(json_shadow);
            LogError("Failure: cannot parse the lastUpdatedTime binary.");
            return __FAILURE__;
        }
    }

    int result = UpdateShadow(handle, device, key, requestId, version, json_shadow, json_lastUpdatedTime);

    json_value_free(json_shadow);
    if (NULL != json_lastUpdatedTime)
    {
        json_value_free(json_lastUpdatedTime);
    }

    return result;
}

int OnSubAckCallback(QOS_VALUE* qosReturn, size_t qosCount, void *context)
{
    for (int i = 0; i < qosCount; ++i)
    {
        if (qosReturn[i] == DELIVER_FAILURE)
        {
            LogError("Failed to subscribe");
            return 0;
        }
    }
    IOTDM_CLIENT_HANDLE handle = context;
    handle->subscribed = true;
    LogInfo("Subscribed topics");
    return 0;
}

static size_t CalTopicSize(IOTDM_CLIENT_HANDLE handle)
{
    return handle->enableOta ? SUB_TOPIC_SIZE + SUB_METHOD_TOPIC_SIZE : SUB_TOPIC_SIZE;
}

/* Send a Method request to cloud */
static int SendMethodReq(const IOTDM_CLIENT_HANDLE handle, const char *device, const char *methodName, JSON_Value *payload,
                  const char *requestId) {
    if (NULL == requestId)
    {
        LogError("Failure: request id should not be NULL.");
        if (payload != NULL) {
            json_value_free(payload);
        }
        return __FAILURE__;
    }

    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_REQUEST_ID, requestId);
    json_object_set_string(root, KEY_METHOD_NAME, methodName);
    if (payload != NULL) {
        json_object_set_value(root, KEY_PAYLOAD, payload);
    }
    char* topic = GenerateTopic(PUB_METHOD_CLOUD_REQ, device);
    char* s = json_serialize_to_string(request);
    if (s != NULL)
    {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Sending method request:\n%s\n%s", topic, s);
        json_free_serialized_string(s);
    }
    return SendRequest(handle, topic, request);
}

/* Send a Method response to cloud */
int SendMethodResp(const IOTDM_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT *msgContext, const char *methodName, JSON_Value *payload) {
    JSON_Value* response = json_value_init_object();
    JSON_Object* root = json_object(response);
    json_object_set_string(root, KEY_REQUEST_ID, msgContext->requestId);
    json_object_set_string(root, KEY_METHOD_NAME, methodName);
    if (payload != NULL) {
        json_object_set_value(root, KEY_PAYLOAD, payload);
    }
    char* topic = GenerateTopic(PUB_METHOD_DEVICE_RESP, msgContext->device);
    char* s = json_serialize_to_string(response);
    if (s != NULL)
    {
        LOG(AZ_LOG_TRACE, LOG_LINE, "Sending method response:\n%s\n%s", topic, s);
        json_free_serialized_string(s);
    }
    return SendRequest(handle, topic, response);
}

IOTDM_CLIENT_HANDLE iotdm_client_init(char* broker, char* name)
{
    if (NULL == broker || NULL == name)
    {
        LogError("Failure: parameters broker and name should not be NULL.");
        return NULL;
    }

    IOTDM_CLIENT_HANDLE handle = malloc(sizeof(IOTDM_CLIENT));
    if (NULL == handle)
    {
        LogError("Failure: init memory for iotdm client.");
        return NULL;
    }

    ResetIotDmClient(handle);
    handle->endpoint = GetBrokerEndpoint(broker, &(handle->mqttConnType));
    if (NULL == handle->endpoint)
    {
        LogError("Failure: get the endpoint from broker address.");
        free(handle);
        return NULL;
    }
    handle->name = name;

    return handle;
}

void iotdm_client_deinit(IOTDM_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        size_t topicSize = CalTopicSize(handle);
        char** topics = malloc(topicSize * sizeof(char*));
        if (topics == NULL)
        {
            LogError("Failure: failed to alloc");
            return;
        }
        int amount = GetSubscription(handle, topics, topicSize);
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
        }
        else if (amount > 0)
        {
            unsubscribe_mqtt_topics(handle->mqttClient, (const char**) topics, amount);
            ReleaseSubscription(topics, topicSize);
        }
        free(topics);

        iothub_mqtt_destroy(handle->mqttClient);
        if (NULL != handle->endpoint)
        {
            free(handle->endpoint);
        }

        ResetIotDmClient(handle);
        free(handle);
    }
}

int iotdm_client_connect(IOTDM_CLIENT_HANDLE handle, const IOTDM_CLIENT_OPTIONS *options)
{
    if (NULL == handle)
    {
        LogError("IOTDM_CLIENT_HANDLE handle should not be NULL.");
        return __FAILURE__;
    }
    if (NULL == options || NULL == options->username || NULL == options->password)
    {
        LogError("Failure: the username and password in options should not be NULL.");
        return __FAILURE__;
    }

    InitIotHubClient(handle, options);
    if (NULL == handle->mqttClient)
    {
        LogError("Failure: cannot initialize the mqtt connection.");
        return __FAILURE__;
    }

    handle->mqttClient->callbackContext = handle;
    do
    {
        iothub_mqtt_dowork(handle->mqttClient);
        ThreadAPI_Sleep(10);
    } while (MQTT_CLIENT_STATUS_CONNECTED != handle->mqttClient->mqttClientStatus && handle->mqttClient->isRecoverableError);

    handle->mqttClient->isDestroyCalled = false;
    handle->mqttClient->isDisconnectCalled = false;
    handle->enableOta = options->enableOta;
    return 0;
}

int iotdm_client_dowork(const IOTDM_CLIENT_HANDLE handle)
{
    if (handle->mqttClient->isDestroyCalled || handle->mqttClient->isDisconnectCalled)
    {
        return -1;
    }

    if (handle->mqttClient->isConnectionLost)
    {
        handle->subscribed = false;
        handle->subscribeSentTimestamp = 0;
    }
    if (handle->mqttClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED && !(handle->subscribed))
    {
        time_t current = time(NULL);
        double elipsed = difftime(current, handle->subscribeSentTimestamp);
        if (elipsed > 10) {
            size_t topicSize = CalTopicSize(handle);
            char** topics = malloc(topicSize * sizeof(char*));
            if (topics == NULL)
            {
                LogError("Failure: failed to alloc");
                return __FAILURE__;
            }

            int amount = GetSubscription(handle, topics, topicSize);
            if (amount < 0)
            {
                LogError("Failure: failed to get the subscribing topics.");
                free(topics);
                return __FAILURE__;
            }
            else if (amount > 0)
            {
                SUBSCRIBE_PAYLOAD* subscribe = malloc(topicSize * sizeof(SUBSCRIBE_PAYLOAD));
                for (size_t index = 0; index < (size_t)amount; ++index)
                {
                    subscribe[index].subscribeTopic = topics[index];
                    subscribe[index].qosReturn = DELIVER_AT_LEAST_ONCE;
                }
                int result = subscribe_mqtt_topics(handle->mqttClient, subscribe, amount, OnSubAckCallback, handle);
                if (result == 0)
                {
                    handle->subscribeSentTimestamp = time(NULL);
                }
                ReleaseSubscription(topics, topicSize);
                free(subscribe);
                free(topics);
                if (0 != result)
                {
                    LogError("Failure: failed to subscribe the topics.");
                    return __FAILURE__;
                }
            }
        }
    }

    iothub_mqtt_dowork(handle->mqttClient);

    return 0;
}

void iotdm_client_register_delta(IOTDM_CLIENT_HANDLE handle, SHADOW_DELTA_CALLBACK callback, void* callbackContext)
{
    handle->callback.delta = callback;
    handle->context.delta = callbackContext;
}

void iotdm_client_register_get_accepted(IOTDM_CLIENT_HANDLE handle, SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    handle->callback.getAccepted = callback;
    handle->context.getAccepted = callbackContext;
}

void iotdm_client_register_get_rejected(IOTDM_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.getRejected = callback;
    handle->context.getRejected = callbackContext;
}

void iotdm_client_register_update_accepted(IOTDM_CLIENT_HANDLE handle, SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateAccepted = callback;
    handle->context.updateAccepted = callbackContext;
}

void iotdm_client_register_update_rejected(IOTDM_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateRejected = callback;
    handle->context.updateRejected = callbackContext;
}

void iotdm_client_register_update_documents(IOTDM_CLIENT_HANDLE handle, SHADOW_DOCUMENTS_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateDocuments = callback;
    handle->context.updateDocuments = callbackContext;
}

void iotdm_client_register_update_snapshot(IOTDM_CLIENT_HANDLE handle, SHADOW_SNAPSHOT_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateSnapshot = callback;
    handle->context.updateSnapshot = callbackContext;
}

void iotdm_client_register_delete_accepted(IOTDM_CLIENT_HANDLE handle, SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    handle->callback.deleteAccepted = callback;
    handle->context.deleteAccepted = callbackContext;
}

void iotdm_client_register_delete_rejected(IOTDM_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.deleteRejected = callback;
    handle->context.deleteRejected = callbackContext;
}

int iotdm_client_get_shadow(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId)
{
    if (NULL == requestId)
    {
        LogError("Failure: request id should not be NULL.");
        return __FAILURE__;
    }

    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_REQUEST_ID, requestId);
    char* topic = GenerateTopic(PUB_GET, device);
    return SendRequest(handle, topic, request);
}

int iotdm_client_delete_shadow(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId)
{
    if (NULL == requestId)
    {
        LogError("Failure: request id should not be NULL.");
        return __FAILURE__;
    }

    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_REQUEST_ID, requestId);
    char* topic = GenerateTopic(PUB_DELETE, device);
    return SendRequest(handle, topic, request);
}

int iotdm_client_update_desired(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* desired, JSON_Value* lastUpdatedTime)
{
    return UpdateShadow(handle, device, KEY_DESIRED, requestId, version, desired, lastUpdatedTime);
}

int iotdm_client_update_shadow(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* reported, JSON_Value* lastUpdatedTime)
{
    return UpdateShadow(handle, device, KEY_REPORTED, requestId, version, reported, lastUpdatedTime);
}

int iotdm_client_update_desired_with_binary(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* desired, const char* lastUpdatedTime)
{
    return UpdateShadowWithBinary(handle, device, KEY_DESIRED, requestId, version, desired, lastUpdatedTime);
}

int iotdm_client_update_shadow_with_binary(const IOTDM_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* reported, const char* lastUpdatedTime)
{
    return UpdateShadowWithBinary(handle, device, KEY_REPORTED, requestId, version, reported, lastUpdatedTime);
}


void iotdm_client_ota_register_job(const IOTDM_CLIENT_HANDLE handle, SHADOW_OTA_JOB_CALLBACK callback,
                                           void *callbackContext)
{
    handle->callback.otaJob = callback;
    handle->context.otaJob = callbackContext;
}

void iotdm_client_ota_register_report_start(const IOTDM_CLIENT_HANDLE handle, SHADOW_OTA_REPORT_RESULT_CALLBACK callback, void* callbackContext)
{
    handle->callback.otaReportStart = callback;
    handle->context.otaReportStart = callbackContext;
}

void iotdm_client_ota_register_report_result(const IOTDM_CLIENT_HANDLE handle, SHADOW_OTA_REPORT_RESULT_CALLBACK callback, void* callbackContext)
{
    handle->callback.otaReportResult = callback;
    handle->context.otaReportResult = callbackContext;
}

int iotdm_client_ota_get_job(const IOTDM_CLIENT_HANDLE handle, const char *firmwareVersion,
                                     const char *requestId)
{
    JSON_Value* request = NULL;

    if (firmwareVersion != NULL)
    {
        request = json_value_init_object();
        JSON_Object* root = json_object(request);
        json_object_set_string(root, KEY_FIRMWARE_VERSION, firmwareVersion);
    }
    return SendMethodReq(handle, handle->name, METHOD_GET_FIRMWARE, request, requestId);
}

int iotdm_client_ota_report_start(const IOTDM_CLIENT_HANDLE handle, const char* jobId, const char* requestId)
{
    if (NULL == jobId) {
        LogError("Failure: jobId should not be NULL");
        return __FAILURE__;
    }
    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_JOB_ID, jobId);
    return SendMethodReq(handle, handle->name, METHOD_REPORT_FIRMWARE_UPDATE_START, request, requestId);
}

int iotdm_client_ota_report_result(const IOTDM_CLIENT_HANDLE handle, const char* jobId, bool isSuccess, const char* requestId)
{
    if (NULL == jobId) {
        LogError("Failure: jobId should not be NULL");
        return __FAILURE__;
    }
    JSON_Value* request = json_value_init_object();
    JSON_Object* root = json_object(request);
    json_object_set_string(root, KEY_RESULT, isSuccess ? VALUE_RESULT_SUCCESS : VALUE_RESULT_FAILURE);
    json_object_set_string(root, KEY_JOB_ID, jobId);
    return SendMethodReq(handle, handle->name, METHOD_REPORT_FIRMWARE_UPDATE_RESULT, request, requestId);
}
