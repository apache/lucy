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

#define C_CFISH_OBJ
#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/Obj.h"
#include "Clownfish/Err.h"

uint32_t
Obj_get_refcount(Obj *self) {
    return self->refcount;
}

Obj*
Obj_inc_refcount(Obj *self) {
    self->refcount++;
    return self;
}

uint32_t
Obj_dec_refcount(Obj *self) {
    uint32_t modified_refcount = INT32_MAX;
    switch (self->refcount) {
        case 0:
            THROW(ERR, "Illegal refcount of 0");
            break; // useless
        case 1:
            modified_refcount = 0;
            Obj_Destroy(self);
            break;
        default:
            modified_refcount = --self->refcount;
            break;
    }
    return modified_refcount;
}

void*
Obj_to_host(Obj *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}


