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

if(${use_installed_dependencies})
    #These need to be set for the functions included by c-utility 
    set(SHARED_UTIL_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/c-utility/src" CACHE INTERNAL "")
    set(SHARED_UTIL_ADAPTER_FOLDER "${CMAKE_CURRENT_LIST_DIR}/c-utility/adapters")
    set(SHARED_UTIL_FOLDER "${CMAKE_CURRENT_LIST_DIR}/c-utility")
    set_platform_files("${CMAKE_CURRENT_LIST_DIR}/c-utility")
    if(NOT umock_c_FOUND)
        find_package(umock_c REQUIRED CONFIG)
        include_directories(${UMOCK_C_INCLUDES})
    endif()
endif()
