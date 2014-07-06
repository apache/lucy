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
#define LUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/BBSortEx.h"

#include "Lucy/Index/Segment.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

BBSortEx*
BBSortEx_new(uint32_t mem_threshold, VArray *external) {
    BBSortEx *self = (BBSortEx*)Class_Make_Obj(BBSORTEX);
    return BBSortEx_init(self, mem_threshold, external);
}

BBSortEx*
BBSortEx_init(BBSortEx *self, uint32_t mem_threshold, VArray *external) {
    SortEx_init((SortExternal*)self);
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    ivars->external_tick = 0;
    ivars->external = (VArray*)INCREF(external);
    ivars->mem_consumed = 0;
    BBSortEx_Set_Mem_Thresh(self, mem_threshold);
    return self;
}

void
BBSortEx_Destroy_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    DECREF(ivars->external);
    SUPER_DESTROY(self, BBSORTEX);
}

void
BBSortEx_Clear_Buffer_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    ivars->mem_consumed = 0;
    BBSortEx_Clear_Buffer_t super_clear_buffer
        = SUPER_METHOD_PTR(BBSORTEX, LUCY_BBSortEx_Clear_Buffer);
    super_clear_buffer(self);
}

void
BBSortEx_Feed_IMP(BBSortEx *self, Obj *item) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    BBSortEx_Feed_t super_feed
        = SUPER_METHOD_PTR(BBSORTEX, LUCY_BBSortEx_Feed);
    super_feed(self, item);

    // Flush() if necessary.
    ByteBuf *bytebuf = (ByteBuf*)CERTIFY(item, BYTEBUF);
    ivars->mem_consumed += BB_Get_Size(bytebuf);
    if (ivars->mem_consumed >= ivars->mem_thresh) {
        BBSortEx_Flush(self);
    }
}

void
BBSortEx_Flush_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    uint32_t     buf_count = ivars->buf_max - ivars->buf_tick;
    Obj        **buffer = ivars->buffer;
    VArray      *elems;

    if (!buf_count) { return; }
    else            { elems = VA_new(buf_count); }

    // Sort, then create a new run.
    BBSortEx_Sort_Buffer(self);
    for (uint32_t i = ivars->buf_tick; i < ivars->buf_max; i++) {
        VA_Push(elems, buffer[i]);
    }
    BBSortEx *run = BBSortEx_new(0, elems);
    DECREF(elems);
    BBSortEx_Add_Run(self, (SortExternal*)run);

    // Blank the buffer vars.
    ivars->buf_tick += buf_count;
    BBSortEx_Clear_Buffer(self);
}

uint32_t
BBSortEx_Refill_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);

    // Make sure buffer is empty, then set buffer tick vars.
    if (ivars->buf_max - ivars->buf_tick > 0) {
        THROW(ERR, "Refill called but buffer contains %u32 items",
              ivars->buf_max - ivars->buf_tick);
    }
    ivars->buf_tick = 0;
    ivars->buf_max  = 0;

    // Read in elements.
    while (1) {
        ByteBuf *elem = NULL;

        if (ivars->mem_consumed >= ivars->mem_thresh) {
            ivars->mem_consumed = 0;
            break;
        }
        else if (ivars->external_tick >= VA_Get_Size(ivars->external)) {
            break;
        }
        else {
            elem = (ByteBuf*)VA_Fetch(ivars->external, ivars->external_tick);
            ivars->external_tick++;
            // Should be + sizeof(ByteBuf), but that's ok.
            ivars->mem_consumed += BB_Get_Size(elem);
        }

        if (ivars->buf_max == ivars->buf_cap) {
            BBSortEx_Grow_Buffer(self,
                                 Memory_oversize(ivars->buf_max + 1,
                                                 sizeof(Obj*)));
        }
        ivars->buffer[ivars->buf_max++] = INCREF(elem);
    }

    return ivars->buf_max;
}

void
BBSortEx_Flip_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    uint32_t run_mem_thresh = 65536;

    BBSortEx_Flush(self);

    // Recalculate the approximate mem allowed for each run.
    uint32_t num_runs = VA_Get_Size(ivars->runs);
    if (num_runs) {
        run_mem_thresh = (ivars->mem_thresh / 2) / num_runs;
        if (run_mem_thresh < 65536) {
            run_mem_thresh = 65536;
        }
    }

    for (uint32_t i = 0; i < num_runs; i++) {
        BBSortEx *run = (BBSortEx*)VA_Fetch(ivars->runs, i);
        BBSortEx_Set_Mem_Thresh(run, run_mem_thresh);
    }

    // OK to fetch now.
    ivars->flipped = true;
}

int
BBSortEx_Compare_IMP(BBSortEx *self, void *va, void *vb) {
    UNUSED_VAR(self);
    return BB_compare((ByteBuf**)va, (ByteBuf**)vb);
}

VArray*
BBSortEx_Peek_Cache_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    uint32_t   count  = ivars->buf_max - ivars->buf_tick;
    Obj      **buffer = ivars->buffer;
    VArray    *retval = VA_new(count);

    for (uint32_t i = ivars->buf_tick; i < ivars->buf_max; ++i) {
        VA_Push(retval, INCREF(buffer[i]));
    }

    return retval;
}

Obj*
BBSortEx_Peek_Last_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    uint32_t count = ivars->buf_max - ivars->buf_tick;
    if (count == 0) { return NULL; }
    return INCREF(ivars->buffer[count-1]);
}

uint32_t
BBSortEx_Get_Num_Runs_IMP(BBSortEx *self) {
    BBSortExIVARS *const ivars = BBSortEx_IVARS(self);
    return VA_Get_Size(ivars->runs);
}


