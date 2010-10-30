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

#define C_KINO_OBJ
#define C_KINO_VTABLE
#include "xs/XSBind.h"

#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Util/Memory.h"

kino_Obj*
kino_VTable_foster_obj(kino_VTable *self, void *host_obj)
{
    kino_Obj *obj 
        = (kino_Obj*)kino_Memory_wrapped_calloc(self->obj_alloc_size, 1);
    SV *inner_obj = SvRV((SV*)host_obj);
    obj->vtable = self;
    sv_setiv(inner_obj, PTR2IV(obj));
    obj->ref.host_obj = inner_obj;
    return obj;
}

void
kino_VTable_register_with_host(kino_VTable *singleton, kino_VTable *parent)
{
    // Register class with host. 
    kino_Host_callback(KINO_VTABLE, "_register", 2, 
        KINO_ARG_OBJ("singleton", singleton), KINO_ARG_OBJ("parent", parent));
}

kino_VArray*
kino_VTable_novel_host_methods(const kino_CharBuf *class_name)
{
    return (kino_VArray*)kino_Host_callback_obj(KINO_VTABLE, 
        "novel_host_methods", 1, KINO_ARG_STR("class_name", class_name));
}

kino_CharBuf*
kino_VTable_find_parent_class(const kino_CharBuf *class_name)
{
    return kino_Host_callback_str(KINO_VTABLE, "find_parent_class", 1, 
        KINO_ARG_STR("class_name", class_name));
}

void*
kino_VTable_to_host(kino_VTable *self)
{
    chy_bool_t first_time = self->ref.count < 4 ? true : false;
    kino_VTable_to_host_t to_host = (kino_VTable_to_host_t)
        LUCY_SUPER_METHOD(KINO_VTABLE, VTable, To_Host);
    SV *host_obj = (SV*)to_host(self);
    if (first_time) {
        SvSHARE((SV*)self->ref.host_obj);
    }
    return host_obj;
}


