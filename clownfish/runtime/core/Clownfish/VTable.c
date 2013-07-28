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

#define C_CFISH_VTABLE
#define C_CFISH_OBJ
#define C_CFISH_CHARBUF
#define C_CFISH_METHOD
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Clownfish/VTable.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/LockFreeRegistry.h"
#include "Clownfish/Method.h"
#include "Clownfish/Num.h"
#include "Clownfish/VArray.h"
#include "Clownfish/Util/Atomic.h"
#include "Clownfish/Util/Memory.h"

size_t VTable_offset_of_parent = offsetof(VTable, parent);

// Remove spaces and underscores, convert to lower case.
static void
S_scrunch_charbuf(CharBuf *source, CharBuf *target);

static Method*
S_find_method(VTable *self, const char *meth_name);

static int32_t
S_claim_parcel_id(void);

LockFreeRegistry *VTable_registry = NULL;

void
VTable_bootstrap(const VTableSpec *specs, size_t num_specs)
{
    int32_t parcel_id = S_claim_parcel_id();

    /* Pass 1:
     * - Initialize IVARS_OFFSET.
     * - Allocate memory.
     * - Initialize parent, flags, obj_alloc_size, vt_alloc_size.
     * - Assign parcel_id.
     * - Initialize method pointers.
     */
    for (size_t i = 0; i < num_specs; ++i) {
        const VTableSpec *spec = &specs[i];
        VTable *parent = spec->parent ? *spec->parent : NULL;

        size_t ivars_offset = 0;
        if (spec->ivars_offset_ptr != NULL) {
            if (parent) {
                VTable *ancestor = parent;
                while (ancestor && ancestor->parcel_id == parcel_id) {
                    ancestor = ancestor->parent;
                }
                ivars_offset = ancestor ? ancestor->obj_alloc_size : 0;
                *spec->ivars_offset_ptr = ivars_offset;
            }
            else {
                *spec->ivars_offset_ptr = 0;
            }
        }

        size_t novel_offset = parent
                              ? parent->vt_alloc_size
                              : offsetof(VTable, method_ptrs);
        size_t vt_alloc_size = novel_offset
                               + spec->num_novel_meths
                                 * sizeof(cfish_method_t);
        VTable *vtable = (VTable*)Memory_wrapped_calloc(vt_alloc_size, 1);

        vtable->parent         = parent;
        vtable->parcel_id      = parcel_id;
        vtable->flags          = 0;
        vtable->obj_alloc_size = ivars_offset + spec->ivars_size;
        vtable->vt_alloc_size  = vt_alloc_size;

        if (parent) {
            size_t parent_ptrs_size = parent->vt_alloc_size
                                      - offsetof(VTable, method_ptrs);
            memcpy(vtable->method_ptrs, parent->method_ptrs, parent_ptrs_size);
        }

        for (size_t i = 0; i < spec->num_inherited_meths; ++i) {
            const InheritedMethSpec *mspec = &spec->inherited_meth_specs[i];
            *mspec->offset = *mspec->parent_offset;
        }

        for (size_t i = 0; i < spec->num_overridden_meths; ++i) {
            const OverriddenMethSpec *mspec = &spec->overridden_meth_specs[i];
            *mspec->offset = *mspec->parent_offset;
            VTable_override(vtable, mspec->func, *mspec->offset);
        }

        for (size_t i = 0; i < spec->num_novel_meths; ++i) {
            const NovelMethSpec *mspec = &spec->novel_meth_specs[i];
            *mspec->offset = novel_offset;
            novel_offset += sizeof(cfish_method_t);
            VTable_override(vtable, mspec->func, *mspec->offset);
        }

        *spec->vtable = vtable;
    }

    /* Pass 2:
     * - Initialize 'vtable' instance variable.
     * - Initialize refcount.
     */
    for (size_t i = 0; i < num_specs; ++i) {
        const VTableSpec *spec = &specs[i];
        VTable *vtable = *spec->vtable;

        VTable_init_obj(VTABLE, vtable);
    }

    /* Now it's safe to call methods.
     *
     * Pass 3:
     * - Inititalize name and method array.
     * - Register vtable.
     */
    for (size_t i = 0; i < num_specs; ++i) {
        const VTableSpec *spec = &specs[i];
        VTable *vtable = *spec->vtable;

        vtable->name    = CB_newf("%s", spec->name);
        vtable->methods = VA_new(0);

        for (size_t i = 0; i < spec->num_novel_meths; ++i) {
            const NovelMethSpec *mspec = &spec->novel_meth_specs[i];
            CharBuf *name = CB_newf("%s", mspec->name);
            Method *method = Method_new(name, mspec->callback_func,
                                        *mspec->offset);
            VA_Push(vtable->methods, (Obj*)method);
            DECREF(name);
        }

        VTable_add_to_registry(vtable);
    }
}

