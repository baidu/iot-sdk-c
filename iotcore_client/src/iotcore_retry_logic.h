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

#ifndef IOTCORE_RETRY_LOGIC_H
#define IOTCORE_RETRY_LOGIC_H

#include "iotcore_retry.h"
#include "iotcore_type.h"
#include "azure_umqtt_c/mqtt_client.h"

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef int(*FN_RETRY_POLICY)(int *permit, size_t* delay, void* retry_context_callback);

typedef struct RETRY_LOGIC_TAG
{
    IOTCORE_RETRY_POLICY retry_policy;       // Type of policy we're using
    size_t retry_timeout_limit_in_sec;       // If we don't connect in this many seconds, give up even trying.
    FN_RETRY_POLICY fn_retry_policy;         // Pointer to the policy function
    time_t start;                            // When did we start retrying?
    time_t last_connect;                     // When did we last try to connect?
    iot_bool retry_started;                  // true if the retry timer is set and we're trying to retry
    iot_bool retry_expired;                  // true if we haven't connected in retry_timeout_limit_in_sec seconds
    iot_bool first_attempt;                  // true on init so we can connect the first time without waiting for any timeouts.
    size_t retry_count;                      // How many times have we tried connecting?
    size_t delay_from_last_connect_to_retry; // last time delta betweewn retry attempts.
} RETRY_LOGIC;

RETRY_LOGIC* create_retry_logic(IOTCORE_RETRY_POLICY retry_policy, size_t retry_timeout_limit_in_sec);

void destroy_retry_logic(RETRY_LOGIC *retry_logic);

void stop_retry_timer(RETRY_LOGIC *retry_logic);

iot_bool can_retry(RETRY_LOGIC *retry_logic);

#ifdef __cplusplus
}
#endif

#endif // IOTCORE_RETRY_LOGIC_H