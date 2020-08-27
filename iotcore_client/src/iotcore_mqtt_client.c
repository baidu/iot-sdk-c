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

#include "iotcore_mqtt_client.h"
#include "iotcore_param_util.h"
#include "iotcore_retry_logic.h"

#include <limits.h>

#include <azure_c_shared_utility/tlsio.h>
#include <azure_c_shared_utility/threadapi.h>
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include <azure_c_shared_utility/strings_types.h>
#include <azure_c_shared_utility/tickcounter.h>
#include <azure_c_shared_utility/doublylinkedlist.h>
#include "azure_umqtt_c/mqtt_client.h"
#include "certs.h"

#define MQTT_CONNECTION_TCP_PORT 1883
#define MQTT_CONNECTION_TLS_PORT 1884

#define MQTT_KEEP_ALIVE_INTERVAL    20

typedef struct MQTT_PUB_CALLBACK_INFO_TAG
{
    uint16_t packet_id;
    PUB_CALLBACK pub_callback;
    MQTT_MESSAGE_HANDLE msg_handle;
    void *context;
    DLIST_ENTRY entry;
} MQTT_PUB_CALLBACK_INFO,* PMQTT_PUB_CALLBACK_INFO;

typedef struct MQTT_SUB_CALLBACK_INFO_TAG
{
    uint16_t packet_id;
    SUB_CALLBACK sub_callback;
    void *context;
    DLIST_ENTRY entry;
} MQTT_SUB_CALLBACK_INFO,* PMQTT_SUB_CALLBACK_INFO;

typedef struct IOTCORE_MQTT_CLIENT_TAG
{
    MQTT_CONNECTION_TYPE conn_type;
    char* endpoint;
    char* client_cert;
    char* client_key;
    MQTT_CLIENT_OPTIONS* options;

    // Protocol
    MQTT_CLIENT_HANDLE mqtt_client;
    XIO_HANDLE xio_transport;

    // Session - connection
    uint16_t packet_id;

    // Connection state control
    IOTCORE_MQTT_STATUS mqtt_client_status;

    tickcounter_ms_t mqtt_connect_time;
    size_t connect_fail_count;
    tickcounter_ms_t connect_tick;
    TICK_COUNTER_HANDLE msg_tick_counter;

    //Retry Logic
    RETRY_LOGIC* retry_logic;

    // Set this member variable to pass additional structure for user defined callback handle
    void* callback_context;

    // pub ack wait queue
    DLIST_ENTRY pub_ack_waiting_queue;

    // sub ack wait queue
    DLIST_ENTRY sub_ack_waiting_queue;

    // subscribe message callback handle
    RECV_MSG_CALLBACK recv_callback;
} IOTCORE_MQTT_CLIENT;


static uint16_t get_next_packet_id(IOTCORE_MQTT_CLIENT_HANDLE transport_data)
{
    if (transport_data->packet_id+1 >= USHRT_MAX)
    {
        transport_data->packet_id = 1;
    }
    else
    {
        transport_data->packet_id++;
    }
    return transport_data->packet_id;
}

static const char* retrieve_mqtt_return_codes(CONNECT_RETURN_CODE rtn_code)
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

