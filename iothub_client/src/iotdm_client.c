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
#include "iotdm_client.h"
#include "iothub_mqtt_client.h"

#define     SUB_TOPIC_SIZE                  3

#define     SLASH                           '/'

#define     TOPIC_PREFIX                    "$baidu/iot/shadow/"

#define     TOPIC_SUFFIX_DELTA              "delta"
#define     TOPIC_SUFFIX_GET_REJECTED       "get/rejected"
#define     TOPIC_SUFFIX_UPDATE_REJECTED    "update/rejected"

#define     PUB_GET                         "$baidu/iot/shadow/%s/get"
#define     PUB_UPDATE                      "$baidu/iot/shadow/%s/update"

#define     SUB_DELTA                       "$baidu/iot/shadow/%s/delta"
#define     SUB_GET_REJECTED                "$baidu/iot/shadow/%s/get/rejected"
#define     SUB_UPDATE_REJECTED             "$baidu/iot/shadow/%s/update/rejected"

#define     KEY_CODE                        "code"
#define     KEY_DESIRED                     "desired"
#define     KEY_LASTUPDATEDTIME             "lastUpdatedTime"
#define     KEY_MESSAGE                     "message"
#define     KEY_REPORTED                    "reported"
#define     KEY_REQUEST_ID                  "requestId"
#define     KEY_VERSION                     "profileVersion"

typedef struct SHADOW_CALLBACK_TAG
{
    SHADOW_DELTA_CALLBACK delta;
    SHADOW_ERROR_CALLBACK getRejected;
    SHADOW_ERROR_CALLBACK updateRejected;
} SHADOW_CALLBACK;

typedef struct SHADOW_CALLBACK_CONTEXT_TAG
{
    void* delta;
    void* getRejected;
    void* updateRejected;
} SHADOW_CALLBACK_CONTEXT;

typedef struct IOTDM_CLIENT_TAG
{
    bool subscribed;
    char* endpoint;
    char* name;
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;
    MQTT_CONNECTION_TYPE mqttConnType;
    SHADOW_CALLBACK callback;
    SHADOW_CALLBACK_CONTEXT context;
} IOTDM_CLIENT;

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
        LogError("Failue: both formate and device should not be NULL.");
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

    // TODO: support more topics for subscription handle.
    if (true == StringCmp(TOPIC_SUFFIX_DELTA, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_DELTA;
    }
    else if (true == StringCmp(TOPIC_SUFFIX_GET_REJECTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_GET_REJECTED;
    }
    else if (true == StringCmp(TOPIC_SUFFIX_UPDATE_REJECTED, topic, end + 1, StringLength(topic) + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_REJECTED;
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
            LogError("Feailure: failed to generate the sub topic 'delta'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.getRejected)
    {
        subscribe[index] = GenerateTopic(SUB_GET_REJECTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Feailure: failed to generate the sub topic 'get/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateRejected)
    {
        subscribe[index] = GenerateTopic(SUB_UPDATE_REJECTED, handle->name);
        if (NULL == subscribe[index++])
        {
            LogError("Feailure: failed to generate the sub topic 'update/rejected'.");
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

static void OnRecvCallbackForError(const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root, const SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    SHADOW_ERROR error;
    error.code = (int)json_object_get_number(root, KEY_CODE);
    error.message = json_object_get_string(root, KEY_MESSAGE);

    (*callback)(msgContext, &error, callbackContext);
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
        LogError("Failue: cannot get the device name from the subscribed topic.");
        return;
    }

    IOTHUB_MQTT_CLIENT_HANDLE mqttClient = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    IOTDM_CLIENT_HANDLE handle = (IOTDM_CLIENT_HANDLE)mqttClient->callbackContext;
    JSON_Value* data = json_parse_string((char*) payload->message);
    if (NULL == data)
    {
        LogError("Failure: failed to deserialize the payload to json.");
        free(msgContext.device);
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

            case SHADOW_CALLBACK_TYPE_GET_REJECTED:
                OnRecvCallbackForError(&msgContext, root, handle->callback.getRejected, handle->context.getRejected);
                break;

            case SHADOW_CALLBACK_TYPE_UPDATE_REJECTED:
                OnRecvCallbackForError(&msgContext, root, handle->callback.updateRejected, handle->context.updateRejected);
                break;

            default:
                LogError("Failure: the shadow callback type is not supported.");
        }
    }

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

        handle->endpoint = NULL;
        handle->name = NULL;
        handle->mqttClient = NULL;

        handle->callback.delta = NULL;
        handle->callback.getRejected = NULL;
        handle->callback.updateRejected = NULL;

        handle->context.delta = NULL;
        handle->context.getRejected = NULL;
        handle->context.updateRejected = NULL;
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
            result = publish_mqtt_message(handle->mqttClient, topic, DELIVER_AT_LEAST_ONCE, (uint8_t*)encoded, StringLength(encoded), NULL, NULL);
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
        char* topics[SUB_TOPIC_SIZE];
        int amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE);
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
        }
        else if (amount > 0)
        {
            unsubscribe_mqtt_topics(handle->mqttClient, (const char**) topics, amount);
            ReleaseSubscription(topics, SUB_TOPIC_SIZE);
        }

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
    } while (MQTT_CLIENT_STATUS_CONNECTED != handle->mqttClient->mqttClientStatus && handle->mqttClient->isRecoverableError);

    handle->mqttClient->isDestroyCalled = false;
    handle->mqttClient->isDisconnectCalled = false;

    return 0;
}

int iotdm_client_dowork(const IOTDM_CLIENT_HANDLE handle)
{
    if (handle->mqttClient->isDestroyCalled || handle->mqttClient->isDisconnectCalled)
    {
        return -1;
    }

    handle->subscribed = !(handle->mqttClient->isConnectionLost) && handle->subscribed;
    if (MQTT_CLIENT_STATUS_CONNECTED && !(handle->subscribed))
    {
        char* topics[SUB_TOPIC_SIZE];
        int amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE);
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
            return __FAILURE__;
        }
        else if (amount > 0)
        {
            SUBSCRIBE_PAYLOAD subscribe[SUB_TOPIC_SIZE];
            for (size_t index = 0; index < (size_t)amount; ++index)
            {
                subscribe[index].subscribeTopic = topics[index];
                subscribe[index].qosReturn = DELIVER_AT_LEAST_ONCE;
            }
            int result = subscribe_mqtt_topics(handle->mqttClient, subscribe, amount);
            ReleaseSubscription(topics, SUB_TOPIC_SIZE);
            if (0 != result)
            {
                LogError("Failure: failed to subscribe the topics.");
                return __FAILURE__;
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

void iotdm_client_register_get_rejected(IOTDM_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.getRejected = callback;
    handle->context.getRejected = callbackContext;
}

void iotdm_client_register_update_rejected(IOTDM_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateRejected = callback;
    handle->context.updateRejected = callbackContext;
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