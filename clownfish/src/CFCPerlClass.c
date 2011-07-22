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
#include "CFCPerlClass.h"
#include "CFCUtil.h"
#include "CFCClass.h"
#include "CFCParcel.h"

struct CFCPerlClass {
    CFCBase base;
    CFCParcel *parcel;
    char *class_name;
    CFCClass *client;
    char *xs_code;
};

CFCPerlClass*
CFCPerlClass_new(CFCParcel *parcel, const char *class_name, CFCClass *client, 
                 const char *xs_code) {
    CFCPerlClass *self
        = (CFCPerlClass*)CFCBase_allocate(sizeof(CFCPerlClass),
                                          "Clownfish::Binding::Perl::Class");
    return CFCPerlClass_init(self, parcel, class_name, client, xs_code);
}

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, CFCParcel *parcel, const char *class_name,
                  CFCClass *client, const char *xs_code) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(class_name);
    self->parcel = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->client = (CFCClass*)CFCBase_incref((CFCBase*)client);
    self->class_name = CFCUtil_strdup(class_name);
    self->xs_code = xs_code ? CFCUtil_strdup(xs_code) : NULL;
    return self;
}

void
CFCPerlClass_destroy(CFCPerlClass *self) {
    CFCBase_destroy((CFCBase*)self);
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->client);
    FREEMEM(self->class_name);
    FREEMEM(self->xs_code);
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