static void on_mqtt_operation_complete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_RESULT action_result, const void* msg_info, void* callback_ctx)
{
    (void)msg_info;
    IOTCORE_MQTT_CLIENT_HANDLE _iotcore_client = (IOTCORE_MQTT_CLIENT_HANDLE)callback_ctx;
    switch (action_result)
    {
        case MQTT_CLIENT_ON_CONNACK:
        {
            const CONNECT_ACK* connack = (const CONNECT_ACK*)msg_info;
            if (connack != NULL)
            {
                if (connack->returnCode == CONNECTION_ACCEPTED)
                {
                    // The connect packet has been acked
                    _iotcore_client->mqtt_client_status.is_recoverable_error = true;
                    _iotcore_client->mqtt_client_status.is_connection_lost = false;
                    _iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_CONNECTED;
                    stop_retry_timer(_iotcore_client->retry_logic);
                }
                else
                {
                    _iotcore_client->mqtt_client_status.is_connection_lost = true;
                    if (connack->returnCode == CONN_REFUSED_BAD_USERNAME_PASSWORD)
                    {
                        _iotcore_client->mqtt_client_status.is_recoverable_error = false;
                    }
                    else if (connack->returnCode == CONN_REFUSED_NOT_AUTHORIZED)
                    {
                        _iotcore_client->mqtt_client_status.is_recoverable_error = false;
                    }
                    else if (connack->returnCode == CONN_REFUSED_UNACCEPTABLE_VERSION)
                    {
                        _iotcore_client->mqtt_client_status.is_recoverable_error = false;
                    }
                    LogError("Connection Not Accepted: 0x%x: %s", connack->returnCode, retrieve_mqtt_return_codes(connack->returnCode));
                    (void)mqtt_client_disconnect(_iotcore_client->mqtt_client, NULL, NULL);
                    _iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED;
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
            const SUBSCRIBE_ACK* suback = (const SUBSCRIBE_ACK*)msg_info;
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

                PDLIST_ENTRY _current_list_entry = _iotcore_client->sub_ack_waiting_queue.Flink;
                // when ack_waiting_queue.Flink points to itself, return directly
                while (_current_list_entry != &_iotcore_client->sub_ack_waiting_queue)
                {
                    PMQTT_SUB_CALLBACK_INFO _sub_handle_entry = containingRecord(_current_list_entry, MQTT_SUB_CALLBACK_INFO, entry);
                    DLIST_ENTRY _save_list_entry;
                    _save_list_entry.Flink = _current_list_entry->Flink;

                    if (suback->packetId == _sub_handle_entry->packet_id)
                    {
                        (void)DList_RemoveEntryList(_current_list_entry); //First remove the item from Waiting for Ack List.
                        _sub_handle_entry->sub_callback((IOTCORE_MQTT_QOS*)(suback->qosReturn), suback->qosCount, _sub_handle_entry->context);
                        free(_sub_handle_entry);
                        break;
                    }
                    _current_list_entry = _save_list_entry.Flink;
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
            const PUBLISH_ACK* puback = (const PUBLISH_ACK*)msg_info;
            if (puback != NULL)
            {
                PDLIST_ENTRY _current_list_entry = _iotcore_client->pub_ack_waiting_queue.Flink;
                // when ack_waiting_queue.Flink points to itself, return directly
                while (_current_list_entry != &_iotcore_client->pub_ack_waiting_queue)
                {
                    PMQTT_PUB_CALLBACK_INFO _mqtt_msg_entry = containingRecord(_current_list_entry, MQTT_PUB_CALLBACK_INFO, entry);
                    DLIST_ENTRY _save_list_entry;
                    _save_list_entry.Flink = _current_list_entry->Flink;

                    if (puback->packetId == _mqtt_msg_entry->packet_id)
                    {
                        (void)DList_RemoveEntryList(_current_list_entry); //First remove the item from Waiting for Ack List.
                        _mqtt_msg_entry->pub_callback(MQTT_PUB_SUCCESS, _mqtt_msg_entry->context);
                        // release mqtt message memory
                        mqttmessage_destroy(_mqtt_msg_entry->msg_handle);
                        free(_mqtt_msg_entry);
                        break;
                    }
                    _current_list_entry = _save_list_entry.Flink;
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
            _iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED;
            _iotcore_client->mqtt_client_status.is_disconnect_called = true;
            break;
        }
        case MQTT_CLIENT_ON_UNSUBSCRIBE_ACK:
        {
            const UNSUBSCRIBE_ACK* unsuback = (const UNSUBSCRIBE_ACK*)msg_info;
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
            (void)printf("unexpected value received for enumeration (%d)\n", (int)action_result);
        }
    }
}

IOTCORE_MQTT_STATUS iotcore_get_mqtt_status(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    return iotcore_client->mqtt_client_status;
}

int publish_mqtt_message(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* pub_topic_name,
                         IOTCORE_MQTT_QOS qos_value, const uint8_t* pub_msg, size_t pub_msg_length, PUB_CALLBACK pub_callback, void* context)
{
    int _result = 0;

    if ( qos_value == DELIVER_EXACTLY_ONCE)
    {
        LogError("Does not support qos = DELIVER_EXACTLY_ONCE");
        _result = IOTCORE_ERR_NOT_SUPPORT;
        return _result;
    }

    uint16_t packet_id = get_next_packet_id(iotcore_client);

    MQTT_MESSAGE_HANDLE mqtt_get_msg = mqttmessage_create(packet_id, pub_topic_name, (QOS_VALUE)qos_value, pub_msg, pub_msg_length);
    if (mqtt_get_msg == NULL)
    {
        LogError("Failed constructing mqtt message.");
        _result = IOTCORE_ERR_ERROR;
    }
    else
    {
        PMQTT_PUB_CALLBACK_INFO _pub_callback_handle = NULL;
        if (qos_value == DELIVER_AT_LEAST_ONCE && pub_callback != NULL)
        {
            _pub_callback_handle = (PMQTT_PUB_CALLBACK_INFO)malloc(sizeof(MQTT_PUB_CALLBACK_INFO));
            if (_pub_callback_handle == NULL)
            {
                LogError("Fail to allocate memory for MQTT_PUB_CALLBACK_INFO");
                mqttmessage_destroy(mqtt_get_msg);
                _result = IOTCORE_ERR_OUT_OF_MEMORY;
                return _result;
            }
            _pub_callback_handle->packet_id = packet_id;
            _pub_callback_handle->msg_handle = mqtt_get_msg;
            _pub_callback_handle->pub_callback = pub_callback;
            _pub_callback_handle->context = context;
            _pub_callback_handle->entry.Flink = NULL;
            _pub_callback_handle->entry.Blink = NULL;

            DList_InsertTailList(&iotcore_client->pub_ack_waiting_queue, &_pub_callback_handle->entry);
        }

        if (mqtt_client_publish(iotcore_client->mqtt_client, mqtt_get_msg) != 0)
        {
            // release memory for pub callback handle
            if (_pub_callback_handle != NULL)
            {
                DList_RemoveEntryList(&_pub_callback_handle->entry);
                free(_pub_callback_handle);
                _pub_callback_handle = NULL;
            }
            // Call callback handle directly
            if (pub_callback != NULL)
            {
                pub_callback(MQTT_PUB_FAILED, context);
            }
            LogError("Failed publishing to mqtt client.");
            _result = IOTCORE_ERR_ERROR;
        }
        else
        {
            if (qos_value == DELIVER_AT_MOST_ONCE && pub_callback != NULL)
            {
                pub_callback(MQTT_PUB_SUCCESS, context);
            }
        }
        
        if (_pub_callback_handle == NULL || qos_value == DELIVER_AT_MOST_ONCE)
        {
            mqttmessage_destroy(mqtt_get_msg);
        }
    }

    return _result;
}


int subscribe_mqtt_topic(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* sub_topic, 
    IOTCORE_MQTT_QOS ret_qos, SUB_CALLBACK sub_callback, void* context)
{
    int _result = 0;

    if (!sub_topic) {
        _result = IOTCORE_ERR_INVALID_PARAMETER;
        LogError("Failure: maybe sub_topic is invalid");
    }
    else
    {
        uint16_t packet_id = get_next_packet_id(iotcore_client);
        if (sub_callback != NULL) {
            PMQTT_SUB_CALLBACK_INFO _sub_callback_handle = NULL;
            _sub_callback_handle = (PMQTT_SUB_CALLBACK_INFO)malloc(sizeof(MQTT_SUB_CALLBACK_INFO));
            if (_sub_callback_handle == NULL)
            {
                LogError("Fail to allocate memory for MQTT_PUB_CALLBACK_INFO");
                _result = IOTCORE_ERR_OUT_OF_MEMORY;
                return _result;
            }
            _sub_callback_handle->packet_id = packet_id;
            _sub_callback_handle->sub_callback = sub_callback;
            _sub_callback_handle->context = context;
            _sub_callback_handle->entry.Flink = NULL;
            _sub_callback_handle->entry.Blink = NULL;

            DList_InsertTailList(&iotcore_client->sub_ack_waiting_queue, &_sub_callback_handle->entry);
        }

        SUBSCRIBE_PAYLOAD _sub_payload = { 0 };
        _sub_payload.subscribeTopic = sub_topic;
        _sub_payload.qosReturn = (QOS_VALUE)ret_qos;

        if (mqtt_client_subscribe(iotcore_client->mqtt_client, packet_id, &_sub_payload, 1) != 0)
        {
            LogError("Failure: mqtt_client_subscribe returned error.");
            _result = IOTCORE_ERR_ERROR;
        }
    }

    return _result;
}

int unsubscribe_mqtt_topics(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* unsubscribe)
{
    int _result = 0;

    do
    {
        if (unsubscribe == NULL)
        {
            LogError("Failure: maybe unsubscribe is invalid.");
            _result = IOTCORE_ERR_INVALID_PARAMETER;
            break;
        }

        const char* unsubscribe_list[1];
        unsubscribe_list[0] = unsubscribe;
        if (mqtt_client_unsubscribe(iotcore_client->mqtt_client, 
            get_next_packet_id(iotcore_client), unsubscribe_list, 1))
        {
            LogError("Failure: mqtt_client_unsubscribe returned error.");
            _result = IOTCORE_ERR_ERROR;
        }
    } while(0);

    return _result;
}

static void notify_subscribe_ack_failure(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client) {
    LogError("notify all waiting for subscribe ack messages as failure");
    PDLIST_ENTRY _current_list_entry = iotcore_client->sub_ack_waiting_queue.Flink;
    // when ack_waiting_queue.Flink points to itself, return directly
    while (_current_list_entry != &iotcore_client->sub_ack_waiting_queue) {
        PMQTT_SUB_CALLBACK_INFO _sub_ack_handle_entry = 
            containingRecord(_current_list_entry, MQTT_SUB_CALLBACK_INFO, entry);
        DLIST_ENTRY _save_list_entry;
        _save_list_entry.Flink = _current_list_entry->Flink;

        _sub_ack_handle_entry->sub_callback(NULL, 0, _sub_ack_handle_entry->context);
        free(_sub_ack_handle_entry);
        (void)DList_RemoveEntryList(_current_list_entry); //First remove the item from Waiting for Ack List.
        _current_list_entry = _save_list_entry.Flink;
    }
}

static void notify_publish_ack_failure(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    LogError("notify all waiting for publish ack messages as failure");
    PDLIST_ENTRY _current_list_entry = iotcore_client->pub_ack_waiting_queue.Flink;
    // when ack_waiting_queue.Flink points to itself, return directly
    while (_current_list_entry != &iotcore_client->pub_ack_waiting_queue)
    {
        PMQTT_PUB_CALLBACK_INFO _mqtt_msg_entry = containingRecord(_current_list_entry, MQTT_PUB_CALLBACK_INFO, entry);
        DLIST_ENTRY _save_list_entry;
        _save_list_entry.Flink = _current_list_entry->Flink;

        (void)DList_RemoveEntryList(_current_list_entry); //First remove the item from Waiting for Ack List.
        _mqtt_msg_entry->pub_callback(MQTT_PUB_FAILED, _mqtt_msg_entry->context);
        mqttmessage_destroy(_mqtt_msg_entry->msg_handle);
        free(_mqtt_msg_entry);
        _current_list_entry = _save_list_entry.Flink;
    }
}

static void on_mqtt_error_complete(MQTT_CLIENT_HANDLE handle, MQTT_CLIENT_EVENT_ERROR error, void* callback_ctx)
{
    IOTCORE_MQTT_CLIENT_HANDLE iotcore_client = (IOTCORE_MQTT_CLIENT_HANDLE)callback_ctx;
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
    iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED;
    iotcore_client->mqtt_client_status.is_connection_lost = true;
    // enumerate all waiting for sub ack handle
    notify_subscribe_ack_failure(iotcore_client);
    // enumerate all waiting for ack publish message and send them a failure notification
    notify_publish_ack_failure(iotcore_client);
}

static void _clear_mqtt_options(MQTT_CLIENT_OPTIONS* mqtt_options)
{
    free(mqtt_options->clientId);
    mqtt_options->clientId = NULL;
    free(mqtt_options->willTopic);
    mqtt_options->willTopic = NULL;
    free(mqtt_options->willMessage);
    mqtt_options->willMessage = NULL;
    free(mqtt_options->username);
    mqtt_options->username = NULL;
    free(mqtt_options->password);
    mqtt_options->password = NULL;
    free(mqtt_options);
}

static void _init_mqtt_options(MQTT_CLIENT_OPTIONS* mqtt_options)
{
    mqtt_options->clientId = NULL;
    mqtt_options->willTopic = NULL;
    mqtt_options->willMessage = NULL;
    mqtt_options->username = NULL;
    mqtt_options->password = NULL;
    mqtt_options->keepAliveInterval = 10;
    mqtt_options->messageRetain = false;
    mqtt_options->useCleanSession = true;
    mqtt_options->qualityOfServiceValue = DELIVER_AT_MOST_ONCE;
    mqtt_options->log_trace = false;
}

static MQTT_CLIENT_OPTIONS* _construct_mqtt_options(
    const char* client_id,
    const char* username,
    const char* passwd,
    const char* will_topic,
    const char* will_payload,
    int keep_alive_interval,
    bool message_retain,
    bool use_clean_session,
    bool log_trace,
    IOTCORE_MQTT_QOS qos
) 
{
    // will_topic and will_payload might be NULL
    if (!client_id || !username || !passwd) 
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "invalid param");
        return NULL;
    }
    
    // will_topic and will_payload should be NULL or not NULL at the same time
    if ((will_topic && !will_payload) || (!will_topic && will_payload))
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "invalid param");
        return NULL;
    }

    int _need_clean = 1;
    MQTT_CLIENT_OPTIONS* _options = (MQTT_CLIENT_OPTIONS *)malloc(sizeof(MQTT_CLIENT_OPTIONS));

    do 
    {
        if (_options == NULL) {
            LOG(AZ_LOG_ERROR, LOG_LINE, "malloc MQTT_CLIENT_OPTIONS failed");
            break;
        }

        _init_mqtt_options(_options);

        if (mallocAndStrcpy_s(&(_options->clientId), client_id) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s clientId");
            break;
        }

        if (mallocAndStrcpy_s(&(_options->username), username) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s username");
            break;
        }

        if (mallocAndStrcpy_s(&(_options->password), passwd) != 0)
        {
            LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s password");
            break;
        }

        if (will_topic && will_payload) 
        {
            if (mallocAndStrcpy_s(&(_options->willTopic), will_topic) != 0)
            {
                LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s willTopic");
                break;
            }

            if (mallocAndStrcpy_s(&(_options->willMessage), will_payload) != 0)
            {
                LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s willMessage");
                break;
            }
        }

        _options->keepAliveInterval = keep_alive_interval;
        _options->messageRetain = message_retain;
        _options->useCleanSession = use_clean_session;
        _options->qualityOfServiceValue = (QOS_VALUE)qos;
        _options->log_trace = log_trace;

        _need_clean = 0;
    } while (0);

    if (_need_clean) {
        _clear_mqtt_options(_options);
        _options = NULL;
    }
    return _options;
}

static XIO_HANDLE create_tcp_connection(const char *endpoint)
{
    SOCKETIO_CONFIG config = {endpoint, MQTT_CONNECTION_TCP_PORT, NULL};

    XIO_HANDLE xio = xio_create(socketio_get_interface_description(), &config);

    return xio;
}

static XIO_HANDLE create_tls_connection(const char *endpoint)
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

static XIO_HANDLE create_mutual_tls_connection(
    const char *endpoint, const char *client_cert_in_option, const char *client_key_in_option)
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

static void disconnect_from_client(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    (void)mqtt_client_disconnect(iotcore_client->mqtt_client, NULL, NULL);
    xio_destroy(iotcore_client->xio_transport);
    iotcore_client->xio_transport = NULL;

    iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED;
}

static int send_mqtt_connect_message(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    int _result = 0;

	if (!iotcore_client->xio_transport) 
	{
		switch (iotcore_client->conn_type)
		{
			case MQTT_CONNECTION_TCP:
				iotcore_client->xio_transport = create_tcp_connection(iotcore_client->endpoint);
				break;
			case MQTT_CONNECTION_TLS:
				iotcore_client->xio_transport = create_tls_connection(iotcore_client->endpoint);
				break;
			case MQTT_CONNECTION_MUTUAL_TLS:
				iotcore_client->xio_transport = create_mutual_tls_connection(iotcore_client->endpoint,
																	   iotcore_client->client_cert,
																	   iotcore_client->client_key);
		}
	}

    if (iotcore_client->xio_transport == NULL)
    {
        LogError("failed to create connection with server");
        // TODO: add code to handle release data for IOTCORE_MQTT_CLIENT_HANDLE
        _result = IOTCORE_ERR_ERROR;
    }
    else
    {
        if (mqtt_client_connect(iotcore_client->mqtt_client, iotcore_client->xio_transport,
            iotcore_client->options) != 0)
        {
            LogError("failed to initialize mqtt connection with server");
            // TODO: add code to handle release data for IOTCORE_MQTT_CLIENT_HANDLE
            _result = IOTCORE_ERR_ERROR;
        }
        else
        {
            (void)tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &(iotcore_client->mqtt_connect_time));
            _result = 0;
        }
    }

    return _result;
}

