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

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"

struct CFCHierarchy {
    CFCBase base;
    char *source;
    char *dest;
};

CFCHierarchy*
CFCHierarchy_new(const char *source, const char *dest)
{
    CFCHierarchy *self = (CFCHierarchy*)CFCBase_allocate(sizeof(CFCHierarchy),
        "Clownfish::Hierarchy");
    return CFCHierarchy_init(self, source, dest);
}

CFCHierarchy*
CFCHierarchy_init(CFCHierarchy *self, const char *source, const char *dest) 
{
    if (!source || !strlen(source) || !dest || !strlen(dest)) {
        croak("Both 'source' and 'dest' are required");
    }
    self->source   = CFCUtil_strdup(source);
    self->dest     = CFCUtil_strdup(dest);
    return self;
}

void
CFCHierarchy_destroy(CFCHierarchy *self)
{
    FREEMEM(self->source);
    FREEMEM(self->dest);
    CFCBase_destroy((CFCBase*)self);
}

const char*
CFCHierarchy_get_source(CFCHierarchy *self)
{
    return self->source;
}

const char*
CFCHierarchy_get_dest(CFCHierarchy *self)
{
    return self->dest;
}

