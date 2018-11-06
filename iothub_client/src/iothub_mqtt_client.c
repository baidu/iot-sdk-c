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

#include "iothub_mqtt_client.h"
#include <azure_c_shared_utility/tlsio.h>
#include <azure_c_shared_utility/threadapi.h>
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include "certs.h"

#define EPOCH_TIME_T_VALUE          0
#define EPOCH_TIME_T_VALUE          0
#define PERSIST_NAME_SUFFIX         "PUB"
#define ERROR_TIME_FOR_RETRY_SECS   5       // We won't retry more than once every 5 seconds

#define MQTT_CONNECTION_TCP_PORT 1883
#define MQTT_CONNECTION_TLS_PORT 1884

struct RETRY_LOGIC_TAG
{
    IOTHUB_CLIENT_RETRY_POLICY retryPolicy; // Type of policy we're using
    size_t retryTimeoutLimitInSeconds;      // If we don't connect in this many seconds, give up even trying.
    RETRY_POLICY fnRetryPolicy;             // Pointer to the policy function
    time_t start;                           // When did we start retrying?
    time_t lastConnect;                     // When did we last try to connect?
    bool retryStarted;                      // true if the retry timer is set and we're trying to retry
    bool retryExpired;                      // true if we haven't connected in retryTimeoutLimitInSeconds seconds
    bool firstAttempt;                      // true on init so we can connect the first time without waiting for any timeouts.
    size_t retrycount;                      // How many times have we tried connecting?
    size_t delayFromLastConnectToRetry;     // last time delta betweewn retry attempts.
};

typedef struct MQTT_PUB_CALLBACK_INFO_TAG
{
    uint16_t packetId;
    PUB_CALLBACK pubCallback;
    MQTT_MESSAGE_HANDLE msgHandle;
    void *context;
    DLIST_ENTRY entry;
} MQTT_PUB_CALLBACK_INFO,* PMQTT_PUB_CALLBACK_INFO;

typedef struct MQTT_SUB_CALLBACK_INFO_TAG
{
    uint16_t packetId;
    SUB_CALLBACK subCallback;
    void *context;
    DLIST_ENTRY entry;
} MQTT_SUB_CALLBACK_INFO,* PMQTT_SUB_CALLBACK_INFO;

static int RetryPolicy_Exponential_BackOff_With_Jitter(bool *permit, size_t* delay, void* retryContextCallback)
{
    int result;

    if (retryContextCallback != NULL && permit != NULL && delay != NULL)
    {
        RETRY_LOGIC *retryLogic = (RETRY_LOGIC*)retryContextCallback;
        int numOfFailures = (int) retryLogic->retrycount;
        *permit = true;

        /*Intentionally not evaluating fraction of a second as it woudn't work on all platforms*/

        /* Exponential backoff with jitter
            1. delay = (pow(2, attempt) - 1 ) / 2
            2. delay with jitter = delay + rand_between(0, delay/2)
        */

        size_t halfDelta = ((1 << (size_t)numOfFailures) - 1) / 4;
        if (halfDelta > 0)
        {
            *delay = halfDelta + (rand() % (int)halfDelta);
        }
        else
        {
            *delay = halfDelta;
        }

        result = 0;
    }
    else
    {
        result = __FAILURE__;
    }
    return result;
}

static RETRY_LOGIC* CreateRetryLogic(IOTHUB_CLIENT_RETRY_POLICY retryPolicy, size_t retryTimeoutLimitInSeconds)
{
    RETRY_LOGIC *retryLogic;
    retryLogic = (RETRY_LOGIC*)malloc(sizeof(RETRY_LOGIC));
    if (retryLogic != NULL)
    {
        retryLogic->retryPolicy = retryPolicy;
        retryLogic->retryTimeoutLimitInSeconds = retryTimeoutLimitInSeconds;
        switch (retryLogic->retryPolicy)
        {
            case IOTHUB_CLIENT_RETRY_NONE:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_IMMEDIATE:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_INTERVAL:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_LINEAR_BACKOFF:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_RANDOM:
                // LogError("Not implemented chosing default");
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
            case IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER:
            default:
                retryLogic->fnRetryPolicy = &RetryPolicy_Exponential_BackOff_With_Jitter;
                break;
        }
        retryLogic->start = EPOCH_TIME_T_VALUE;
        retryLogic->delayFromLastConnectToRetry = 0;
        retryLogic->lastConnect = EPOCH_TIME_T_VALUE;
        retryLogic->retryStarted = false;
        retryLogic->retryExpired = false;
        retryLogic->firstAttempt = true;
        retryLogic->retrycount = 0;
    }
    else
    {
        LogError("Init retry logic failed");
    }
    return retryLogic;
}

static void DestroyRetryLogic(RETRY_LOGIC *retryLogic)
{
    if (retryLogic != NULL)
    {
        free(retryLogic);
        retryLogic = NULL;
    }
}

// Called when first attempted to re-connect
static void StartRetryTimer(RETRY_LOGIC *retryLogic)
{
    if (retryLogic != NULL)
    {
        if (retryLogic->retryStarted == false)
        {
            retryLogic->start = get_time(NULL);
            retryLogic->delayFromLastConnectToRetry = 0;
            retryLogic->retryStarted = true;
        }

        retryLogic->firstAttempt = false;
    }
    else
    {
        LogError("Retry Logic parameter. NULL.");
    }
}

