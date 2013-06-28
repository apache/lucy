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
    RAMDirHandleIVARS *const ivars = RAMDH_IVARS(self);
    ivars->folder = (RAMFolder*)INCREF(folder);
    ivars->elems  = Hash_Keys(RAMFolder_IVARS(ivars->folder)->entries);
    ivars->tick   = -1;
    return self;
}

bool
RAMDH_close(RAMDirHandle *self) {
    RAMDirHandleIVARS *const ivars = RAMDH_IVARS(self);
    if (ivars->elems) {
        VA_Dec_RefCount(ivars->elems);
        ivars->elems = NULL;
    }
    if (ivars->folder) {
        RAMFolder_Dec_RefCount(ivars->folder);
        ivars->folder = NULL;
    }
    return true;
}

bool
RAMDH_next(RAMDirHandle *self) {
    RAMDirHandleIVARS *const ivars = RAMDH_IVARS(self);
    if (ivars->elems) {
        ivars->tick++;
        if (ivars->tick < (int32_t)VA_Get_Size(ivars->elems)) {
            CharBuf *path = (CharBuf*)CERTIFY(
                                VA_Fetch(ivars->elems, ivars->tick), CHARBUF);
            CB_Mimic(ivars->entry, (Obj*)path);
            return true;
        }
        else {
            ivars->tick--;
            return false;
        }
    }
    return false;
}

bool
RAMDH_entry_is_dir(RAMDirHandle *self) {
    RAMDirHandleIVARS *const ivars = RAMDH_IVARS(self);
    if (ivars->elems) {
        CharBuf *name = (CharBuf*)VA_Fetch(ivars->elems, ivars->tick);
        if (name) {
            return RAMFolder_Local_Is_Directory(ivars->folder, name);
        }
    }
    return false;
}

bool
RAMDH_entry_is_symlink(RAMDirHandle *self) {
    UNUSED_VAR(self);
    return false;
}


