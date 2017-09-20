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

#ifndef IOTHUB_CLIENT_PERSISTENCE_SQLITE_H
#define IOTHUB_CLIENT_PERSISTENCE_SQLITE_H

#include <stdlib.h>
#include <stdbool.h>
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "iothub_client_persistence.h"
#include <sqlite3.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SQLITE_HANDLE_DATA_TAG SQLITE_HANDLE_DATA;

MOCKABLE_FUNCTION(, int, initialize_sqlite_persist_handle, PERSIST_CONCRETE_HANDLE*, sqliteDataHandle,
                    const char *, clientId, const char *, suffix);

MOCKABLE_FUNCTION(, int, sqlite_add_pub_message, PERSIST_CONCRETE_HANDLE, handleData, int, messageId,
                           const char *, topic, int, topicLen, int, qos, const char*, payload, int, length);

MOCKABLE_FUNCTION(, int, sqlite_get_messages, PERSIST_CONCRETE_HANDLE, handleData, PUBLISH_DATA**, messages, int*, messageCount);

MOCKABLE_FUNCTION(, int, sqlite_delete_pub_message, PERSIST_CONCRETE_HANDLE, handleData, int, messageId);

MOCKABLE_FUNCTION(, int, sqlite_destroy_db, PERSIST_CONCRETE_HANDLE, handleData);

MOCKABLE_FUNCTION(, int, release_sqlite_message, PUBLISH_DATA*, messages, int, messageCount);

MOCKABLE_FUNCTION(, const PERSIST_INTERFACE_DESCRIPTION*, sqlite_get_persist_interface_description);

#ifdef __cplusplus
}
#endif

#endif //IOTHUB_CLIENT_PERSISTENCE_SQLITE_H
