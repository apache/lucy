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

#include "dso.h"

static void Obj_hello(obj_t *obj);

void thunk3(obj_t *obj);

class_t *OBJ;
size_t Obj_Hello_OFFSET;
method_t Obj_Hello_THUNK_PTR;

void
bootstrap() {
    size_t method_idx = 3;
    size_t class_size = offsetof(class_t, vtable)
                        + (method_idx + 1) * sizeof(method_t);

    OBJ = (class_t*)calloc(1, class_size);

    OBJ->name       = "Obj";
    OBJ->class_size = class_size;

    Obj_Hello_OFFSET = offsetof(class_t, vtable)
                       + method_idx * sizeof(method_t);
    OBJ->vtable[method_idx] = Obj_hello;
    Obj_Hello_THUNK_PTR = thunk3;
}

obj_t*
Obj_new() {
    obj_t *self = (obj_t *)malloc(sizeof(obj_t));

    self->refcount = 1;
    self->klass    = OBJ;
    self->value    = 0;

    return self;
}

static void
Obj_hello(obj_t *obj) {
    ++obj->value;
}

void
thunk3(obj_t *obj) {
    obj->klass->vtable[3](obj);
}

