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

#ifndef IOTHUB_MQTT_CLIENT_H
#define IOTHUB_MQTT_CLIENT_H

#include <stdlib.h>
#include <limits.h>
#include <azure_c_shared_utility/strings_types.h>
#include <azure_c_shared_utility/tickcounter.h>
#include <azure_c_shared_utility/doublylinkedlist.h>
#include "azure_umqtt_c/mqtt_client.h"
#include "iothub_client.h"
#include "iothub_client_persistence.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum MQTT_CLIENT_STATUS_TAG
{
    MQTT_CLIENT_STATUS_NOT_CONNECTED,
    MQTT_CLIENT_STATUS_CONNECTING,
    MQTT_CLIENT_STATUS_CONNECTED
} MQTT_CLIENT_STATUS;

typedef enum MQTT_CONNECTION_TYPE_TAG
{
    MQTT_CONNECTION_TCP,
    MQTT_CONNECTION_TLS,
    MQTT_CONNECTION_MUTUAL_TLS

} MQTT_CONNECTION_TYPE;

typedef enum MQTT_PUB_STATUS_TYPE_TAG
{
    MQTT_PUB_SUCCESS,
    MQTT_PUB_FAILED
} MQTT_PUB_STATUS_TYPE;

typedef int(*PUB_CALLBACK)(MQTT_PUB_STATUS_TYPE status, void *context);

typedef int(*SUB_CALLBACK)(QOS_VALUE* qosReturn, size_t qosCount, void *context);

typedef int(*RETRY_POLICY)(bool *permit, size_t* delay, void* retryContextCallback);

typedef struct RETRY_LOGIC_TAG RETRY_LOGIC;

typedef struct IOTHUB_MQTT_CLIENT_TAG
{
    MQTT_CONNECTION_TYPE connType;
    char* endpoint;
    char* client_cert;
    char* client_key;
    MQTT_CLIENT_OPTIONS* options;

    // Protocol
    MQTT_CLIENT_HANDLE mqttClient;
    XIO_HANDLE xioTransport;

    // Session - connection
    uint16_t packetId;

    // Connection state control
    MQTT_CLIENT_STATUS mqttClientStatus;
    bool isDisconnectCalled;
    bool isConnectionLost;
    bool isDestroyCalled;
    bool isRecoverableError;

    tickcounter_ms_t mqtt_connect_time;
    size_t connectFailCount;
    tickcounter_ms_t connectTick;
    TICK_COUNTER_HANDLE msgTickCounter;

    //Retry Logic
    RETRY_LOGIC* retryLogic;

    // Set this member variable to pass additional structure for user defined callback handle
    void* callbackContext;

    // pub ack wait queue
    DLIST_ENTRY pub_ack_waiting_queue;

    // sub ack wait queue
    DLIST_ENTRY sub_ack_waiting_queue;

    // subscribe message callback handle
    ON_MQTT_MESSAGE_RECV_CALLBACK recvCallback;

    // handle to do publish message persistence
    bool needPublishPersistMsg;
    PERSIST_CONCRETE_HANDLE persistHandle;
    const PERSIST_INTERFACE_DESCRIPTION* persistInterDesc;

} IOTHUB_MQTT_CLIENT;

typedef struct IOTHUB_MQTT_CLIENT_TAG* IOTHUB_MQTT_CLIENT_HANDLE;


MOCKABLE_FUNCTION(, int, initialize_mqtt_connection, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient);

MOCKABLE_FUNCTION(, IOTHUB_MQTT_CLIENT_HANDLE, initialize_mqtt_client_handle, const MQTT_CLIENT_OPTIONS*, options, const char*, endpoint,
                                                        MQTT_CONNECTION_TYPE, connType, ON_MQTT_MESSAGE_RECV_CALLBACK, callback,
                                                        IOTHUB_CLIENT_RETRY_POLICY, retryPolicy, size_t, retryTimeoutLimitInSeconds);

MOCKABLE_FUNCTION(, int, publish_mqtt_message, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient, const char*, topicName,
                         QOS_VALUE, qosValue, const uint8_t*, appMsg, size_t, appMsgLength, PUB_CALLBACK, handle, void*, context);

MOCKABLE_FUNCTION(, int, subscribe_mqtt_topics, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient, SUBSCRIBE_PAYLOAD*, subPayloads, size_t, subSize, SUB_CALLBACK, subCallback, void*, context);

MOCKABLE_FUNCTION(, int, unsubscribe_mqtt_topics, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient, const char**, unsubscribeList, size_t, count);

MOCKABLE_FUNCTION(, void, iothub_mqtt_dowork, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient);

MOCKABLE_FUNCTION(, int, iothub_mqtt_disconnect, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient);

MOCKABLE_FUNCTION(, void, iothub_mqtt_destroy, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient);

MOCKABLE_FUNCTION(, int, iothub_mqtt_doconnect, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient, size_t, timeout);

MOCKABLE_FUNCTION(,void, set_client_cert, IOTHUB_MQTT_CLIENT_HANDLE, iotHubClient, const char*, client_cert, const char*, client_key);

#ifdef __cplusplus
}
#endif

#endif //IOTHUB_MQTT_CLIENT_H
