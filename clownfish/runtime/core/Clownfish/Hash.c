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
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include <stdlib.h>

#include "Clownfish/VTable.h"

#include "Clownfish/Hash.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/VArray.h"
#include "Clownfish/Util/Memory.h"

static HashTombStone *TOMBSTONE;

#define HashEntry lucy_HashEntry

typedef struct HashEntry {
    Obj     *key;
    Obj     *value;
    int32_t  hash_sum;
} HashEntry;

// Reset the iterator.  Hash_Iterate must be called to restart iteration.
static INLINE void
SI_kill_iter(Hash *self);

// Return the entry associated with the key, if any.
static INLINE HashEntry*
SI_fetch_entry(Hash *self, const Obj *key, int32_t hash_sum);

// Double the number of buckets and redistribute all entries.
static INLINE HashEntry*
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
Hash_destroy(Hash *self) {
    if (self->entries) {
        Hash_Clear(self);
        FREEMEM(self->entries);
    }
    SUPER_DESTROY(self, HASH);
}

Hash*
Hash_dump(Hash *self) {
    Hash *dump = Hash_new(self->size);
    Obj *key;
    Obj *value;

    Hash_Iterate(self);
    while (Hash_Next(self, &key, &value)) {
        // Since JSON only supports text hash keys, Dump() can only support
        // text hash keys.
        CERTIFY(key, CHARBUF);
        Hash_Store(dump, key, Obj_Dump(value));
    }

    return dump;
}

Obj*
Hash_load(Hash *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    UNUSED_VAR(self);

    // Assume that the presence of the "_class" key paired with a valid class
    // name indicates the output of a Dump rather than an ordinary Hash. */
    if (class_name && CB_Is_A(class_name, CHARBUF)) {
        VTable *vtable = VTable_fetch_vtable(class_name);

        if (!vtable) {
            CharBuf *parent_class = VTable_find_parent_class(class_name);
            if (parent_class) {
                VTable *parent = VTable_singleton(parent_class, NULL);
                vtable = VTable_singleton(class_name, parent);
                DECREF(parent_class);
            }
            else {
                // TODO: Fix Hash_Load() so that it works with ordinary hash
                // keys named "_class".
                THROW(ERR, "Can't find class '%o'", class_name);
            }
        }

        // Dispatch to an alternate Load() method.
        if (vtable) {
            Obj_Load_t load = METHOD_PTR(vtable, Lucy_Obj_Load);
            if (load == Obj_load) {
                THROW(ERR, "Abstract method Load() not defined for %o",
                      VTable_Get_Name(vtable));
            }
            else if (load != (Obj_Load_t)Hash_load) { // stop inf loop
                return VTable_Load_Obj(vtable, dump);
            }
        }
    }

    // It's an ordinary Hash.
    Hash *loaded = Hash_new(source->size);
    Obj *key;
    Obj *value;
    Hash_Iterate(source);
    while (Hash_Next(source, &key, &value)) {
        Hash_Store(loaded, key, Obj_Load(value, value));
    }

    return (Obj*)loaded;

}

void
Hash_clear(Hash *self) {
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
Hash_store(Hash *self, Obj *key, Obj *value) {
    Hash_do_store(self, key, value, Obj_Hash_Sum(key), false);
}

void
Hash_store_str(Hash *self, const char *key, size_t key_len, Obj *value) {
    ZombieCharBuf *key_buf = ZCB_WRAP_STR((char*)key, key_len);
    Hash_do_store(self, (Obj*)key_buf, value,
                  ZCB_Hash_Sum(key_buf), false);
}

Obj*
Hash_make_key(Hash *self, Obj *key, int32_t hash_sum) {
    UNUSED_VAR(self);
    UNUSED_VAR(hash_sum);
    return Obj_Clone(key);
}

Obj*
Hash_fetch_str(Hash *self, const char *key, size_t key_len) {
    ZombieCharBuf *key_buf = ZCB_WRAP_STR(key, key_len);
    return Hash_fetch(self, (Obj*)key_buf);
}

static INLINE HashEntry*
SI_fetch_entry(Hash *self, const Obj *key, int32_t hash_sum) {
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
Hash_fetch(Hash *self, const Obj *key) {
    HashEntry *entry = SI_fetch_entry(self, key, Obj_Hash_Sum(key));
    return entry ? entry->value : NULL;
}

Obj*
Hash_delete(Hash *self, const Obj *key) {
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
Hash_delete_str(Hash *self, const char *key, size_t key_len) {
    ZombieCharBuf *key_buf = ZCB_WRAP_STR(key, key_len);
    return Hash_delete(self, (Obj*)key_buf);
}

uint32_t
Hash_iterate(Hash *self) {
    SI_kill_iter(self);
    return self->size;
}

static INLINE void
SI_kill_iter(Hash *self) {
    self->iter_tick = -1;
}

bool
Hash_next(Hash *self, Obj **key, Obj **value) {
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
Hash_find_key(Hash *self, const Obj *key, int32_t hash_sum) {
    HashEntry *entry = SI_fetch_entry(self, key, hash_sum);
    return entry ? entry->key : NULL;
}

VArray*
Hash_keys(Hash *self) {
    Obj *key;
    Obj *val;
    VArray *keys = VA_new(self->size);
    Hash_Iterate(self);
    while (Hash_Next(self, &key, &val)) {
        VA_push(keys, INCREF(key));
    }
    return keys;
}

VArray*
Hash_values(Hash *self) {
    Obj *key;
    Obj *val;
    VArray *values = VA_new(self->size);
    Hash_Iterate(self);
    while (Hash_Next(self, &key, &val)) { VA_push(values, INCREF(val)); }
    return values;
}

bool
Hash_equals(Hash *self, Obj *other) {
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
Hash_get_capacity(Hash *self) {
    return self->capacity;
}

uint32_t
Hash_get_size(Hash *self) {
    return self->size;
}

static INLINE HashEntry*
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
HashTombStone_get_refcount(HashTombStone* self) {
    CHY_UNUSED_VAR(self);
    return 1;
}

HashTombStone*
HashTombStone_inc_refcount(HashTombStone* self) {
    return self;
}

uint32_t
HashTombStone_dec_refcount(HashTombStone* self) {
    UNUSED_VAR(self);
    return 1;
}


