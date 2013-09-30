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

#define C_CFISH_HASH
#define C_CFISH_HASHTOMBSTONE
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "charmony.h"

#include <string.h>
#include <stdlib.h>

#include "Clownfish/VTable.h"

#include "Clownfish/Hash.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/VArray.h"
#include "Clownfish/Util/Memory.h"

static HashTombStone *TOMBSTONE;

#define HashEntry cfish_HashEntry

typedef struct HashEntry {
    Obj     *key;
    Obj     *value;
    int32_t  hash_sum;
} HashEntry;

// Reset the iterator.  Hash_Iterate must be called to restart iteration.
static CFISH_INLINE void
SI_kill_iter(Hash *self);

// Return the entry associated with the key, if any.
static CFISH_INLINE HashEntry*
SI_fetch_entry(Hash *self, Obj *key, int32_t hash_sum);

// Double the number of buckets and redistribute all entries.
static CFISH_INLINE HashEntry*
SI_rebuild_hash(Hash *self);

void
Hash_init_class() {
    TOMBSTONE = (HashTombStone*)VTable_Make_Obj(HASHTOMBSTONE);
}

Hash*
Hash_new(uint32_t capacity) {
    Hash *self = (Hash*)VTable_Make_Obj(HASH);
    return Hash_init(self, capacity);
}

Hash*
Hash_init(Hash *self, uint32_t capacity) {
    // Allocate enough space to hold the requested number of elements without
    // triggering a rebuild.
    uint32_t requested_capacity = capacity < INT32_MAX ? capacity : INT32_MAX;
    uint32_t threshold;
    capacity = 16;
    while (1) {
        threshold = (capacity / 3) * 2;
        if (threshold > requested_capacity) { break; }
        capacity *= 2;
    }

    // Init.
    self->size         = 0;
    self->iter_tick    = -1;

    // Derive.
    self->capacity     = capacity;
    self->entries      = (HashEntry*)CALLOCATE(capacity, sizeof(HashEntry));
    self->threshold    = threshold;

    return self;
}

void
Hash_Destroy_IMP(Hash *self) {
    if (self->entries) {
        Hash_Clear(self);
        FREEMEM(self->entries);
    }
    SUPER_DESTROY(self, HASH);
}

void
Hash_Clear_IMP(Hash *self) {
    HashEntry *entry       = (HashEntry*)self->entries;
    HashEntry *const limit = entry + self->capacity;

    // Iterate through all entries.
    for (; entry < limit; entry++) {
        if (!entry->key) { continue; }
        DECREF(entry->key);
        DECREF(entry->value);
        entry->key       = NULL;
        entry->value     = NULL;
        entry->hash_sum  = 0;
    }

    self->size = 0;
}

void
Hash_do_store(Hash *self, Obj *key, Obj *value,
              int32_t hash_sum, bool use_this_key) {
    HashEntry *entries = self->size >= self->threshold
                         ? SI_rebuild_hash(self)
                         : (HashEntry*)self->entries;
    uint32_t       tick = hash_sum;
    const uint32_t mask = self->capacity - 1;

    while (1) {
        tick &= mask;
        HashEntry *entry = entries + tick;
        if (entry->key == (Obj*)TOMBSTONE || !entry->key) {
            if (entry->key == (Obj*)TOMBSTONE) {
                // Take note of diminished tombstone clutter.
                self->threshold++;
            }
            entry->key       = use_this_key
                               ? key
                               : Hash_Make_Key(self, key, hash_sum);
            entry->value     = value;
            entry->hash_sum  = hash_sum;
            self->size++;
            break;
        }
        else if (entry->hash_sum == hash_sum
                 && Obj_Equals(key, entry->key)
                ) {
            DECREF(entry->value);
            entry->value = value;
            break;
        }
        tick++; // linear scan
    }
}

void
Hash_Store_IMP(Hash *self, Obj *key, Obj *value) {
    Hash_do_store(self, key, value, Obj_Hash_Sum(key), false);
}

void
Hash_Store_Utf8_IMP(Hash *self, const char *key, size_t key_len, Obj *value) {
    StackString *key_buf = SSTR_WRAP_UTF8((char*)key, key_len);
    Hash_do_store(self, (Obj*)key_buf, value,
                  SStr_Hash_Sum(key_buf), false);
}

Obj*
Hash_Make_Key_IMP(Hash *self, Obj *key, int32_t hash_sum) {
    UNUSED_VAR(self);
    UNUSED_VAR(hash_sum);
    return Obj_Clone(key);
}

Obj*
Hash_Fetch_Utf8_IMP(Hash *self, const char *key, size_t key_len) {
    StackString *key_buf = SSTR_WRAP_UTF8(key, key_len);
    return Hash_Fetch_IMP(self, (Obj*)key_buf);
}

