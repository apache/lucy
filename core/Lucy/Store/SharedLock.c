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

#define C_LUCY_SHAREDLOCK
#include "Lucy/Util/ToolSet.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "Lucy/Store/SharedLock.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

SharedLock*
ShLock_new(Folder *folder, const CharBuf *name, const CharBuf *host,
           int32_t timeout, int32_t interval) {
    SharedLock *self = (SharedLock*)VTable_Make_Obj(SHAREDLOCK);
    return ShLock_init(self, folder, name, host, timeout, interval);
}

SharedLock*
ShLock_init(SharedLock *self, Folder *folder, const CharBuf *name,
            const CharBuf *host, int32_t timeout, int32_t interval) {
    LFLock_init((LockFileLock*)self, folder, name, host, timeout, interval);
    SharedLockIVARS *const ivars = ShLock_IVARS(self);

    // Override.
    DECREF(ivars->lock_path);
    ivars->lock_path = CB_newf("");

    return self;
}

bool
ShLock_Shared_IMP(SharedLock *self) {
    UNUSED_VAR(self);
    return true;
}

bool
ShLock_Request_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);
    uint32_t i = 0;
    ShLock_Request_t super_request
        = SUPER_METHOD_PTR(SHAREDLOCK, LUCY_ShLock_Request);

    // Empty lock_path indicates whether this particular instance is locked.
    if (ivars->lock_path
        && !CB_Equals_Str(ivars->lock_path, "", 0)
        && Folder_Exists(ivars->folder, ivars->lock_path)
       ) {
        // Don't allow double obtain.
        Err_set_error((Err*)LockErr_new(CB_newf("Lock already obtained via '%o'",
                                                ivars->lock_path)));
        return false;
    }

    do {
        DECREF(ivars->lock_path);
        ivars->lock_path = CB_newf("locks/%o-%u32.lock", ivars->name, ++i);
    } while (Folder_Exists(ivars->folder, ivars->lock_path));

    bool success = super_request(self);
    if (!success) { ERR_ADD_FRAME(Err_get_error()); }
    return success;
}

void
ShLock_Release_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);
    if (ivars->lock_path && !CB_Equals_Str(ivars->lock_path, "", 0)) {
        ShLock_Release_t super_release
            = SUPER_METHOD_PTR(SHAREDLOCK, LUCY_ShLock_Release);
        super_release(self);

        // Empty out lock_path.
        DECREF(ivars->lock_path);
        ivars->lock_path = CB_newf("");
    }
}


void
ShLock_Clear_Stale_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);

    CharBuf *lock_dir_name = (CharBuf*)SSTR_WRAP_STR("locks", 5);
    if (!Folder_Find_Folder(ivars->folder, lock_dir_name)) {
        return;
    }

    DirHandle *dh = Folder_Open_Dir(ivars->folder, lock_dir_name);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    // Take a stab at any file that begins with our lock name.
    while (DH_Next(dh)) {
        CharBuf *entry = DH_Get_Entry(dh);
        if (CB_Starts_With(entry, ivars->name)
            && CB_Ends_With_Str(entry, ".lock", 5)
           ) {
            CharBuf *candidate = CB_newf("%o/%o", lock_dir_name, entry);
            ShLock_Maybe_Delete_File(self, candidate, false, true);
            DECREF(candidate);
        }
        DECREF(entry);
    }

    DECREF(dh);
}

bool
ShLock_Is_Locked_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);

    CharBuf *lock_dir_name = (CharBuf*)SSTR_WRAP_STR("locks", 5);
    if (!Folder_Find_Folder(ivars->folder, lock_dir_name)) {
        return false;
    }

    DirHandle *dh = Folder_Open_Dir(ivars->folder, lock_dir_name);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    while (DH_Next(dh)) {
        CharBuf *entry = DH_Get_Entry(dh);
        // Translation:  $locked = 1 if $entry =~ /^\Q$name-\d+\.lock$/
        if (CB_Starts_With(entry, ivars->name)
            && CB_Ends_With_Str(entry, ".lock", 5)
           ) {
            StackString *scratch = SSTR_WRAP(entry);
            SStr_Chop(scratch, sizeof(".lock") - 1);
            while (isdigit(SStr_Code_Point_From(scratch, 1))) {
                SStr_Chop(scratch, 1);
            }
            if (SStr_Code_Point_From(scratch, 1) == '-') {
                SStr_Chop(scratch, 1);
                if (SStr_Equals(scratch, (Obj*)ivars->name)) {
                    DECREF(entry);
                    DECREF(dh);
                    return true;
                }
            }
        }
        DECREF(entry);
    }

    DECREF(dh);
    return false;
}


