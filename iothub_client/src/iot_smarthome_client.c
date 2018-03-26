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
#include <azure_c_shared_utility/strings.h>
#include "iot_smarthome_client.h"
#include "iothub_mqtt_client.h"

#define     SUB_TOPIC_SIZE                  7

#define     SLASH                           '/'

#define     TOPIC_PREFIX                    "$baidu/iot/shadow/"

#define     TOPIC_SUFFIX_DELTA              "delta"
#define     TOPIC_SUFFIX_GET_ACCEPTED       "get/accepted"
#define     TOPIC_SUFFIX_GET_REJECTED       "get/rejected"
#define     TOPIC_SUFFIX_UPDATE_ACCETPED    "update/accepted"
#define     TOPIC_SUFFIX_UPDATE_REJECTED    "update/rejected"
#define     TOPIC_SUFFIX_UPDATE_DOCUMENTS   "update/documents"
#define     TOPIC_SUFFIX_UPDATE_SNAPSHOT    "update/snapshot"

#define     PUB_GET                         "$baidu/iot/shadow/%s/get"
#define     PUB_UPDATE                      "$baidu/iot/shadow/%s/update"
#define     GATEWAY_SUBDEVICE_PUB_OBJECT    "%s/subdevice/%s"

#define     SUB_DELTA                       "$baidu/iot/shadow/%s/delta"
#define     SUB_GET_ACCEPTED                "$baidu/iot/shadow/%s/get/accepted"
#define     SUB_GET_REJECTED                "$baidu/iot/shadow/%s/get/rejected"
#define     SUB_UPDATE_ACCEPTED             "$baidu/iot/shadow/%s/update/accepted"
#define     SUB_UPDATE_REJECTED             "$baidu/iot/shadow/%s/update/rejected"
#define     SUB_UPDATE_DOCUMENTS            "$baidu/iot/shadow/%s/update/documents"
#define     SUB_UPDATE_SNAPSHOT             "$baidu/iot/shadow/%s/update/snapshot"
#define     SUB_GATEWAY_WILDCARD            "%s/subdevice/+"
#define     KEY_CODE                        "code"
#define     KEY_DESIRED                     "desired"
#define     KEY_LASTUPDATEDTIME             "lastUpdatedTime"
#define     KEY_MESSAGE                     "message"
#define     KEY_REPORTED                    "reported"
#define     KEY_REQUEST_ID                  "requestId"
#define     KEY_VERSION                     "profileVersion"
#define     KEY_CURRENT                     "current"
#define     KEY_PREVIOUS                    "previous"

#define     ENDPOINT                        "baidu-smarthome.mqtt.iot.gz.baidubce.com"

typedef struct SHADOW_CALLBACK_TAG
{
    SHADOW_DELTA_CALLBACK delta;
    SHADOW_ACCEPTED_CALLBACK getAccepted;
    SHADOW_ERROR_CALLBACK getRejected;
    SHADOW_ERROR_CALLBACK updateRejected;
    SHADOW_ACCEPTED_CALLBACK updateAccepted;
    SHADOW_DOCUMENTS_CALLBACK updateDocuments;
    SHADOW_SNAPSHOT_CALLBACK updateSnapshot;
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
} SHADOW_CALLBACK_CONTEXT;

typedef struct IOT_SH_CLIENT_TAG
{
    bool subscribed;
    char* endpoint;
    char* name;
    IOTHUB_MQTT_CLIENT_HANDLE mqttClient;
    MQTT_CONNECTION_TYPE mqttConnType;
    SHADOW_CALLBACK callback;
    SHADOW_CALLBACK_CONTEXT context;
    bool isGateway;
} IOT_SH_CLIENT;

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

static const char* GenerateGatewaySubdevicePubObject(const char* gateway, const char* subdevice) {
    STRING_HANDLE stringHandle = STRING_construct_sprintf(GATEWAY_SUBDEVICE_PUB_OBJECT, gateway, subdevice);
    if (stringHandle == NULL) {
        LogError("fail to generate $gateway/subdevice/$device string");
    }
    return STRING_c_str(stringHandle);
}

/**
 * if topic is $prefix/$device/$suffix, then message->device = $device
 * if topic is $prefix/$gateway/subdevice/$subdevice/$suffix, then message->device = $gatewat, message->subdevice = $subdevice
 */
