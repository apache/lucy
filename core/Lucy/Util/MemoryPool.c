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

#define C_LUCY_MEMORYPOOL
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Util/MemoryPool.h"

static void
S_init_arena(MemoryPool *self, MemoryPoolIVARS *ivars, size_t amount);

#define DEFAULT_BUF_SIZE 0x100000 // 1 MiB

// Enlarge amount so pointers will always be aligned.
#define INCREASE_TO_WORD_MULTIPLE(_amount) \
    do { \
        const size_t _remainder = _amount % sizeof(void*); \
        if (_remainder) { \
            _amount += sizeof(void*); \
            _amount -= _remainder; \
        } \
    } while (0)

MemoryPool*
MemPool_new(uint32_t arena_size) {
    MemoryPool *self = (MemoryPool*)Class_Make_Obj(MEMORYPOOL);
    return MemPool_init(self, arena_size);
}

MemoryPool*
MemPool_init(MemoryPool *self, uint32_t arena_size) {
    MemoryPoolIVARS *const ivars = MemPool_IVARS(self);
    ivars->arena_size = arena_size == 0 ? DEFAULT_BUF_SIZE : arena_size;
    ivars->arenas     = VA_new(16);
    ivars->tick       = -1;
    ivars->buf        = NULL;
    ivars->limit      = NULL;
    ivars->consumed   = 0;

    return self;
}

void
MemPool_Destroy_IMP(MemoryPool *self) {
    MemoryPoolIVARS *const ivars = MemPool_IVARS(self);
    DECREF(ivars->arenas);
    SUPER_DESTROY(self, MEMORYPOOL);
}

static void
S_init_arena(MemoryPool *self, MemoryPoolIVARS *ivars, size_t amount) {
    UNUSED_VAR(self);
    ByteBuf *bb;

    // Indicate which arena we're using at present.
    ivars->tick++;

    if (ivars->tick < (int32_t)VA_Get_Size(ivars->arenas)) {
        // In recycle mode, use previously acquired memory.
        bb = (ByteBuf*)VA_Fetch(ivars->arenas, ivars->tick);
        if (amount >= BB_Get_Size(bb)) {
            BB_Grow(bb, amount);
            BB_Set_Size(bb, amount);
        }
    }
    else {
        // In add mode, get more mem from system.
        size_t buf_size = (amount + 1) > ivars->arena_size
                          ? (amount + 1)
                          : ivars->arena_size;
        char *ptr = (char*)MALLOCATE(buf_size);
        bb = BB_new_steal_bytes(ptr, buf_size - 1, buf_size);
        VA_Push(ivars->arenas, (Obj*)bb);
    }

    // Recalculate consumption to take into account blocked off space.
    ivars->consumed = 0;
    for (int32_t i = 0; i < ivars->tick; i++) {
        ByteBuf *bb = (ByteBuf*)VA_Fetch(ivars->arenas, i);
        ivars->consumed += BB_Get_Size(bb);
    }

    ivars->buf   = BB_Get_Buf(bb);
    ivars->limit = ivars->buf + BB_Get_Size(bb);
}

size_t
MemPool_Get_Consumed_IMP(MemoryPool *self) {
    return MemPool_IVARS(self)->consumed;
}

void*
MemPool_Grab_IMP(MemoryPool *self, size_t amount) {
    MemoryPoolIVARS *const ivars = MemPool_IVARS(self);
    INCREASE_TO_WORD_MULTIPLE(amount);
    ivars->last_buf = ivars->buf;

    // Verify that we have enough stocked up, otherwise get more.
    ivars->buf += amount;
    if (ivars->buf >= ivars->limit) {
        // Get enough mem from system or die trying.
        S_init_arena(self, ivars, amount);
        ivars->last_buf = ivars->buf;
        ivars->buf += amount;
    }

    // Track bytes we've allocated from this pool.
    ivars->consumed += amount;

    return ivars->last_buf;
}

void
MemPool_Resize_IMP(MemoryPool *self, void *ptr, size_t new_amount) {
    MemoryPoolIVARS *const ivars = MemPool_IVARS(self);
    const size_t last_amount = ivars->buf - ivars->last_buf;
    INCREASE_TO_WORD_MULTIPLE(new_amount);

    if (ptr != ivars->last_buf) {
        THROW(ERR, "Not the last pointer allocated.");
    }
    else {
        if (new_amount <= last_amount) {
            const size_t difference = last_amount - new_amount;
            ivars->buf      -= difference;
            ivars->consumed -= difference;
        }
        else {
            THROW(ERR, "Can't resize to greater amount: %u64 > %u64",
                  (uint64_t)new_amount, (uint64_t)last_amount);
        }
    }
}

void
MemPool_Release_All_IMP(MemoryPool *self) {
    MemoryPoolIVARS *const ivars = MemPool_IVARS(self);
    DECREF(ivars->arenas);
    ivars->arenas   = VA_new(16);
    ivars->tick     = -1;
    ivars->buf      = NULL;
    ivars->last_buf = NULL;
    ivars->limit    = NULL;
    ivars->consumed = 0;
}

