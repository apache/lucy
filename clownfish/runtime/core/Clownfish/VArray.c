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

#define C_CFISH_VARRAY
#include <string.h>
#include <stdlib.h>

#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/VTable.h"
#include "Clownfish/VArray.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/SortUtils.h"

VArray*
VA_new(uint32_t capacity) {
    VArray *self = (VArray*)VTable_Make_Obj(VARRAY);
    VA_init(self, capacity);
    return self;
}

VArray*
VA_init(VArray *self, uint32_t capacity) {
    // Init.
    self->size = 0;

    // Assign.
    self->cap = capacity;

    // Derive.
    self->elems = (Obj**)CALLOCATE(capacity, sizeof(Obj*));

    return self;
}

void
VA_destroy(VArray *self) {
    if (self->elems) {
        Obj **elems        = self->elems;
        Obj **const limit  = elems + self->size;
        for (; elems < limit; elems++) {
            DECREF(*elems);
        }
        FREEMEM(self->elems);
    }
    SUPER_DESTROY(self, VARRAY);
}

VArray*
VA_dump(VArray *self) {
    VArray *dump = VA_new(self->size);
    for (uint32_t i = 0, max = self->size; i < max; i++) {
        Obj *elem = VA_Fetch(self, i);
        if (elem) { VA_Store(dump, i, Obj_Dump(elem)); }
    }
    return dump;
}

VArray*
VA_load(VArray *self, Obj *dump) {
    VArray *source = (VArray*)CERTIFY(dump, VARRAY);
    VArray *loaded = VA_new(source->size);
    UNUSED_VAR(self);

    for (uint32_t i = 0, max = source->size; i < max; i++) {
        Obj *elem_dump = VA_Fetch(source, i);
        if (elem_dump) {
            VA_Store(loaded, i, Obj_Load(elem_dump, elem_dump));
        }
    }

    return loaded;
}

VArray*
VA_clone(VArray *self) {
    VArray *twin = VA_new(self->size);

    // Clone each element.
    for (uint32_t i = 0; i < self->size; i++) {
        Obj *elem = self->elems[i];
        if (elem) {
            twin->elems[i] = Obj_Clone(elem);
        }
    }

    // Ensure that size is the same if NULL elems at end.
    twin->size = self->size;

    return twin;
}

VArray*
VA_shallow_copy(VArray *self) {
    // Dupe, then increment refcounts.
    VArray *twin = VA_new(self->size);
    Obj **elems = twin->elems;
    memcpy(elems, self->elems, self->size * sizeof(Obj*));
    twin->size = self->size;
    for (uint32_t i = 0; i < self->size; i++) {
        if (elems[i] != NULL) {
            (void)INCREF(elems[i]);
        }
    }

    return twin;
}

void
VA_push(VArray *self, Obj *element) {
    if (self->size == self->cap) {
        VA_Grow(self, Memory_oversize(self->size + 1, sizeof(Obj*)));
    }
    self->elems[self->size] = element;
    self->size++;
}

void
VA_push_varray(VArray *self, VArray *other) {
    uint32_t tick = self->size;
    uint32_t new_size = self->size + other->size;
    if (new_size > self->cap) {
        VA_Grow(self, Memory_oversize(new_size, sizeof(Obj*)));
    }
    for (uint32_t i = 0; i < other->size; i++, tick++) {
        Obj *elem = VA_Fetch(other, i);
        if (elem != NULL) {
            self->elems[tick] = INCREF(elem);
        }
    }
    self->size = new_size;
}

Obj*
VA_pop(VArray *self) {
    if (!self->size) {
        return NULL;
    }
    self->size--;
    return  self->elems[self->size];
}

void
VA_unshift(VArray *self, Obj *elem) {
    if (self->size == self->cap) {
        VA_Grow(self, Memory_oversize(self->size + 1, sizeof(Obj*)));
    }
    memmove(self->elems + 1, self->elems, self->size * sizeof(Obj*));
    self->elems[0] = elem;
    self->size++;
}

Obj*
VA_shift(VArray *self) {
    if (!self->size) {
        return NULL;
    }
    else {
        Obj *const return_val = self->elems[0];
        self->size--;
        if (self->size > 0) {
            memmove(self->elems, self->elems + 1,
                    self->size * sizeof(Obj*));
        }
        return return_val;
    }
}

