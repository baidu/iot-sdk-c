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

#ifndef IOTHUB_CLIENT_H
#define IOTHUB_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define IOTHUB_CLIENT_RETRY_POLICY_VALUES     \
    IOTHUB_CLIENT_RETRY_NONE,                   \
    IOTHUB_CLIENT_RETRY_IMMEDIATE,                  \
    IOTHUB_CLIENT_RETRY_INTERVAL,      \
    IOTHUB_CLIENT_RETRY_LINEAR_BACKOFF,      \
    IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF,                 \
    IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER,                 \
    IOTHUB_CLIENT_RETRY_RANDOM


DEFINE_ENUM(IOTHUB_CLIENT_RETRY_POLICY, IOTHUB_CLIENT_RETRY_POLICY_VALUES);




#ifdef __cplusplus
}
#endif

#endif //IOTHUB_CLIENT_H
