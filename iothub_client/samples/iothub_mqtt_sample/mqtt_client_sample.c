// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <azure_c_shared_utility/tlsio.h>
#include <azure_c_shared_utility/wsio.h>
#include "mqtt_client_sample.h"
#include "azure_umqtt_c/mqtt_client.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxxxx"

static const char* TOPIC_NAME_A = "msgA";
static const char* TOPIC_NAME_B = "msgB";
static const char* TOPIC_NAME_ACK = "ackTopic";
static const char* APP_NAME_A = "This is the app msg A.";
static const char* PUBLISH_ACK_MESSAGE = "This is ack message when receive message.";

static uint16_t PACKET_ID_VALUE = 11;
static bool g_continue = true;

#define PORT_NUM_UNENCRYPTED        1883
#define PORT_NUM_ENCRYPTED          8883
#define PORT_NUM_ENCRYPTED_CERT     8884

#define DEFAULT_MSG_TO_SEND         1

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

typedef struct Mqtt_Message_Context_TAG {
    MQTT_CLIENT_HANDLE  handle;
} Mqtt_Context, *Mqtt_Context_Handle;


static void sendMessage(MQTT_CLIENT_HANDLE handle) {
    MQTT_MESSAGE_HANDLE msg = mqttmessage_create(PACKET_ID_VALUE++, TOPIC_NAME_ACK, DELIVER_AT_MOST_ONCE, (const uint8_t*)PUBLISH_ACK_MESSAGE, strlen(PUBLISH_ACK_MESSAGE));
    if (msg == NULL)
    {
        (void)printf("%d: mqttmessage_create failed\r\n", __LINE__);
        g_continue = false;
    }
    else
    {
        if (mqtt_client_publish(handle, msg))
        {
            (void)printf("%d: mqtt_client_publish failed\r\n", __LINE__);
            g_continue = false;
        }
        mqttmessage_destroy(msg);
    }
}

static void OnRecvCallback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    MQTT_CLIENT_HANDLE handle = ((Mqtt_Context_Handle)context)->handle;
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);

    (void)printf("Incoming Msg: Packet Id: %d\r\nQOS: %s\r\nTopic Name: %s\r\nIs Retained: %s\r\nIs Duplicate: %s\r\nApp Msg: ", mqttmessage_getPacketId(msgHandle),
        QosToString(mqttmessage_getQosType(msgHandle) ),
        mqttmessage_getTopicName(msgHandle),
        mqttmessage_getIsRetained(msgHandle) ? "true" : "fale",
        mqttmessage_getIsDuplicateMsg(msgHandle) ? "true" : "fale"
        );
    for (size_t index = 0; index < appMsg->length; index++)
    {
        (void)printf("0x%x", appMsg->message[index]);
    }

    (void)printf("\r\n");

    sendMessage(handle);
}

static void OnCloseComplete(void* context)
{
    (void)context;

    (void)printf("%d: On Close Connection failed\r\n", __LINE__);
}