static iot_bool _is_iotcore_connect_param_valid(const IOTCORE_CONNECT_MQTT_PARAM *param) 
{
    int _is_valid = IOT_FALSE;
    if (param && param->broker_addr && param->broker_addr[0]
        && param->client_id && param->client_id[0]
        && param->passwd && param->passwd[0]
        && param->user_name && param->user_name[0])
    {
        _is_valid = IOT_TRUE;
    }
    return _is_valid;
}

static void on_recv_callback(MQTT_MESSAGE_HANDLE msg_handle, void* context)
{
    const APP_PAYLOAD* pub_msg = mqttmessage_getApplicationMsg(msg_handle);
    IOTCORE_MQTT_CLIENT_HANDLE _client_handle = (IOTCORE_MQTT_CLIENT_HANDLE)context;

    if (!_client_handle->recv_callback)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "get recv callback failed");
        return;
    }
    _client_handle->recv_callback(
        mqttmessage_getPacketId(msg_handle), 
        (IOTCORE_MQTT_QOS)mqttmessage_getQosType(msg_handle), 
        mqttmessage_getTopicName(msg_handle),
        (const IOTCORE_MQTT_PAYLOAD*)mqttmessage_getApplicationMsg(msg_handle),
        mqttmessage_getIsDuplicateMsg(msg_handle)
    );
}