static int GetDeviceFromTopic(const char* topic, IOT_SH_CLIENT_HANDLE handle, SHADOW_CALLBACK_TYPE* type, SHADOW_MESSAGE_CONTEXT* messageContext)
{
    char* prefix = TOPIC_PREFIX;

    size_t prefixLength = StringLength(prefix);
    if (false == StringCmp(prefix, topic, 0, prefixLength))
    {
        LogError("Failure: the topic prefix is illegal.");
        return -1;
    }
    size_t topicLength = StringLength(topic);

    size_t head = prefixLength;
    size_t end, tmp;

    // TODO: support more topics for subscription handle.
    if (StringCmp(TOPIC_SUFFIX_DELTA, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_DELTA), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_DELTA;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_GET_ACCEPTED, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_GET_ACCEPTED), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_GET_ACCEPTED;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_GET_REJECTED, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_GET_REJECTED), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_GET_REJECTED;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_ACCETPED, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_UPDATE_ACCETPED), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_ACCEPTED;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_REJECTED, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_UPDATE_REJECTED), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_REJECTED;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_DOCUMENTS, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_UPDATE_DOCUMENTS), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_DOCUMENTS;
        end = tmp - 1;
    }
    else if (StringCmp(TOPIC_SUFFIX_UPDATE_SNAPSHOT, topic, tmp = topicLength - StringLength(TOPIC_SUFFIX_UPDATE_SNAPSHOT), topicLength + 1))
    {
        *type = SHADOW_CALLBACK_TYPE_UPDATE_SNAPSHOT;
        end = tmp - 1;
    }
    else
    {
        LogError("Failure: the subscribe handle is not supported.");
        return -1;
    }

    char* device = malloc(sizeof(char) * (end - head + 1));
    if (NULL == device)
    {
        LogError("Failure: failed to allocate memory for device.");
        return -1;
    }

    device[end - head] = '\0';
    StringCpy(device, topic, head, end);

    if (strlen(device) == strlen(handle->name) && StringCmp(device, handle->name, 0, strlen(device))) {
        messageContext->device = device;
    }
    else {
        // device should be in format of "$gateway/subdevice/%s"
        size_t offset = strlen(handle->name) + strlen("/subdevice/");

        if (!StringCmp(device, handle->name, 0, strlen(handle->name)) || strlen(device) <= offset) {
            LogError("Invalid SUB topic %s", topic);
            return -1;
        }

        STRING_HANDLE stringHandle = STRING_new();
        STRING_copy_n(stringHandle, device + offset, strlen(device) - offset);
        messageContext->device = handle->name;
        messageContext->subdevice = (char *) STRING_c_str(stringHandle);
    }
    return 0;
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