Obj*
VA_fetch(VArray *self, uint32_t num) {
    if (num >= self->size) {
        return NULL;
    }

    return self->elems[num];
}

void
VA_store(VArray *self, uint32_t tick, Obj *elem) {
    if (tick >= self->cap) {
        VA_Grow(self, Memory_oversize(tick + 1, sizeof(Obj*)));
    }
    if (tick < self->size) { DECREF(self->elems[tick]); }
    else                   { self->size = tick + 1; }
    self->elems[tick] = elem;
}

void
VA_grow(VArray *self, uint32_t capacity) {
    if (capacity > self->cap) {
        self->elems = (Obj**)REALLOCATE(self->elems, capacity * sizeof(Obj*));
        self->cap   = capacity;
        memset(self->elems + self->size, 0,
               (capacity - self->size) * sizeof(Obj*));
    }
}

Obj*
VA_delete(VArray *self, uint32_t num) {
    Obj *elem = NULL;
    if (num < self->size) {
        elem = self->elems[num];
        self->elems[num] = NULL;
    }
    return elem;
}

void
VA_excise(VArray *self, uint32_t offset, uint32_t length) {
    if (self->size <= offset)              { return; }
    else if (self->size < offset + length) { length = self->size - offset; }

    for (uint32_t i = 0; i < length; i++) {
        DECREF(self->elems[offset + i]);
    }

    uint32_t num_to_move = self->size - (offset + length);
    memmove(self->elems + offset, self->elems + offset + length,
            num_to_move * sizeof(Obj*));
    self->size -= length;
}

void
VA_clear(VArray *self) {
    VA_excise(self, 0, self->size);
}

void
VA_resize(VArray *self, uint32_t size) {
    if (size < self->size) {
        VA_Excise(self, size, self->size - size);
    }
    else if (size > self->size) {
        VA_Grow(self, size);
    }
    self->size = size;
}

uint32_t
VA_get_size(VArray *self) {
    return self->size;
}

uint32_t
VA_get_capacity(VArray *self) {
    return self->cap;
}

static int
S_default_compare(void *context, const void *va, const void *vb) {
    Obj *a = *(Obj**)va;
    Obj *b = *(Obj**)vb;
    UNUSED_VAR(context);
    if (a != NULL && b != NULL)      { return Obj_Compare_To(a, b); }
    else if (a == NULL && b == NULL) { return 0;  }
    else if (a == NULL)              { return 1;  } // NULL to the back
    else  /* b == NULL */            { return -1; } // NULL to the back
}

void
VA_sort(VArray *self, Cfish_Sort_Compare_t compare, void *context) {
    if (!compare) { compare = S_default_compare; }
    Sort_quicksort(self->elems, self->size, sizeof(void*), compare, context);
}

bool
VA_equals(VArray *self, Obj *other) {
    VArray *twin = (VArray*)other;
    if (twin == self)             { return true; }
    if (!Obj_Is_A(other, VARRAY)) { return false; }
    if (twin->size != self->size) {
        return false;
    }
    else {
        for (uint32_t i = 0, max = self->size; i < max; i++) {
            Obj *val       = self->elems[i];
            Obj *other_val = twin->elems[i];
            if ((val && !other_val) || (other_val && !val)) { return false; }
            if (val && !Obj_Equals(val, other_val))         { return false; }
        }
    }
    return true;
}

VArray*
VA_gather(VArray *self, VA_Gather_Test_t test, void *data) {
    VArray *gathered = VA_new(self->size);
    for (uint32_t i = 0, max = self->size; i < max; i++) {
        if (test(self, i, data)) {
            Obj *elem = self->elems[i];
            VA_Push(gathered, elem ? INCREF(elem) : NULL);
        }
    }
    return gathered;
}

VArray*
VA_slice(VArray *self, uint32_t offset, uint32_t length) {
    // Adjust ranges if necessary.
    if (offset >= self->size) {
        offset = 0;
        length = 0;
    }
    else if (length > UINT32_MAX - offset
             || offset + length > self->size
            ) {
        length = self->size - offset;
    }

    // Copy elements.
    VArray *slice = VA_new(length);
    slice->size = length;
    Obj **slice_elems = slice->elems;
    Obj **my_elems    = self->elems;
    for (uint32_t i = 0; i < length; i++) {
        slice_elems[i] = INCREF(my_elems[offset + i]);
    }

    return slice;
}

