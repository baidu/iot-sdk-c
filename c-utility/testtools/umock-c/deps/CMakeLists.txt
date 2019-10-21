#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (${run_unittests} OR ${run_int_tests})
	add_subdirectory(ctest)
	add_subdirectory(testrunner)
endif()