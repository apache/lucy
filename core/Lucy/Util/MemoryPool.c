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
S_init_arena(MemoryPool *self, size_t amount);

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
    MemoryPool *self = (MemoryPool*)VTable_Make_Obj(MEMORYPOOL);
    return MemPool_init(self, arena_size);
}

MemoryPool*
MemPool_init(MemoryPool *self, uint32_t arena_size) {
    self->arena_size = arena_size == 0 ? DEFAULT_BUF_SIZE : arena_size;
    self->arenas     = VA_new(16);
    self->tick       = -1;
    self->buf        = NULL;
    self->limit      = NULL;
    self->consumed   = 0;

    return self;
}

void
MemPool_destroy(MemoryPool *self) {
    DECREF(self->arenas);
    SUPER_DESTROY(self, MEMORYPOOL);
}

static void
S_init_arena(MemoryPool *self, size_t amount) {
    ByteBuf *bb;
    int32_t i;

    // Indicate which arena we're using at present.
    self->tick++;

    if (self->tick < (int32_t)VA_Get_Size(self->arenas)) {
        // In recycle mode, use previously acquired memory.
        bb = (ByteBuf*)VA_Fetch(self->arenas, self->tick);
        if (amount >= BB_Get_Size(bb)) {
            BB_Grow(bb, amount);
            BB_Set_Size(bb, amount);
        }
    }
    else {
        // In add mode, get more mem from system.
        size_t buf_size = (amount + 1) > self->arena_size
                          ? (amount + 1)
                          : self->arena_size;
        char *ptr = (char*)MALLOCATE(buf_size);
        bb = BB_new_steal_bytes(ptr, buf_size - 1, buf_size);
        VA_Push(self->arenas, (Obj*)bb);
    }

    // Recalculate consumption to take into account blocked off space.
    self->consumed = 0;
    for (i = 0; i < self->tick; i++) {
        ByteBuf *bb = (ByteBuf*)VA_Fetch(self->arenas, i);
        self->consumed += BB_Get_Size(bb);
    }

    self->buf   = BB_Get_Buf(bb);
    self->limit = self->buf + BB_Get_Size(bb);
}

size_t
MemPool_get_consumed(MemoryPool *self) {
    return self->consumed;
}

void*
MemPool_grab(MemoryPool *self, size_t amount) {
    INCREASE_TO_WORD_MULTIPLE(amount);
    self->last_buf = self->buf;

    // Verify that we have enough stocked up, otherwise get more.
    self->buf += amount;
    if (self->buf >= self->limit) {
        // Get enough mem from system or die trying.
        S_init_arena(self, amount);
        self->last_buf = self->buf;
        self->buf += amount;
    }

    // Track bytes we've allocated from this pool.
    self->consumed += amount;

    return self->last_buf;
}

void
MemPool_resize(MemoryPool *self, void *ptr, size_t new_amount) {
    const size_t last_amount = self->buf - self->last_buf;
    INCREASE_TO_WORD_MULTIPLE(new_amount);

    if (ptr != self->last_buf) {
        THROW(ERR, "Not the last pointer allocated.");
    }
    else {
        if (new_amount <= last_amount) {
            const size_t difference = last_amount - new_amount;
            self->buf      -= difference;
            self->consumed -= difference;
        }
        else {
            THROW(ERR, "Can't resize to greater amount: %u64 > %u64",
                  (uint64_t)new_amount, (uint64_t)last_amount);
        }
    }
}

void
MemPool_release_all(MemoryPool *self) {
    self->tick     = -1;
    self->buf      = NULL;
    self->last_buf = NULL;
    self->limit    = NULL;
}

void
MemPool_eat(MemoryPool *self, MemoryPool *other) {
    int32_t i;
    if (self->buf != NULL) {
        THROW(ERR, "Memory pool is not empty");
    }

    // Move active arenas from other to self.
    for (i = 0; i <= other->tick; i++) {
        ByteBuf *arena = (ByteBuf*)VA_Shift(other->arenas);
        // Maybe displace existing arena.
        VA_Store(self->arenas, i, (Obj*)arena);
    }
    self->tick     = other->tick;
    self->last_buf = other->last_buf;
    self->buf      = other->buf;
    self->limit    = other->limit;
}