IOTCORE_MQTT_CLIENT_HANDLE initialize_mqtt_client_handle(
    const IOTCORE_INFO* info, const char* will_topic, const char* will_payload,
    MQTT_CONNECTION_TYPE conn_type, RECV_MSG_CALLBACK callback,
    IOTCORE_RETRY_POLICY retry_policy, size_t retry_timeout_limit_in_sec)
{
    //IOTCORE_INFO info = {"axhyvwu", "test_dev", "sPmyrDmZmWIJWlhw", "axhyvwu.iot.gz.baidubce.com"};
    IOTCORE_MQTT_CLIENT_HANDLE _iotcore_client = (IOTCORE_MQTT_CLIENT_HANDLE)malloc(sizeof(IOTCORE_MQTT_CLIENT));

    if (_iotcore_client == NULL) {
        LOG(AZ_LOG_ERROR, LOG_LINE, "malloc IOTCORE_MQTT_CLIENT failed");
        return NULL;
    }
    memset(_iotcore_client, 0, sizeof(IOTCORE_MQTT_CLIENT));

    // construct mqtt connection parameter
    IOTCORE_CONNECT_MQTT_PARAM param;
    iotcore_construct_connect_mqtt_param(info, &param);

    // check param valid
    if (_is_iotcore_connect_param_valid(&param) == IOT_FALSE)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "mqtt connection param invalid");
        return NULL;
    }

    if ((_iotcore_client->msg_tick_counter = tickcounter_create()) == NULL)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "fail to create tickCounter");
        free(_iotcore_client);
        return NULL;
    }

    _iotcore_client->options = _construct_mqtt_options(
        param.client_id, param.user_name, param.passwd, will_topic, will_payload, 
        MQTT_KEEP_ALIVE_INTERVAL, false, true, false, QOS_1_AT_LEAST_ONCE);
    if (!_iotcore_client->options) 
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "construct mqtt options failed");
        return NULL;
    }

    if (mallocAndStrcpy_s(&(_iotcore_client->endpoint), param.broker_addr) != 0)
    {
        LOG(AZ_LOG_ERROR, LOG_LINE, "mallocAndStrcpy_s endpoint");
        tickcounter_destroy(_iotcore_client->msg_tick_counter);
        _clear_mqtt_options(_iotcore_client->options);
        free(_iotcore_client);
        return NULL;
    }

    _iotcore_client->conn_type = conn_type;
    _iotcore_client->recv_callback = callback;

    // set callback context to mqttClientHandle, since this handle has all required data to process message
    _iotcore_client->callback_context = _iotcore_client;

    _iotcore_client->mqtt_client = mqtt_client_init(on_recv_callback, on_mqtt_operation_complete, 
        _iotcore_client, on_mqtt_error_complete, _iotcore_client);

    if(_iotcore_client->mqtt_client == NULL)
    {
        LogError("failure initializing mqtt client.");
        tickcounter_destroy(_iotcore_client->msg_tick_counter);
        _clear_mqtt_options(_iotcore_client->options);
        free(_iotcore_client->endpoint);
        free(_iotcore_client);
        return NULL;
    }

    DList_InitializeListHead(&_iotcore_client->pub_ack_waiting_queue);
    DList_InitializeListHead(&_iotcore_client->sub_ack_waiting_queue);
    _iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED;
    _iotcore_client->mqtt_client_status.is_recoverable_error = true;
    _iotcore_client->retry_logic = create_retry_logic(retry_policy, retry_timeout_limit_in_sec);
    _iotcore_client->packet_id = 0;
    _iotcore_client->xio_transport = NULL;

    return _iotcore_client;
}

