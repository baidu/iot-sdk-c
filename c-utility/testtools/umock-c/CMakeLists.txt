#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if(TARGET umock_c)
    return()
endif()

option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(run_int_tests "set run_int_tests to ON to run the integration tests (default is OFF)" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)
option(use_cppunittest "set use_cppunittest to ON to build CppUnitTest tests on Windows (default is ON)" OFF)

project(umock_c)

set(UMOCK_C_VERSION 1.1.19)

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build with -fPIC always
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Build with -Werror
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
elseif(UNIX) # LINUX OR APPLE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()

include (CTest)
include (CheckIncludeFiles)

# Add dependencies or submodules if use_installed_dependencies is OFF
include(dependencies.cmake)

set(umock_c_c_files
./src/umock_c.c
./src/umock_c_negative_tests.c
./src/umockalloc.c
./src/umockcall.c
./src/umockcallrecorder.c
./src/umocktypename.c
./src/umocktypes.c
./src/umocktypes_bool.c
./src/umocktypes_c.c
./src/umocktypes_stdint.c
./src/umocktypes_charptr.c
./src/umockcallpairs.c
./src/umockstring.c
./src/umockautoignoreargs.c
./src/umock_log.c
)

set(umock_c_h_files
./inc/umock_c.h
./inc/umock_c_internal.h
./inc/umock_c_negative_tests.h
./inc/umockalloc.h
./inc/umockcall.h
./inc/umockcallrecorder.h
./inc/umocktypename.h
./inc/umocktypes.h
./inc/umocktypes_bool.h
./inc/umocktypes_c.h
./inc/umocktypes_stdint.h
./inc/umocktypes_charptr.h
./inc/umockcallpairs.h
./inc/umockstring.h
./inc/umockautoignoreargs.h
./inc/umock_log.h
)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Make warning as error
    add_definitions(/WX)
ENDIF()

add_library(umock_c ${umock_c_c_files} ${umock_c_h_files})

set_target_properties(umock_c
               PROPERTIES
               FOLDER "test_tools") 

#these are the include folders
#the following "set" statetement exports across the project a global variable called UMOCK_C_INC_FOLDER that expands to whatever needs to included when using umock_c library
set(UMOCK_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using umock_c lib" FORCE)

include_directories(${UMOCK_C_INC_FOLDER} ${SHARED_UTIL_INC_FOLDER})

CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)

if ((NOT HAVE_STDINT_H) OR (NOT HAVE_STDBOOL_H))
    include_directories(${UMOCK_C_INC_FOLDER}/aux_inc)
endif()

include("configs/umock_cFunctions.cmake")

if (${run_unittests} OR ${run_int_tests})
    add_subdirectory(tests)
endif()

if(${use_installed_dependencies})
    # Set CMAKE_INSTALL_* if not defined
    include(GNUInstallDirs)

    if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
        set(CMAKE_INSTALL_LIBDIR "lib")
    endif()

    # Install UMock C
    set(package_location "cmake")

    install(TARGETS umock_c EXPORT umock_cTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/../bin
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(FILES ${umock_c_h_files} "./inc/umock_c_prod.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${UMOCK_C_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_file("configs/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        COPYONLY
    )

    install(EXPORT umock_cTargets
        FILE
            "${PROJECT_NAME}Targets.cmake"
        DESTINATION
            ${package_location}
    )

    install(
        FILES
            "configs/${PROJECT_NAME}Functions.cmake"
            "configs/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION
            ${package_location}
    )
endif()
