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

#include "iothub_client_persistence_sqlite.h"
#include <azure_c_shared_utility/xlogging.h>

int main(void)
{
    int result = 0;
    PERSIST_CONCRETE_HANDLE handle;
    if (initialize_sqlite_persist_handle(&handle, "iotthingbbb", "_bj") != 0)
    {
        LogError("fail to create sqlite persistence handle");
        result =  __FAILURE__;
    }
    else
    {
        const char* topicName = "/china/shanghai";
        const char* payload = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        if (sqlite_add_pub_message(handle, 1, topicName, strlen(topicName), 1, payload, strlen(payload)) != 0)
        {
            LogError("fail to insert data into table");
            result = __FAILURE__;
        }
        else
        {
            PUBLISH_DATA* messages;
            int messageCount;
            if(sqlite_get_messages(handle, &messages, &messageCount) != 0)
            {
                LogError("fail to query data from table");
                result = __FAILURE__;
            }

            for (int i =0 ;i < messageCount; ++i)
            {
                if(messages[i].messageId != 1)
                {
                    LogError("query data does not match inserted data");
                    result = __FAILURE__;
                }
                else
                {
                    LogInfo("query data matches inserted data");
                }
            }

            release_sqlite_message(messages, messageCount);

            // now delete row
            if (sqlite_delete_pub_message(handle, 1) != 0)
            {
                LogError("Fail to delete data");
                result = __FAILURE__;
            }
            else
            {
                if(sqlite_get_messages(handle, &messages, &messageCount) != 0)
                {
                    LogError("fail to query data from table");
                    result = __FAILURE__;
                }
                else
                {
                    if (messageCount == 0)
                    {
                        LogInfo("delete row successfully");
                    }
                    else
                    {
                        LogError("delete row failed");
                        result = __FAILURE__;
                    }
                }
            }
        }
        sqlite_destroy_db(handle);
    }

    return result;
}