// Called when connected
static void StopRetryTimer(RETRY_LOGIC *retryLogic)
{
    if (retryLogic != NULL)
    {
        if (retryLogic->retryStarted == true)
        {
            retryLogic->retryStarted = false;
            retryLogic->delayFromLastConnectToRetry = 0;
            retryLogic->lastConnect = EPOCH_TIME_T_VALUE;
            retryLogic->retrycount = 0;
        }
        else
        {
            LogError("Start retry logic before stopping");
        }
    }
    else
    {
        LogError("Retry Logic parameter. NULL.");
    }
}

// Called for every do_work when connection is broken
static bool CanRetry(RETRY_LOGIC *retryLogic)
{
    bool result;
    time_t now = get_time(NULL);

    if (retryLogic == NULL)
    {
        LogError("Retry Logic is not created, retrying forever");
        result = true;
    }
    else if (now < 0 || retryLogic->start < 0)
    {
        LogError("Time could not be retrieved, retrying forever");
        result = true;
    }
    else if (retryLogic->retryExpired)
    {
        // We've given up trying to retry.  Don't do anything.
        result = false;
    }
    else if (retryLogic->firstAttempt)
    {
        // This is the first time ever running through this code.  We need to try connecting no matter what.
        StartRetryTimer(retryLogic);
        retryLogic->lastConnect = now;
        retryLogic->retrycount++;
        result = true;
    }
    else
    {
        // Are we trying to retry?
        if (retryLogic->retryStarted)
        {
            // How long since we last tried to connect?  Store this in difftime.
            double diffTime = get_difftime(now, retryLogic->lastConnect);

            // Has it been less than 5 seconds since we tried last?  If so, we have to
            // be careful so we don't hit the server too quickly.
            if (diffTime <= ERROR_TIME_FOR_RETRY_SECS)
            {
                // As do_work can be called within as little as 1 ms, wait to avoid throtling server
                result = false;
            }
            else if (diffTime < retryLogic->delayFromLastConnectToRetry)
            {
                // delayFromLastConnectionToRetry is either 0 (the first time around)
                // or it's the backoff delta from the last time through the loop.
                // If we're less than that, don't even bother trying.  It's
                // Too early to retry
                result = false;
            }
            else
            {
                // last retry time evaluated have crossed, determine when to try next
                // In other words, it migth be time to retry, so we should validate with the retry policy function.
                bool permit = false;
                size_t delay;

                if (retryLogic->fnRetryPolicy != NULL && (retryLogic->fnRetryPolicy(&permit, &delay, retryLogic) == 0))
                {
                    // Does the policy function want us to retry (permit == true), or are we still allowed to retry?
                    // (in other words, are we within retryTimeoutLimitInSeconds seconds since starting to retry?)
                    // If so, see if we _really_ want to retry.
                    if ((permit == true) && ((retryLogic->retryTimeoutLimitInSeconds == 0) || retryLogic->retryTimeoutLimitInSeconds >= (delay + get_difftime(now, retryLogic->start))))
                    {
                        retryLogic->delayFromLastConnectToRetry = delay;

                        LogInfo("Evaluated delay time %d sec.  Retry attempt count %d\n", delay, retryLogic->retrycount);

                        // If the retry policy is telling us to connect right away ( <= ERROR_TIME_FOR_RETRY_SECS),
                        // or if enough time has elapsed, then we retry.
                        if ((retryLogic->delayFromLastConnectToRetry <= ERROR_TIME_FOR_RETRY_SECS) ||
                            (diffTime >= retryLogic->delayFromLastConnectToRetry))
                        {
                            retryLogic->lastConnect = now;
                            retryLogic->retrycount++;
                            result = true;
                        }
                        else
                        {
                            // To soon to retry according to policy.
                            result = false;
                        }
                    }
                    else
                    {
                        // Retry expired.  Stop trying.
                        LogError("Retry timeout expired after %d attempts", retryLogic->retrycount);
                        retryLogic->retryExpired = true;
                        StopRetryTimer(retryLogic);
                        result = false;
                    }
                }
                else
                {
                    // We don't have a retry policy.  Sorry, can't even guess.  Don't bother even trying to retry.
                    LogError("Cannot evaluate the next best time to retry");
                    result = false;
                }
            }
        }
        else
        {
            // Since this function is only called when the connection is
            // already broken, we can start doing the rety logic.  We'll do the
            // actual interval checking next time around this loop.
            StartRetryTimer(retryLogic);
            //wait for next do work to evaluate next best attempt
            result = false;
        }
    }
    return result;
}


static uint16_t GetNextPacketId(IOTHUB_MQTT_CLIENT_HANDLE transport_data)
{
    if (transport_data->packetId+1 >= USHRT_MAX)
    {
        transport_data->packetId = 1;
    }
    else
    {
        transport_data->packetId++;
    }
    return transport_data->packetId;
}

static const char* RetrieveMqttReturnCodes(CONNECT_RETURN_CODE rtn_code)
{
    switch (rtn_code)
    {
        case CONNECTION_ACCEPTED:
            return "Accepted";
        case CONN_REFUSED_UNACCEPTABLE_VERSION:
            return "Unacceptable Version";
        case CONN_REFUSED_ID_REJECTED:
            return "Id Rejected";
        case CONN_REFUSED_SERVER_UNAVAIL:
            return "Server Unavailable";
        case CONN_REFUSED_BAD_USERNAME_PASSWORD:
            return "Bad Username/Password";
        case CONN_REFUSED_NOT_AUTHORIZED:
            return "Not Authorized";
        case CONN_REFUSED_UNKNOWN:
        default:
            return "Unknown";
    }
}