void set_client_cert(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* client_cert, const char* client_key) {
    iotcore_client->client_cert = (char *) client_cert;
    iotcore_client->client_key = (char *) client_key;
}

int initialize_mqtt_connection(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    int _result = 0;

    // Make sure we're not destroying the object
    if (!iotcore_client->mqtt_client_status.is_destroy_called)
    {
        // If we are MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED then check to see if we need
        // to back off the connecting to the server
        if (!iotcore_client->mqtt_client_status.is_disconnect_called
            && iotcore_client->mqtt_client_status.connect_status == MQTT_CLIENT_CONNECT_STATUS_NOT_CONNECTED
            && IOT_TRUE == can_retry(iotcore_client->retry_logic))
        {
            if (tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &iotcore_client->connect_tick) != 0)
            {
                iotcore_client->connect_fail_count++;
                _result = IOTCORE_ERR_ERROR;
            }
            else
            {
                if (iotcore_client->mqtt_client_status.is_connection_lost 
                    && iotcore_client->mqtt_client_status.is_recoverable_error) {
                    xio_destroy(iotcore_client->xio_transport);
                    iotcore_client->xio_transport = NULL;
                    mqtt_client_deinit(iotcore_client->mqtt_client);
                    iotcore_client->mqtt_client = mqtt_client_init(on_recv_callback, on_mqtt_operation_complete,
                                                                iotcore_client, on_mqtt_error_complete, iotcore_client);
                }

                if (send_mqtt_connect_message(iotcore_client) != 0)
                {
                    iotcore_client->connect_fail_count++;

                    _result = IOTCORE_ERR_ERROR;
                }
                else
                {
                    iotcore_client->mqtt_client_status.connect_status = MQTT_CLIENT_CONNECT_STATUS_CONNECTING;
                    iotcore_client->connect_fail_count = 0;
                    _result = 0;
                }
            }
        }
        // Codes_SRS_IOTCORE_TRANSPORT_MQTT_COMMON_09_001: [ iotcoreTransport_MQTT_Common_DoWork shall trigger reconnection if the mqtt_client_connect does not complete within `keepalive` seconds]
        else if (iotcore_client->mqtt_client_status.connect_status == MQTT_CLIENT_CONNECT_STATUS_CONNECTING)
        {
            tickcounter_ms_t current_time;
            if (tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &current_time) != 0)
            {
                LogError("failed verifying MQTT_CLIENT_CONNECT_STATUS_CONNECTING timeout");
                _result = IOTCORE_ERR_ERROR;
            }
            else if ((current_time - iotcore_client->mqtt_connect_time) / 1000 > iotcore_client->options->keepAliveInterval)
            {
                LogError("mqtt_client timed out waiting for CONNACK");
                disconnect_from_client(iotcore_client);
                _result = IOTCORE_ERR_ERROR;
            }
        }
        else if (iotcore_client->mqtt_client_status.connect_status == MQTT_CLIENT_CONNECT_STATUS_CONNECTED)
        {
            // We are connected and not being closed
            tickcounter_ms_t current_time;
            if (tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &current_time) != 0)
            {
                iotcore_client->connect_fail_count++;
                _result = IOTCORE_ERR_ERROR;
            }
        }
    }
    return _result;
}

