# Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.11)
#this is CMakeLists for iotcore_client

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

compileAsC99()

option(openssl_enable "set use_openssl to ON if your platform supports openssl" ON)
#option(mbedtls_enable "set use mbedtls to ON if your platform supports mbedtls" ON)
#option(wolfssl_enable "set this if your platform supports wolfssl" ON)

include_directories(../certs)

set(iotcore_client_c_files
        ../certs/certs.c
        ./src/iotcore_mqtt_client.c
        ./src/iotcore_version.c
        ./src/iotcore_retry_logic.c
        ./src/iotcore_param_util.c
        ./src/_md5.c
        )

set(iotcore_client_h_files
        ../certs/certs.h
        ./inc/iotcore_retry.h
        ./inc/iotcore_mqtt_client.h
        ./inc/iotcore_client_version.h
        ./src/iotcore_retry_logic.h
        ./src/iotcore_param_util.h
        ./src/_md5.h
        )

include_directories(${AZURE_C_SHARED_UTILITY_INCLUDES})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${MQTT_INC_FOLDER})

if (WINCE)
include_directories(${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/windowsce) #windowsce SDK doesn't have stdbool.h
ENDIF()

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

set(IOTCORE_CLIENT_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using iotcore_client lib" FORCE)

include_directories(${AZURE_C_SHARED_UTILITY_INCLUDES})
include_directories(${SHARED_UTIL_INC_FOLDER})

if (WINCE)
    include_directories(${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/windowsce) #windowsce SDK doesn't have stdbool.h
ENDIF()

include_directories(${IOTCORE_CLIENT_INC_FOLDER})

add_subdirectory(samples)

if(${build_as_dynamic})
    add_library(iotcore_client SHARED
            ${iotcore_client_c_files}
            ${iotcore_client_h_files}
            ./src/iotcore_client.def
            )
    target_link_libraries(iotcore_client ${iotcore_client_libs})

else()
    add_library(iotcore_client
            ${iotcore_client_c_files}
            ${iotcore_client_h_files}
            )
endif()

if(${mbedtls_enable})
target_link_libraries(iotcore_client ${iotcore_client_libs} ${mbedlibs})
endif()

if(${wolfssl_enable})
    target_link_libraries(iotcore_client ${iotcore_client_libs} ${wolfssllibs})
endif()

linkSharedUtil(iotcore_client)
set(iotcore_client_libs
        iotcore_client
        ${iotcore_client_libs}
        )
