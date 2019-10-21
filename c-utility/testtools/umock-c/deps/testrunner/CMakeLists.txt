#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if(TARGET testrunnerswitcher)
    RETURN()
endif()

project(testrunnerswitcher)

set(TESTRUNNER_VERSION 1.1.19)

# Build with -fPIC always
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include (CTest)

option(run_valgrind "set run_valgrind to ON if tests are to be run under valgrind/helgrind/drd. Default is OFF" OFF)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
  endif()
else()
  set (CMAKE_C_STANDARD 99)
  set (CMAKE_CXX_STANDARD 11)
endif()

set(testrunnerswitcher_c_files
./src/testmutex.c
)

set(testrunnerswitcher_h_files
./inc/testrunnerswitcher.h
./inc/testmutex.h
)

add_library(testrunnerswitcher ${testrunnerswitcher_c_files} ${testrunnerswitcher_h_files})

set_target_properties(testrunnerswitcher
               PROPERTIES
               FOLDER "test_tools") 

#these are the include folders
#the following "set" statetement exports across the project a global variable called TESTRUNNERSWITCHER_INC_FOLDER that expands to whatever needs to included when using tesrtrunnerswitcher library
set(TESTRUNNERSWITCHER_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using testrunnerswitcher header" FORCE)

include_directories(${TESTRUNNERSWITCHER_INC_FOLDER})

set(INSTALL_H_FILES ${INSTALL_H_FILES} "./testtools/testrunnerswitcher/inc/testrunnerswitcher.h" CACHE INTERNAL "Files that will be installed on the system")
set(INSTALL_H_FILES ${INSTALL_H_FILES} "./testtools/testrunnerswitcher/inc/testmutex.h" CACHE INTERNAL "Files that will be installed on the system")
set(INSTALL_H_FILES ${INSTALL_H_FILES} "./testtools/testrunnerswitcher/inc/macro_utils.h" CACHE INTERNAL "Files that will be installed on the system")

# Set CMAKE_INSTALL_* if not defined
include(GNUInstallDirs)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Install Azure Test Runner Switcher
set(package_location "cmake")

install (TARGETS testrunnerswitcher EXPORT testrunnerswitcherTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/../bin
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install (FILES ${testrunnerswitcher_h_files} "./inc/macro_utils.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${TESTRUNNER_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file("configs/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    COPYONLY
)

install(EXPORT testrunnerswitcherTargets
    FILE
        "${PROJECT_NAME}Targets.cmake"
    DESTINATION
        ${package_location}
)
install(
    FILES
        "configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        ${package_location}
)

