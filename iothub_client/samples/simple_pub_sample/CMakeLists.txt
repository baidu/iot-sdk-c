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

#this is CMakeLists.txt for simple_pub_sample

set(simple_pub_sample_c_files
        simple_pub_sample.c
)

if(WIN32)
    set(simple_pub_sample_c_files ${simple_pub_sample_c_files} windows/main.c)
else()
    set(simple_pub_sample_c_files ${simple_pub_sample_c_files} linux/main.c)
endif()

set(simple_pub_sample_h_files
        simple_pub_sample.h
)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

IF(QNX)
    if(${use_openssl})
        add_definitions(-DUSE_OPENSSL)
    endif()
ENDIF(QNX)

include_directories(.)
include_directories(${MQTT_INC_FOLDER})

add_executable(simple_pub_sample ${simple_pub_sample_c_files} ${simple_pub_sample_h_files})

if (WIN32)
    target_link_libraries(simple_pub_sample
        aziotsharedutil
        secur32
        ws2_32
        iothub_client
        umqtt)
elseif(QNX)
    target_link_libraries(simple_pub_sample
        aziotsharedutil
        socket
        m
        iothub_client
        umqtt)
else()
    target_link_libraries(simple_pub_sample
        aziotsharedutil
        pthread
        iothub_client
        umqtt)
endif()

