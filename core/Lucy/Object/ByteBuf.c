#define C_LUCY_BYTEBUF
#define C_LUCY_VIEWBYTEBUF
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Lucy/Object/VTable.h"
#include "Lucy/Object/ByteBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Util/Memory.h"

/* Reallocate if necessary. */
static INLINE void
SI_maybe_grow(ByteBuf *self, size_t capacity);

ByteBuf*
BB_new(size_t capacity) 
{
    ByteBuf *self = (ByteBuf*)VTable_Make_Obj(BYTEBUF);
    return BB_init(self, capacity);
}

ByteBuf*
BB_init(ByteBuf *self, size_t capacity)
{
    size_t amount = capacity ? capacity : sizeof(i64_t);
    self->buf   = NULL;
    self->size  = 0;
    self->cap   = 0;
    SI_maybe_grow(self, amount);
    return self;
}

ByteBuf*
BB_new_bytes(const void *bytes, size_t size) 
{
    ByteBuf *self = (ByteBuf*)VTable_Make_Obj(BYTEBUF);
    BB_init(self, size);
    memcpy(self->buf, bytes, size);
    self->size = size;
    return self;
}

ByteBuf*
BB_new_steal_bytes(void *bytes, size_t size, size_t capacity) 
{
    ByteBuf *self = (ByteBuf*)VTable_Make_Obj(BYTEBUF);
    self->buf  = bytes;
    self->size = size;
    self->cap  = capacity;
    return self;
}

void 
BB_destroy(ByteBuf *self) 
{
    FREEMEM(self->buf);
    SUPER_DESTROY(self, BYTEBUF);
}

ByteBuf*
BB_clone(ByteBuf *self) 
{
    return BB_new_bytes(self->buf, self->size);
}

void
BB_set_size(ByteBuf *self, size_t size) 
{ 
    if (size > self->cap) {
        THROW(ERR, "Can't set size to %u64 ( greater than capacity of %u64)",
            (u64_t)size, (u64_t)self->cap);
    }
    self->size = size; 
}

char*
BB_get_buf(ByteBuf *self)      { return self->buf; }
size_t
BB_get_size(ByteBuf *self)     { return self->size; }
size_t
BB_get_capacity(ByteBuf *self) { return self->cap; }

static INLINE bool_t
SI_equals_bytes(ByteBuf *self, const void *bytes, size_t size)
{
    if (self->size != size) { return false; }
    return (memcmp(self->buf, bytes, self->size) == 0);
}

bool_t
BB_equals(ByteBuf *self, Obj *other)
{
    ByteBuf *const evil_twin = (ByteBuf*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(evil_twin, BYTEBUF)) return false;
    return SI_equals_bytes(self, evil_twin->buf, evil_twin->size);
}

bool_t
BB_equals_bytes(ByteBuf *self, const void *bytes, size_t size)
{
    return SI_equals_bytes(self, bytes, size);
}

i32_t
BB_hash_code(ByteBuf *self)
{
    size_t size = self->size; 
    const u8_t *buf = (const u8_t*)self->buf; 
    u32_t hashvalue = 0; 

    while (size--) { 
        hashvalue += *buf++; 
        hashvalue += (hashvalue << 10); 
        hashvalue ^= (hashvalue >> 6); 
    } 
    hashvalue += (hashvalue << 3); 
    hashvalue ^= (hashvalue >> 11); 
    hashvalue += (hashvalue << 15); 

    return (i32_t) hashvalue;
}

static INLINE void
SI_mimic_bytes(ByteBuf *self, const void *bytes, size_t size) 
{
    SI_maybe_grow(self, size);
    memmove(self->buf, bytes, size);
    self->size = size;
}

void
BB_mimic_bytes(ByteBuf *self, const void *bytes, size_t size) 
{
    SI_mimic_bytes(self, bytes, size);
}

void
BB_mimic(ByteBuf *self, Obj *other)
{
    ByteBuf *evil_twin = (ByteBuf*)CERTIFY(other, BYTEBUF);
    SI_mimic_bytes(self, evil_twin->buf, evil_twin->size);
}

static INLINE void 
SI_cat_bytes(ByteBuf *self, const void *bytes, size_t size) 
{
    const size_t new_size = self->size + size;
    SI_maybe_grow(self, new_size);
    memcpy((self->buf + self->size), bytes, size);
    self->size = new_size;
}

void 
BB_cat_bytes(ByteBuf *self, const void *bytes, size_t size) 
{
    SI_cat_bytes(self, bytes, size);
}

void 
BB_cat(ByteBuf *self, const ByteBuf *other) 
{
    SI_cat_bytes(self, other->buf, other->size);
}

static void
S_grow(ByteBuf *self, size_t capacity)
{
    size_t bleedover = capacity % sizeof(i64_t);
    size_t amount    = capacity + sizeof(i64_t) - bleedover;
    self->buf = (char*)REALLOCATE(self->buf, amount);
    self->cap = amount;
}

static INLINE void
SI_maybe_grow(ByteBuf *self, size_t capacity) 
{
    /* Reallocate only if necessary. */
    if (self->cap < capacity) { S_grow(self, capacity); }
}

char*
BB_grow(ByteBuf *self, size_t capacity) 
{
    SI_maybe_grow(self, capacity);
    return self->buf;
}

int 
BB_compare(const void *va, const void *vb) 
{
    const ByteBuf *a = *(const ByteBuf**)va;
    const ByteBuf *b = *(const ByteBuf**)vb;
    const size_t size = a->size < b->size ? a->size : b->size;

    i32_t comparison = memcmp(a->buf, b->buf, size);

    if (comparison == 0 && a->size != b->size) 
        comparison = a->size < b->size ? -1 : 1;

    return comparison;
}

/******************************************************************/

ViewByteBuf*
ViewBB_new(char *buf, size_t size) 
{
    ViewByteBuf *self = (ViewByteBuf*)VTable_Make_Obj(VIEWBYTEBUF);

    /* Init. */
    self->cap = 0;

    /* Assign. */
    self->buf  = buf;
    self->size = size;
    
    return self;
}

void 
ViewBB_destroy(ViewByteBuf *self) 
{
    Obj_destroy((Obj*)self);
}

void
ViewBB_assign_bytes(ViewByteBuf *self, char*buf, size_t size) 
{
    self->buf  = buf;
    self->size = size;
}

void
ViewBB_assign(ViewByteBuf *self, const ByteBuf *other)
{
    self->buf  = other->buf;
    self->size = other->size;
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

