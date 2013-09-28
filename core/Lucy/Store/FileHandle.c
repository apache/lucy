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

#define C_LUCY_FILEHANDLE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/FileHandle.h"

int32_t FH_object_count = 0;

FileHandle*
FH_do_open(FileHandle *self, String *path, uint32_t flags) {
    FileHandleIVARS *const ivars = FH_IVARS(self);
    ivars->path    = path ? Str_Clone(path) : Str_new_from_trusted_utf8("", 0);
    ivars->flags   = flags;

    // Track number of live FileHandles released into the wild.
    FH_object_count++;

    ABSTRACT_CLASS_CHECK(self, FILEHANDLE);
    return self;
}

void
FH_Destroy_IMP(FileHandle *self) {
    FileHandleIVARS *const ivars = FH_IVARS(self);
    FH_Close(self);
    DECREF(ivars->path);
    SUPER_DESTROY(self, FILEHANDLE);

    // Decrement count of FileHandle objects in existence.
    FH_object_count--;
}

bool
FH_Grow_IMP(FileHandle *self, int64_t length) {
    UNUSED_VAR(self);
    UNUSED_VAR(length);
    return true;
}

void
FH_Set_Path_IMP(FileHandle *self, String *path) {
    FileHandleIVARS *const ivars = FH_IVARS(self);
    DECREF(ivars->path);
    ivars->path = Str_Clone(path);
}

String*
FH_Get_Path_IMP(FileHandle *self) {
    return FH_IVARS(self)->path;
}


