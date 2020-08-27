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
project(iotcore_c_sdk)

set(IOT_SDK_VERSION 1.1.19)

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

#
#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    #on Linux, enable valgrind
    #these commands (MEMORYCHECK...) need to apear BEFORE include(CTest) or they will not have any effect
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
    set(QNX TRUE)
endif()

include (CTest)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4232")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /wd4232")
endif()

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Make warning as error
    add_definitions(/WX)
ELSE()
    # Make warning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
ENDIF(WIN32)

#the following variables are project-wide and can be used with cmake-gui
option(use_http "set use_http to ON if http is to be used, set to OFF to not use http" ON)
option(use_mqtt "set use_mqtt to ON if mqtt is to be used, set to OFF to not use mqtt" ON)
option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF)" OFF)
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_longhaul_tests "set run_longhaul_tests to ON to run longhaul tests (default is OFF)[if possible, they are always build]" OFF)
option(compileOption_C "passes a string to the command line of the C compiler" OFF)
option(compileOption_CXX "passes a string to the command line of the C++ compiler" OFF)
option(dont_use_uploadtoblob "set dont_use_uploadtoblob to ON if the functionality of upload to blob is to be excluded, OFF otherwise. It requires HTTP" OFF)
option(no_logging "disable logging" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)
option(use_firmware_update "build the Raspberry PI firmware_update sample" OFF)
option(build_as_dynamic "build the IoT SDK libaries as dynamic"  OFF)
option(build_network_e2e "build network E2E tests" OFF)

#Work in progress features
#=========================
#These options turn on features that are either not complete or noy yet enabled in the IoTHub service

if(WIN32)
    option(use_openssl "set use_openssl to ON to use OpenSSL." OFF)
else()
    option(use_openssl "set use_openssl to ON to use OpenSSL." ON)
endif()

#setting nuget_e2e_tests will only generate e2e tests to run with nuget packages.  Install-packages from Package Manager Console in VS before building the projects
option(nuget_e2e_tests "set nuget_e2e_tests to ON to generate e2e tests to run with nuget packages (default is OFF)" OFF)

#check for conflicting options
if(NOT ${use_http})
    MESSAGE( "Setting dont_use_uploadtoblob to ON because use_http is OFF")
    set(dont_use_uploadtoblob "ON")
    MESSAGE( STATUS "use_http:         " ${use_http} )
    MESSAGE( STATUS "dont_use_uploadtoblob:         " ${dont_use_uploadtoblob} )
endif()

if(${dont_use_uploadtoblob})
    add_definitions(-DDONT_USE_UPLOADTOBLOB)
endif()

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(IN_OPENWRT)
    ADD_DEFINITIONS("$ENV{TARGET_LDFLAGS}" "$ENV{TARGET_CPPFLAGS}" "$ENV{TARGET_CFLAGS}")
    INCLUDE_DIRECTORIES("$ENV{TOOLCHAIN_DIR}/usr/include" "$ENV{TARGET_LDFLAGS}" "$ENV{TARGET_CPPFLAGS}" "$ENV{TARGET_CFLAGS}")
endif()

if (LINUX)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # now all static libraries use PIC flag for Python shared lib
        set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-fPIC ${CMAKE_CXX_FLAGS}")
        # Report error for variable length array
        set(CMAKE_C_FLAGS "-Wvla ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-Wvla ${CMAKE_CXX_FLAGS}")
    endif()
endif()

#if any compiler has a command line switch called "OFF" then it will need special care
if(NOT "${compileOption_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if(NOT "${compileOption_CXX}" STREQUAL "OFF")
    set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

include("dependencies.cmake")

set_platform_files(${CMAKE_CURRENT_LIST_DIR}/c-utility)

if(${run_unittests} OR ${run_e2e_tests})
    include("dependencies-test.cmake")
endif()

enable_testing()

#this project uses several other projects that are build not by these CMakeFiles
#this project also targets several OSes

include("configs/iotcore_c_sdkFunctions.cmake")

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()
if (MSVC)
    detect_architecture("_M_AMD64" x86_64)
    detect_architecture("_M_IX86" x86)
    detect_architecture("_M_ARM" ARM)
else()
    detect_architecture("__x86_64__" x86_64)
    detect_architecture("__i386__" x86)
    detect_architecture("__arm__" ARM)
endif()
if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "iothub architecture: ${ARCHITECTURE}")

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    if (NOT IN_OPENWRT)
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC99)

macro(compileAsC11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c11 ${CMAKE_C_FLAGS}")
      set (CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
    endif()
  else()
    set (CMAKE_C_STANDARD 11)
    set (CMAKE_CXX_STANDARD 11)
  endif()
endmacro(compileAsC11)

if(WIN32)
    set(LOCK_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/lock_win32.c)
    set(THREAD_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/threadapi_c11.c)
else()
    set(LOCK_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/lock_pthreads.c)
    set(THREAD_C_FILE ${SHARED_UTIL_ADAPTER_FOLDER}/threadapi_pthreads.c)
endif()


if(NOT ${use_http})
    set (build_service_client OFF)
    message(STATUS "iothub_service_client build is disabled (HTTP support is required)")
endif()

add_subdirectory(serializer)
#add_subdirectory(iothub_client)
add_subdirectory(iotcore_client)

#Set CMAKE_INSTALL_LIBDIR if not defined
include(GNUInstallDirs)

if(${use_installed_dependencies})
    set(package_location "cmake")

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${IOT_SDK_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_file("configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        COPYONLY
    )

    install(EXPORT iotcore_c_sdkTargets
        FILE
            "${PROJECT_NAME}Targets.cmake"
        DESTINATION
            ${package_location}
    )

    install(
        FILES
            "configs/${PROJECT_NAME}Config.cmake"
            "configs/${PROJECT_NAME}Functions.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION
            ${package_location}
    )
endif()

if (${build_network_e2e})
    add_subdirectory(network_e2e/tests)
endif()
