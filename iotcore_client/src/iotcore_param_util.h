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

#ifndef IOTCORE_PARAM_UTIL_H
#define IOTCORE_PARAM_UTIL_H

#include "iotcore_param.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _IOTCORE_CONNECT_MQTT_PARAM
{
    char* broker_addr;
    char* user_name;
    char* passwd;
    char* client_id;
} IOTCORE_CONNECT_MQTT_PARAM;

// construct IOTCORE_CONNECT_MQTT_PARAM struct from IOTCORE_INFO struct
int iotcore_construct_connect_mqtt_param(const IOTCORE_INFO* info, IOTCORE_CONNECT_MQTT_PARAM* param);

// free IOTCORE_CONNECT_MQTT_PARAM object members
// not free IOTCORE_CONNECT_MQTT_PARAM object self
void iotcore_clear_connect_mqtt_param(IOTCORE_CONNECT_MQTT_PARAM* param);

#ifdef __cplusplus
}
#endif

#endif // IOTCORE_PARAM_UTIL_H