static void OnMqttOperationComplete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_RESULT actionResult, const void* msgInfo, void* callbackCtx)
{
    (void)msgInfo;
    IOTHUB_MQTT_CLIENT_HANDLE iotHubClient = (IOTHUB_MQTT_CLIENT_HANDLE)callbackCtx;
    switch (actionResult)
    {
        case MQTT_CLIENT_ON_CONNACK:
        {
            const CONNECT_ACK* connack = (const CONNECT_ACK*)msgInfo;
            if (connack != NULL)
            {
                if (connack->returnCode == CONNECTION_ACCEPTED)
                {
                    // The connect packet has been acked
                    iotHubClient->isRecoverableError = true;
                    iotHubClient->isConnectionLost = false;
                    iotHubClient->needPublishPersistMsg = true;
                    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_CONNECTED;
                    StopRetryTimer(iotHubClient->retryLogic);
                }
                else
                {
                    iotHubClient->isConnectionLost = true;
                    if (connack->returnCode == CONN_REFUSED_BAD_USERNAME_PASSWORD)
                    {
                        iotHubClient->isRecoverableError = false;
                    }
                    else if (connack->returnCode == CONN_REFUSED_NOT_AUTHORIZED)
                    {
                        iotHubClient->isRecoverableError = false;
                    }
                    else if (connack->returnCode == CONN_REFUSED_UNACCEPTABLE_VERSION)
                    {
                        iotHubClient->isRecoverableError = false;
                    }
                    LogError("Connection Not Accepted: 0x%x: %s", connack->returnCode, RetrieveMqttReturnCodes(connack->returnCode));
                    (void)mqtt_client_disconnect(iotHubClient->mqttClient, NULL, NULL);
                    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_NOT_CONNECTED;
                }
            }
            else
            {
                LogError("MQTT_CLIENT_ON_CONNACK CONNACK parameter is NULL.");
            }
            break;
        }
        case MQTT_CLIENT_ON_SUBSCRIBE_ACK:
        {
            const SUBSCRIBE_ACK* suback = (const SUBSCRIBE_ACK*)msgInfo;
            if (suback != NULL)
            {
                size_t index = 0;
                for (index = 0; index < suback->qosCount; index++)
                {
                    if (suback->qosReturn[index] == DELIVER_FAILURE)
                    {
                        LogError("Subscribe delivery failure of subscribe %zu", index);
                    }
                }

                PDLIST_ENTRY currentListEntry = iotHubClient->sub_ack_waiting_queue.Flink;
                // when ack_waiting_queue.Flink points to itself, return directly
                while (currentListEntry != &iotHubClient->sub_ack_waiting_queue)
                {
                    PMQTT_SUB_CALLBACK_INFO subHandleEntry = containingRecord(currentListEntry, MQTT_SUB_CALLBACK_INFO, entry);
                    DLIST_ENTRY saveListEntry;
                    saveListEntry.Flink = currentListEntry->Flink;

                    if (suback->packetId == subHandleEntry->packetId)
                    {
                        (void)DList_RemoveEntryList(currentListEntry); //First remove the item from Waiting for Ack List.
                        subHandleEntry->subCallback(suback->qosReturn, suback->qosCount, subHandleEntry->context);
                        free(subHandleEntry);
                        break;
                    }
                    currentListEntry = saveListEntry.Flink;
                }
            }
            else
            {
                LogError("Failure: MQTT_CLIENT_ON_SUBSCRIBE_ACK SUBSCRIBE_ACK parameter is NULL.");
            }
            break;
        }
        case MQTT_CLIENT_ON_PUBLISH_ACK:
        {
            const PUBLISH_ACK* puback = (const PUBLISH_ACK*)msgInfo;
            if (puback != NULL)
            {
                PDLIST_ENTRY currentListEntry = iotHubClient->pub_ack_waiting_queue.Flink;
                // when ack_waiting_queue.Flink points to itself, return directly
                while (currentListEntry != &iotHubClient->pub_ack_waiting_queue)
                {
                    PMQTT_PUB_CALLBACK_INFO mqttMsgEntry = containingRecord(currentListEntry, MQTT_PUB_CALLBACK_INFO, entry);
                    DLIST_ENTRY saveListEntry;
                    saveListEntry.Flink = currentListEntry->Flink;

                    if (puback->packetId == mqttMsgEntry->packetId)
                    {
                        (void)DList_RemoveEntryList(currentListEntry); //First remove the item from Waiting for Ack List.
                        mqttMsgEntry->pubCallback(MQTT_PUB_SUCCESS, mqttMsgEntry->context);
                        // release mqtt message memory
                        mqttmessage_destroy(mqttMsgEntry->msgHandle);
                        free(mqttMsgEntry);
                        break;
                    }
                    currentListEntry = saveListEntry.Flink;
                }
            }
            else
            {
                LogError("Failure: MQTT_CLIENT_ON_PUBLISH_ACK publish_ack structure NULL.");
            }
            break;
        }
        case MQTT_CLIENT_ON_PUBLISH_RECV:
        {
            break;
        }
        case MQTT_CLIENT_ON_PUBLISH_REL:
        {
            break;
        }
        case MQTT_CLIENT_ON_PUBLISH_COMP:
        {
            // Done so send disconnect
            mqtt_client_disconnect(handle, NULL, NULL);
            break;
        }
        case MQTT_CLIENT_ON_DISCONNECT:
        {
            iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_NOT_CONNECTED;
            iotHubClient->isDisconnectCalled = true;
            break;
        }
        case MQTT_CLIENT_ON_UNSUBSCRIBE_ACK:
        {
            const UNSUBSCRIBE_ACK* unsuback = (const UNSUBSCRIBE_ACK*)msgInfo;
            if (unsuback != NULL)
            {
                // handle unsubscribe ack
            }
            else
            {
                LogError("Failure: MQTT_CLIENT_ON_UNSUBSCRIBE_ACK UNSUBSCRIBE_ACK parameter is NULL.");
            }
            break;
        }
        case MQTT_CLIENT_ON_PING_RESPONSE:
        {
            break;
        }
        default:
        {
            (void)printf("unexpected value received for enumeration (%d)\n", (int)actionResult);
        }
    }
}