// timeout is used to set timeout window for connection in seconds
int iotcore_mqtt_doconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, size_t timeout)
{
    int _result = 0;
    tickcounter_ms_t currentTime, lastSendTime;
    tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &lastSendTime);

    do
    {
        iotcore_mqtt_dowork(iotcore_client);
        ThreadAPI_Sleep(10);
        tickcounter_get_current_ms(iotcore_client->msg_tick_counter, &currentTime);
    } while (iotcore_client->mqtt_client_status.connect_status != MQTT_CLIENT_CONNECT_STATUS_CONNECTED
             && iotcore_client->mqtt_client_status.is_recoverable_error
             && (currentTime - lastSendTime) / 1000 <= timeout);

    if ((currentTime - lastSendTime) / 1000 > timeout)
    {
        LogError("failed to waitfor connection complete in %d seconds", timeout);
        _result = IOTCORE_ERR_ERROR;
        return _result;
    }

    if (iotcore_client->mqtt_client_status.connect_status == MQTT_CLIENT_CONNECT_STATUS_CONNECTED)
    {
        _result = 0;
    }
    else
    {
        _result = IOTCORE_ERR_NOT_CONNECT;
    }

    return _result;
}

void iotcore_mqtt_dowork(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    if (initialize_mqtt_connection(iotcore_client) != 0)
    {
        LogError("fail to establish connection with server");
    }
    else
    {
        mqtt_client_dowork(iotcore_client->mqtt_client);
    }
}

int iotcore_mqtt_disconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    int _result = mqtt_client_disconnect(iotcore_client->mqtt_client, NULL, NULL);
    iotcore_client->mqtt_client_status.is_disconnect_called = true;
    return _result;
}

void iotcore_mqtt_destroy(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client)
{
    if (iotcore_client != NULL)
    {
        iotcore_client->mqtt_client_status.is_destroy_called = true;
        iotcore_client->mqtt_client_status.is_connection_lost = true;

        disconnect_from_client(iotcore_client);

        free(iotcore_client->endpoint);
        // don't set iotcore_client->endpoint, free iotcore_client at end of this function
        // iotcore_client->endpoint = NULL;
        mqtt_client_deinit(iotcore_client->mqtt_client);
        // iotcore_client->mqtt_client = NULL;
        _clear_mqtt_options(iotcore_client->options);

        tickcounter_destroy(iotcore_client->msg_tick_counter);
        destroy_retry_logic(iotcore_client->retry_logic);

        free(iotcore_client);
    }
}
