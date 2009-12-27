#define C_LUCY_VTABLE
#define C_LUCY_OBJ
#define C_LUCY_ZOMBIECHARBUF
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include <ctype.h>

#include "Lucy/Object/VTable.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/Hash.h"
#include "Lucy/Object/Undefined.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Util/Memory.h"

size_t lucy_VTable_offset_of_parent = offsetof(lucy_VTable, parent);

/* Clean up a VTable when its refcount drops to 0. 
 */
static void 
S_remove_from_registry(const CharBuf *name);

/* Remove spaces and underscores, convert to lower case. */
static void
S_scrunch_charbuf(CharBuf *source, CharBuf *target);

Hash *VTable_registry = NULL;

void
VTable_destroy(VTable *self)
{
    if (self->flags & VTABLE_F_IMMORTAL) {
        THROW(ERR, "Attempt to destroy immortal VTable for class %o", self->name);
    }
    if (self->parent && (VTable_Get_RefCount(self->parent) == 2)) {
        S_remove_from_registry(self->parent->name);
    }
    DECREF(self->name);
    DECREF(self->parent);
    SUPER_DESTROY(self, VTABLE);
}

VTable*
VTable_clone(VTable *self)
{
    VTable *evil_twin 
        = (VTable*)Memory_wrapped_calloc(self->vt_alloc_size, 1);

    memcpy(evil_twin, self, self->vt_alloc_size);
    INCREF(evil_twin->vtable);
    evil_twin->name = CB_Clone(self->name);
    evil_twin->ref.count = 1; 

    /* Mark evil_twin as dynamic. */
    evil_twin->flags = self->flags & ~VTABLE_F_IMMORTAL;

    if (evil_twin->parent != NULL)
        (void)INCREF(evil_twin->parent);

    return evil_twin;
}

u32_t
VTable_dec_refcount(VTable *self)
{
    VTable_dec_refcount_t super_decref 
        = (VTable_dec_refcount_t)SUPER_METHOD(VTABLE, VTable, Dec_RefCount);
    u32_t modified_refcount = super_decref(self);
    if (modified_refcount == 1) {
        S_remove_from_registry(self->name);
        modified_refcount--;
    }
    return modified_refcount;
}

void
VTable_override(VTable *self, lucy_method_t method, size_t offset) 
{
    union { char *char_ptr; lucy_method_t *func_ptr; } pointer;
    pointer.char_ptr = ((char*)self) + offset;
    pointer.func_ptr[0] = method;
}

CharBuf*
VTable_get_name(VTable *self)   { return self->name; }
VTable*
VTable_get_parent(VTable *self) { return self->parent; }

void
VTable_init_registry()
{
    VTable_registry = Hash_new(0);
}

VTable*
VTable_singleton(const CharBuf *subclass_name, VTable *parent)
{
    VTable *singleton;

    if (VTable_registry == NULL)
        VTable_init_registry();

    singleton = (VTable*)Hash_Fetch(VTable_registry, (Obj*)subclass_name);

    if (singleton == NULL) {
        VArray *novel_host_methods;
        u32_t num_novel;

        if (parent == NULL) {
            CharBuf *parent_class = VTable_find_parent_class(subclass_name);
            if (parent_class == NULL) {
                THROW(ERR, "Class '%o' doesn't descend from %o", subclass_name,
                    OBJ->name);
            }
            else {
                parent = VTable_singleton(parent_class, NULL);
                DECREF(parent_class);
            }
        }
        (void)INCREF(parent);

        /* Copy source vtable. */
        singleton = VTable_Clone(parent);
        DECREF(singleton->vtable);
        singleton->vtable = (VTable*)INCREF(VTABLE);

        /* Turn clone into child. */
        DECREF(singleton->parent);
        singleton->parent = parent; 
        DECREF(singleton->name);
        singleton->name = CB_Clone(subclass_name);
        
        /* Allow host methods to override. */
        novel_host_methods = VTable_novel_host_methods(subclass_name);
        num_novel = VA_Get_Size(novel_host_methods);
        if (num_novel) {
            Hash *meths = Hash_new(num_novel);
            u32_t i;
            CharBuf *scrunched = CB_new(0);
            ZombieCharBuf callback_name = ZCB_BLANK;
            for (i = 0; i < num_novel; i++) {
                CharBuf *meth = (CharBuf*)VA_fetch(novel_host_methods, i);
                S_scrunch_charbuf(meth, scrunched);
                Hash_Store(meths, (Obj*)scrunched, INCREF(UNDEF));
            }
            for (i = 0; singleton->callbacks[i] != NULL; i++) {
                lucy_Callback *const callback = singleton->callbacks[i];
                ZCB_Assign_Str(&callback_name, callback->name,
                    callback->name_len);
                S_scrunch_charbuf((CharBuf*)&callback_name, scrunched);
                if (Hash_Fetch(meths, (Obj*)scrunched)) {
                    VTable_Override(singleton, callback->func, 
                        callback->offset);
                }
            }
            DECREF(scrunched);
            DECREF(meths);
        }
        DECREF(novel_host_methods);

        /* Register the new class, both locally and with host. */
        Hash_Store(VTable_registry, (Obj*)subclass_name, (Obj*)singleton);
        VTable_register_with_host(singleton, parent);
    }
    
    return singleton;
}

Obj*
VTable_make_obj(VTable *self)
{
    Obj *obj = (Obj*)Memory_wrapped_calloc(self->obj_alloc_size, 1);
    obj->vtable = (VTable*)INCREF(self);
    obj->ref.count = 1;
    return obj;
}

Obj*
VTable_load_obj(VTable *self, Obj *dump)
{
    Obj_load_t load = (Obj_load_t)METHOD(self, Obj, Load);
    if (load == Obj_load) {
        THROW(ERR, "Abstract method Load() not defined for %o", self->name);
    }
    return load(NULL, dump);
}

static void
S_scrunch_charbuf(CharBuf *source, CharBuf *target)
{
    ZombieCharBuf iterator = ZCB_make(source);
    CB_Set_Size(target, 0);
    while (ZCB_Get_Size(&iterator)) {
        u32_t code_point = ZCB_Nip_One(&iterator);
        if (code_point > 127) {
            THROW(ERR, "Can't fold case for %o", source);
        }
        else if (code_point != '_') {
            CB_Cat_Char(target, tolower(code_point));
        }
    }
}

void
VTable_add_to_registry(VTable *vtable)
{
    VTable *fetched;

    if (VTable_registry == NULL)
        VTable_init_registry();
    fetched = (VTable*)Hash_Fetch(VTable_registry, (Obj*)vtable->name);
    if (fetched) {
        if (fetched != vtable) {
            THROW(ERR, "Attempt to redefine a vtable for '%o'", vtable->name);
        }
    }
    else {
        Hash_Store(VTable_registry, (Obj*)vtable->name, INCREF(vtable));
    }
}

VTable*
VTable_fetch_vtable(const CharBuf *class_name)
{
    VTable *vtable = NULL;
    if (VTable_registry != NULL) {
        vtable = (VTable*)Hash_Fetch(VTable_registry, (Obj*)class_name);
    }
    return vtable;
}

static void 
S_remove_from_registry(const CharBuf *name)
{
    if (VTable_registry == NULL) {
        THROW(ERR, "Attempt to remove '%o', but registry is NULL", name);
    }
    else {
        VTable *vtable = (VTable*)Hash_Delete(VTable_registry, (Obj*)name);
        if (vtable) {
            VTable_Dec_RefCount(vtable);
        }
    }
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

