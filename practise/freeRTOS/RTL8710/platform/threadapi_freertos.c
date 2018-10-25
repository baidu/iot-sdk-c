// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/xlogging.h"

#include "azure_c_shared_utility/threadapi.h"

#ifndef CONFIG_FREERTOS_HZ
   #define CONFIG_FREERTOS_HZ 1000
#endif

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    // TODO
    //xxx_task_sleep((milliseconds * CONFIG_FREERTOS_HZ) / 1000);
}

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
    LogError("mbed does not support multi-threading.");
    return THREADAPI_ERROR;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
    LogError("mbed does not support multi-threading.");
    return THREADAPI_ERROR;
}

void ThreadAPI_Exit(int res)
{
    LogError("mbed does not support multi-threading.");
}
