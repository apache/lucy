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

#define C_LUCY_BLOBSORTEX
#define LUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/BlobSortEx.h"

#include "Clownfish/Blob.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

BlobSortEx*
BlobSortEx_new(uint32_t mem_threshold, Vector *external) {
    BlobSortEx *self = (BlobSortEx*)Class_Make_Obj(BLOBSORTEX);
    return BlobSortEx_init(self, mem_threshold, external);
}

BlobSortEx*
BlobSortEx_init(BlobSortEx *self, uint32_t mem_threshold, Vector *external) {
    SortEx_init((SortExternal*)self);
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    ivars->external_tick = 0;
    ivars->external = (Vector*)INCREF(external);
    ivars->mem_consumed = 0;
    BlobSortEx_Set_Mem_Thresh(self, mem_threshold);
    return self;
}

void
BlobSortEx_Destroy_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    DECREF(ivars->external);
    SUPER_DESTROY(self, BLOBSORTEX);
}

void
BlobSortEx_Clear_Buffer_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    ivars->mem_consumed = 0;
    BlobSortEx_Clear_Buffer_t super_clear_buffer
        = SUPER_METHOD_PTR(BLOBSORTEX, LUCY_BlobSortEx_Clear_Buffer);
    super_clear_buffer(self);
}

void
BlobSortEx_Feed_IMP(BlobSortEx *self, Obj *item) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    BlobSortEx_Feed_t super_feed
        = SUPER_METHOD_PTR(BLOBSORTEX, LUCY_BlobSortEx_Feed);
    super_feed(self, item);

    // Flush() if necessary.
    Blob *blob = (Blob*)CERTIFY(item, BLOB);
    ivars->mem_consumed += Blob_Get_Size(blob);
    if (ivars->mem_consumed >= ivars->mem_thresh) {
        BlobSortEx_Flush(self);
    }
}

void
BlobSortEx_Flush_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    uint32_t     buf_count = ivars->buf_max - ivars->buf_tick;
    Obj        **buffer = ivars->buffer;
    Vector      *elems;

    if (!buf_count) { return; }
    else            { elems = Vec_new(buf_count); }

    // Sort, then create a new run.
    BlobSortEx_Sort_Buffer(self);
    for (uint32_t i = ivars->buf_tick; i < ivars->buf_max; i++) {
        Vec_Push(elems, buffer[i]);
    }
    BlobSortEx *run = BlobSortEx_new(0, elems);
    DECREF(elems);
    BlobSortEx_Add_Run(self, (SortExternal*)run);

    // Blank the buffer vars.
    ivars->buf_tick += buf_count;
    BlobSortEx_Clear_Buffer(self);
}

uint32_t
BlobSortEx_Refill_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);

    // Make sure buffer is empty, then set buffer tick vars.
    if (ivars->buf_max - ivars->buf_tick > 0) {
        THROW(ERR, "Refill called but buffer contains %u32 items",
              ivars->buf_max - ivars->buf_tick);
    }
    ivars->buf_tick = 0;
    ivars->buf_max  = 0;

    // Read in elements.
    while (1) {
        Blob *elem = NULL;

        if (ivars->mem_consumed >= ivars->mem_thresh) {
            ivars->mem_consumed = 0;
            break;
        }
        else if (ivars->external_tick >= Vec_Get_Size(ivars->external)) {
            break;
        }
        else {
            elem = (Blob*)Vec_Fetch(ivars->external, ivars->external_tick);
            ivars->external_tick++;
            // Should be + sizeof(Blob), but that's ok.
            ivars->mem_consumed += Blob_Get_Size(elem);
        }

        if (ivars->buf_max == ivars->buf_cap) {
            BlobSortEx_Grow_Buffer(self,
                                 Memory_oversize(ivars->buf_max + 1,
                                                 sizeof(Obj*)));
        }
        ivars->buffer[ivars->buf_max++] = INCREF(elem);
    }

    return ivars->buf_max;
}

void
BlobSortEx_Flip_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    uint32_t run_mem_thresh = 65536;

    BlobSortEx_Flush(self);

    // Recalculate the approximate mem allowed for each run.
    uint32_t num_runs = (uint32_t)Vec_Get_Size(ivars->runs);
    if (num_runs) {
        run_mem_thresh = (ivars->mem_thresh / 2) / num_runs;
        if (run_mem_thresh < 65536) {
            run_mem_thresh = 65536;
        }
    }

    for (uint32_t i = 0; i < num_runs; i++) {
        BlobSortEx *run = (BlobSortEx*)Vec_Fetch(ivars->runs, i);
        BlobSortEx_Set_Mem_Thresh(run, run_mem_thresh);
    }

    // OK to fetch now.
    ivars->flipped = true;
}

int
BlobSortEx_Compare_IMP(BlobSortEx *self, Obj **ptr_a, Obj **ptr_b) {
    UNUSED_VAR(self);
    return Obj_Compare_To(*ptr_a, *ptr_b);
}

Vector*
BlobSortEx_Peek_Cache_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    uint32_t   count  = ivars->buf_max - ivars->buf_tick;
    Obj      **buffer = ivars->buffer;
    Vector    *retval = Vec_new(count);

    for (uint32_t i = ivars->buf_tick; i < ivars->buf_max; ++i) {
        Vec_Push(retval, INCREF(buffer[i]));
    }

    return retval;
}

Obj*
BlobSortEx_Peek_Last_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    uint32_t count = ivars->buf_max - ivars->buf_tick;
    if (count == 0) { return NULL; }
    return INCREF(ivars->buffer[count-1]);
}

uint32_t
BlobSortEx_Get_Num_Runs_IMP(BlobSortEx *self) {
    BlobSortExIVARS *const ivars = BlobSortEx_IVARS(self);
    return (uint32_t)Vec_Get_Size(ivars->runs);
}


