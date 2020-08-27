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

#include "iotcore_param_util.h"
#include "iotcore_type.h"
#include "_md5.h"

#include <stdio.h>  // sprintf
#include <stdlib.h> // NULL
#include <time.h>   // time_t

#include "azure_c_shared_utility/agenttime.h" // get_time

#define FREE_POINTER(ptr) if(ptr){free(ptr);ptr = NULL;}

// default connect param
// only support "thingidp" at present
static const char* s_adp_type = "thingidp";
// support "MD5" and "SHA256", default use "MD5"
static const char* s_algorithm_type = "MD5";

static iot_bool _iotcore_param_valid(const IOTCORE_INFO* info)
{
    if (!info || !info->device_key || !info->device_secret || !info->iot_core_id || !info->broker_addr)
    {
        return IOT_FALSE;
    }
    return IOT_TRUE;
}

int iotcore_construct_connect_mqtt_param(const IOTCORE_INFO* info, IOTCORE_CONNECT_MQTT_PARAM* mqtt_param)
{
    int _ret = IOTCORE_ERR_ERROR;
    char* _passwd_plain_data = NULL;

    do
    {
        if (!mqtt_param || IOT_FALSE == _iotcore_param_valid(info)) 
        {
            _ret = IOTCORE_ERR_INVALID_PARAMETER;
            break;
        }
        
        /**
         * construct user name
         */
        // username format: {adp_type}@{IoTCoreId}|{DeviceKey}|{timestamp}|{algorithm_type}
        time_t now_time = get_time(NULL);
        unsigned int _user_name_len_max =
            strlen(s_adp_type)          // adp_type len
            + 1                         // @
            + strlen(info->iot_core_id)  // IoTCoreId len
            + 1                         // |
            + strlen(info->device_key)   // DeviceKey len
            + 1                         // |
            + 10                        // unixtime len
            + 1                         // |
            + strlen(s_algorithm_type)  // algorithm_type len
            + 1                         // \0
            ;
        mqtt_param->user_name = (char*)malloc(_user_name_len_max);
        if (!mqtt_param->user_name)
        {
            _ret = IOTCORE_ERR_OUT_OF_MEMORY;
            break;
        }
        memset(mqtt_param->user_name, 0, _user_name_len_max);
        sprintf(mqtt_param->user_name, "%s@%s|%s|%ld|%s", 
            s_adp_type, info->iot_core_id, info->device_key, (long int)now_time, s_algorithm_type);
        // printf("**** \nuser name: %s\n****\n", mqtt_param->user_name);

        /**
         * construct passwd
         */
        // passwd plain data format: {device_key}&{timestamp}&{algorithm_type}{device_secret}
        int _passwd_plain_data_len_max =
            strlen(info->device_key)      // device_key len
            + 1                          // &
            + 10                         // unixtime len
            + 1                          // &
            + strlen(s_algorithm_type)   // algorithm_type len
            + strlen(info->device_secret) // device_secret len
            + 1                          // \0
            ;
        _passwd_plain_data = (char*)malloc(_passwd_plain_data_len_max);
        if (!_passwd_plain_data)
        {
            _ret = IOTCORE_ERR_OUT_OF_MEMORY;
            break;
        }
        memset(_passwd_plain_data, 0, _passwd_plain_data_len_max);
        sprintf(_passwd_plain_data, "%s&%ld&%s%s",
            info->device_key, (long int)now_time, s_algorithm_type, info->device_secret);
        // printf("**** \npasswd plain: %s\n****\n", _passwd_plain_data);

        // calculate passwd
        MD5_CTX _md5_ctx;
        unsigned char _md5_result[16] = { 0 };
        _MD5Init(&_md5_ctx);
        _MD5Update(&_md5_ctx, (unsigned char*)_passwd_plain_data, strlen(_passwd_plain_data));
        _MD5Final(&_md5_ctx, _md5_result);

        // TODO: to support sha256 algrithm
        mqtt_param->passwd = (char*)malloc(33); // MD5 passwd len has 32 characters
        if (!mqtt_param->passwd)
        {
            _ret = IOTCORE_ERR_OUT_OF_MEMORY;
            break;
        }
        memset(mqtt_param->passwd, 0, 33);
        for (int i = 0; i < 16; i++) {
            sprintf(mqtt_param->passwd + (i * 2), "%02x", _md5_result[i]);
        }
        // printf("**** \npasswd: %s\n****\n", mqtt_param->passwd);

        /**
         * construct client id
         */
        int _device_key_len = strlen(info->device_key);
        mqtt_param->client_id = (char*)malloc(_device_key_len + 1);
        if (!mqtt_param->client_id)
        {
            _ret = IOTCORE_ERR_OUT_OF_MEMORY;
            break;
        }
        memcpy(mqtt_param->client_id, info->device_key, _device_key_len);
        mqtt_param->client_id[_device_key_len] = '\0';
        // printf("**** \nclient_id: %s\n****\n", mqtt_param->client_id);

        /**
         * construct broker address
         */
        int _broker_addr_len = strlen(info->broker_addr);
        mqtt_param->broker_addr = (char*)malloc(_broker_addr_len + 1);
        if (!mqtt_param->broker_addr)
        {
            _ret = IOTCORE_ERR_OUT_OF_MEMORY;
            break;
        }
        memcpy(mqtt_param->broker_addr, info->broker_addr, _broker_addr_len);
        mqtt_param->broker_addr[_broker_addr_len] = '\0';
        // printf("**** \nbroker addr: %s\n****\n", mqtt_param->broker_addr);

        _ret = IOTCORE_ERR_OK;
    }while (0);

    FREE_POINTER(_passwd_plain_data)

    if (_ret != IOTCORE_ERR_OK)
    {
        iotcore_clear_connect_mqtt_param(mqtt_param);
    }
    return _ret;
}

// not free IOTCORE_CONNECT_MQTT_PARAM object self
void iotcore_clear_connect_mqtt_param(IOTCORE_CONNECT_MQTT_PARAM* param)
{
    if (!param) 
    {
        return;
    }

    FREE_POINTER(param->broker_addr)
    FREE_POINTER(param->client_id)
    FREE_POINTER(param->passwd)
    FREE_POINTER(param->user_name)
}
