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

#define C_LUCY_BBSORTEX
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Util/BBSortEx.h"

#include "Lucy/Index/Segment.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

BBSortEx*
BBSortEx_new(uint32_t mem_threshold, VArray *external) {
    BBSortEx *self = (BBSortEx*)VTable_Make_Obj(BBSORTEX);
    return BBSortEx_init(self, mem_threshold, external);
}

BBSortEx*
BBSortEx_init(BBSortEx *self, uint32_t mem_threshold, VArray *external) {
    SortEx_init((SortExternal*)self, sizeof(Obj*));
    self->external_tick = 0;
    self->external = (VArray*)INCREF(external);
    self->mem_consumed = 0;
    BBSortEx_Set_Mem_Thresh(self, mem_threshold);
    return self;
}

void
BBSortEx_destroy(BBSortEx *self) {
    DECREF(self->external);
    SUPER_DESTROY(self, BBSORTEX);
}

void
BBSortEx_clear_cache(BBSortEx *self) {
    Obj **const cache = (Obj**)self->cache;
    for (uint32_t i = self->cache_tick, max = self->cache_max; i < max; i++) {
        DECREF(cache[i]);
    }
    self->mem_consumed = 0;
    BBSortEx_clear_cache_t super_clear_cache
        = (BBSortEx_clear_cache_t)SUPER_METHOD(
              self->vtable, SortEx, Clear_Cache);
    super_clear_cache(self);
}

void
BBSortEx_feed(BBSortEx *self, void *data) {
    SortEx_feed((SortExternal*)self, data);

    // Flush() if necessary.
    ByteBuf *bytebuf = (ByteBuf*)CERTIFY(*(ByteBuf**)data, BYTEBUF);
    self->mem_consumed += BB_Get_Size(bytebuf);
    if (self->mem_consumed >= self->mem_thresh) {
        BBSortEx_Flush(self);
    }
}

void
BBSortEx_flush(BBSortEx *self) {
    uint32_t     cache_count = self->cache_max - self->cache_tick;
    Obj        **cache = (Obj**)self->cache;
    VArray      *elems;

    if (!cache_count) { return; }
    else              { elems = VA_new(cache_count); }

    // Sort, then create a new run.
    BBSortEx_Sort_Cache(self);
    for (uint32_t i = self->cache_tick; i < self->cache_max; i++) {
        VA_Push(elems, cache[i]);
    }
    BBSortEx *run = BBSortEx_new(0, elems);
    DECREF(elems);
    BBSortEx_Add_Run(self, (SortExternal*)run);

    // Blank the cache vars.
    self->cache_tick += cache_count;
    BBSortEx_Clear_Cache(self);
}

uint32_t
BBSortEx_refill(BBSortEx *self) {
    // Make sure cache is empty, then set cache tick vars.
    if (self->cache_max - self->cache_tick > 0) {
        THROW(ERR, "Refill called but cache contains %u32 items",
              self->cache_max - self->cache_tick);
    }
    self->cache_tick = 0;
    self->cache_max  = 0;

    // Read in elements.
    while (1) {
        ByteBuf *elem = NULL;

        if (self->mem_consumed >= self->mem_thresh) {
            self->mem_consumed = 0;
            break;
        }
        else if (self->external_tick >= VA_Get_Size(self->external)) {
            break;
        }
        else {
            elem = (ByteBuf*)VA_Fetch(self->external, self->external_tick);
            self->external_tick++;
            // Should be + sizeof(ByteBuf), but that's ok.
            self->mem_consumed += BB_Get_Size(elem);
        }

        if (self->cache_max == self->cache_cap) {
            BBSortEx_Grow_Cache(self,
                                Memory_oversize(self->cache_max + 1, self->width));
        }
        Obj **cache = (Obj**)self->cache;
        cache[self->cache_max++] = INCREF(elem);
    }

    return self->cache_max;
}

void
BBSortEx_flip(BBSortEx *self) {
    uint32_t run_mem_thresh = 65536;

    BBSortEx_Flush(self);

    // Recalculate the approximate mem allowed for each run.
    uint32_t num_runs = VA_Get_Size(self->runs);
    if (num_runs) {
        run_mem_thresh = (self->mem_thresh / 2) / num_runs;
        if (run_mem_thresh < 65536) {
            run_mem_thresh = 65536;
        }
    }

    for (uint32_t i = 0; i < num_runs; i++) {
        BBSortEx *run = (BBSortEx*)VA_Fetch(self->runs, i);
        BBSortEx_Set_Mem_Thresh(run, run_mem_thresh);
    }

    // OK to fetch now.
    self->flipped = true;
}

int
BBSortEx_compare(BBSortEx *self, void *va, void *vb) {
    UNUSED_VAR(self);
    return BB_compare((ByteBuf**)va, (ByteBuf**)vb);
}