int publish_mqtt_message(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient, const char* topicName,
                         QOS_VALUE qosValue, const uint8_t* appMsg, size_t appMsgLength, PUB_CALLBACK pubCallback, void* context)
{
    int result = 0;

    if ( qosValue == DELIVER_EXACTLY_ONCE)
    {
        LogError("Does not support qos = DELIVER_EXACTLY_ONCE");
        result = __FAILURE__;
        return result;
    }

    uint16_t packet_id = GetNextPacketId(iotHubClient);

    // write publish message to persistent storage only when client status is not connected
    if (qosValue == DELIVER_AT_LEAST_ONCE && iotHubClient->mqttClientStatus == MQTT_CLIENT_STATUS_NOT_CONNECTED)
    {
        if (iotHubClient->persistInterDesc != NULL)
        {
            if (iotHubClient->persistInterDesc->concrete_add_message(iotHubClient->persistHandle, packet_id,
                                                                 topicName, strlen(topicName), qosValue,
                                                                 (const char*)appMsg, appMsgLength) != 0)
            {
                LogError("Fail to write publish message to persistent storage");
                result = __FAILURE__;
                return result;
            }
        }
    }

    MQTT_MESSAGE_HANDLE mqtt_get_msg = mqttmessage_create(packet_id, topicName, qosValue, appMsg, appMsgLength);
    if (mqtt_get_msg == NULL)
    {
        LogError("Failed constructing mqtt message.");
        result = __FAILURE__;
    }
    else
    {
        PMQTT_PUB_CALLBACK_INFO pubCallbackHandle = NULL;
        if (qosValue == DELIVER_AT_LEAST_ONCE && pubCallback != NULL)
        {
            pubCallbackHandle = (PMQTT_PUB_CALLBACK_INFO)malloc(sizeof(MQTT_PUB_CALLBACK_INFO));
            if (pubCallbackHandle == NULL)
            {
                LogError("Fail to allocate memory for MQTT_PUB_CALLBACK_INFO");
                mqttmessage_destroy(mqtt_get_msg);
                result = __FAILURE__;
                return result;
            }
            pubCallbackHandle->packetId = packet_id;
            pubCallbackHandle->msgHandle = mqtt_get_msg;
            pubCallbackHandle->pubCallback = pubCallback;
            pubCallbackHandle->context = context;
            pubCallbackHandle->entry.Flink = NULL;
            pubCallbackHandle->entry.Blink = NULL;

            DList_InsertTailList(&iotHubClient->pub_ack_waiting_queue, &pubCallbackHandle->entry);
        }

        if (mqtt_client_publish(iotHubClient->mqttClient, mqtt_get_msg) != 0)
        {
            // release memory for pub callback handle
            if (pubCallbackHandle != NULL)
            {
                DList_RemoveEntryList(&pubCallbackHandle->entry);
                free(pubCallbackHandle);
                pubCallbackHandle = NULL;
            }
            // Call callback handle directly
            if (pubCallback != NULL)
            {
                pubCallback(MQTT_PUB_FAILED, context);
            }
            LogError("Failed publishing to mqtt client.");
            result = __FAILURE__;
        }
        else
        {
            if (qosValue == DELIVER_AT_MOST_ONCE && pubCallback != NULL)
            {
                pubCallback(MQTT_PUB_SUCCESS, context);
            }
        }
        
        if (pubCallbackHandle == NULL || qosValue == DELIVER_AT_MOST_ONCE)
        {
            mqttmessage_destroy(mqtt_get_msg);
        }
    }

    return result;
}


int subscribe_mqtt_topics(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient, SUBSCRIBE_PAYLOAD *subPayloads,
                          size_t subSize, SUB_CALLBACK subCallback, void* context)
{

    int result = 0;

    if (subPayloads == NULL || subSize == 0) {
        result = __FAILURE__;
        LogError("Failure: maybe subPayload is invalid, or subSize is zero.");
    }
    else
    {
        bool isQosCheckSuccess = true;
        for (int i = 0; i< subSize; ++i)
        {
            if(subPayloads[i].qosReturn == DELIVER_EXACTLY_ONCE)
            {
                LogError("IoT Hub does not support qos = DELIVER_EXACTLY_ONCE");
                result = __FAILURE__;
                isQosCheckSuccess = false;
                break;
            }
        }

        if (isQosCheckSuccess) {
            uint16_t packet_id = GetNextPacketId(iotHubClient);
            if (subCallback != NULL) {
                PMQTT_SUB_CALLBACK_INFO subCallbackHandle = NULL;
                subCallbackHandle = (PMQTT_SUB_CALLBACK_INFO)malloc(sizeof(MQTT_SUB_CALLBACK_INFO));
                if (subCallbackHandle == NULL)
                {
                    LogError("Fail to allocate memory for MQTT_PUB_CALLBACK_INFO");
                    result = __FAILURE__;
                    return result;
                }
                subCallbackHandle->packetId = packet_id;
                subCallbackHandle->subCallback = subCallback;
                subCallbackHandle->context = context;
                subCallbackHandle->entry.Flink = NULL;
                subCallbackHandle->entry.Blink = NULL;

                DList_InsertTailList(&iotHubClient->sub_ack_waiting_queue, &subCallbackHandle->entry);
            }

            if (mqtt_client_subscribe(iotHubClient->mqttClient, packet_id, subPayloads, subSize) != 0)
            {
                LogError("Failure: mqtt_client_subscribe returned error.");
                result = __FAILURE__;
            }
        }
    }

    return result;
}


