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

#ifndef IOTCORE_TYPE_H
#define IOTCORE_TYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char iot_bool;
#define IOT_TRUE    1
#define IOT_FALSE   0

// succeed
static const int IOTCORE_ERR_OK = 0;
// general error
static const int IOTCORE_ERR_ERROR = 1;
// invalid input parameter
static const int IOTCORE_ERR_INVALID_PARAMETER = 2;
// malloc memery failed
static const int IOTCORE_ERR_OUT_OF_MEMORY = 3;
// not support features
static const int IOTCORE_ERR_NOT_SUPPORT = 4;
// iotcore not connected
static const int IOTCORE_ERR_NOT_CONNECT = 5;

#ifdef __cplusplus
}
#endif

#endif // IOTCORE_TYPE_H