static int GetSubscription(IOT_SH_CLIENT_HANDLE handle, char** subscribe, size_t length, size_t startIndex, const char* subObject)
{
    if (NULL == subscribe) return 0;

    size_t index = startIndex;
    if (startIndex == 0) {
        ResetSubscription(subscribe, length);
    }

    if (NULL != handle->callback.delta) {
        subscribe[index] = GenerateTopic(SUB_DELTA, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'delta'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.getAccepted) {
        subscribe[index] = GenerateTopic(SUB_GET_ACCEPTED, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'get/accepted'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.getRejected) {
        subscribe[index] = GenerateTopic(SUB_GET_REJECTED, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'get/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateAccepted) {
        subscribe[index] = GenerateTopic(SUB_UPDATE_ACCEPTED, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'update/accepted'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateRejected) {
        subscribe[index] = GenerateTopic(SUB_UPDATE_REJECTED, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'update/rejected'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateDocuments) {
        subscribe[index] = GenerateTopic(SUB_UPDATE_DOCUMENTS, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'update/documents'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }
    if (NULL != handle->callback.updateSnapshot) {
        subscribe[index] = GenerateTopic(SUB_UPDATE_SNAPSHOT, subObject);
        if (NULL == subscribe[index++]) {
            LogError("Failure: failed to generate the sub topic 'update/snapshot'.");
            ReleaseSubscription(subscribe, length);
            return -1;
        }
    }

    return index;
}

static void OnRecvCallbackForDelta(const IOT_SH_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
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

static void OnRecvCallbackForDocuments(const IOT_SH_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
{
    SHADOW_DOCUMENTS shadow_documents;
    shadow_documents.profileVersion = (int)json_object_get_number(root, KEY_VERSION);
    shadow_documents.current = json_object_get_object(root, KEY_CURRENT);
    shadow_documents.previous = json_object_get_object(root, KEY_PREVIOUS);

    (*(handle->callback.updateDocuments))(msgContext, &shadow_documents, handle->context.updateDocuments);
}

static void OnRecvCallbackForSnapshot(const IOT_SH_CLIENT_HANDLE handle, const SHADOW_MESSAGE_CONTEXT* msgContext, const JSON_Object* root)
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

static void OnRecvCallback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const char* topic = mqttmessage_getTopicName(msgHandle);
    const APP_PAYLOAD* payload = mqttmessage_getApplicationMsg(msgHandle);
    if (NULL == topic || NULL == payload)
    {
        LogError("Failure: cannot find topic or payload in the message received.");
        return;
    }

    IOTHUB_MQTT_CLIENT_HANDLE mqttClient = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    IOT_SH_CLIENT_HANDLE handle = (IOT_SH_CLIENT_HANDLE)mqttClient->callbackContext;

    SHADOW_CALLBACK_TYPE type;
    SHADOW_MESSAGE_CONTEXT msgContext;
    GetDeviceFromTopic(topic, handle, &type, &msgContext);
    if (NULL == msgContext.device)
    {
        LogError("Failure: cannot get the device name from the subscribed topic.");
        return;
    }

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

            default:
                LogError("Failure: the shadow callback type is not supported.");
        }
    }

    free(msgContext.device);
    json_value_free(data);
}

static void InitIotHubClient(IOT_SH_CLIENT_HANDLE handle, const IOT_SH_CLIENT_OPTIONS* options) {
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

static void ResetIotDmClient(IOT_SH_CLIENT_HANDLE handle)
{
    if (NULL != handle)
    {
        handle->subscribed = false;

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

        handle->context.delta = NULL;
        handle->context.getAccepted = NULL;
        handle->context.getRejected = NULL;
        handle->context.updateAccepted = NULL;
        handle->context.updateRejected = NULL;
        handle->context.updateSnapshot = NULL;
        handle->context.updateDocuments = NULL;
    }
}

static int SendRequest(const IOT_SH_CLIENT_HANDLE handle, char* topic, JSON_Value* request)
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

int UpdateShadow(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* key, const char* requestId, uint32_t version, JSON_Value* shadow, JSON_Value* lastUpdatedTime)
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

int UpdateShadowWithBinary(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* key, const char* requestId, uint32_t version, const char* shadow, const char* lastUpdatedTime)
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

IOT_SH_CLIENT_HANDLE iot_smarthome_client_init(bool isGatewayDevice)
{
    IOT_SH_CLIENT_HANDLE handle = malloc(sizeof(IOT_SH_CLIENT));
    if (NULL == handle)
    {
        LogError("Failure: init memory for iot_smarthome client.");
        return NULL;
    }

    ResetIotDmClient(handle);

    handle->endpoint = ENDPOINT;

    handle->isGateway = isGatewayDevice;

    handle->mqttConnType = MQTT_CONNECTION_MUTUAL_TLS;

    return handle;
}

void iot_smarthome_client_deinit(IOT_SH_CLIENT_HANDLE handle)
{
    if (NULL != handle) {
        size_t topicSize = handle->isGateway == true ? SUB_TOPIC_SIZE * 2 : SUB_TOPIC_SIZE;
        char * topics[topicSize];


        int amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE, 0, handle->name);

        if (handle->isGateway == true) {
            const char* gatewayOnBehalfOfSubdevices = GenerateTopic(SUB_GATEWAY_WILDCARD, handle->name);
            amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE, amount, gatewayOnBehalfOfSubdevices);
        }

        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
        }
        else if (amount > 0)
        {
            unsubscribe_mqtt_topics(handle->mqttClient, (const char**) topics, amount);
            ReleaseSubscription(topics, topicSize);
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

int iot_smarthome_client_connect(IOT_SH_CLIENT_HANDLE handle, const IOT_SH_CLIENT_OPTIONS *options)
{
    if (NULL == handle)
    {
        LogError("IOT_SH_CLIENT_HANDLE handle should not be NULL.");
        return __FAILURE__;
    }
    if (NULL == options || NULL == options->username || NULL == options->password)
    {
        LogError("Failure: the username and password in options should not be NULL.");
        return __FAILURE__;
    }

    handle->name = options->clientId;

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

int iot_smarthome_client_dowork(const IOT_SH_CLIENT_HANDLE handle)
{
    if (handle->mqttClient->isDestroyCalled || handle->mqttClient->isDisconnectCalled)
    {
        return -1;
    }

    handle->subscribed = !(handle->mqttClient->isConnectionLost) && handle->subscribed;
    if (handle->mqttClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED && !(handle->subscribed))
    {
        size_t topicSize = handle->isGateway == true ? SUB_TOPIC_SIZE * 2 : SUB_TOPIC_SIZE;
        char* topics[topicSize];
        int amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE, 0, handle->name);
        if (handle->isGateway == true) {
            amount = GetSubscription(handle, topics, SUB_TOPIC_SIZE, amount, GenerateTopic(SUB_GATEWAY_WILDCARD, handle->name));
        }
        if (amount < 0)
        {
            LogError("Failure: failed to get the subscribing topics.");
            return __FAILURE__;
        }
        else if (amount > 0)
        {
            SUBSCRIBE_PAYLOAD subscribe[topicSize];
            for (size_t index = 0; index < (size_t)amount; ++index)
            {
                subscribe[index].subscribeTopic = topics[index];
                subscribe[index].qosReturn = DELIVER_AT_LEAST_ONCE;
            }
            int result = subscribe_mqtt_topics(handle->mqttClient, subscribe, amount);
            ReleaseSubscription(topics, topicSize);
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

void iot_smarthome_client_register_delta(IOT_SH_CLIENT_HANDLE handle, SHADOW_DELTA_CALLBACK callback, void* callbackContext)
{
    handle->callback.delta = callback;
    handle->context.delta = callbackContext;
}

void iot_smarthome_client_register_get_accepted(IOT_SH_CLIENT_HANDLE handle, SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    handle->callback.getAccepted = callback;
    handle->context.getAccepted = callbackContext;
}

void iot_smarthome_client_register_get_rejected(IOT_SH_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.getRejected = callback;
    handle->context.getRejected = callbackContext;
}

void iot_smarthome_client_register_update_accepted(IOT_SH_CLIENT_HANDLE handle, SHADOW_ACCEPTED_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateAccepted = callback;
    handle->context.updateAccepted = callbackContext;
}

void iot_smarthome_client_register_update_rejected(IOT_SH_CLIENT_HANDLE handle, SHADOW_ERROR_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateRejected = callback;
    handle->context.updateRejected = callbackContext;
}

void iot_smarthome_client_register_update_documents(IOT_SH_CLIENT_HANDLE handle, SHADOW_DOCUMENTS_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateDocuments = callback;
    handle->context.updateDocuments = callbackContext;
}

void iot_smarthome_client_register_update_snapshot(IOT_SH_CLIENT_HANDLE handle, SHADOW_SNAPSHOT_CALLBACK callback, void* callbackContext)
{
    handle->callback.updateSnapshot = callback;
    handle->context.updateSnapshot = callbackContext;
}

int iot_smarthome_client_get_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId)
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

int iot_smarthome_client_get_subdevice_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId)
{
    return iot_smarthome_client_get_shadow(handle, GenerateGatewaySubdevicePubObject(gateway, subdevice), requestId);
}

int iot_smarthome_client_update_desired(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* desired, JSON_Value* lastUpdatedTime)
{
    return UpdateShadow(handle, device, KEY_DESIRED, requestId, version, desired, lastUpdatedTime);
}

int iot_smarthome_client_update_subdevice_desired(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, JSON_Value* desired, JSON_Value* lastUpdatedTime)
{
    return iot_smarthome_client_update_desired(handle, GenerateGatewaySubdevicePubObject(gateway, subdevice), requestId, version, desired, lastUpdatedTime);
}

int iot_smarthome_client_update_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, JSON_Value* reported, JSON_Value* lastUpdatedTime)
{
    return UpdateShadow(handle, device, KEY_REPORTED, requestId, version, reported, lastUpdatedTime);
}

int iot_smarthome_client_update_subdevice_shadow(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, JSON_Value* reported, JSON_Value* lastUpdatedTime)
{
    return iot_smarthome_client_update_shadow(handle, GenerateGatewaySubdevicePubObject(gateway, subdevice), requestId, version, reported, lastUpdatedTime);
}

int iot_smarthome_client_update_desired_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* desired, const char* lastUpdatedTime)
{
    return UpdateShadowWithBinary(handle, device, KEY_DESIRED, requestId, version, desired, lastUpdatedTime);
}

int iot_smarthome_client_update_subdevice_desired_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, const char* desired, const char* lastUpdatedTime)
{
    return iot_smarthome_client_update_desired_with_binary(handle, GenerateGatewaySubdevicePubObject(gateway, subdevice), requestId, version, desired, lastUpdatedTime);
}

int iot_smarthome_client_update_shadow_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* device, const char* requestId, uint32_t version, const char* reported, const char* lastUpdatedTime)
{
    return UpdateShadowWithBinary(handle, device, KEY_REPORTED, requestId, version, reported, lastUpdatedTime);
}

int iot_smarthome_client_update_subdevice_shadow_with_binary(const IOT_SH_CLIENT_HANDLE handle, const char* gateway, const char* subdevice, const char* requestId, uint32_t version, const char* reported, const char* lastUpdatedTime)
{
    return iot_smarthome_client_update_shadow_with_binary(handle, GenerateGatewaySubdevicePubObject(gateway, subdevice), requestId, version, reported, lastUpdatedTime);
}