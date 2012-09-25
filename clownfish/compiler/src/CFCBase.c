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

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCUtil.h"

CFCBase*
CFCBase_allocate(const CFCMeta *meta) {
    CFCBase *self = (CFCBase*)CALLOCATE(meta->obj_alloc_size, 1);
    self->refcount = 1;
    self->meta = meta;
    return self;
}

void
CFCBase_destroy(CFCBase *self) {
    FREEMEM(self);
}

CFCBase*
CFCBase_incref(CFCBase *self) {
    if (self) {
        self->refcount++;
    }
    return self;
}

unsigned
CFCBase_decref(CFCBase *self) {
    if (!self) { return 0; }
    unsigned modified_refcount = --self->refcount;
    if (modified_refcount == 0) {
        self->meta->destroy(self);
    }
    return modified_refcount;
}

unsigned
CFCBase_get_refcount(CFCBase *self) {
    return self->refcount;
}

const char*
CFCBase_get_cfc_class(CFCBase *self) {
    return self->meta->cfc_class;
}


