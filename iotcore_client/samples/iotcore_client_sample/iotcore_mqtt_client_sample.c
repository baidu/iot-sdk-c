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

/**
 * README for this sample
 * 
 * For demonstrating all the iotcore_client interface in one sample,
 * This sample program do the following steps:
 *  1. Publish one topic to the broker and subscribe the same topic.
 *  2. Publish the same topic every PUB_TIME_INTERVAL seconds.
 *  3. After publish UNSUB_AFTER_PUB_COUNT times, unsubscribe that topic.
 * 
 * For implement the above target, you should config in IoTCore as follow:
 *  1. Add a device template, and add a topic in the template with PUB and 
 *     SUB authority at the same time.
 *  2. Topic name like $iot/{deviceName}/user/xxx. Set the following macros
 *     TEST_SUB_TOPIC_FORMAT and TEST_PUB_TOPIC_FORMAT to "$iot/%s/user/xxx".
 *  3. Add a device in device list using the above device template.
 *  4. Acquire connection info in device infomation page, and set the info
 *     to relevant macro below.
 *    4.1 ENDPOINT can be found in device list page.
 *    4.2 If you chose authorize method as password, you should set macros
 *        as follow: ENDPOINT, IOTCORE_ID, DEVICE_KEY and DEVICE_SECRET. 
 *    4.3 If you chose authorize method as certificate, you should set macros
 *        as follow: ENDPOINT, IOTCORE_ID, DEVICE_KEY. And 
 *  5. If you chose authorize method as password, macro CONNECTION_TYPE 
 *     can set to TCP or TLS. Variables client_cert and client_key can be 
 *     ignored.
 *  6. If you chose authorize method as certificate, macro CONNECTION_TYPE 
 *     can set to MUTUAL_TLS. And you should set variables client_cert and 
 *     client_key according to the cert-and-keys.txt file downloaded when 
 *     creating device.
 */

#include "iotcore_mqtt_client_sample.h"
#include "iotcore_mqtt_client.h"

#include <stdio.h>
#include <time.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>  // ThreadAPI_Sleep
#include "azure_c_shared_utility/agenttime.h" // get_time

// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"
// IOTCORE_ID and DEVICE_KEY is need to be set accroding to IoTCore device info page.
#define         IOTCORE_ID                  "xxxxxxx"
#define         DEVICE_KEY                  "xxxxxxx"
// if CONNECTION_TYPE is MUTUAL_TLS, this macro can be anything, it will be ignored internal
// otherwise, this macro is necessary.
#define         DEVICE_SECRET               "xxxxxxxxxxxxxxxx"

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TCP"
// #define         CONNECTION_TYPE              "TLS"
// #define         CONNECTION_TYPE              "MUTUAL_TLS"

#define         PUB_TIME_INTERVAL           2
#define         UNSUB_AFTER_PUB_COUNT       5

//The following certificate and key should be set if CONNECTION_TYPE set to 'MUTUAL_TLS'.
static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
        //"you client cert\r\n"
        "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
        // "your client key\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";

// subscribe topic and publish topic is the same
// the demo can recive the same message it published, to prove the fuction of sub and pub working
static const char* TEST_SUB_TOPIC_FORMAT = "$iot/%s/user/test";
static const char* TEST_PUB_TOPIC_FORMAT = "$iot/%s/user/test";
static const char* TEST_MESSAGE = "this is a message for test.";

static const char* QosToString(IOTCORE_MQTT_QOS qos_value)
{
    switch (qos_value)
    {
        case QOS_0_AT_MOST_ONCE: return "0_Deliver_At_Most_Once";
        case QOS_1_AT_LEAST_ONCE: return "1_Deliver_At_Least_Once";
        case QOS_2_EXACTLY_ONCE: return "2_Deliver_Exactly_Once";
        case QOS_FAILURE: return "Deliver_Failure";
    }
    return "";
}

void recv_callback(uint16_t packet_id, IOTCORE_MQTT_QOS qos, const char* topic, const IOTCORE_MQTT_PAYLOAD* msg, int is_retained)
{
    char* recv_msg = (char*)malloc(msg->length + 1);
    memset(recv_msg, 0, msg->length + 1);
    memcpy(recv_msg, msg->message, msg->length);

    printf("****************** Incoming Msg ******************\n"
        "* Packet Id: %d\r\n* QOS: %s\r\n* Topic Name: %s\r\n* Is Retained: %s\r\n* App Msg: [%s]\n"
        "**************************************************\n",
        packet_id, QosToString(qos), topic, is_retained == 1 ? "true": "false", recv_msg); 
    free(recv_msg);
}

int pub_least_ack_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    if (status == MQTT_PUB_SUCCESS)
    {
        printf(" - received publish ack from mqtt server\r\n");
    }
    else
    {
        printf(" - fail to publish message to mqtt server\r\n");
    }

    return 0;
}

