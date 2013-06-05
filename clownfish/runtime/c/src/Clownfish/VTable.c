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

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define C_CFISH_OBJ
#define C_CFISH_VTABLE

#include "Clownfish/VTable.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/VArray.h"

Obj*
VTable_make_obj(VTable *self) {
    Obj *obj = (Obj*)Memory_wrapped_calloc(self->obj_alloc_size, 1);
    obj->vtable = self;
    obj->refcount = 1;
    return obj;
}

Obj*
VTable_init_obj(VTable *self, void *allocation) {
    Obj *obj = (Obj*)allocation;
    obj->vtable = self;
    obj->refcount = 1;
    return obj;
}

Obj*
VTable_foster_obj(VTable *self, void *host_obj) {
    UNUSED_VAR(self);
    UNUSED_VAR(host_obj);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Obj*);
}

void
VTable_register_with_host(VTable *singleton, VTable *parent) {
    UNUSED_VAR(singleton);
    UNUSED_VAR(parent);
}

VArray*
VTable_fresh_host_methods(const CharBuf *class_name) {
    UNUSED_VAR(class_name);
    return VA_new(0);
}

CharBuf*
VTable_find_parent_class(const CharBuf *class_name) {
    UNUSED_VAR(class_name);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(CharBuf*);
}

void*
VTable_to_host(VTable *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