int unsubscribe_mqtt_topics(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient, const char** unsubscribeList, size_t count)
{
    int result = 0;

    if (unsubscribeList == NULL || count == 0)
    {
        LogError("Failure: maybe unsubscribeList is invalid, or count is zero.");
        result = __FAILURE__;
    }
    else if (mqtt_client_unsubscribe(iotHubClient->mqttClient, GetNextPacketId(iotHubClient), unsubscribeList, count))
    {
        LogError("Failure: mqtt_client_unsubscribe returned error.");
        result = __FAILURE__;
    }

    return result;
}

static void NotifySubscribeAckFailure(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient) {
    LogError("notify all waiting for subscribe ack messages as failure");
    PDLIST_ENTRY currentListEntry = iotHubClient->sub_ack_waiting_queue.Flink;
    // when ack_waiting_queue.Flink points to itself, return directly
    while (currentListEntry != &iotHubClient->sub_ack_waiting_queue) {
        PMQTT_SUB_CALLBACK_INFO subAckHandleEntry = containingRecord(currentListEntry, MQTT_SUB_CALLBACK_INFO, entry);
        DLIST_ENTRY saveListEntry;
        saveListEntry.Flink = currentListEntry->Flink;

        subAckHandleEntry->subCallback(NULL, 0, subAckHandleEntry->context);
        free(subAckHandleEntry);
        (void)DList_RemoveEntryList(currentListEntry); //First remove the item from Waiting for Ack List.
        currentListEntry = saveListEntry.Flink;
    }
}

static void NotifyPublishAckFailure(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    LogError("notify all waiting for publish ack messages as failure");
    PDLIST_ENTRY currentListEntry = iotHubClient->pub_ack_waiting_queue.Flink;
    // when ack_waiting_queue.Flink points to itself, return directly
    while (currentListEntry != &iotHubClient->pub_ack_waiting_queue)
    {
        PMQTT_PUB_CALLBACK_INFO mqttMsgEntry = containingRecord(currentListEntry, MQTT_PUB_CALLBACK_INFO, entry);
        DLIST_ENTRY saveListEntry;
        saveListEntry.Flink = currentListEntry->Flink;

        (void)DList_RemoveEntryList(currentListEntry); //First remove the item from Waiting for Ack List.
        mqttMsgEntry->pubCallback(MQTT_PUB_FAILED, mqttMsgEntry->context);
        // add mqtt message into persistent storage
        if (iotHubClient->persistInterDesc != NULL)
        {
            const APP_PAYLOAD* payload = mqttmessage_getApplicationMsg(mqttMsgEntry->msgHandle);
            const char *topicName = mqttmessage_getTopicName(mqttMsgEntry->msgHandle);
            int rc = iotHubClient->persistInterDesc->concrete_add_message(iotHubClient->persistHandle,
                                                                          mqttmessage_getPacketId(mqttMsgEntry->msgHandle),
                                                                          topicName,
                                                                          strlen(topicName),
                                                                          mqttmessage_getQosType(mqttMsgEntry->msgHandle),
                                                                          (const char* )payload->message, payload->length);

            if (rc != 0)
            {
                LogError("Fail to insert message into persitent storage when handle miss pub ack message");
            }
        }
        mqttmessage_destroy(mqttMsgEntry->msgHandle);
        free(mqttMsgEntry);
        currentListEntry = saveListEntry.Flink;
    }
}

static void OnMqttErrorComplete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_ERROR error, void* callbackCtx)
{
    IOTHUB_MQTT_CLIENT_HANDLE iotHubClient = (IOTHUB_MQTT_CLIENT_HANDLE)callbackCtx;
    (void)handle;
    switch (error)
    {
        case MQTT_CLIENT_CONNECTION_ERROR:
            LogError("receive mqtt client connection error");
            break;
        case MQTT_CLIENT_PARSE_ERROR:
            LogError("receive mqtt client parse error");
            break;
        case MQTT_CLIENT_MEMORY_ERROR:
            LogError("receive mqtt client memory error");
            break;
        case MQTT_CLIENT_COMMUNICATION_ERROR:
            LogError("receive mqtt client communication error");
            break;
        case MQTT_CLIENT_NO_PING_RESPONSE:
            LogError("receive mqtt client ping error");
            break;
        case MQTT_CLIENT_UNKNOWN_ERROR:
            LogError("receive mqtt client unknown error");
            break;
    }
    // mark connection as not connected
    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_NOT_CONNECTED;
    iotHubClient->isConnectionLost = true;
    // enumerate all waiting for sub ack handle
    NotifySubscribeAckFailure(iotHubClient);
    // enumerate all waiting for ack publish message and send them a failure notification
    NotifyPublishAckFailure(iotHubClient);
}

static void ClearMqttOptions(MQTT_CLIENT_OPTIONS* mqttOptions)
{
    free(mqttOptions->clientId);
    mqttOptions->clientId = NULL;
    free(mqttOptions->willTopic);
    mqttOptions->willTopic = NULL;
    free(mqttOptions->willMessage);
    mqttOptions->willMessage = NULL;
    free(mqttOptions->username);
    mqttOptions->username = NULL;
    free(mqttOptions->password);
    mqttOptions->password = NULL;
    free(mqttOptions);
}

