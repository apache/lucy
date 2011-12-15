/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCMemPool.h"
#include "CFCUtil.h"

struct CFCMemPool {
    CFCBase base;
    size_t arena_size;
    size_t remaining;
    char *current;
    size_t num_arenas;
    char **arenas;
};

const static CFCMeta CFCMEMPOOL_META = {
    "Clownfish::MemPool",
    sizeof(CFCMemPool),
    (CFCBase_destroy_t)CFCMemPool_destroy
};

CFCMemPool*
CFCMemPool_new(size_t arena_size) {
    CFCMemPool *self = (CFCMemPool*)CFCBase_allocate(&CFCMEMPOOL_META);
    return CFCMemPool_init(self, arena_size);
}

CFCMemPool*
CFCMemPool_init(CFCMemPool *self, size_t arena_size) {
    arena_size = arena_size ? arena_size : 0x400; // 1k
    self->current    = NULL;
    self->arena_size = arena_size;
    self->remaining  = 0;
    self->num_arenas = 0;
    self->arenas     = NULL;
    return self;
}

void*
CFCMemPool_allocate(CFCMemPool *self, size_t size) {
    size_t overage = (8 - (size % 8)) % 8;
    size_t amount = size + overage;
    size_t arena_size = self->arena_size > amount
                        ? self->arena_size : amount;
    if (amount > self->remaining) {
        self->num_arenas += 1;
        self->arenas = (char**)REALLOCATE(self->arenas,
                                          self->num_arenas * sizeof(char*));
        self->current = (char*)MALLOCATE(arena_size);
        self->arenas[self->num_arenas - 1] = self->current;
        self->remaining = amount;
    }
    size_t offset = arena_size - self->remaining;
    void *result = self->current + offset;
    self->remaining -= amount;
    return result;
}

void
CFCMemPool_destroy(CFCMemPool *self) {
    for (size_t i = 0; i < self->num_arenas; i++) {
        FREEMEM(self->arenas[i]);
    }
    FREEMEM(self->arenas);
    CFCBase_destroy((CFCBase*)self);
}