static void OnOperationComplete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_RESULT actionResult, const void* msgInfo, void* callbackCtx)
{
    (void)msgInfo;
    (void)callbackCtx;
    switch (actionResult)
    {
        case MQTT_CLIENT_ON_CONNACK:
        {
            (void)printf("ConnAck function called\r\n");

            SUBSCRIBE_PAYLOAD subscribe[2];
            subscribe[0].subscribeTopic = TOPIC_NAME_A;
            subscribe[0].qosReturn = DELIVER_AT_MOST_ONCE;
            subscribe[1].subscribeTopic = TOPIC_NAME_B;
            subscribe[1].qosReturn = DELIVER_AT_MOST_ONCE;

            if (mqtt_client_subscribe(handle, PACKET_ID_VALUE++, subscribe, sizeof(subscribe) / sizeof(subscribe[0])) != 0)
            {
                (void)printf("%d: mqtt_client_subscribe failed\r\n", __LINE__);
                g_continue = false;
            }
            break;
        }
        case MQTT_CLIENT_ON_SUBSCRIBE_ACK:
        {
            MQTT_MESSAGE_HANDLE msg = mqttmessage_create(PACKET_ID_VALUE++, TOPIC_NAME_A, DELIVER_AT_MOST_ONCE, (const uint8_t*)APP_NAME_A, strlen(APP_NAME_A));
            if (msg == NULL)
            {
                (void)printf("%d: mqttmessage_create failed\r\n", __LINE__);
                g_continue = false;
            }
            else
            {
                if (mqtt_client_publish(handle, msg))
                {
                    (void)printf("%d: mqtt_client_publish failed\r\n", __LINE__);
                    g_continue = false;
                }
                mqttmessage_destroy(msg);
            }
            // Now send a message that will get 
            break;
        }
        case MQTT_CLIENT_ON_PUBLISH_ACK:
        {
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
            g_continue = false;
            break;
        case MQTT_CLIENT_ON_UNSUBSCRIBE_ACK:
        {
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

static void OnErrorComplete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_ERROR error, void* callbackCtx)
{
    (void)callbackCtx;
    (void)handle;
    switch (error)
    {
    case MQTT_CLIENT_CONNECTION_ERROR:
    case MQTT_CLIENT_PARSE_ERROR:
    case MQTT_CLIENT_MEMORY_ERROR:
    case MQTT_CLIENT_COMMUNICATION_ERROR:
    case MQTT_CLIENT_NO_PING_RESPONSE:
    case MQTT_CLIENT_UNKNOWN_ERROR:
        g_continue = false;
        break;
    }
}

static XIO_HANDLE create_tcp_connection(const char *endpoint)
{
    SOCKETIO_CONFIG config = {endpoint, 1883, NULL};

    XIO_HANDLE xio = xio_create(socketio_get_interface_description(), &config);

    return xio;
}

static XIO_HANDLE create_tls_connection(const char *endpoint)
{
    TLSIO_CONFIG tlsio_config = { endpoint, 1884 };

    XIO_HANDLE xio = xio_create(platform_get_default_tlsio(), &tlsio_config);

    return xio;
}

static XIO_HANDLE create_wss_connection(const char *endpoint)
{

    const char *fully_qualified_name = endpoint;
    int port = 8884;
    const IO_INTERFACE_DESCRIPTION* io_interface_description = wsio_get_interface_description();
    TLSIO_CONFIG tls_io_config;

    WSIO_CONFIG ws_io_config;

    ws_io_config.hostname = fully_qualified_name;
    ws_io_config.port = port;
    ws_io_config.protocol = "MQTT";
    ws_io_config.resource_name = "/mqtt";
    ws_io_config.underlying_io_interface = platform_get_default_tlsio();

    ws_io_config.underlying_io_parameters = &tls_io_config;

    tls_io_config.hostname = fully_qualified_name;
    tls_io_config.port = port;
    tls_io_config.underlying_io_interface = NULL;
    tls_io_config.underlying_io_parameters = NULL;

    XIO_HANDLE  xio = xio_create(io_interface_description, &ws_io_config);
    return xio;
}


void mqtt_client_sample_run()
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
    }
    else
    {
        Mqtt_Context_Handle contextHandle = (Mqtt_Context_Handle)malloc(sizeof(Mqtt_Context));
        MQTT_CLIENT_HANDLE mqttHandle = mqtt_client_init(OnRecvCallback, OnOperationComplete, contextHandle, OnErrorComplete, contextHandle);
        if (mqttHandle == NULL)
        {
            (void)printf("mqtt_client_init failed\r\n");
        }
        else
        {
            contextHandle->handle = mqttHandle;

            const char *endpoint = ENDPOINT;

            MQTT_CLIENT_OPTIONS options = { 0 };
            options.clientId = "baiduiotclient";
            options.willMessage = NULL;
            options.willTopic = NULL;
            options.username = USERNAME;
            options.password = PASSWORD;
            options.keepAliveInterval = 10;
            options.useCleanSession = true;
            options.qualityOfServiceValue = DELIVER_AT_MOST_ONCE;

            // SOCKETIO_CONFIG config = {"127.0.0.1", PORT_NUM_UNENCRYPTED, NULL};

            // XIO_HANDLE xio = xio_create(socketio_get_interface_description(), &config);
            // XIO_HANDLE  xio = create_tcp_connection();
            XIO_HANDLE  xio = create_tls_connection(endpoint);
            // XIO_HANDLE  xio = create_wss_connection();
            if (xio == NULL)
            {
                (void)printf("xio_create failed\r\n");
            }
            else
            {
                if (mqtt_client_connect(mqttHandle, xio, &options) != 0)
                {
                    (void)printf("mqtt_client_connect failed\r\n");
                }
                else
                {
                    do
                    {
                        mqtt_client_dowork(mqttHandle);
                    } while (g_continue);
                }
                if (xio_close(xio, OnCloseComplete, NULL) != 0)
                {
                    (void)printf("xio_close failed\r\n");
                }
            }
            mqtt_client_deinit(mqttHandle);
        }
        platform_deinit();
        free(contextHandle);
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}
