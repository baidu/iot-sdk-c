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

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

compileAsC99()

set(iothub_persistence_sample_c_files
        iothub_sqlite_persistence_sample.c)

if(SQLITE3_LIB)

    add_library(sqlite3 SHARED IMPORTED)
    if(WIN32)
        set_target_properties(sqlite3 PROPERTIES IMPORTED_LOCATION ${SQLITE3_DLL})
        set_target_properties(sqlite3 PROPERTIES IMPORTED_IMPLIB ${SQLITE3_LIB})
    else(WIN32)
        set_target_properties(sqlite3 PROPERTIES IMPORTED_LOCATION ${SQLITE3_LIB})
    endif(WIN32)

endif(SQLITE3_LIB)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

include_directories(.)
include_directories(${IOTHUB_CLIENT_INC_FOLDER})

add_executable(iothub_sqlite_persistence_sample ${iothub_persistence_sample_c_files})

target_link_libraries(iothub_sqlite_persistence_sample
        aziotsharedutil
        iothub_client)

target_link_libraries(iothub_sqlite_persistence_sample
        sqlite3)