void
VTable_destroy(VTable *self) {
    THROW(ERR, "Insane attempt to destroy VTable for class '%o'", self->name);
}

VTable*
VTable_clone(VTable *self) {
    VTable *twin
        = (VTable*)Memory_wrapped_calloc(self->vt_alloc_size, 1);

    memcpy(twin, self, self->vt_alloc_size);
    VTable_Init_Obj(self->vtable, twin); // Set refcount.
    twin->name = CB_Clone(self->name);

    return twin;
}

Obj*
VTable_inc_refcount(VTable *self) {
    return (Obj*)self;
}

uint32_t
VTable_dec_refcount(VTable *self) {
    UNUSED_VAR(self);
    return 1;
}

uint32_t
VTable_get_refcount(VTable *self) {
    UNUSED_VAR(self);
    /* VTable_Get_RefCount() lies to other Clownfish code about the refcount
     * because we don't want to have to synchronize access to the cached host
     * object to which we have delegated responsibility for keeping refcounts.
     * It always returns 1 because 1 is a positive number, and thus other
     * Clownfish code will be fooled into believing it never needs to take
     * action such as initiating a destructor.
     *
     * It's possible that the host has in fact increased the refcount of the
     * cached host object if there are multiple refs to it on the other side
     * of the Clownfish/host border, but returning 1 is good enough to fool
     * Clownfish code.
     */
    return 1;
}

void
VTable_override(VTable *self, cfish_method_t method, size_t offset) {
    union { char *char_ptr; cfish_method_t *func_ptr; } pointer;
    pointer.char_ptr = ((char*)self) + offset;
    pointer.func_ptr[0] = method;
}

CharBuf*
VTable_get_name(VTable *self) {
    return self->name;
}

VTable*
VTable_get_parent(VTable *self) {
    return self->parent;
}

size_t
VTable_get_obj_alloc_size(VTable *self) {
    return self->obj_alloc_size;
}

VArray*
VTable_get_methods(VTable *self) {
    return self->methods;
}

void
VTable_init_registry() {
    LockFreeRegistry *reg = LFReg_new(256);
    if (Atomic_cas_ptr((void*volatile*)&VTable_registry, NULL, reg)) {
        return;
    }
    else {
        DECREF(reg);
    }
}

VTable*
VTable_singleton(const CharBuf *class_name, VTable *parent) {
    if (VTable_registry == NULL) {
        VTable_init_registry();
    }

    VTable *singleton = (VTable*)LFReg_Fetch(VTable_registry, (Obj*)class_name);
    if (singleton == NULL) {
        VArray *fresh_host_methods;
        uint32_t num_fresh;

        if (parent == NULL) {
            CharBuf *parent_class = VTable_find_parent_class(class_name);
            if (parent_class == NULL) {
                THROW(ERR, "Class '%o' doesn't descend from %o", class_name,
                      OBJ->name);
            }
            else {
                parent = VTable_singleton(parent_class, NULL);
                DECREF(parent_class);
            }
        }

        // Copy source vtable.
        singleton = VTable_Clone(parent);

        // Turn clone into child.
        singleton->parent = parent;
        DECREF(singleton->name);
        singleton->name = CB_Clone(class_name);

        // Allow host methods to override.
        fresh_host_methods = VTable_fresh_host_methods(class_name);
        num_fresh = VA_Get_Size(fresh_host_methods);
        if (num_fresh) {
            Hash *meths = Hash_new(num_fresh);
            CharBuf *scrunched = CB_new(0);
            for (uint32_t i = 0; i < num_fresh; i++) {
                CharBuf *meth = (CharBuf*)VA_fetch(fresh_host_methods, i);
                S_scrunch_charbuf(meth, scrunched);
                Hash_Store(meths, (Obj*)scrunched, (Obj*)CFISH_TRUE);
            }
            for (VTable *vtable = parent; vtable; vtable = vtable->parent) {
                uint32_t max = VA_Get_Size(vtable->methods);
                for (uint32_t i = 0; i < max; i++) {
                    Method *method = (Method*)VA_Fetch(vtable->methods, i);
                    if (method->callback_func) {
                        S_scrunch_charbuf(method->name, scrunched);
                        if (Hash_Fetch(meths, (Obj*)scrunched)) {
                            VTable_Override(singleton, method->callback_func,
                                            method->offset);
                        }
                    }
                }
            }
            DECREF(scrunched);
            DECREF(meths);
        }
        DECREF(fresh_host_methods);

        // Register the new class, both locally and with host.
        if (VTable_add_to_registry(singleton)) {
            // Doing this after registering is racy, but hard to fix. :(
            VTable_register_with_host(singleton, parent);
        }
        else {
            DECREF(singleton);
            singleton = (VTable*)LFReg_Fetch(VTable_registry, (Obj*)class_name);
            if (!singleton) {
                THROW(ERR, "Failed to either insert or fetch VTable for '%o'",
                      class_name);
            }
        }
    }

    return singleton;
}

