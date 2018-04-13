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

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/utf8_checker.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iothub_mqtt_client.h"
#include "iothub_mqtt_client_sample.h"

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxx"

// The connection type is TLS or MUTUAL_TLS. If MUTUAL_TLS selected certificate and key should be set in 'certs.c'.
#define         CONNECTION_TYPE              "MUTUAL_TLS"

static const char* TOPIC_NAME_A = "msgA";
static const char* TOPIC_NAME_B = "msgB";

static const char* QosToString(QOS_VALUE qosValue)
{
    switch (qosValue)
    {
        case DELIVER_AT_LEAST_ONCE: return "Deliver_At_Least_Once";
        case DELIVER_EXACTLY_ONCE: return "Deliver_Exactly_Once";
        case DELIVER_AT_MOST_ONCE: return "Deliver_At_Most_Once";
        case DELIVER_FAILURE: return "Deliver_Failure";
    }
    return "";
}

void on_recv_callback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    (void)printf("Incoming Msg: Packet Id: %d\r\nQOS: %s\r\nTopic Name: %s\r\nIs Retained: %s\r\nIs Duplicate: %s\r\nApp Msg: ", mqttmessage_getPacketId(msgHandle),
                 QosToString(mqttmessage_getQosType(msgHandle) ),
                 mqttmessage_getTopicName(msgHandle),
                 mqttmessage_getIsRetained(msgHandle) ? "true" : "fale",
                 mqttmessage_getIsDuplicateMsg(msgHandle) ? "true" : "fale"
    );

    bool isValidUtf8 = utf8_checker_is_valid_utf8((unsigned char *)appMsg->message, appMsg->length);
    printf("content is valid UTF8:%s message length:%d\r\n", isValidUtf8? "true": "false", (int)appMsg->length);
    for (size_t index = 0; index < appMsg->length; index++)
    {
        if (isValidUtf8)
        {
            (void)printf("%c", appMsg->message[index]);
        }
        else
        {
            (void)printf("0x%x", appMsg->message[index]);
        }
    }

    (void)printf("\r\n");

    // when receive message is "stop", call destroy method to exit
    // trigger stop by send a message to topic "msgA" and payload with "stop"
    if (strcmp((const char *)appMsg->message, "stop") == 0)
    {
        iothub_mqtt_disconnect(clientHandle);
    }
}

int pub_least_ack_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    printf("call publish at least once handle\r\n");
    if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        printf("hub is connected\r\n");
    }

    if (status == MQTT_PUB_SUCCESS)
    {
        printf("received publish ack from mqtt server when deliver at least once message\r\n");
    }
    else
    {
        printf("fail to publish message to mqtt server\r\n");
    }

    return 0;
}

int pub_most_once_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    printf("call publish at most once handle\r\n");

    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        printf("hub is connected\r\n");
    }

    if (status == MQTT_PUB_SUCCESS)
    {
        printf("received publish ack from mqtt server when deliver at most once message\r\n");
    }
    else
    {
        printf("fail to publish message to mqtt server\r\n");
    }
    return 0;
}

int iothub_mqtt_client_run(void)
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return __FAILURE__;
    }
    else
    {
        MQTT_CLIENT_OPTIONS options = { 0 };
        options.clientId = "iotclient2017";
        options.willMessage = NULL;
        options.willTopic = NULL;
        options.username = USERNAME;
        options.password = PASSWORD;
        options.keepAliveInterval = 10;
        options.useCleanSession = true;
        options.qualityOfServiceValue = DELIVER_AT_MOST_ONCE;

        const char *endpoint = ENDPOINT;

        MQTT_CONNECTION_TYPE type;
        if (CONNECTION_TYPE == "TLS")
        {
             type = MQTT_CONNECTION_TLS;
        }
        else if (CONNECTION_TYPE == "MUTUAL_TLS")
        {
             type = MQTT_CONNECTION_MUTUAL_TLS;
        }



        IOTHUB_CLIENT_RETRY_POLICY retryPolicy = IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF;

        size_t retryTimeoutLimitInSeconds = 1000;

        IOTHUB_MQTT_CLIENT_HANDLE clientHandle = initialize_mqtt_client_handle(&options, endpoint, type, on_recv_callback,
                                                                               retryPolicy, retryTimeoutLimitInSeconds);

        if (clientHandle == NULL)
        {
            printf("Error: fail to initialize IOTHUB_MQTT_CLIENT_HANDLE");
            return 0;
        }

        int result = iothub_mqtt_doconnect(clientHandle, 60);

        if (result == __FAILURE__)
        {
            printf("fail to establish connection with server");
            return __FAILURE__;
        }

        SUBSCRIBE_PAYLOAD subscribe[2];
        subscribe[0].subscribeTopic = TOPIC_NAME_A;
        subscribe[0].qosReturn = DELIVER_AT_MOST_ONCE;
        subscribe[1].subscribeTopic = TOPIC_NAME_B;
        subscribe[1].qosReturn = DELIVER_AT_MOST_ONCE;

        subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), NULL, NULL);
        const char* publishData = "publish message to topic /china/sh.";

        result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_EXACTLY_ONCE, (const uint8_t*)publishData,
                             strlen(publishData), pub_least_ack_process , clientHandle);

        if (result == __FAILURE__) {
            printf("Does not support DELIVER_EXACTLY_ONCE\r\n");
        }

        result = publish_mqtt_message(clientHandle, "/china/bj", DELIVER_AT_MOST_ONCE, (const uint8_t*)publishData,
                             strlen(publishData), pub_most_once_process , clientHandle);

        result = publish_mqtt_message(clientHandle, "/china/bj", DELIVER_AT_MOST_ONCE, (const uint8_t*)publishData,
                                      strlen(publishData), NULL , NULL);

        result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData,
                                      strlen(publishData), pub_least_ack_process , clientHandle);

        TICK_COUNTER_HANDLE tickCounterHandle = tickcounter_create();
        tickcounter_ms_t currentTime, lastSendTime;
        tickcounter_get_current_ms(tickCounterHandle, &lastSendTime);
        bool needSubscribeTopic = false;
        do
        {
            iothub_mqtt_dowork(clientHandle);
            tickcounter_get_current_ms(tickCounterHandle, &currentTime);
            if (clientHandle->isConnectionLost && !needSubscribeTopic)
            {
                needSubscribeTopic = true;
            }

            if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED && needSubscribeTopic)
            {
                needSubscribeTopic = false;
                subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), NULL, NULL);
            }

            // send a publish message every 5 seconds
            if (!clientHandle->isConnectionLost && (currentTime - lastSendTime) / 1000 > 5)
            {
                result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData,
                                              strlen(publishData), pub_least_ack_process , clientHandle);
                lastSendTime = currentTime;
            }
            ThreadAPI_Sleep(10);
        } while (!clientHandle->isDestroyCalled && !clientHandle->isDisconnectCalled);

        iothub_mqtt_destroy(clientHandle);
        tickcounter_destroy(tickCounterHandle);
        return 0;
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

