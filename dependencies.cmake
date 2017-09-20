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
    if(NOT azure_c_shared_utility_FOUND)
        find_package(azure_c_shared_utility REQUIRED CONFIG)
    endif()
    
    if(${use_mqtt})
        if(NOT umqtt_FOUND)
            find_package(umqtt REQUIRED CONFIG)
        endif()
    endif()	

else()
    add_subdirectory(c-utility)

    if(${use_mqtt})
        add_subdirectory(umqtt)
    endif()
endif()
