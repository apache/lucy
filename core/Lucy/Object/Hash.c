#define C_LUCY_HASH
#define C_LUCY_ZOMBIECHARBUF
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include <stdlib.h>

#include "Lucy/Object/VTable.h"

#include "Lucy/Object/Hash.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/Undefined.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"
#include "Lucy/Util/Memory.h"

#define HashEntry lucy_HashEntry

typedef struct lucy_HashEntry {
    Obj *key;
    Obj *value;
    i32_t hash_code;
} lucy_HashEntry;

/* Reset the iterator.  Hash_iter_init must be called to restart iteration.
 */
static INLINE void
SI_kill_iter(Hash *self);

/* Return the entry associated with the key, if any.
 */
static INLINE HashEntry*
SI_fetch_entry(Hash *self, const Obj *key, i32_t hash_code);

/* Double the number of buckets and redistribute all entries. 
 *
 * This should be a static inline function, but right now we need to suppress
 * memory leaks from it because the VTable_registry never gets completely
 * cleaned up.
 */
HashEntry*
lucy_Hash_rebuild_hash(Hash *self);

Hash*
Hash_new(u32_t capacity)
{
    Hash *self = (Hash*)VTable_Make_Obj(HASH);
    return Hash_init(self, capacity);
}

Hash*
Hash_init(Hash *self, u32_t capacity)
{
    /* Allocate enough space to hold the requested number of elements without
     * triggering a rebuild. */
    u32_t requested_capacity = capacity < I32_MAX ? capacity : I32_MAX;
    u32_t threshold;
    capacity = 16;
    while (1) {
        threshold = (capacity / 3) * 2;
        if (threshold > requested_capacity) { break; }
        capacity *= 2;
    }

    /* Init. */
    self->size         = 0;
    self->iter_tick    = -1;

    /* Derive. */
    self->capacity     = capacity;
    self->entries      = (HashEntry*)CALLOCATE(capacity, sizeof(HashEntry));
    self->threshold    = threshold;

    return self;
}

void 
Hash_destroy(Hash *self) 
{
    if (self->entries) {
        Hash_Clear(self);
        FREEMEM(self->entries);
    }
    SUPER_DESTROY(self, HASH);
}

Hash*
Hash_dump(Hash *self)
{
    Hash *dump = Hash_new(self->size);
    Obj *key;
    Obj *value;

    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &value)) {
        /* Since JSON only supports text hash keys, Dump() can only support
         * text hash keys. */
        CERTIFY(key, CHARBUF);
        Hash_Store(dump, key, Obj_Dump(value));
    }

    return dump;
}

Obj*
Hash_load(Hash *self, Obj *dump)
{
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    UNUSED_VAR(self);

    /* Assume that the presence of the "_class" key paired with a valid class
     * name indicates the output of a Dump rather than an ordinary Hash. */
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
                /* TODO: Fix Hash_Load() so that it works with ordinary hash
                 * keys named "_class". */
                THROW(ERR, "Can't find class '%o'", class_name);
            }
        }

        /* Dispatch to an alternate Load() method. */
        if (vtable) {
            Obj_load_t load = (Obj_load_t)METHOD(vtable, Obj, Load);
            if (load == Obj_load) {
                THROW(ERR, "Abstract method Load() not defined for %o", 
                    VTable_Get_Name(vtable));
            }
            else if (load != (Obj_load_t)Hash_load) { /* stop inf loop */
                return load(NULL, dump);
            }
        }
    }

    /* It's an ordinary Hash. */
    {
        Hash *loaded = Hash_new(source->size);
        Obj *key;
        Obj *value;

        Hash_Iter_Init(source);
        while (Hash_Iter_Next(source, &key, &value)) {
            Hash_Store(loaded, key, Obj_Load(value, value));
        }

        return (Obj*)loaded;
    }
}