static void
S_scrunch_charbuf(CharBuf *source, CharBuf *target) {
    ZombieCharBuf *iterator = ZCB_WRAP(source);
    CB_Set_Size(target, 0);
    while (ZCB_Get_Size(iterator)) {
        uint32_t code_point = ZCB_Nip_One(iterator);
        if (code_point > 127) {
            THROW(ERR, "Can't fold case for %o", source);
        }
        else if (code_point != '_') {
            CB_Cat_Char(target, tolower(code_point));
        }
    }
}

bool
VTable_add_to_registry(VTable *vtable) {
    if (VTable_registry == NULL) {
        VTable_init_registry();
    }
    if (LFReg_Fetch(VTable_registry, (Obj*)vtable->name)) {
        return false;
    }
    else {
        CharBuf *klass = CB_Clone(vtable->name);
        bool retval
            = LFReg_Register(VTable_registry, (Obj*)klass, (Obj*)vtable);
        DECREF(klass);
        return retval;
    }
}

bool
VTable_add_alias_to_registry(VTable *vtable, CharBuf *alias) {
    if (VTable_registry == NULL) {
        VTable_init_registry();
    }
    if (LFReg_Fetch(VTable_registry, (Obj*)alias)) {
        return false;
    }
    else {
        CharBuf *klass = CB_Clone(alias);
        bool retval
            = LFReg_Register(VTable_registry, (Obj*)klass, (Obj*)vtable);
        DECREF(klass);
        return retval;
    }
}

VTable*
VTable_fetch_vtable(const CharBuf *class_name) {
    VTable *vtable = NULL;
    if (VTable_registry != NULL) {
        vtable = (VTable*)LFReg_Fetch(VTable_registry, (Obj*)class_name);
    }
    return vtable;
}

void
VTable_add_host_method_alias(VTable *self, const char *alias,
                             const char *meth_name) {
    Method *method = S_find_method(self, meth_name);
    if (!method) {
        fprintf(stderr, "Method %s not found\n", meth_name);
        abort();
    }
    method->host_alias = CB_newf("%s", alias);
}

void
VTable_exclude_host_method(VTable *self, const char *meth_name) {
    Method *method = S_find_method(self, meth_name);
    if (!method) {
        fprintf(stderr, "Method %s not found\n", meth_name);
        abort();
    }
    method->is_excluded = true;
}

static Method*
S_find_method(VTable *self, const char *name) {
    size_t   name_len = strlen(name);
    uint32_t size     = VA_Get_Size(self->methods);

    for (uint32_t i = 0; i < size; i++) {
        Method *method = (Method*)VA_Fetch(self->methods, i);
        if (CB_Equals_Str(method->name, name, name_len)) {
            return method;
        }
    }

    return NULL;
}

static size_t parcel_count;

static int32_t
S_claim_parcel_id(void) {
    // TODO: use ordinary cas rather than cas_ptr.
    union { size_t num; void *ptr; } old_value;
    union { size_t num; void *ptr; } new_value;

    bool succeeded = false;
    do {
        old_value.num = parcel_count;
        new_value.num = old_value.num + 1;
        succeeded = Atomic_cas_ptr((void*volatile*)&parcel_count,
                                   old_value.ptr, new_value.ptr);
    } while (!succeeded);

    return new_value.num;
}