static void InitMqttOptions(MQTT_CLIENT_OPTIONS* mqttOptions)
{
    mqttOptions->clientId = NULL;
    mqttOptions->willTopic = NULL;
    mqttOptions->willMessage = NULL;
    mqttOptions->username = NULL;
    mqttOptions->password = NULL;
    mqttOptions->keepAliveInterval = 10;
    mqttOptions->messageRetain = false;
    mqttOptions->useCleanSession = true;
    mqttOptions->qualityOfServiceValue = DELIVER_AT_MOST_ONCE;
    mqttOptions->log_trace = false;
}

static int CloneMqttOptions(MQTT_CLIENT_OPTIONS** newMqttOptions, const MQTT_CLIENT_OPTIONS* mqttOptions)
{
    int result = 0;
    *newMqttOptions = (MQTT_CLIENT_OPTIONS *)malloc(sizeof(MQTT_CLIENT_OPTIONS));
    if (*newMqttOptions == NULL) {
        LOG(AZ_LOG_ERROR, LOG_LINE, "malloc MQTT_CLIENT_OPTIONS failed");
        result = __FAILURE__;

        return result;
    }

    InitMqttOptions(*newMqttOptions);

    if (mqttOptions->clientId != NULL)
    {
        if (mallocAndStrcpy_s(&((*newMqttOptions)->clientId), mqttOptions->clientId) != 0)
        {
            result = __FAILURE__;
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s clientId");
        }
    }
    if (result == 0 && mqttOptions->willTopic != NULL)
    {
        if (mallocAndStrcpy_s(&((*newMqttOptions)->willTopic), mqttOptions->willTopic) != 0)
        {
            result = __FAILURE__;
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s willTopic");
        }
    }
    if (result == 0 && mqttOptions->willMessage != NULL)
    {
        if (mallocAndStrcpy_s(&((*newMqttOptions)->willMessage), mqttOptions->willMessage) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s willMessage");
            result = __FAILURE__;
        }
    }
    if (result == 0 && mqttOptions->username != NULL)
    {
        if (mallocAndStrcpy_s(&((*newMqttOptions)->username), mqttOptions->username) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s username");
            result = __FAILURE__;
        }
    }
    if (result == 0 && mqttOptions->password != NULL)
    {
        if (mallocAndStrcpy_s(&((*newMqttOptions)->password), mqttOptions->password) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s password");
            result = __FAILURE__;
        }
    }
    if (result == 0)
    {
        (*newMqttOptions)->keepAliveInterval = mqttOptions->keepAliveInterval;
        (*newMqttOptions)->messageRetain = mqttOptions->messageRetain;
        (*newMqttOptions)->useCleanSession = mqttOptions->useCleanSession;
        (*newMqttOptions)->qualityOfServiceValue = mqttOptions->qualityOfServiceValue;
    }
    else
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "release memory for newly allocated mqtt options");
        ClearMqttOptions(*newMqttOptions);
        *newMqttOptions = NULL;
    }
    return result;
}

static XIO_HANDLE CreateTcpConnection(const char *endpoint)
{
    SOCKETIO_CONFIG config = {endpoint, MQTT_CONNECTION_TCP_PORT, NULL};

    XIO_HANDLE xio = xio_create(socketio_get_interface_description(), &config);

    return xio;
}

static XIO_HANDLE CreateTlsConnection(const char *endpoint)
{
    TLSIO_CONFIG tlsio_config = { endpoint, MQTT_CONNECTION_TLS_PORT };
    // enable wolfssl by set certificates
    // tlsio_config.certificate = certificates;

    XIO_HANDLE xio = xio_create(platform_get_default_tlsio(), &tlsio_config);

    if (xio_setoption(xio, "TrustedCerts", certificates) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Fail to assign trusted cert chain");
    }
    return xio;
}

static XIO_HANDLE CreateMutualTlsConnection(const char *endpoint, const char *client_cert_in_option, const char *client_key_in_option)
{
    TLSIO_CONFIG tlsio_config = { endpoint, MQTT_CONNECTION_TLS_PORT };
    // enable wolfssl by set certificates
    // tlsio_config.certificate = certificates;

    XIO_HANDLE xio = xio_create(platform_get_default_tlsio(), &tlsio_config);

    if (xio_setoption(xio, "TrustedCerts", certificates) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Fail to assign trusted cert chain");
    }
    if (xio_setoption(xio, "x509certificate", client_cert_in_option) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Fail to assign client cert");
    }
    if (xio_setoption(xio, "x509privatekey", client_key_in_option) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "Fail to assign client private key");
    }
    return xio;
}

static void DisconnectFromClient(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    (void)mqtt_client_disconnect(iotHubClient->mqttClient, NULL, NULL);
    xio_destroy(iotHubClient->xioTransport);
    iotHubClient->xioTransport = NULL;

    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_NOT_CONNECTED;
}

static int SendMqttConnectMessage(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    int result = 0;

	if (!iotHubClient->xioTransport) 
	{
		switch (iotHubClient->connType)
		{
			case MQTT_CONNECTION_TCP:
				iotHubClient->xioTransport = CreateTcpConnection(iotHubClient->endpoint);
				break;
			case MQTT_CONNECTION_TLS:
				iotHubClient->xioTransport = CreateTlsConnection(iotHubClient->endpoint);
				break;
			case MQTT_CONNECTION_MUTUAL_TLS:
				iotHubClient->xioTransport = CreateMutualTlsConnection(iotHubClient->endpoint,
																	   iotHubClient->client_cert,
																	   iotHubClient->client_key);
		}
	}

    if (iotHubClient->xioTransport == NULL)
    {
        LogError("failed to create connection with server");
        // TODO: add code to handle release data for IOTHUB_MQTT_CLIENT_HANDLE
        result = __FAILURE__;
    }
    else
    {
        if (mqtt_client_connect(iotHubClient->mqttClient, iotHubClient->xioTransport, iotHubClient->options) != 0)
        {
            LogError("failed to initialize mqtt connection with server");
            // TODO: add code to handle release data for IOTHUB_MQTT_CLIENT_HANDLE
            result = __FAILURE__;
        }
        else
        {
            (void)tickcounter_get_current_ms(iotHubClient->msgTickCounter, &(iotHubClient->mqtt_connect_time));
            result = 0;
        }
    }

    return result;
}

