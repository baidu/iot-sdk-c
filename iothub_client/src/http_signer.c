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

#include <azure_c_shared_utility/urlencode.h>
#include <azure_c_shared_utility/sha.h>
#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/hmacsha256.h>
#include "http_signer.h"

static const char httpApiRequestString[5][7] = {"GET", "POST", "PUT", "DELETE", "PATCH"};

static const char hexToASCII[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

/* Local Function Prototypes */
static const char *HTTP_GetRequestTypeString(HTTPAPI_REQUEST_TYPE requestType);
static STRING_HANDLE URL_EncodeExceptSlash(STRING_HANDLE text);
static STRING_HANDLE HMACSHA256_ComputeHashHex(const unsigned char* key, size_t keyLen, const unsigned char* payload, size_t payloadLen);

/* Query parameter is not supported for now since it's not used in BOS upload/download. */
int HTTPSigner_Sign(HTTP_HEADERS_HANDLE requestHttpHeaders, HTTPAPI_REQUEST_TYPE requestType, const char *host, STRING_HANDLE relativePath, const char *ak, const char *sk)
{
    char timestamp[26];
    time_t time = get_time(NULL);

    struct tm *gmtTime = get_gmtime(&time);
    strftime(timestamp, 26, "%Y-%m-%dT%H:%M:%SZ", gmtTime);

    int result = __FAILURE__;
    const char *method = HTTP_GetRequestTypeString(requestType);
    STRING_HANDLE canonicalHeaders = STRING_new();
    STRING_HANDLE canonicalRequest = STRING_new();
    STRING_HANDLE authStringPrefix = STRING_new();
    STRING_HANDLE authorization = STRING_new();
    STRING_HANDLE canonicalTimestamp = NULL;
    STRING_HANDLE signingKey = NULL;
    STRING_HANDLE signature = NULL;

    if (canonicalHeaders == NULL || canonicalRequest == NULL || authStringPrefix == NULL || authorization == NULL)
    {
        LogError("oom");
    }
    else
    {
        canonicalTimestamp = URL_EncodeString(timestamp);
        if (canonicalTimestamp == NULL)
        {
            LogError("failure in URL_EncodeString");
        }
        else
        {
            STRING_HANDLE canonicalUri = URL_EncodeExceptSlash(relativePath);
            if (canonicalUri == NULL)
            {
                LogError("unable to encode relativePath");
            }
            else
            {
                if (0 != STRING_sprintf(canonicalHeaders, "host:%s", host) //|| STRING_lower(canonicalHeaders) == false
                    || 0 != STRING_sprintf(canonicalRequest, "%s\n%s\n\n%s", method, STRING_c_str(canonicalUri), STRING_c_str(canonicalHeaders)))
                {
                    LogError("failure in STRING_sprintf");
                }
                else
                {
                    if (0 != STRING_sprintf(authStringPrefix, "bce-auth-v1/%s/%s/1800", ak, timestamp))
                    {
                        LogError("failure in STRING_sprintf");
                    }
                    else
                    {
                        signingKey = HMACSHA256_ComputeHashHex((const unsigned char *)sk, strlen(sk), (const unsigned char *) STRING_c_str(authStringPrefix), STRING_length(authStringPrefix));
                        if (NULL == signingKey)
                        {
                            LogError("failure in HMACSHA256_ComputeHashHex");
                        }
                        else
                        {
                            signature = HMACSHA256_ComputeHashHex((const unsigned char *) STRING_c_str(signingKey), STRING_length(signingKey), (const unsigned char *)STRING_c_str(canonicalRequest), STRING_length(canonicalRequest));
                            if (NULL == signature)
                            {
                                LogError("failure in HMACSHA256_ComputeHashHex");
                            }
                            else
                            {
                                if (0 != STRING_sprintf(authorization, "bce-auth-v1/%s/%s/1800/host/%s", ak, timestamp, STRING_c_str(signature)))
                                {
                                    LogError("failure in STRING_sprintf");
                                }
                                else
                                {
                                    HTTP_HEADERS_RESULT r1 = HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, "Authorization", STRING_c_str(authorization));
                                    HTTP_HEADERS_RESULT r2 = HTTPHeaders_AddHeaderNameValuePair(requestHttpHeaders, "x-bce-date", timestamp);
                                    if (r1 == HTTP_HEADERS_OK && r2 == HTTP_HEADERS_OK)
                                    {
                                        result = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                STRING_delete(canonicalUri);
            }
        }
    }

    STRING_delete(canonicalHeaders);
    STRING_delete(canonicalRequest);
    STRING_delete(authStringPrefix);
    STRING_delete(authorization);
    STRING_delete(signingKey);
    STRING_delete(signature);
    STRING_delete(canonicalTimestamp);

    return result;
}

const char *HTTP_GetRequestTypeString(HTTPAPI_REQUEST_TYPE requestType)
{
    return (const char *) httpApiRequestString[requestType];
}

/* Replace %2f with / */
STRING_HANDLE URL_EncodeExceptSlash(STRING_HANDLE text)
{
    STRING_HANDLE encoded = URL_Encode(text);
    if (encoded != NULL)
    {
        char * s = (char *) STRING_c_str(encoded);
        size_t length = STRING_length(encoded);
        int i, j;
        for (i = 0, j = 0; j < length; ++i, ++j)
        {
            if (j < length - 2)
            {
                if (s[j] == '%' && s[j + 1] == '2' && (s[j + 2] == 'f' || s[j + 2] == 'F'))
                {
                    s[i] = '/';
                    j += 2;
                    continue;
                }
            }

            s[i] = s[j];
        }

        s[i] = '\0';
    }

    return encoded;
}

STRING_HANDLE HMACSHA256_ComputeHashHex(const unsigned char* key, size_t keyLen, const unsigned char* payload, size_t payloadLen)
{
    char buffer[SHA256HashSize * 2 + 1];
    STRING_HANDLE hashHex = NULL;
    BUFFER_HANDLE hash = BUFFER_new();
    if (hash == NULL)
    {
        LogError("oom");
    }
    else
    {
        HMACSHA256_RESULT result = HMACSHA256_ComputeHash(key, keyLen, payload, payloadLen, hash);
        if (result == HMACSHA256_OK)
        {
            int i;
            for (i = 0; i < BUFFER_length(hash); ++i)
            {
                buffer[i * 2] = hexToASCII[(BUFFER_u_char(hash)[i] & 0xF0) >> 4];
                buffer[i * 2 + 1] = hexToASCII[BUFFER_u_char(hash)[i] & 0x0F];
            }
            buffer[SHA256HashSize * 2] = '\0';
            hashHex = STRING_construct(buffer);
            if (hashHex == NULL)
            {
                LogError("oom");
            }
        }
        BUFFER_delete(hash);
    }

    return hashHex;
}