static CFISH_INLINE HashEntry*
SI_fetch_entry(Hash *self, Obj *key, int32_t hash_sum) {
    uint32_t tick = hash_sum;
    HashEntry *const entries = (HashEntry*)self->entries;
    HashEntry *entry;

    while (1) {
        tick &= self->capacity - 1;
        entry = entries + tick;
        if (!entry->key) {
            // Failed to find the key, so return NULL.
            return NULL;
        }
        else if (entry->hash_sum == hash_sum
                 && Obj_Equals(key, entry->key)
                ) {
            return entry;
        }
        tick++;
    }
}

Obj*
Hash_Fetch_IMP(Hash *self, Obj *key) {
    HashEntry *entry = SI_fetch_entry(self, key, Obj_Hash_Sum(key));
    return entry ? entry->value : NULL;
}

Obj*
Hash_Delete_IMP(Hash *self, Obj *key) {
    HashEntry *entry = SI_fetch_entry(self, key, Obj_Hash_Sum(key));
    if (entry) {
        Obj *value = entry->value;
        DECREF(entry->key);
        entry->key       = (Obj*)TOMBSTONE;
        entry->value     = NULL;
        entry->hash_sum  = 0;
        self->size--;
        self->threshold--; // limit number of tombstones
        return value;
    }
    else {
        return NULL;
    }
}

Obj*
Hash_Delete_Utf8_IMP(Hash *self, const char *key, size_t key_len) {
    StackString *key_buf = SSTR_WRAP_UTF8(key, key_len);
    return Hash_Delete_IMP(self, (Obj*)key_buf);
}

uint32_t
Hash_Iterate_IMP(Hash *self) {
    SI_kill_iter(self);
    return self->size;
}

static CFISH_INLINE void
SI_kill_iter(Hash *self) {
    self->iter_tick = -1;
}

bool
Hash_Next_IMP(Hash *self, Obj **key, Obj **value) {
    while (1) {
        if (++self->iter_tick >= (int32_t)self->capacity) {
            // Bail since we've completed the iteration.
            --self->iter_tick;
            *key   = NULL;
            *value = NULL;
            return false;
        }
        else {
            HashEntry *const entry
                = (HashEntry*)self->entries + self->iter_tick;
            if (entry->key && entry->key != (Obj*)TOMBSTONE) {
                // Success!
                *key   = entry->key;
                *value = entry->value;
                return true;
            }
        }
    }
}

Obj*
Hash_Find_Key_IMP(Hash *self, Obj *key, int32_t hash_sum) {
    HashEntry *entry = SI_fetch_entry(self, key, hash_sum);
    return entry ? entry->key : NULL;
}

VArray*
Hash_Keys_IMP(Hash *self) {
    Obj *key;
    Obj *val;
    VArray *keys = VA_new(self->size);
    Hash_Iterate(self);
    while (Hash_Next(self, &key, &val)) {
        VA_Push(keys, INCREF(key));
    }
    return keys;
}

VArray*
Hash_Values_IMP(Hash *self) {
    Obj *key;
    Obj *val;
    VArray *values = VA_new(self->size);
    Hash_Iterate(self);
    while (Hash_Next(self, &key, &val)) { VA_Push(values, INCREF(val)); }
    return values;
}

bool
Hash_Equals_IMP(Hash *self, Obj *other) {
    Hash    *twin = (Hash*)other;
    Obj     *key;
    Obj     *val;

    if (twin == self)             { return true; }
    if (!Obj_Is_A(other, HASH))   { return false; }
    if (self->size != twin->size) { return false; }

    Hash_Iterate(self);
    while (Hash_Next(self, &key, &val)) {
        Obj *other_val = Hash_Fetch(twin, key);
        if (!other_val || !Obj_Equals(other_val, val)) { return false; }
    }

    return true;
}

uint32_t
Hash_Get_Capacity_IMP(Hash *self) {
    return self->capacity;
}

uint32_t
Hash_Get_Size_IMP(Hash *self) {
    return self->size;
}

static CFISH_INLINE HashEntry*
SI_rebuild_hash(Hash *self) {
    HashEntry *old_entries = (HashEntry*)self->entries;
    HashEntry *entry       = old_entries;
    HashEntry *limit       = old_entries + self->capacity;

    SI_kill_iter(self);
    self->capacity *= 2;
    self->threshold = (self->capacity / 3) * 2;
    self->entries   = (HashEntry*)CALLOCATE(self->capacity, sizeof(HashEntry));
    self->size      = 0;

    for (; entry < limit; entry++) {
        if (!entry->key || entry->key == (Obj*)TOMBSTONE) {
            continue;
        }
        Hash_do_store(self, entry->key, entry->value,
                      entry->hash_sum, true);
    }

    FREEMEM(old_entries);

    return (HashEntry*)self->entries;
}

/***************************************************************************/

uint32_t
HashTombStone_Get_RefCount_IMP(HashTombStone* self) {
    CHY_UNUSED_VAR(self);
    return 1;
}

HashTombStone*
HashTombStone_Inc_RefCount_IMP(HashTombStone* self) {
    return self;
}

uint32_t
HashTombStone_Dec_RefCount_IMP(HashTombStone* self) {
    UNUSED_VAR(self);
    return 1;
}