IOTHUB_MQTT_CLIENT_HANDLE initialize_mqtt_client_handle(const MQTT_CLIENT_OPTIONS *options, const char* endpoint,
                                                        MQTT_CONNECTION_TYPE connType, ON_MQTT_MESSAGE_RECV_CALLBACK callback,
                                                        IOTHUB_CLIENT_RETRY_POLICY retryPolicy, size_t retryTimeoutLimitInSeconds )
{
    IOTHUB_MQTT_CLIENT_HANDLE iotHubClient = (IOTHUB_MQTT_CLIENT_HANDLE)malloc(sizeof(IOTHUB_MQTT_CLIENT));

    if (iotHubClient == NULL) {
        LOG(AZ_LOG_ERROR, LOG_LINE, "malloc IOTHUB_MQTT_CLIENT failed");
        return NULL;
    }

    memset(iotHubClient, 0, sizeof(IOTHUB_MQTT_CLIENT));

    // check options has required parameters
    if (options->clientId == NULL)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "client id is required when create mqtt connection");
        return NULL;
    }

    if ((iotHubClient->msgTickCounter = tickcounter_create()) == NULL)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "fail to create tickCounter");
        free(iotHubClient);
        return NULL;
    }
    else if(CloneMqttOptions(&(iotHubClient->options), options) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "fail to clone mqtt options");
        tickcounter_destroy(iotHubClient->msgTickCounter);
        free(iotHubClient);
        return NULL;
    }
    else if (mallocAndStrcpy_s(&(iotHubClient->endpoint), endpoint) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s endpoint");
        tickcounter_destroy(iotHubClient->msgTickCounter);
        ClearMqttOptions(iotHubClient->options);
        free(iotHubClient);
        return NULL;
    }

    iotHubClient->connType = connType;
    iotHubClient->recvCallback = callback;

    // set callback context to mqttClientHandle, since this handle has all required data to process message
    iotHubClient->callbackContext = iotHubClient;

    iotHubClient->mqttClient = mqtt_client_init(callback, OnMqttOperationComplete, iotHubClient, OnMqttErrorComplete, iotHubClient);

    if(iotHubClient->mqttClient == NULL)
    {
        LogError("failure initializing mqtt client.");
        tickcounter_destroy(iotHubClient->msgTickCounter);
        ClearMqttOptions(iotHubClient->options);
        free(iotHubClient->endpoint);
        free(iotHubClient);
        return NULL;
    }

    DList_InitializeListHead(&iotHubClient->pub_ack_waiting_queue);
    DList_InitializeListHead(&iotHubClient->sub_ack_waiting_queue);
    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_NOT_CONNECTED;
    iotHubClient->isRecoverableError = true;
    iotHubClient->retryLogic = CreateRetryLogic(retryPolicy, retryTimeoutLimitInSeconds);
    iotHubClient->packetId = 0;
    iotHubClient->xioTransport = NULL;

    // initialize mqtt persist interface and handle
    iotHubClient->persistInterDesc = get_default_persist_interface_description();
    if (iotHubClient->persistInterDesc != NULL)
    {
        if (iotHubClient->persistInterDesc->concrete_initialize_handle(&iotHubClient->persistHandle, options->clientId, PERSIST_NAME_SUFFIX) != 0)
        {
            LogError("fail to initialize persist handle");
            free(iotHubClient->endpoint);
            ClearMqttOptions(iotHubClient->options);
            mqtt_client_deinit(iotHubClient->mqttClient);
            tickcounter_destroy(iotHubClient->msgTickCounter);
            DestroyRetryLogic(iotHubClient->retryLogic);
            free(iotHubClient);
            return NULL;
        }
    }
    return iotHubClient;
}

void set_client_cert(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient, const char* client_cert, const char* client_key) {
    iotHubClient->client_cert = (char *) client_cert;
    iotHubClient->client_key = (char *) client_key;
}

