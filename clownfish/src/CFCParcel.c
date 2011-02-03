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

#include <stdlib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
  #define true 1
  #define false 0
#endif

#include "CFCParcel.h"

struct CFCParcel {
    const char *name;
    const char *cnick;
    const char *prefix;
    const char *Prefix;
    const char *PREFIX;
};

static int
S_validate_name_or_cnick(const char *orig)
{
    const char *ptr = orig;
    for ( ; *ptr != 0; ptr++) {
        if (!isalpha(*ptr)) { return false; }
    }
    return true;
}

CFCParcel*
CFCParcel_new(const char *name, const char *cnick)
{
    CFCParcel *self = (CFCParcel*)calloc(sizeof(CFCParcel), 1);
    if (!self) { croak("malloc failed"); }
    return CFCParcel_init(self, name, cnick);
}

CFCParcel*
CFCParcel_init(CFCParcel *self, const char *name, const char *cnick)
{
    // Validate name.
    if (!name || !S_validate_name_or_cnick(name)) {
        croak("Invalid name: '%s'", name ? name : "[NULL]");
    }
    self->name = savepv(name);

    // Validate or derive cnick.
    if (cnick) {
        if (!S_validate_name_or_cnick(cnick)) {
            croak("Invalid cnick: '%s'", cnick);
        }
        self->cnick = savepv(cnick);
    }
    else {
        self->cnick = savepv(name);
    }
    
    // Derive prefix, Prefix, PREFIX.
    size_t cnick_len  = strlen(self->cnick);
    size_t prefix_len = cnick_len ? cnick_len + 1 : 0;
    size_t amount     = prefix_len + 1;
    self->prefix = (const char*)malloc(amount);
    self->Prefix = (const char*)malloc(amount);
    self->PREFIX = (const char*)malloc(amount);
    if (!self->prefix || !self->Prefix || !self->PREFIX) {
        croak("malloc failed");
    }
    memcpy((char*)self->Prefix, self->cnick, cnick_len);
    if (cnick_len) {
        *((char*)&self->Prefix[cnick_len])  = '_';
    }
    size_t i;
    for (i = 0; i < amount; i++) {
        *((char*)&self->prefix[i]) = tolower(self->Prefix[i]);
        *((char*)&self->PREFIX[i]) = toupper(self->Prefix[i]);
    }
    *((char*)&self->prefix[prefix_len]) = '\0';
    *((char*)&self->Prefix[prefix_len]) = '\0';
    *((char*)&self->PREFIX[prefix_len]) = '\0';

    return self;
}

void
CFCParcel_destroy(CFCParcel *self)
{
    Safefree(self->name);
    Safefree(self->cnick);
    free((void*)self->prefix);
    free((void*)self->Prefix);
    free((void*)self->PREFIX);
    free(self);
}


const char*
CFCParcel_get_name(CFCParcel *self)
{
    return self->name;
}

const char*
CFCParcel_get_cnick(CFCParcel *self)
{
    return self->cnick;
}

const char*
CFCParcel_get_prefix(CFCParcel *self)
{
    return self->prefix;
}

const char*
CFCParcel_get_Prefix(CFCParcel *self)
{
    return self->Prefix;
}

const char*
CFCParcel_get_PREFIX(CFCParcel *self)
{
    return self->PREFIX;
}


