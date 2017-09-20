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

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/gballoc.h"
#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/crt_abstractions.h>
#include "iothub_client_persistence_sqlite.h"

struct SQLITE_HANDLE_DATA_TAG
{
    sqlite3 *db;
    const char *dbName;
    const char *tableName;
};

static int CreateSqliteHandle(PERSIST_CONCRETE_HANDLE* sqliteDataHandle, const char *dbName, const char *tableName)
{
    SQLITE_HANDLE_DATA *handleData = (SQLITE_HANDLE_DATA *)malloc(sizeof(SQLITE_HANDLE_DATA));

    if (handleData == NULL)
    {
        return __FAILURE__;
    }
    handleData->dbName = dbName;
    handleData->tableName = tableName;

    *sqliteDataHandle = handleData;
    return 0;
}

static int CreateTableCallback(void * data, int argc, char ** argv, char ** azColName)
{
    int result = 0;
    int i;
    if (data != NULL) {
        LogError("Errors: %s", (const char*)data);
        result = __FAILURE__;
    }
    else
    {
        for(i = 0; i < argc; i++){
            LogInfo("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
        }
    }

    return result;
}

static int SqliteOpenDb(PERSIST_CONCRETE_HANDLE persistHandle)
{
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    char *zErrMsg = 0;
    int result = 0;

    // int rc = sqlite3_open(handleData->dbName, &handleData->db);
    int rc = sqlite3_open_v2(handleData->dbName, &handleData->db, SQLITE_OPEN_READWRITE
                                                                  | SQLITE_OPEN_CREATE | SQLITE_OPEN_DELETEONCLOSE, NULL);
    if(rc)
    {
        LogError("Can't open database: %s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
        return result;
    }
    else
    {
        LogInfo("Open database:%s successfully", handleData->dbName);
    }

    /* Create SQL statement */
    char formatSql[256];
    int len= snprintf(formatSql, 256, "CREATE TABLE if not exists %s ("  \
         "ID INT PRIMARY KEY  NOT NULL," \
         "TOPIC          VARCHAR(256)    NOT NULL," \
         "QOS            INTEGER  NOT NULL," \
         "PAYLOAD        BLOB);", handleData->tableName);

    if (len == 256) {
        LogError("create table clause is too long, suggest less than 256 chars");
        result = __FAILURE__;
        return result;
    }
    /* Execute SQL statement */
    rc = sqlite3_exec(handleData->db, formatSql, CreateTableCallback, 0, &zErrMsg);

    if( rc != SQLITE_OK )
    {
        LogError("SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        result = __FAILURE__;
        return result;
    }
    else
    {
        LogInfo("Table:%s created successfully", handleData->tableName);
    }

    return result;
}

int initialize_sqlite_persist_handle(PERSIST_CONCRETE_HANDLE* sqliteDataHandle, const char *clientId, const char *suffix)
{
    int result = 0;
    int dbNameLen = strlen(clientId) + 1 +strlen(suffix) + 4;
    char *dbName = (char *)malloc(dbNameLen);
    if (dbName == NULL)
    {
        LogError("Fail to allocate memory to hold database name");
        result = __FAILURE__;
    }
    else
    {
        strcpy(dbName, clientId);
        int offset = strlen(clientId);
        dbName[offset++] = '_';
        strcpy(dbName + offset, suffix);
        offset = offset + strlen(suffix);
        strcpy(dbName + offset, ".db");

        char *tableName = NULL;
        if (mallocAndStrcpy_s(&tableName, "PublishMessage") != 0)
        {
            LogError("Fail to allocate memory to hold table name");
            free(dbName);
            result = __FAILURE__;
        }
        else
        {
            if(CreateSqliteHandle(sqliteDataHandle, dbName, tableName) !=0)
            {
                LogError("Fail to allocate memory for sqlite handle");
                free(dbName);
                free(tableName);
                result = __FAILURE__;
            }
            else
            {
                if (SqliteOpenDb(*sqliteDataHandle) !=0)
                {
                    LogError("Fail to open sqlite DB handle");
                    free(dbName);
                    free(tableName);
                    free(*sqliteDataHandle);
                    result = __FAILURE__;
                }
            }
        }
    }

    return result;
}

int sqlite_add_pub_message(PERSIST_CONCRETE_HANDLE persistHandle, int messageId,
                           const char * topic, int topicLen, int qos, const char* payload, int length)
{
    int result = 0;
    char formatSql[256];
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    int len = snprintf(formatSql, 256,"INSERT INTO %s (ID, TOPIC, QOS, PAYLOAD) "
            "VALUES (?,?,?,?);", handleData->tableName);

    if (len == 256) {
        LogError("insert row clause is too long, suggest less than 256 chars");
        result = __FAILURE__;
        return result;
    }

    sqlite3_stmt* stmt1;
    int rc = sqlite3_prepare_v2(handleData->db, formatSql, -1, &stmt1, 0);
    if (rc != SQLITE_OK)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
        return result;
    }

    sqlite3_bind_int(stmt1, 1, messageId);
    sqlite3_bind_text(stmt1, 2, topic, topicLen, NULL);
    sqlite3_bind_int(stmt1, 3, qos);
    sqlite3_bind_blob(stmt1, 4, payload, length, SQLITE_STATIC);

    rc = sqlite3_step(stmt1);
    if (rc != SQLITE_DONE )
    {
        LogError("Fail to execute sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    rc = sqlite3_finalize(stmt1);
    if (rc != SQLITE_OK )
    {
        LogError("Fail to destroy statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    return result;
}

int sqlite_delete_pub_message(PERSIST_CONCRETE_HANDLE persistHandle, int messageId)
{
    int result = 0;
    char formatSql[256];
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    int len = snprintf(formatSql, 256, "DELETE FROM %s WHERE ID = ?;", handleData->tableName);

    if (len == 256) {
        LogError("DELETE clause is too long, suggest less than 256 chars");
        result = __FAILURE__;
        return result;
    }

    sqlite3_stmt* stmt1;
    int rc = sqlite3_prepare_v2(handleData->db, formatSql, -1, &stmt1, 0);
    if (rc)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
        return result;
    }

    sqlite3_bind_int(stmt1, 1, messageId);

    rc = sqlite3_step(stmt1);
    if (rc != SQLITE_DONE)
    {
        LogError("Fail to execute sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    rc = sqlite3_finalize(stmt1);
    if (rc != SQLITE_OK)
    {
        LogError("Fail to destroy statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    return result;
}

static int getMessageCount(PERSIST_CONCRETE_HANDLE persistHandle, int *count)
{
    int result = 0;
    char formatSql[256];
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    int len = snprintf(formatSql, 256, "SELECT COUNT(ID) FROM %s;", handleData->tableName);

    if (len == 256) {
        LogError("select count row clause is too long, suggest less than 256 chars");
        result = __FAILURE__;
        return result;
    }

    sqlite3_stmt* stmt1;
    int rc = sqlite3_prepare_v2(handleData->db, formatSql, -1, &stmt1, 0);
    if (rc != SQLITE_OK)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
        return result;
    }

    rc = sqlite3_step(stmt1);
    if (rc != SQLITE_ROW)
    {
        *count = 0;
        LogError("Fail to get message count:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }
    else
    {
        *count = sqlite3_column_int(stmt1, 0);
    }

    rc = sqlite3_finalize(stmt1);
    if (rc != SQLITE_OK) {
        LogError("Fail to destroy statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    return result;
}

int sqlite_get_messages(PERSIST_CONCRETE_HANDLE persistHandle, PUBLISH_DATA** messages, int* messageCount)
{
    int result = 0;
    char formatSql[256];
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    int len = snprintf(formatSql, 256, "SELECT ID, TOPIC, QOS, PAYLOAD FROM %s;", handleData->tableName);

    if (len == 256) {
        LogError("select row clause is too long, suggest less than 256 chars");
        result = __FAILURE__;
        return result;
    }

    int count;
    if(getMessageCount(handleData, &count) != 0) {
        LogError("Fail to get message count from table:%s", handleData->tableName);
        result = __FAILURE__;
        return result;
    }

    *messageCount = count;
    if (count == 0)
    {
        return result;
    }

    PUBLISH_DATA* pPublishMsg = (PUBLISH_DATA *)malloc(sizeof(PUBLISH_DATA) * count);
    *messages = pPublishMsg;

    sqlite3_stmt* stmt1;
    int rc = sqlite3_prepare_v2(handleData->db, formatSql, -1, &stmt1, 0);
    if (rc != SQLITE_OK)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
        return result;
    }

    int idx = 0;
    while((rc = sqlite3_step(stmt1)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt1, 0);
        int topicLen = sqlite3_column_bytes(stmt1, 1);
        char *topicName = (char *)malloc(topicLen);
        memcpy(topicName, sqlite3_column_text(stmt1, 1), topicLen);
        int qos= sqlite3_column_int(stmt1, 2);
        int nBlob = sqlite3_column_bytes(stmt1, 3);
        unsigned char *pzBlob = (unsigned char *)malloc(nBlob);
        memcpy(pzBlob, sqlite3_column_blob(stmt1, 3), nBlob);
        pPublishMsg[idx].messageId = id;
        pPublishMsg[idx].qos = qos;
        pPublishMsg[idx].topicName = topicName;
        pPublishMsg[idx].topicLen = topicLen;
        pPublishMsg[idx].payload = pzBlob;
        pPublishMsg[idx].payloadLen = nBlob;
        ++idx;
    }

    if (rc == SQLITE_ERROR)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    rc = sqlite3_finalize(stmt1);
    if (rc != SQLITE_OK) {
        LogError("Fail to destroy statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    return result;
}

int release_sqlite_message(PUBLISH_DATA* messages, int messageCount)
{
    for (int i=0; i < messageCount; ++i)
    {
        free((void *)messages[i].topicName);
        free((void *)messages[i].payload);
    }
    free(messages);
}

int sqlite_destroy_db(PERSIST_CONCRETE_HANDLE persistHandle)
{
    int result = 0;
    SQLITE_HANDLE_DATA* handleData = (SQLITE_HANDLE_DATA *)persistHandle;
    int rc = sqlite3_close(handleData->db);

    if (rc != SQLITE_OK)
    {
        LogError("Fail to prepare sql statement:%s", sqlite3_errmsg(handleData->db));
        result = __FAILURE__;
    }

    // first delete db file
    remove(handleData->dbName);

    // then release memory for db name and table name
    free((void *)handleData->dbName);
    free((void *)handleData->tableName);
    free(persistHandle);
    return result;
}

static const PERSIST_INTERFACE_DESCRIPTION sqlite_persist_interface_description =
{
    initialize_sqlite_persist_handle,
    sqlite_add_pub_message,
    sqlite_get_messages,
    sqlite_delete_pub_message,
    sqlite_destroy_db,
    release_sqlite_message
};

const PERSIST_INTERFACE_DESCRIPTION* sqlite_get_persist_interface_description(void)
{
    return &sqlite_persist_interface_description;
}
