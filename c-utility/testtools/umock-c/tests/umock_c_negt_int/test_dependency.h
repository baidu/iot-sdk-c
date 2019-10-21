// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_DEPENDENCY_H
#define TEST_DEPENDENCY_H

#include "umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

typedef void* SOME_OTHER_HANDLE;

    MOCKABLE_FUNCTION(, int, function_1);
    MOCKABLE_FUNCTION(, int, function_2);
    MOCKABLE_FUNCTION(, void, function_3_void_return);
    MOCKABLE_FUNCTION(, void*, function_3_void_ptr_return, void*, a);
    MOCKABLE_FUNCTION(, SOME_OTHER_HANDLE, some_other_create, int, a);
    MOCKABLE_FUNCTION(, void, some_other_destroy, SOME_OTHER_HANDLE, h);

#ifdef __cplusplus
}
#endif

#endif /* TEST_DEPENDENCY_H */
