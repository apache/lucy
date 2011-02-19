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

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCFile.h"
#include "CFCUtil.h"

struct CFCFile {
    CFCBase base;
    int modified;
    char *source_class;
};

CFCFile*
CFCFile_new(const char *source_class)
{

    CFCFile *self = (CFCFile*)CFCBase_allocate(sizeof(CFCFile),
        "Clownfish::File");
    return CFCFile_init(self, source_class);
}

CFCFile*
CFCFile_init(CFCFile *self, const char *source_class) 
{
    CFCUTIL_NULL_CHECK(source_class);
    self->modified = false;
    self->source_class = CFCUtil_strdup(source_class);
    return self;
}

void
CFCFile_destroy(CFCFile *self)
{
    free(self->source_class);
    CFCBase_destroy((CFCBase*)self);
}

void
CFCFile_set_modified(CFCFile *self, int modified)
{
    self->modified = !!modified;
}

int
CFCFile_get_modified(CFCFile *self)
{
    return self->modified;
}

const char*
CFCFile_get_source_class(CFCFile *self)
{
    return self->source_class;
}

