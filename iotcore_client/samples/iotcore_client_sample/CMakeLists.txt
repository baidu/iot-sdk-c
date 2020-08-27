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

#this is CMakeLists.txt for iotcore_mqtt_client_sample

set(iotcore_mqtt_client_sample_c_files
        iotcore_mqtt_client_sample.c)

if(WIN32)
    set(iotcore_mqtt_client_sample_c_files ${iotcore_mqtt_client_sample_c_files} windows/main.c)
else()
    set(iotcore_mqtt_client_sample_c_files ${iotcore_mqtt_client_sample_c_files} linux/main.c)
endif()

set(iotcore_mqtt_client_sample_h_files
        iotcore_mqtt_client_sample.h
        )

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

include_directories(.)
include_directories(${MQTT_INC_FOLDER})
include_directories(${IOTCORE_CLIENT_INC_FOLDER})

add_executable(iotcore_client_sample ${iotcore_mqtt_client_sample_c_files} ${iotcore_mqtt_client_sample_h_files})

if (WIN32)
    target_link_libraries(iotcore_client_sample
        aziotsharedutil
        umqtt
        secur32
        ws2_32
        iotcore_client)

    if(${use_openssl})
        target_link_libraries(iotcore_client_sample $ENV{OpenSSLDir}/lib/ssleay32.lib $ENV{OpenSSLDir}/lib/libeay32.lib)
        file(COPY $ENV{OpenSSLDir}/bin/libeay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
        file(COPY $ENV{OpenSSLDir}/bin/ssleay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
    endif()
    if(${use_wolfssl})
        target_link_libraries(iotcore_client_sample $ENV{WolfSSLDir}/Debug/wolfssl.lib)
    endif()
elseif(QNX)
    target_link_libraries(iotcore_client_sample
        aziotsharedutil
        iotcore_client
        socket
        m
        umqtt)
    #if(${use_openssl})
        target_link_libraries(iotcore_client_sample ${OPENSSL_LIBRARIES})
    #endif()
else()
    target_link_libraries(iotcore_client_sample
        aziotsharedutil
        pthread
        iotcore_client
        umqtt)

    if (${sqlite_persistence})
        target_link_libraries(iotcore_client_sample sqlite3)
    endif()
    #if(${use_openssl})
        target_link_libraries(iotcore_client_sample ${OPENSSL_LIBRARIES})
    #endif()
endif()

