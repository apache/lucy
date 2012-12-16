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

#define C_LUCY_VTABLE
#define C_LUCY_OBJ
#define C_LUCY_CHARBUF
#define C_LUCY_METHOD
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

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

LockFreeRegistry *VTable_registry = NULL;

void
VTable_bootstrap(VTableSpec *specs, size_t num_specs)
{
    /* Pass 1:
     * - Allocate memory.
     * - Initialize refcount, parent, flags, obj_alloc_size, vt_alloc_size.
     * - Initialize method pointers.
     */
    for (int i = 0; i < num_specs; ++i) {
        VTableSpec *spec   = &specs[i];
        VTable     *parent = spec->parent ? *spec->parent : NULL;

        size_t vt_alloc_size = parent
                               ? parent->vt_alloc_size
                               : offsetof(VTable, method_ptrs);
        vt_alloc_size += spec->num_novel * sizeof(cfish_method_t);
        VTable *vtable = (VTable*)Memory_wrapped_calloc(vt_alloc_size, 1);

        vtable->ref.count      = 1;
        vtable->parent         = parent;
        vtable->flags          = 0;
        vtable->obj_alloc_size = spec->obj_alloc_size;
        vtable->vt_alloc_size  = vt_alloc_size;

        if (parent) {
            size_t parent_ptrs_size = parent->vt_alloc_size
                                      - offsetof(VTable, method_ptrs);
            memcpy(vtable->method_ptrs, parent->method_ptrs, parent_ptrs_size);
        }

        for (int i = 0; i < spec->num_fresh; ++i) {
            MethodSpec *mspec = &spec->method_specs[i];
            VTable_override(vtable, mspec->func, *mspec->offset);
        }

        *spec->vtable = vtable;
    }

    /* Pass 2:
     * - Initialize 'vtable' instance variable.
     */
    for (int i = 0; i < num_specs; ++i) {
        VTableSpec *spec   = &specs[i];
        VTable     *vtable = *spec->vtable;

        vtable->vtable = VTABLE;
    }

    /* Now it's safe to call methods.
     *
     * Pass 3:
     * - Inititalize name and method array.
     * - Register vtable.
     */
    for (int i = 0; i < num_specs; ++i) {
        VTableSpec *spec   = &specs[i];
        VTable     *vtable = *spec->vtable;

        vtable->name    = CB_newf("%s", spec->name);
        vtable->methods = VA_new(0);

        for (int i = 0; i < spec->num_fresh; ++i) {
            MethodSpec *mspec = &spec->method_specs[i];
            if (mspec->is_novel) {
                CharBuf *name = CB_newf("%s", mspec->name);
                Method *method = Method_new(name, mspec->callback_func,
                                            *mspec->offset);
                VA_Push(vtable->methods, (Obj*)method);
                DECREF(name);
            }
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
    twin->name = CB_Clone(self->name);
    twin->ref.count = 1;

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
    /* VTable_Get_RefCount() lies to other Lucy code about the refcount
     * because we don't want to have to synchronize access to the cached host
     * object to which we have delegated responsibility for keeping refcounts.
     * It always returns 1 because 1 is a positive number, and thus other Lucy
     * code will be fooled into believing it never needs to take action such
     * as initiating a destructor.
     *
     * It's possible that the host has in fact increased the refcount of the
     * cached host object if there are multiple refs to it on the other side
     * of the Lucy/host border, but returning 1 is good enough to fool Lucy
     * code.
     */
    return 1;
}

void
VTable_override(VTable *self, lucy_method_t method, size_t offset) {
    union { char *char_ptr; lucy_method_t *func_ptr; } pointer;
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
            ZombieCharBuf *callback_name = ZCB_BLANK();
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

Obj*
VTable_make_obj(VTable *self) {
    Obj *obj = (Obj*)Memory_wrapped_calloc(self->obj_alloc_size, 1);
    obj->vtable = self;
    obj->ref.count = 1;
    return obj;
}

Obj*
VTable_init_obj(VTable *self, void *allocation) {
    Obj *obj = (Obj*)allocation;
    obj->vtable = self;
    obj->ref.count = 1;
    return obj;
}

Obj*
VTable_load_obj(VTable *self, Obj *dump) {
    Obj_Load_t load = METHOD_PTR(self, Lucy_Obj_Load);
    if (load == Obj_load) {
        THROW(ERR, "Abstract method Load() not defined for %o", self->name);
    }
    Obj *invoker = VTable_Make_Obj(self);
    Obj *loaded = load(invoker, dump);
    DECREF(invoker);
    return loaded;
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


