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
#include "CFCClass.h"
#include "CFCUtil.h"

struct CFCHierarchy {
    CFCBase base;
    char *source;
    char *dest;
    CFCClass **trees;
    size_t num_trees;
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
    self->source    = CFCUtil_strdup(source);
    self->dest      = CFCUtil_strdup(dest);
    self->trees     = (CFCClass**)CALLOCATE(1, sizeof(CFCClass*));
    self->num_trees = 0;
    return self;
}

void
CFCHierarchy_destroy(CFCHierarchy *self)
{
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->trees[i]);
    }
    FREEMEM(self->trees);
    FREEMEM(self->source);
    FREEMEM(self->dest);
    CFCBase_destroy((CFCBase*)self);
}

void
CFCHierarchy_add_tree(CFCHierarchy *self, CFCClass *klass)
{
    CFCUTIL_NULL_CHECK(klass);
    const char *full_struct_sym = CFCClass_full_struct_sym(klass);
    size_t i;
    for (i = 0; self->trees[i] != NULL; i++) {
        const char *existing = CFCClass_full_struct_sym(self->trees[i]);
        if (strcmp(full_struct_sym, existing) == 0) {
            Util_die("Tree '%s' alread added", full_struct_sym);
        }
    }
    self->num_trees++;
    size_t size = (self->num_trees + 1) * sizeof(CFCClass*);
    self->trees = (CFCClass**)REALLOCATE(self->trees, size);
    self->trees[self->num_trees - 1] 
        = (CFCClass*)CFCBase_incref((CFCBase*)klass);
    self->trees[self->num_trees] = NULL;
}

CFCClass**
CFCHierarchy_trees(CFCHierarchy *self)
{
    return self->trees;
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

