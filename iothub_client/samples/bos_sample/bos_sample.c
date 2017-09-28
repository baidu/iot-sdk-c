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
#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/platform.h>
#include "bos.h"
#include "bos_sample.h"


#define         BOS_SERVER                      "gz.bcebos.com"

#define         BOS_BUCKET                      "xxx"

// Key of BOS object. Like 'folder1/folder2/my-file.txt' or 'my-file.txt'. Please note folder should be included in object's key.
// For BOS core concepts please refer to https://cloud.baidu.com/doc/BOS/ProductDescription.html#.E6.A0.B8.E5.BF.83.E6.A6.82.E5.BF.B5
#define         BOS_OBJECT_KEY                  "xxx"

#define         AK                              "xxx"

#define         SK                              "xxx"

int bos_run_upload(void)
{
    size_t bufferSize = 2;
    unsigned char *buffer = malloc(bufferSize);
    unsigned int httpStatus;
    BUFFER_HANDLE response = BUFFER_new();
    BOS_RESULT result = BOS_Upload(BOS_SERVER, AK, SK, BOS_BUCKET, BOS_OBJECT_KEY, buffer, bufferSize, &httpStatus, response);
    free(buffer);
    BUFFER_delete(response);

    LogInfo("Upload finished. result = %d, httpStatus=%d.", result, httpStatus);
    if (!IS_SUCCESS_STATUS(httpStatus))
    {
        LogError("failure in BOS_Upload");
        return __FAILURE__;
    }
    else
    {
        return 0;
    }
}

int bos_run_download(void)
{
    unsigned int httpStatus;
    BUFFER_HANDLE response = BUFFER_new();
    BOS_RESULT result = BOS_Download(BOS_SERVER, AK, SK, BOS_BUCKET, BOS_OBJECT_KEY, &httpStatus, response);
    BUFFER_delete(response);

    LogInfo("Download finished. result = %d, httpStatus=%d, content size=%d.", result, httpStatus, BUFFER_length(response));
    if (!IS_SUCCESS_STATUS(httpStatus))
    {
        LogError("failure in BOS_Download");
        return __FAILURE__;
    }
    else
    {
        return 0;
    }
}

int bos_run(void)
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return __FAILURE__;
    }
    else
    {
        if (0 != bos_run_upload() || 0 != bos_run_download())
        {
            return __FAILURE__;
        }
    }

    return 0;
}