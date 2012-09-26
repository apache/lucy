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

#define C_LUCY_OBJ
#define C_LUCY_VTABLE
#include "XSBind.h"

#include "Clownfish/VTable.h"
#include "Clownfish/Host.h"
#include "Clownfish/Util/Memory.h"

lucy_Obj*
lucy_VTable_foster_obj(lucy_VTable *self, void *host_obj) {
    lucy_Obj *obj
        = (lucy_Obj*)lucy_Memory_wrapped_calloc(self->obj_alloc_size, 1);
    SV *inner_obj = SvRV((SV*)host_obj);
    obj->vtable = self;
    sv_setiv(inner_obj, PTR2IV(obj));
    obj->ref.host_obj = inner_obj;
    return obj;
}

void
lucy_VTable_register_with_host(lucy_VTable *singleton, lucy_VTable *parent) {
    // Register class with host.
    lucy_Host_callback(LUCY_VTABLE, "_register", 2,
                       CFISH_ARG_OBJ("singleton", singleton),
                       CFISH_ARG_OBJ("parent", parent));
}

lucy_VArray*
lucy_VTable_fresh_host_methods(const lucy_CharBuf *class_name) {
    return (lucy_VArray*)lucy_Host_callback_obj(
               LUCY_VTABLE,
               "fresh_host_methods", 1,
               CFISH_ARG_STR("class_name", class_name));
}

lucy_CharBuf*
lucy_VTable_find_parent_class(const lucy_CharBuf *class_name) {
    return lucy_Host_callback_str(LUCY_VTABLE, "find_parent_class", 1,
                                  CFISH_ARG_STR("class_name", class_name));
}

void*
lucy_VTable_to_host(lucy_VTable *self) {
    chy_bool_t first_time = self->ref.count < 4 ? true : false;
    Lucy_VTable_To_Host_t to_host
        = CFISH_SUPER_METHOD_PTR(LUCY_VTABLE, Lucy_VTable_To_Host);
    SV *host_obj = (SV*)to_host(self);
    if (first_time) {
        SvSHARE((SV*)self->ref.host_obj);
    }
    return host_obj;
}