int initialize_mqtt_connection(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    int result = 0;

    // Make sure we're not destroying the object
    if (!iotHubClient->isDestroyCalled)
    {
        // If we are MQTT_CLIENT_STATUS_NOT_CONNECTED then check to see if we need
        // to back off the connecting to the server
        if (!iotHubClient->isDisconnectCalled
            && iotHubClient->mqttClientStatus == MQTT_CLIENT_STATUS_NOT_CONNECTED
            && CanRetry(iotHubClient->retryLogic))
        {
            if (tickcounter_get_current_ms(iotHubClient->msgTickCounter, &iotHubClient->connectTick) != 0)
            {
                iotHubClient->connectFailCount++;
                result = __FAILURE__;
            }
            else
            {
                if (iotHubClient->isConnectionLost && iotHubClient->isRecoverableError) {
                    xio_destroy(iotHubClient->xioTransport);
                    iotHubClient->xioTransport = NULL;
                    mqtt_client_deinit(iotHubClient->mqttClient);
                    iotHubClient->mqttClient = mqtt_client_init(iotHubClient->recvCallback, OnMqttOperationComplete,
                                                                iotHubClient, OnMqttErrorComplete, iotHubClient);
                }

                if (SendMqttConnectMessage(iotHubClient) != 0)
                {
                    iotHubClient->connectFailCount++;

                    result = __FAILURE__;
                }
                else
                {
                    iotHubClient->mqttClientStatus = MQTT_CLIENT_STATUS_CONNECTING;
                    iotHubClient->connectFailCount = 0;
                    result = 0;
                }
            }
        }
            // Codes_SRS_IOTHUB_TRANSPORT_MQTT_COMMON_09_001: [ IoTHubTransport_MQTT_Common_DoWork shall trigger reconnection if the mqtt_client_connect does not complete within `keepalive` seconds]
        else if (iotHubClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTING)
        {
            tickcounter_ms_t current_time;
            if (tickcounter_get_current_ms(iotHubClient->msgTickCounter, &current_time) != 0)
            {
                LogError("failed verifying MQTT_CLIENT_STATUS_CONNECTING timeout");
                result = __FAILURE__;
            }
            else if ((current_time - iotHubClient->mqtt_connect_time) / 1000 > iotHubClient->options->keepAliveInterval)
            {
                LogError("mqtt_client timed out waiting for CONNACK");
                DisconnectFromClient(iotHubClient);
                result = __FAILURE__;
            }
        }
        else if (iotHubClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
        {
            // We are connected and not being closed
            tickcounter_ms_t current_time;
            if (tickcounter_get_current_ms(iotHubClient->msgTickCounter, &current_time) != 0)
            {
                iotHubClient->connectFailCount++;
                result = __FAILURE__;
            }
        }
    }
    return result;

}

// timeout is used to set timeout window for connection in seconds
int iothub_mqtt_doconnect(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient, size_t timeout)
{
    int result = 0;
    tickcounter_ms_t currentTime, lastSendTime;
    tickcounter_get_current_ms(iotHubClient->msgTickCounter, &lastSendTime);

    do
    {
        iothub_mqtt_dowork(iotHubClient);
        ThreadAPI_Sleep(10);
        tickcounter_get_current_ms(iotHubClient->msgTickCounter, &currentTime);
    } while (iotHubClient->mqttClientStatus != MQTT_CLIENT_STATUS_CONNECTED
             && iotHubClient->isRecoverableError
             && (currentTime - lastSendTime) / 1000 <= timeout);

    if ((currentTime - lastSendTime) / 1000 > timeout)
    {
        LogError("failed to waitfor connection complete in %d seconds", timeout);
        result = __FAILURE__;
        return result;
    }

    if (iotHubClient->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        result = 0;
    }
    else
    {
        result = __FAILURE__;
    }

    return result;
}

void iothub_mqtt_dowork(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    if (initialize_mqtt_connection(iotHubClient) != 0)
    {
        LogError("fail to establish connection with server");
    }
    else
    {
        if (iotHubClient->needPublishPersistMsg)
        {
            iotHubClient->needPublishPersistMsg = false;

            if (iotHubClient->persistInterDesc != NULL)
            {
                PUBLISH_DATA *pubMessages;
                int pubMsgCount = 0;
                int rc = iotHubClient->persistInterDesc->concrete_get_messages(iotHubClient->persistHandle, &pubMessages, &pubMsgCount);
                if (rc != 0)
                {
                    LogError("Fail to read data from persist storage");
                }
                else if (pubMsgCount != 0)
                {
                    for (int i = 0;i < pubMsgCount; ++i)
                    {
                        rc = publish_mqtt_message(iotHubClient, pubMessages[i].topicName,
                                             pubMessages[i].qos, (const uint8_t *)pubMessages[i].payload, pubMessages[i].payloadLen, NULL, NULL);

                        if (rc != 0)
                        {
                            LogError("Fail to publish message get from persist storage");
                        }

                        // delete message from persist storage
                        rc = iotHubClient->persistInterDesc->concrete_delete_message(iotHubClient->persistHandle, pubMessages[i].messageId);
                        if (rc != 0)
                        {
                            LogError("Fail to delete message from persist storage");
                        }
                    }
                    // release memory allocated for publish message
                    iotHubClient->persistInterDesc->concrete_release_message(pubMessages, pubMsgCount);
                }
            }
        }
        mqtt_client_dowork(iotHubClient->mqttClient);
    }
}

int iothub_mqtt_disconnect(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    int result = mqtt_client_disconnect(iotHubClient->mqttClient, NULL, NULL);
    iotHubClient->isDisconnectCalled = true;
    return result;
}

void iothub_mqtt_destroy(IOTHUB_MQTT_CLIENT_HANDLE iotHubClient)
{
    if (iotHubClient != NULL)
    {
        iotHubClient->isDestroyCalled = true;
        iotHubClient->isConnectionLost = true;

        DisconnectFromClient(iotHubClient);

        free(iotHubClient->endpoint);
        // don't set iotHubClient->endpoint, free iotHubClient at end of this function
        // iotHubClient->endpoint = NULL;
        mqtt_client_deinit(iotHubClient->mqttClient);
        // iotHubClient->mqttClient = NULL;
        ClearMqttOptions(iotHubClient->options);

        tickcounter_destroy(iotHubClient->msgTickCounter);
        DestroyRetryLogic(iotHubClient->retryLogic);
        // release persist handle
        if (iotHubClient->persistInterDesc != NULL)
        {
            iotHubClient->persistInterDesc->concrete_storage_destroy(iotHubClient->persistHandle);
        }

        free(iotHubClient);
    }
}
