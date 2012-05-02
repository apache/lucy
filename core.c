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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"

struct Object {
    MetaClass *metaclass;
};

struct MetaClass {
    MetaClass *metaclass;
    MetaClass *parent;
    char *name;
    size_t obj_alloc_size;
    size_t num_methods;
    size_t method_count;
    Object superself;
};

uint64_t Obj_destroy_OFFSETS;
static void
S_Obj_destroy(Object *self) {
    ptrdiff_t fixup = (ptrdiff_t)self->metaclass->obj_alloc_size
                      - (ptrdiff_t)sizeof(Object);
    char *ptr = (char*)self - fixup;
    free(ptr);
}

/************************************************************************/

// Methods are allocated immediately after the end of the MetaClass struct.
method_t*
S_methods(MetaClass *self) {
    return (method_t*)((char*)self + sizeof(MetaClass));
}

uint64_t MetaClass_destroy_OFFSETS;
static void
S_MetaClass_destroy(MetaClass *self);

uint64_t MetaClass_make_obj_OFFSETS;
static void*
S_MetaClass_make_obj(MetaClass *self);

uint64_t MetaClass_add_novel_method_OFFSETS;
static uint64_t
S_MetaClass_add_novel_method(MetaClass *self, method_t method);

uint64_t MetaClass_override_method_OFFSETS;
static uint64_t
S_MetaClass_override_method(MetaClass *self, method_t method, uint64_t offsets);

uint64_t MetaClass_inherit_method_OFFSETS;
static uint64_t
S_MetaClass_inherit_method(MetaClass *self, uint64_t offsets);

uint64_t MetaClass_get_num_methods_OFFSETS;
static size_t
S_MetaClass_get_num_methods(MetaClass *self);

uint64_t MetaClass_get_obj_alloc_size_OFFSETS;
static size_t
S_MetaClass_get_obj_alloc_size(MetaClass *self);

MetaClass *cMETACLASS = NULL;
MetaClass *cOBJECT    = NULL;

void
Core_bootstrap(void) {
    // Init Object's MetaClass.
    cOBJECT = MetaClass_new(NULL, "Object", sizeof(Object), 1);
    Obj_destroy_OFFSETS
        = S_MetaClass_add_novel_method(cOBJECT, (method_t)S_Obj_destroy);

    // Init MetaClass's MetaClass.
    cMETACLASS = MetaClass_new(cOBJECT, "MetaClass", sizeof(MetaClass), 8);
    MetaClass_destroy_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_destroy);
    MetaClass_make_obj_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_make_obj);
    MetaClass_add_novel_method_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_add_novel_method);
    MetaClass_override_method_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_override_method);
    MetaClass_inherit_method_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_inherit_method);
    MetaClass_get_num_methods_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_get_num_methods);
    MetaClass_get_obj_alloc_size_OFFSETS
        = S_MetaClass_add_novel_method(cMETACLASS, (method_t)S_MetaClass_get_obj_alloc_size);

    // Hack around circular dependency.
    cOBJECT->metaclass = cMETACLASS;
}

void
Core_tear_down(void) {
    S_MetaClass_destroy(cOBJECT);
    S_MetaClass_destroy(cMETACLASS);
}

static void
S_dummy_method(void *self) {
    (void)self;
    abort();
}

MetaClass*
MetaClass_new(MetaClass *parent, const char *name, size_t obj_alloc_size,
              size_t num_methods) {
    size_t size = sizeof(MetaClass) + sizeof(method_t) * num_methods;
    MetaClass *self      = (MetaClass*)calloc(size, 1);
    self->metaclass      = cMETACLASS;
    self->name           = strdup(name);
    self->num_methods    = num_methods;
    self->method_count   = 0;
    self->obj_alloc_size = obj_alloc_size;
    self->parent         = parent;
    self->superself.metaclass = cMETACLASS;

    // Dupe parent's method pointers, use a dummy placeholder for the rest.
    method_t *methods = S_methods(self);
    if (parent) {
        method_t *parent_methods = S_methods(parent);
        while (self->method_count < parent->method_count) {
            methods[self->method_count] = parent_methods[self->method_count];
            self->method_count++;
        }
    }
    for (size_t i = self->method_count; i < self->num_methods; i++) {
        methods[i] = S_dummy_method;
    }

    return self;
}

static void
S_MetaClass_destroy(MetaClass *self) {
    free(self->name);
    free(self);
}

static void*
S_MetaClass_make_obj(MetaClass *self) {
    Object *obj = (Object*)calloc(1, self->obj_alloc_size);
    MetaClass *ancestor = self;
    Object *view = obj;
    while (ancestor) {
        view->metaclass = self;
        if (ancestor->parent) {
            size_t fixup = ancestor->obj_alloc_size
                           - ancestor->parent->obj_alloc_size;
            view = (Object*)((char*)view + fixup);
        }
        ancestor = ancestor->parent;
    }
    return obj;
}

static uint64_t
S_MetaClass_add_novel_method(MetaClass *self, method_t method) {
    if (self->method_count >= self->num_methods) {
        abort();
    }

    // Store the method pointer at the next location.
    size_t offset = sizeof(MetaClass) + self->method_count * sizeof(method_t);
    union { char *char_ptr; method_t *func_ptr; } pointer;
    pointer.char_ptr = ((char*)self) + offset;
    pointer.func_ptr[0] = method;
    self->method_count++;

    // Encode method vtable offset in the lower 32 bits.  The upper 32-bits,
    // used to encode the "self" pointer fixup, are left all zeroes because no
    // fixup is necessary for a "fresh" method implementation.
    if (offset > UINT32_MAX) {
        abort(); // Guard against unlikely integer overflow.
    }
    return offset;
}

static uint64_t 
S_MetaClass_override_method(MetaClass *self, method_t method,
                            uint64_t offsets) {
    // Store the supplied method pointer at the specified location.
    union { char *char_ptr; method_t *func_ptr; } pointer;
    pointer.char_ptr = ((char*)self) + (uint32_t)offsets;
    pointer.func_ptr[0] = method;

    // Return an offsets var with the method offset in the lower 32 bits, and
    // the top 32 bits all zeros to encode a fixup of 0 for this "fresh"
    // method.
    return (offsets & 0xFFFFFFFF);
}

static uint64_t
S_MetaClass_inherit_method(MetaClass *self, uint64_t offsets) {
    // Encode "self" pointer fixup in top 32 bits of OFFSETS var.
    int32_t fixup = (int32_t)(offsets >> 32);
    fixup += (self->obj_alloc_size - self->parent->obj_alloc_size);
    int64_t new_offsets = ((int64_t)fixup) << 32;

    // Encode method vtable offset in lower 32 bits.
    new_offsets |= (offsets & 0xFFFFFFFF);

    return new_offsets;
}

static size_t
S_MetaClass_get_num_methods(MetaClass *self) {
    return self->num_methods;
}

static size_t
S_MetaClass_get_obj_alloc_size(MetaClass *self) {
    return self->obj_alloc_size;
}
