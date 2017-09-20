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
#this is CMakeLists for iothub_client

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

compileAsC99()

option(sqlite_persistence "set sqlite_persistence to ON which enable use sqlite as persistent storage. Default is OFF" OFF)
option(device_management "set iotdm_client to OFF to skip building for iot device mangement client. Default is ON" ON)

include_directories(../certs)

set(iothub_client_c_files
        ../certs/certs.c
        ./src/iothub_client_retry_control.c
        ./src/iothub_mqtt_client.c
        ./src/iothub_client_persistence.c
        ./src/version.c
        )

set(iothub_client_h_files
        ../certs/certs.h
        ./inc/iothub_client_retry_control.h
        ./inc/iothub_client.h
        ./inc/iothub_client_persistence.h
        ./inc/iothub_mqtt_client.h
        ./inc/iothub_client_version.h)

if (${sqlite_persistence})
    add_definitions(-DUSE_SQLITE_PERSISTENCE)
    set(iothub_client_c_files ${iothub_client_c_files} ./src/iothub_client_persistence_sqlite.c)
    set(iothub_client_h_files ${iothub_client_h_files} ./inc/iothub_client_persistence_sqlite.h)
endif()


if (${device_management})
    include_directories(../parson)
    set(iothub_client_c_files ${iothub_client_c_files} ../parson/parson.c ./src/iotdm_client.c)
    set(iothub_client_h_files ${iothub_client_h_files} ../parson/parson.h ./inc/iotdm_client.h ./inc/iotdm_callback.h)
endif()

if(NOT ${dont_use_uploadtoblob})
    include_directories(../parson)
endif()

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

set(IOTHUB_CLIENT_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using iothub_client lib" FORCE)

include_directories(${AZURE_C_SHARED_UTILITY_INCLUDES})
include_directories(${SHARED_UTIL_INC_FOLDER})

if (WINCE)
    include_directories(${SHARED_UTIL_INC_FOLDER}/azure_c_shared_utility/windowsce) #windowsce SDK doesn't have stdbool.h
ENDIF()

include_directories(${IOTHUB_CLIENT_INC_FOLDER})

add_subdirectory(samples)

if(${build_as_dynamic})
    add_library(iothub_client SHARED
            ${iothub_client_c_files}
            ${iothub_client_h_files}
            ./src/iothub_client.def
            )
    target_link_libraries(iothub_client ${iothub_client_libs})

else()
    add_library(iothub_client
            ${iothub_client_c_files}
            ${iothub_client_h_files}
            )
endif()

linkSharedUtil(iothub_client)
set(iothub_client_libs
        iothub_client
        ${iothub_client_libs}
        )