void
Hash_serialize(Hash *self, OutStream *outstream)
{
    Obj *key;
    Obj *val;
    u32_t charbuf_count = 0;
    OutStream_Write_C32(outstream, self->size);

    /* Write CharBuf keys first.  CharBuf keys are the common case; grouping
     * them together is a form of run-length-encoding and saves space, since
     * we omit the per-key class name. */
    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) {
        if (Obj_Is_A(key, CHARBUF)) { charbuf_count++; }
    }
    OutStream_Write_C32(outstream, charbuf_count);
    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) {
        if (Obj_Is_A(key, CHARBUF)) { 
            Obj_Serialize(key, outstream);
            FREEZE(val, outstream);
        }
    }

    /* Punt on the classes of the remaining keys. */
    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) {
        if (!Obj_Is_A(key, CHARBUF)) { 
            FREEZE(key, outstream);
            FREEZE(val, outstream);
        }
    }
}

Hash*
Hash_deserialize(Hash *self, InStream *instream)
{
    u32_t    size         = InStream_Read_C32(instream);
    u32_t    num_charbufs = InStream_Read_C32(instream);
    u32_t    num_other    = size - num_charbufs;
    CharBuf *key          = num_charbufs ? CB_new(0) : NULL;

    if (self) Hash_init(self, size);
    else self = Hash_new(size);
 
    /* Read key-value pairs with CharBuf keys. */
    while (num_charbufs--) {
        u32_t len = InStream_Read_C32(instream);
        char *key_buf = CB_Grow(key, len);
        InStream_Read_Bytes(instream, key_buf, len);
        key_buf[len] = '\0';
        CB_Set_Size(key, len);
        Hash_Store(self, (Obj*)key, THAW(instream));
    }
    DECREF(key);
    
    /* Read remaining key/value pairs. */
    while (num_other--) {
        Obj *k = THAW(instream);
        Hash_Store(self, k, THAW(instream));
        DECREF(k);
    }

    return self;
}

void
Hash_clear(Hash *self) 
{
    HashEntry *entry       = (HashEntry*)self->entries;
    HashEntry *const limit = entry + self->capacity;

    /* Iterate through all entries. */
    for ( ; entry < limit; entry++) {
        if (!entry->key) { continue; }
        DECREF(entry->key);
        DECREF(entry->value);
        entry->key       = NULL;
        entry->value     = NULL;
        entry->hash_code = 0;
    }

    self->size = 0;
}

void
lucy_Hash_do_store(Hash *self, Obj *key, Obj *value, 
                   i32_t hash_code, bool_t use_this_key)
{
    HashEntry   *entries = self->size >= self->threshold
                         ? lucy_Hash_rebuild_hash(self)
                         : (HashEntry*)self->entries;
    HashEntry   *entry;
    u32_t        tick    = hash_code;
    const u32_t  mask    = self->capacity - 1;

    while (1) {
        tick &= mask;
        entry = entries + tick;
        if (entry->key == (Obj*)UNDEF || !entry->key) {
            if (entry->key == (Obj*)UNDEF) { 
                /* Take note of diminished tombstone clutter. */
                self->threshold++; 
            }
            entry->key       = use_this_key 
                             ? key 
                             : Hash_Make_Key(self, key, hash_code);
            entry->value     = value;
            entry->hash_code = hash_code;
            self->size++;
            break;
        }
        else if (   entry->hash_code == hash_code
                 && Obj_Equals(key, entry->key)
        ) {
            DECREF(entry->value);
            entry->value = value;
            break;
        }
        tick++; /* linear scan */
    }
}

void
Hash_store(Hash *self, Obj *key, Obj *value) 
{
    lucy_Hash_do_store(self, key, value, Obj_Hash_Code(key), false);
}

void
Hash_store_str(Hash *self, const char *key, size_t key_len, Obj *value)
{
    ZombieCharBuf key_buf = ZCB_make_str((char*)key, key_len);
    lucy_Hash_do_store(self, (Obj*)&key_buf, value, 
        ZCB_Hash_Code(&key_buf), false);
}

Obj*
Hash_make_key(Hash *self, Obj *key, i32_t hash_code)
{
    UNUSED_VAR(self);
    UNUSED_VAR(hash_code);
    return Obj_Clone(key);
}

Obj*
Hash_fetch_str(Hash *self, const char *key, size_t key_len) 
{
    ZombieCharBuf key_buf = ZCB_make_str(key, key_len);
    return Hash_fetch(self, (Obj*)&key_buf);
}

