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

#ifndef BOS_H
#define BOS_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

#include "parson.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include <azure_c_shared_utility/buffer_.h>


#define BOS_RESULT_VALUES \
    BOS_OK,               \
    BOS_ERROR,            \
    BOS_NOT_IMPLEMENTED,  \
    BOS_HTTP_ERROR,       \
    BOS_INVALID_ARG

DEFINE_ENUM(BOS_RESULT, BOS_RESULT_VALUES)

#define IS_SUCCESS_STATUS(code) ((code) >= 200 && (code) < 300)

typedef struct BOS_RANGE_TAG
{
    long start; /* include */
    long end; /* include */
} BOS_RANGE;

typedef struct BOS_CONTENT_RANGE_TAG
{
    long start; /* include */
    long end; /* include */
    long totalLength;
} BOS_CONTENT_RANGE;

/**
* @brief	Synchronously uploads a byte array to BOS
*
* @param	host	        host of BOS server. For example 'bj.bcebos.com' or 'gz.bcebos.com'
* @param	ak	            Access key ID.
* @param	sk	            Secret key.
* @param	bucket		    BOS bucket to upload to. For BOS core concepts please refer to https://cloud.baidu.com/doc/BOS/ProductDescription.html#.E6.A0.B8.E5.BF.83.E6.A6.82.E5.BF.B5
* @param	objectKey		BOS object's key to upload to. Like 'folder1/folder2/my-file.txt' or 'my-file.txt'.
* @param	source		    A pointer to the byte array to be uploaded (can be NULL, but then size needs to be zero)
* @param	size		    Size of the source
* @param    httpStatus      A pointer to an out argument receiving the HTTP status (available only when the return value is BOS_OK)
* @param    httpResponse    A BUFFER_HANDLE that receives the HTTP response from the server (available only when the return value is BOS_OK)
*
* @return	A @c BOS_RESULT.  BOS_OK means the object has been uploaded successfully. Any other value indicates an error
*/
MOCKABLE_FUNCTION(, BOS_RESULT, BOS_Upload, const char*, host, const char *, ak, const char *, sk, const char *, bucket, const char *, objectKey, const unsigned char*, source, size_t, size, unsigned int*, httpStatus, BUFFER_HANDLE, httpResponse)

/**
* @brief	Synchronously download an object from BOS
*
* @param	host	        host of BOS server. For example 'bj.bcebos.com' or 'gz.bcebos.com'
* @param	ak	            Access key ID.
* @param	sk	            Secret key.
* @param	bucket		    BOS bucket to download from. For BOS core concepts please refer to https://cloud.baidu.com/doc/BOS/ProductDescription.html#.E6.A0.B8.E5.BF.83.E6.A6.82.E5.BF.B5
* @param	objectKey		BOS object's key to download from. Like 'folder1/folder2/my-file.txt' or 'my-file.txt'.
* @param    httpStatus      A pointer to an out argument receiving the HTTP status (available only when the return value is BOS_OK)
* @param    httpResponse    A BUFFER_HANDLE that receives the HTTP response from the server (available only when the return value is BOS_OK). Actual content of the BOS object is in it.
*
* @return	A @c BOS_RESULT.  BOS_OK means the object has been uploaded successfully. Any other value indicates an error
*/
MOCKABLE_FUNCTION(, BOS_RESULT, BOS_Download, const char*, host, const char *, ak, const char *, sk, const char *, bucket, const char *, objectKey, unsigned int*, httpStatus, BUFFER_HANDLE, httpResponse)

/**
* @brief	Synchronously download an pre-signed url from BOS. AK/SK not required for pre-signed URL.
*
* @param	url		        A pre-signed BOS URL.
* @param    range           Specify range of the file to return. Optional. By default return all data of the file.
* @param    contentRange    A pointer to an out argument receiving the Content-Range header of the response. It has the total length of the file.
* @param    httpStatus      A pointer to an out argument receiving the HTTP status (available only when the return value is BOS_OK)
* @param    httpResponse    A BUFFER_HANDLE that receives the HTTP response from the server (available only when the return value is BOS_OK). Actual content of the BOS object is in it.
*
* @return	A @c BOS_RESULT.  BOS_OK means the object has been uploaded successfully. Any other value indicates an error
*/
MOCKABLE_FUNCTION(, BOS_RESULT, BOS_Download_Presigned, const char*, url, BOS_RANGE*, range, BOS_CONTENT_RANGE*, contentRange, unsigned int*, httpStatus, BUFFER_HANDLE, httpResponse)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BOS_H