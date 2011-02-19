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
    char *guard_name;
    char *guard_start;
    char *guard_close;
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

    // Derive include guard name, plus C code for opening and closing the
    // guard.
    size_t len = strlen(source_class);
    self->guard_name = (char*)malloc(len + sizeof("H_") + 1);
    self->guard_start = (char*)malloc(len * 2 + 40);
    self->guard_close = (char*)malloc(len + 20);
    if (!self->guard_name || !self->guard_start || !self->guard_close) {
        croak("malloc failed");
    }
    memcpy(self->guard_name, "H_", 2);
    size_t i, j;
    for (i = 0, j = 2; i < len; i++, j++) {
        char c = source_class[i];
        if (c == ':') {
            self->guard_name[j] = '_';
            i++;
        }
        else {
            self->guard_name[j] = toupper(source_class[i]);
        }
    }
    self->guard_name[j] = '\0';
    int check = sprintf(self->guard_start, "#ifndef %s\n#define %s 1\n", 
        self->guard_name, self->guard_name);
    if (check < 0) { croak("sprintf failed"); }
    check = sprintf(self->guard_close, "#endif /* %s */\n", 
        self->guard_name);
    if (check < 0) { croak("sprintf failed"); }

    return self;
}

void
CFCFile_destroy(CFCFile *self)
{
    free(self->guard_name);
    free(self->guard_start);
    free(self->guard_close);
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

const char*
CFCFile_guard_name(CFCFile *self)
{
    return self->guard_name;
}

const char*
CFCFile_guard_start(CFCFile *self)
{
    return self->guard_start;
}

const char*
CFCFile_guard_close(CFCFile *self)
{
    return self->guard_close;
}

