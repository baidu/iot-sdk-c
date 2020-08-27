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

#ifndef IOTCORE_MQTT_CLIENT_H
#define IOTCORE_MQTT_CLIENT_H

#include "iotcore_retry.h"
#include "iotcore_param.h"
#include "iotcore_type.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum MQTT_CLIENT_CONNECT_STATUS_TAG
{
    MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED,
    MQTT_CLIENT_CONNECT_STATUS_CONNECTING,
    MQTT_CLIENT_CONNECT_STATUS_CONNECTED
} MQTT_CLIENT_CONNECT_STATUS;

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

typedef struct IOTCORE_MQTT_STATUS_TAG
{
    MQTT_CLIENT_CONNECT_STATUS connect_status;
    int is_disconnect_called;
    int is_connection_lost;
    int is_destroy_called;
    int is_recoverable_error;
} IOTCORE_MQTT_STATUS;

typedef enum _IOTCORE_MQTT_QOS
{
    QOS_0_AT_MOST_ONCE = 0x00,
    QOS_1_AT_LEAST_ONCE = 0x01,
    QOS_2_EXACTLY_ONCE = 0x02,
    QOS_FAILURE = 0x80
} IOTCORE_MQTT_QOS;

typedef struct _IOTCORE_MQTT_PAYLOAD
{
    uint8_t* message;
    size_t length;
} IOTCORE_MQTT_PAYLOAD;

typedef int(*PUB_CALLBACK)(MQTT_PUB_STATUS_TYPE status, void *context);

typedef int(*SUB_CALLBACK)(IOTCORE_MQTT_QOS* qosReturn, size_t qosCount, void *context);

typedef void(*RECV_MSG_CALLBACK)(uint16_t packet_id, IOTCORE_MQTT_QOS qos, const char* topic, const IOTCORE_MQTT_PAYLOAD* msg, int is_retained);

typedef struct IOTCORE_MQTT_CLIENT_TAG* IOTCORE_MQTT_CLIENT_HANDLE;


int initialize_mqtt_connection(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);

IOTCORE_MQTT_CLIENT_HANDLE initialize_mqtt_client_handle(const IOTCORE_INFO* info, const char* will_topic, const char* will_payload,
                                                        MQTT_CONNECTION_TYPE conn_type, RECV_MSG_CALLBACK callback,
                                                        IOTCORE_RETRY_POLICY retry_policy, size_t retry_timeout_limit_in_sec);

int iotcore_mqtt_doconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, size_t timeout);

int publish_mqtt_message(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* pub_topic_name,
                         IOTCORE_MQTT_QOS qos_value, const uint8_t* pub_msg, size_t pub_msg_length, PUB_CALLBACK handle, void* context);

int subscribe_mqtt_topic(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* sub_topic, IOTCORE_MQTT_QOS ret_qos, SUB_CALLBACK sub_callback, void* context);

int unsubscribe_mqtt_topics(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* unsubscribe);

void iotcore_mqtt_dowork(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);

int iotcore_mqtt_disconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);

void iotcore_mqtt_destroy(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);

void set_client_cert(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* client_cert, const char* client_key);

IOTCORE_MQTT_STATUS iotcore_get_mqtt_status(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);

#ifdef __cplusplus
}
#endif

#endif //IOTCORE_MQTT_CLIENT_H
