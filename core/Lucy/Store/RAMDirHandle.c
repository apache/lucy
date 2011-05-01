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

#define C_LUCY_RAMFOLDER
#define C_LUCY_RAMDIRHANDLE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/RAMDirHandle.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Util/IndexFileNames.h"

RAMDirHandle*
RAMDH_new(RAMFolder *folder) {
    RAMDirHandle *self = (RAMDirHandle*)VTable_Make_Obj(RAMDIRHANDLE);
    return RAMDH_init(self, folder);
}

RAMDirHandle*
RAMDH_init(RAMDirHandle *self, RAMFolder *folder) {
    DH_init((DirHandle*)self, RAMFolder_Get_Path(folder));
    self->folder = (RAMFolder*)INCREF(folder);
    self->elems  = Hash_Keys(self->folder->entries);
    self->tick   = -1;
    return self;
}

bool_t
RAMDH_close(RAMDirHandle *self) {
    if (self->elems) {
        VA_Dec_RefCount(self->elems);
        self->elems = NULL;
    }
    if (self->folder) {
        RAMFolder_Dec_RefCount(self->folder);
        self->folder = NULL;
    }
    return true;
}

bool_t
RAMDH_next(RAMDirHandle *self) {
    if (self->elems) {
        self->tick++;
        if (self->tick < (int32_t)VA_Get_Size(self->elems)) {
            CharBuf *path = (CharBuf*)CERTIFY(
                                VA_Fetch(self->elems, self->tick), CHARBUF);
            CB_Mimic(self->entry, (Obj*)path);
            return true;
        }
        else {
            self->tick--;
            return false;
        }
    }
    return false;
}

bool_t
RAMDH_entry_is_dir(RAMDirHandle *self) {
    if (self->elems) {
        CharBuf *name = (CharBuf*)VA_Fetch(self->elems, self->tick);
        if (name) {
            return RAMFolder_Local_Is_Directory(self->folder, name);
        }
    }
    return false;
}

bool_t
RAMDH_entry_is_symlink(RAMDirHandle *self) {
    UNUSED_VAR(self);
    return false;
}


