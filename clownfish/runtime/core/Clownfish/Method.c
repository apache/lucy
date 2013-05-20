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

#define C_CFISH_METHOD
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Method.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/VTable.h"

Method*
Method_new(const CharBuf *name, lucy_method_t callback_func, size_t offset) {
    Method *self = (Method*)VTable_Make_Obj(METHOD);
    return Method_init(self, name, callback_func, offset);
}

Method*
Method_init(Method *self, const CharBuf *name, lucy_method_t callback_func,
            size_t offset) {
    self->name          = CB_Clone(name);
    self->callback_func = callback_func;
    self->offset        = offset;
    return self;
}

void
Method_destroy(Method *self) {
    THROW(ERR, "Insane attempt to destroy Method '%o'", self->name);
}

Obj*
Method_inc_refcount(Method *self) {
    return (Obj*)self;
}

uint32_t
Method_dec_refcount(Method *self) {
    UNUSED_VAR(self);
    return 1;
}

uint32_t
Method_get_refcount(Method *self) {
    UNUSED_VAR(self);
    // See comments in VTable.c
    return 1;
}


