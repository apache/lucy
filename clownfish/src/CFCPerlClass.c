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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerlClass.h"
#include "CFCUtil.h"
#include "CFCClass.h"
#include "CFCParcel.h"
#include "CFCParamList.h"
#include "CFCFunction.h"
#include "CFCDocuComment.h"
#include "CFCSymbol.h"
#include "CFCPerlPod.h"

struct CFCPerlClass {
    CFCBase base;
    CFCParcel *parcel;
    char *class_name;
    CFCClass *client;
    char *xs_code;
    CFCPerlPod *pod_spec;
};

static CFCPerlClass **registry = NULL;
static size_t registry_size = 0;
static size_t registry_cap  = 0;

const static CFCMeta CFCPERLCLASS_META = {
    "Clownfish::CFC::Binding::Perl::Class",
    sizeof(CFCPerlClass),
    (CFCBase_destroy_t)CFCPerlClass_destroy
};

CFCPerlClass*
CFCPerlClass_new(CFCParcel *parcel, const char *class_name, CFCClass *client,
                 const char *xs_code, CFCPerlPod *pod_spec) {
    CFCPerlClass *self = (CFCPerlClass*)CFCBase_allocate(&CFCPERLCLASS_META);
    return CFCPerlClass_init(self, parcel, class_name, client, xs_code,
                             pod_spec);
}

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, CFCParcel *parcel,
                  const char *class_name, CFCClass *client,
                  const char *xs_code, CFCPerlPod *pod_spec) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(class_name);
    self->parcel = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->client = (CFCClass*)CFCBase_incref((CFCBase*)client);
    self->class_name = CFCUtil_strdup(class_name);
    self->pod_spec = (CFCPerlPod*)CFCBase_incref((CFCBase*)pod_spec);
    self->xs_code = xs_code ? CFCUtil_strdup(xs_code) : NULL;
    return self;
}

void
CFCPerlClass_destroy(CFCPerlClass *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->client);
    CFCBase_decref((CFCBase*)self->pod_spec);
    FREEMEM(self->class_name);
    FREEMEM(self->xs_code);
    CFCBase_destroy((CFCBase*)self);
}

static int
S_compare_cfcperlclass(const void *va, const void *vb) {
    CFCPerlClass *a = *(CFCPerlClass**)va;
    CFCPerlClass *b = *(CFCPerlClass**)vb;
    return strcmp(a->class_name, b->class_name);
}

void
CFCPerlClass_add_to_registry(CFCPerlClass *self) {
    if (registry_size == registry_cap) {
        size_t new_cap = registry_cap + 10;
        registry = (CFCPerlClass**)REALLOCATE(registry,
                                              (new_cap + 1) * sizeof(CFCPerlClass*));
        for (size_t i = registry_cap; i <= new_cap; i++) {
            registry[i] = NULL;
        }
        registry_cap = new_cap;
    }
    CFCPerlClass *existing = CFCPerlClass_singleton(self->class_name);
    if (existing) {
        CFCUtil_die("Class '%s' already registered", self->class_name);
    }
    registry[registry_size] = (CFCPerlClass*)CFCBase_incref((CFCBase*)self);
    registry_size++;
    qsort(registry, registry_size, sizeof(CFCPerlClass*),
          S_compare_cfcperlclass);
}

CFCPerlClass*
CFCPerlClass_singleton(const char *class_name) {
    CFCUTIL_NULL_CHECK(class_name);
    for (size_t i = 0; i < registry_size; i++) {
        CFCPerlClass *existing = registry[i];
        if (strcmp(class_name, existing->class_name) == 0) {
            return existing;
        }
    }
    return NULL;
}

CFCPerlClass**
CFCPerlClass_registry() {
    return registry;
}

void
CFCPerlClass_clear_registry(void) {
    for (size_t i = 0; i < registry_size; i++) {
        CFCBase_decref((CFCBase*)registry[i]);
    }
    FREEMEM(registry);
    registry_size = 0;
    registry_cap  = 0;
    registry      = NULL;
}

CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self) {
    return self->client;
}

const char*
CFCPerlClass_get_class_name(CFCPerlClass *self) {
    return self->class_name;
}

const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self) {
    return self->xs_code;
}

CFCPerlPod*
CFCPerlClass_get_pod_spec(CFCPerlClass *self) {
    return self->pod_spec;
}