static int processSubAckFunction(IOTCORE_MQTT_QOS* qosReturn, size_t qosCount, void *context) {
    printf(" - receive suback from hub server\r\n");
    for (int i =0; i< qosCount; ++i) {
        printf(" - qos return: %d\r\n", qosReturn[i]);
    }

    int *flag = (int *)context;
    *flag = 1;

    return 0;
}

int iotcore_mqtt_client_run(void)
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return IOTCORE_ERR_ERROR;
    }
    else
    {
      IOTCORE_INFO _info = {IOTCORE_ID, DEVICE_KEY, DEVICE_SECRET, ENDPOINT};

        MQTT_CONNECTION_TYPE type;
        if (strcmp(CONNECTION_TYPE, "TCP") == 0)
        {
            type = MQTT_CONNECTION_TCP;
        }
        else if (strcmp(CONNECTION_TYPE, "TLS") == 0)
        {
             type = MQTT_CONNECTION_TLS;
        }
        else if (strcmp(CONNECTION_TYPE, "MUTUAL_TLS") == 0)
        {
             type = MQTT_CONNECTION_MUTUAL_TLS;
        }
        else
        {
            return IOTCORE_ERR_INVALID_PARAMETER;
        }

        IOTCORE_RETRY_POLICY retry_policy = IOTCORE_RETRY_EXPONENTIAL_BACKOFF;

        size_t retry_timeout_limit_in_sec = 1000;

        IOTCORE_MQTT_CLIENT_HANDLE _client_handle = initialize_mqtt_client_handle(&_info, NULL, NULL, type, recv_callback,
                                                                               retry_policy, retry_timeout_limit_in_sec);

        if (strcmp(CONNECTION_TYPE, "MUTUAL_TLS") == 0)
        {
            set_client_cert(_client_handle, client_cert, client_key);
        }

        if (_client_handle == NULL)
        {
            printf("Error: fail to initialize IOTCORE_MQTT_CLIENT_HANDLE.\n");
            return 0;
        }

        int _result = iotcore_mqtt_doconnect(_client_handle, 60);

        if (_result != IOTCORE_ERR_OK)
        {
            printf("fail to establish connection with server.\n");
            return _result;
        }

        // construct test sub and pub topic
        int _test_sub_topic_len_max = strlen(TEST_SUB_TOPIC_FORMAT) + strlen(DEVICE_KEY);
        int _test_pub_topic_len_max = strlen(TEST_PUB_TOPIC_FORMAT) + strlen(DEVICE_KEY);
        char* _test_sub_topic = (char*)malloc(_test_sub_topic_len_max);
        char* _test_pub_topic = (char*)malloc(_test_pub_topic_len_max);
        memset(_test_sub_topic, 0, _test_sub_topic_len_max);
        memset(_test_pub_topic, 0, _test_pub_topic_len_max);
        sprintf(_test_sub_topic, TEST_SUB_TOPIC_FORMAT, DEVICE_KEY);
        sprintf(_test_pub_topic, TEST_PUB_TOPIC_FORMAT, DEVICE_KEY);

        int flag = 0;
        printf("subscibe a message.\n");
        subscribe_mqtt_topic(_client_handle, _test_sub_topic, QOS_1_AT_LEAST_ONCE, processSubAckFunction, &flag);

        _result = publish_mqtt_message(_client_handle, _test_pub_topic, QOS_1_AT_LEAST_ONCE, (const uint8_t*)TEST_MESSAGE,
                                      strlen(TEST_MESSAGE), pub_least_ack_process , _client_handle);

        int _published_count = 0;
        time_t _start = get_time(NULL);
        do
        {
            // publish mqtt message every 2 seconds
            if (get_time(NULL) - _start > PUB_TIME_INTERVAL)
            {
                printf("publish a message.\n");
                publish_mqtt_message(_client_handle, _test_pub_topic, QOS_1_AT_LEAST_ONCE, (const uint8_t*)TEST_MESSAGE,
                                            strlen(TEST_MESSAGE), pub_least_ack_process , _client_handle);
                _published_count++;
                _start = get_time(NULL);
            }

            // unsubscribe the topic after published 5 times, for testing unsubscribe_mqtt_topics function
            if (_published_count == UNSUB_AFTER_PUB_COUNT) {
                _published_count++;
                unsubscribe_mqtt_topics(_client_handle, _test_sub_topic);
            }

            iotcore_mqtt_dowork(_client_handle);
            ThreadAPI_Sleep(10);
        } while (!iotcore_get_mqtt_status(_client_handle).is_destroy_called && !iotcore_get_mqtt_status(_client_handle).is_disconnect_called);

        if (_test_pub_topic) {
            free(_test_pub_topic);
            _test_pub_topic = NULL;
        }
        if (_test_sub_topic) {
            free(_test_sub_topic);
            _test_sub_topic = NULL;
        }
        iotcore_mqtt_destroy(_client_handle);
        return 0;
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

