#define C_LUCY_MEMORY
#include <stdlib.h>
#include <stdio.h>
#define LUCY_USE_SHORT_NAMES
#include "Lucy/Util/Memory.h"

void*
Memory_wrapped_malloc(size_t count)
{
    void *pointer = malloc(count);
    if (pointer == NULL && count != 0) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    return pointer;
}

void*
Memory_wrapped_calloc(size_t count, size_t size)
{
    void *pointer = calloc(count, size);
    if (pointer == NULL && count != 0) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    return pointer;
}

void*
Memory_wrapped_realloc(void *ptr, size_t size)
{
    void *pointer = realloc(ptr, size);
    if (pointer == NULL && size != 0) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    return pointer;
}

void
lucy_Memory_wrapped_free(void *ptr)
{
    free(ptr);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

