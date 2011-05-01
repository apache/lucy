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

#include <stdarg.h>

#define C_LUCY_FILEHANDLE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/FileHandle.h"

int32_t FH_object_count = 0;

FileHandle*
FH_do_open(FileHandle *self, const CharBuf *path, uint32_t flags) {
    self->path    = path ? CB_Clone(path) : CB_new(0);
    self->flags   = flags;

    // Track number of live FileHandles released into the wild.
    FH_object_count++;

    ABSTRACT_CLASS_CHECK(self, FILEHANDLE);
    return self;
}

void
FH_destroy(FileHandle *self) {
    FH_Close(self);
    DECREF(self->path);
    SUPER_DESTROY(self, FILEHANDLE);

    // Decrement count of FileHandle objects in existence.
    FH_object_count--;
}

bool_t
FH_grow(FileHandle *self, int64_t length) {
    UNUSED_VAR(self);
    UNUSED_VAR(length);
    return true;
}

void
FH_set_path(FileHandle *self, const CharBuf *path) {
    CB_Mimic(self->path, (Obj*)path);
}

CharBuf*
FH_get_path(FileHandle *self) {
    return self->path;
}


