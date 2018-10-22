// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PLATFORM_DEP_H
#define PLATFORM_DEP_H


/* add system dependent sys-call mapping macros here ... */

#include <dna_sockets.h>
#include "dna.h"
#include "dna_libc.h"

#define malloc dna_malloc
#define calloc dna_calloc
#define realloc dna_realloc
#define free dna_free
#define printf dna_printf

#endif /* PLATFORM_DEP_H */