static INLINE HashEntry*
SI_fetch_entry(Hash *self, const Obj *key, i32_t hash_code) 
{
    u32_t tick = hash_code;
    HashEntry *const entries = (HashEntry*)self->entries;
    HashEntry *entry;

    while (1) {
        tick &= self->capacity - 1;
        entry = entries + tick;
        if (!entry->key) { 
            /* Failed to find the key, so return NULL. */
            return NULL; 
        }
        else if (   entry->hash_code == hash_code
                 && Obj_Equals(key, entry->key)
        ) {
            return entry;
        }
        tick++;
    }
}

Obj*
Hash_fetch(Hash *self, const Obj *key) 
{
    HashEntry *entry = SI_fetch_entry(self, key, Obj_Hash_Code(key));
    return entry ? entry->value : NULL;
}

Obj*
Hash_delete(Hash *self, const Obj *key) 
{
    HashEntry *entry = SI_fetch_entry(self, key, Obj_Hash_Code(key));
    if (entry) {
        Obj *value = entry->value;
        DECREF(entry->key);
        entry->key       = (Obj*)UNDEF;
        entry->value     = NULL;
        entry->hash_code = 0;
        self->size--;
        self->threshold--; /* limit number of tombstones */
        return value;
    }
    else {
        return NULL;
    }
}

Obj*
Hash_delete_str(Hash *self, const char *key, size_t key_len) 
{
    ZombieCharBuf key_buf = ZCB_make_str(key, key_len);
    return Hash_delete(self, (Obj*)&key_buf);
}

u32_t
Hash_iter_init(Hash *self) 
{
    SI_kill_iter(self);
    return self->size;
}

static INLINE void
SI_kill_iter(Hash *self) 
{
    self->iter_tick = -1;
}

bool_t
Hash_iter_next(Hash *self, Obj **key, Obj **value) 
{
    while (1) {
        if (++self->iter_tick >= (i32_t)self->capacity) {
            /* Bail since we've completed the iteration. */
            --self->iter_tick;
            *key   = NULL;
            *value = NULL;
            return false;
        }
        else {
            HashEntry *const entry 
                = (HashEntry*)self->entries + self->iter_tick;
            if (entry->key && entry->key != (Obj*)UNDEF) {
                /* Success! */
                *key   = entry->key;
                *value = entry->value;
                return true;
            }
        }
    }
}

Obj*
Hash_find_key(Hash *self, const Obj *key, i32_t hash_code)
{
    HashEntry *entry = SI_fetch_entry(self, key, hash_code);
    return entry ? entry->key : NULL;
}

VArray*
Hash_keys(Hash *self) 
{
    Obj *key;
    Obj *val;
    VArray *keys = VA_new(self->size);
    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) {
        VA_push(keys, INCREF(key));
    }
    return keys;
}

VArray*
Hash_values(Hash *self) 
{
    Obj *key;
    Obj *val;
    VArray *values = VA_new(self->size);
    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) VA_push(values, INCREF(val));
    return values;
}

bool_t
Hash_equals(Hash *self, Obj *other)
{
    Hash    *evil_twin = (Hash*)other;
    Obj     *key;
    Obj     *val;

    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, HASH)) return false;
    if (self->size != evil_twin->size) return false;

    Hash_Iter_Init(self);
    while (Hash_Iter_Next(self, &key, &val)) {
        Obj *other_val = Hash_Fetch(evil_twin, key);
        if (!other_val || !Obj_Equals(other_val, val)) return false;
    }

    return true;
}

u32_t
Hash_get_capacity(Hash *self) { return self->capacity; }
u32_t
Hash_get_size(Hash *self)     { return self->size; }

HashEntry*
lucy_Hash_rebuild_hash(Hash *self)
{
    HashEntry *old_entries   = (HashEntry*)self->entries;
    HashEntry *entry         = old_entries;
    HashEntry *limit         = old_entries + self->capacity;

    SI_kill_iter(self);
    self->capacity *= 2;
    self->threshold = (self->capacity / 3) * 2;
    self->entries   = (HashEntry*)CALLOCATE(self->capacity, sizeof(HashEntry));
    self->size      = 0;

    for ( ; entry < limit; entry++) {
        if (!entry->key || entry->key == (Obj*)UNDEF) {
            continue; 
        }
        lucy_Hash_do_store(self, entry->key, entry->value, 
            entry->hash_code, true);
    }

    FREEMEM(old_entries);

    return (HashEntry*)self->entries;
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

