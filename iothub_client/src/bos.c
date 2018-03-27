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

#include <azure_c_shared_utility/gballoc.h>
#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/httpapiex.h>
#include <azure_c_shared_utility/urlencode.h>
#include <certs.h>

#include "bos.h"
#include "http_signer.h"

static BOS_RESULT BOS_UploadDownload(HTTPAPI_REQUEST_TYPE requestType, const char *server, const char *ak, const char *sk, const char *bucket, const char *objectKey, BUFFER_HANDLE requestBuffer, unsigned int *httpStatus, BUFFER_HANDLE httpResponse);

BOS_RESULT BOS_Upload(const char *server, const char *ak, const char *sk, const char *bucket, const char *objectKey, const unsigned char *source, size_t size, unsigned int *httpStatus, BUFFER_HANDLE httpResponse)
{
    BOS_RESULT result = BOS_ERROR;
    if ((size > 0) && (source == NULL))
    {
        LogError("combination of source = %p and size = %zu is invalid", source, size);
        result = BOS_INVALID_ARG;
    }
    else
    {
        BUFFER_HANDLE requestBuffer = BUFFER_create(source, size);
        if (requestBuffer == NULL)
        {
            LogError("oom");
        }
        else
        {
            result = BOS_UploadDownload(HTTPAPI_REQUEST_PUT, server, ak, sk, bucket, objectKey, requestBuffer, httpStatus, httpResponse);
        }
        BUFFER_delete(requestBuffer);
    }

    return result;
}

BOS_RESULT BOS_Download(const char *server, const char *ak, const char *sk, const char *bucket, const char *objectKey, unsigned int *httpStatus, BUFFER_HANDLE httpResponse)
{
    BOS_RESULT result = BOS_ERROR;

    BUFFER_HANDLE requestBuffer = BUFFER_new();
    if (requestBuffer == NULL)
    {
        LogError("oom");
    }
    else
    {
        result = BOS_UploadDownload(HTTPAPI_REQUEST_GET, server, ak, sk, bucket, objectKey, requestBuffer, httpStatus, httpResponse);
        BUFFER_delete(requestBuffer);
    }

    return result;
}

BOS_RESULT BOS_UploadDownload(HTTPAPI_REQUEST_TYPE requestType, const char *server, const char *ak, const char *sk, const char *bucket, const char *objectKey, BUFFER_HANDLE requestBuffer, unsigned int *httpStatus, BUFFER_HANDLE httpResponse)
{
    BOS_RESULT result = BOS_ERROR;

    if (server == NULL || bucket == NULL || objectKey == NULL)
    {
        LogError("parameter server/bucket/object_key is NULL");
        result = BOS_INVALID_ARG;
    }
    else
    {
        STRING_HANDLE relativePath = STRING_new();
        if (relativePath == NULL)
        {
            LogError("oom");
        }
        else
        {
            if (0 != STRING_sprintf(relativePath, "/%s/%s", bucket, objectKey))
            {
                LogError("failure in STRING_sprintf");
            }
            else
            {
                HTTPAPIEX_HANDLE httpApiExHandle = HTTPAPIEX_Create(server);
                if (httpApiExHandle == NULL)
                {
                    LogError("unable to create HTTPAPIEX_HANDLE");
                }
                else
                {
                    if ((HTTPAPIEX_SetOption(httpApiExHandle, "TrustedCerts", bos_root_ca) == HTTPAPIEX_ERROR))
                    {
                        LogError("failure in setting trusted certificates");
                    }
                    else
                    {
                        HTTP_HEADERS_HANDLE requestHttpHeaders = HTTPHeaders_Alloc();
                        if (requestHttpHeaders == NULL)
                        {
                            LogError("unable to HTTPHeaders_Alloc");
                        }
                        else
                        {
                            if (0 != HTTPSigner_Sign(requestHttpHeaders, requestType, server, relativePath, ak, sk))
                            {
                                LogError("Failure in HTTPSigner_Sign");
                            }
                            else
                            {
                                if (HTTPAPIEX_OK != HTTPAPIEX_ExecuteRequest(httpApiExHandle, requestType, STRING_c_str(relativePath), requestHttpHeaders, requestBuffer, httpStatus, NULL, httpResponse))
                                {
                                    LogError("failed to HTTPAPIEX_ExecuteRequest");
                                    result = BOS_HTTP_ERROR;
                                }
                                else
                                {
                                    result = BOS_OK;
                                }
                            }
                            HTTPHeaders_Free(requestHttpHeaders);
                        }
                    }
                    HTTPAPIEX_Destroy(httpApiExHandle);
                }
            }
            STRING_delete(relativePath);
        }
    }

    return result;
}

BOS_RESULT BOS_Download_Presigned(const char* url, unsigned int* httpStatus, BUFFER_HANDLE httpResponse)
{
    BOS_RESULT result = BOS_ERROR;
    char protocol[10];
    char server[100];
    char *relativePath = malloc(strlen(url));
    relativePath[0] = '/';
    memset(server, '\0', 100);
    memset(protocol, '\0', 10);
    int matched = sscanf(url, "%99[^:]://%99[^/]/%s[\n]", protocol, server, relativePath + 1);
        
    if (matched != 3)
    {
        LogError("unable to parse url %s", url);
    }
    else 
    {
        HTTPAPIEX_HANDLE httpApiExHandle = HTTPAPIEX_Create(server);
        if (httpApiExHandle == NULL) {
            LogError("unable to create HTTPAPIEX_HANDLE");
        } else {
            if ((HTTPAPIEX_SetOption(httpApiExHandle, "TrustedCerts", bos_root_ca) == HTTPAPIEX_ERROR)) {
                LogError("failure in setting trusted certificates");
            } else {
                if (HTTPAPIEX_OK !=
                    HTTPAPIEX_ExecuteRequest(httpApiExHandle, HTTPAPI_REQUEST_GET, relativePath, NULL, NULL, httpStatus,
                                             NULL, httpResponse)) {
                    LogError("failed to HTTPAPIEX_ExecuteRequest");
                    result = BOS_HTTP_ERROR;
                } else {
                    result = BOS_OK;
                }
            }
            HTTPAPIEX_Destroy(httpApiExHandle);
        }
    }
    free(relativePath);
    return result;
}
