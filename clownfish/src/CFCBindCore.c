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
#include "CFCBindCore.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"

struct CFCBindCore {
    CFCBase base;
    CFCHierarchy *hierarchy;
    char         *dest;
    char         *header;
    char         *footer;
};

CFCBindCore*
CFCBindCore_new(CFCHierarchy *hierarchy, const char *dest, const char *header, 
                const char *footer) {
    CFCBindCore *self
        = (CFCBindCore*)CFCBase_allocate(sizeof(CFCBindCore),
                                         "Clownfish::Binding::Core");
    return CFCBindCore_init(self, hierarchy, dest, header, footer);
}

CFCBindCore*
CFCBindCore_init(CFCBindCore *self, CFCHierarchy *hierarchy, const char *dest,
                 const char *header, const char *footer) {
    CFCUTIL_NULL_CHECK(hierarchy);
    CFCUTIL_NULL_CHECK(dest);
    CFCUTIL_NULL_CHECK(header);
    CFCUTIL_NULL_CHECK(footer);
    self->hierarchy = (CFCHierarchy*)CFCBase_incref((CFCBase*)hierarchy);
    self->dest      = CFCUtil_strdup(dest);
    self->header    = CFCUtil_strdup(header);
    self->footer    = CFCUtil_strdup(footer);
    return self;
}

void
CFCBindCore_destroy(CFCBindCore *self) {
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->dest);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    CFCBase_destroy((CFCBase*)self);
}

CFCHierarchy*
CFCBindCore_get_hierarchy(CFCBindCore *self) {
    return self->hierarchy;
}

const char*
CFCBindCore_get_dest(CFCBindCore *self) {
    return self->dest;
}

const char*
CFCBindCore_get_header(CFCBindCore *self) {
    return self->header;
}

const char*
CFCBindCore_get_footer(CFCBindCore *self) {
    return self->footer;
}

