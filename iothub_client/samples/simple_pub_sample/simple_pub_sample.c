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
#include <azure_c_shared_utility/socketio.h>
#include "iothub_mqtt_client.h"
#include "simple_pub_sample.h"

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
                 mqttmessage_getIsRetained(msgHandle) ? "true" : "false",
                 mqttmessage_getIsDuplicateMsg(msgHandle) ? "true" : "false"
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
        iothub_mqtt_destroy(clientHandle);
    }
}

int pub_least_ack_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        // printf("hub is connected\r\n");
    }

    if (status == MQTT_PUB_SUCCESS)
    {
        // printf("received publish ack from mqtt server when deliver at least once message\r\n");
    }
    else
    {
        printf("fail to publish message to mqtt server\r\n");
    }
    return 0;
}


int simple_pub_sample_run(
    const char * endpoint, 
    const char * username, 
    const char * password,
    const char * topic,
    const char * clientid,
    char useSsl)
{
    printf("endpoint:%s\r\nusername:%s\r\npassword:%s\r\ntopic:%s\r\nclientid:%s\r\nuseSsl=%d\r\n",
        endpoint, username, password, topic, clientid, useSsl);

    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return __FAILURE__;
    }
    else
    {
        MQTT_CLIENT_OPTIONS options = { 0 };
        options.clientId = (char*) clientid;
        options.willMessage = NULL;
        options.willTopic = NULL;
        options.username = (char*) username;
        options.password = (char*) password;
        options.keepAliveInterval = 10;
        options.useCleanSession = true;
        options.qualityOfServiceValue = DELIVER_AT_MOST_ONCE;

        MQTT_CONNECTION_TYPE type = useSsl == 1 ? MQTT_CONNECTION_TLS : MQTT_CONNECTION_TCP;

        IOTHUB_CLIENT_RETRY_POLICY retryPolicy = IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF;

        size_t retryTimeoutLimitInSeconds = 1000;

        IOTHUB_MQTT_CLIENT_HANDLE clientHandle = initialize_mqtt_client_handle(&options, endpoint, type, on_recv_callback,
                                                                               retryPolicy, retryTimeoutLimitInSeconds);

        
        do
        {
            iothub_mqtt_dowork(clientHandle);
        } while (clientHandle->mqttClientStatus != MQTT_CLIENT_STATUS_CONNECTED && clientHandle->isRecoverableError);

        
        TICK_COUNTER_HANDLE tickCounterHandle = tickcounter_create();
        tickcounter_ms_t currentTime, lastSendTime;
        tickcounter_get_current_ms(tickCounterHandle, &lastSendTime);

        do
        {
            iothub_mqtt_dowork(clientHandle);
            tickcounter_get_current_ms(tickCounterHandle, &currentTime);

            // send a publish message every 5 seconds
            if (!clientHandle->isConnectionLost && (currentTime - lastSendTime) / 1000 > 5)
            {
                time_t nowMs = get_time(NULL);

                char publishData[512];
                sprintf(publishData, "{\"ts\":%d}", (int)nowMs);
                printf("going to send message:%s\r\n", publishData);
                publish_mqtt_message(clientHandle, topic, DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData,
                                              strlen(publishData), pub_least_ack_process , clientHandle);
                lastSendTime = currentTime;
            }
            ThreadAPI_Sleep(50);
        } while (!clientHandle->isDestroyCalled);  // clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED &&

        tickcounter_destroy(tickCounterHandle);
        return 0;
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

