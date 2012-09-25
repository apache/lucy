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
#include <stdio.h>
#include <ctype.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCFileSpec.h"
#include "CFCUtil.h"
#include "CFCClass.h"

struct CFCFileSpec {
    CFCBase base;
    char *source_dir;
    char *path_part;
    int is_included;
};

const static CFCMeta CFCFILESPEC_META = {
    "Clownfish::CFC::Model::FileSpec",
    sizeof(CFCFileSpec),
    (CFCBase_destroy_t)CFCFileSpec_destroy
};

CFCFileSpec*
CFCFileSpec_new(const char *source_dir, const char *path_part,
                int is_included) {
    CFCFileSpec *self = (CFCFileSpec*)CFCBase_allocate(&CFCFILESPEC_META);
    return CFCFileSpec_init(self, source_dir, path_part, is_included);
}

CFCFileSpec*
CFCFileSpec_init(CFCFileSpec *self, const char *source_dir,
                 const char *path_part, int is_included) {
    CFCUTIL_NULL_CHECK(source_dir);
    CFCUTIL_NULL_CHECK(path_part);

    self->source_dir  = CFCUtil_strdup(source_dir);
    self->path_part   = CFCUtil_strdup(path_part);
    self->is_included = !!is_included;

    return self;
}

void
CFCFileSpec_destroy(CFCFileSpec *self) {
    FREEMEM(self->source_dir);
    FREEMEM(self->path_part);
    CFCBase_destroy((CFCBase*)self);
}

const char*
CFCFileSpec_get_source_dir(CFCFileSpec *self) {
    return self->source_dir;
}

const char*
CFCFileSpec_get_path_part(CFCFileSpec *self) {
    return self->path_part;
}

int
CFCFileSpec_included(CFCFileSpec *self) {
    return self->is_included;
}

