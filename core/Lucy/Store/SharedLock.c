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
ShLock_new(Folder *folder, String *name, String *host,
           int32_t timeout, int32_t interval) {
    SharedLock *self = (SharedLock*)Class_Make_Obj(SHAREDLOCK);
    return ShLock_init(self, folder, name, host, timeout, interval);
}

SharedLock*
ShLock_init(SharedLock *self, Folder *folder, String *name,
            String *host, int32_t timeout, int32_t interval) {
    LFLock_init((LockFileLock*)self, folder, name, host, timeout, interval);
    SharedLockIVARS *const ivars = ShLock_IVARS(self);

    // Override.
    DECREF(ivars->lock_path);
    ivars->lock_path = Str_newf("");

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
        && !Str_Equals_Utf8(ivars->lock_path, "", 0)
        && Folder_Exists(ivars->folder, ivars->lock_path)
       ) {
        // Don't allow double obtain.
        Err_set_error((Err*)LockErr_new(Str_newf("Lock already obtained via '%o'",
                                                 ivars->lock_path)));
        return false;
    }

    do {
        DECREF(ivars->lock_path);
        ivars->lock_path = Str_newf("locks/%o-%u32.lock", ivars->name, ++i);
    } while (Folder_Exists(ivars->folder, ivars->lock_path));

    bool success = super_request(self);
    if (!success) { ERR_ADD_FRAME(Err_get_error()); }
    return success;
}

void
ShLock_Release_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);
    if (ivars->lock_path && !Str_Equals_Utf8(ivars->lock_path, "", 0)) {
        ShLock_Release_t super_release
            = SUPER_METHOD_PTR(SHAREDLOCK, LUCY_ShLock_Release);
        super_release(self);

        // Empty out lock_path.
        DECREF(ivars->lock_path);
        ivars->lock_path = Str_newf("");
    }
}


void
ShLock_Clear_Stale_IMP(SharedLock *self) {
    SharedLockIVARS *const ivars = ShLock_IVARS(self);

    String *lock_dir_name = (String*)SSTR_WRAP_UTF8("locks", 5);
    if (!Folder_Find_Folder(ivars->folder, lock_dir_name)) {
        return;
    }

    DirHandle *dh = Folder_Open_Dir(ivars->folder, lock_dir_name);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    // Take a stab at any file that begins with our lock name.
    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (Str_Starts_With(entry, ivars->name)
            && Str_Ends_With_Utf8(entry, ".lock", 5)
           ) {
            String *candidate = Str_newf("%o/%o", lock_dir_name, entry);
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

    String *lock_dir_name = (String*)SSTR_WRAP_UTF8("locks", 5);
    if (!Folder_Find_Folder(ivars->folder, lock_dir_name)) {
        return false;
    }

    DirHandle *dh = Folder_Open_Dir(ivars->folder, lock_dir_name);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        // Translation:  $locked = 1 if $entry =~ /^\Q$name-\d+\.lock$/
        if (Str_Starts_With(entry, ivars->name)) {
            StringIterator *iter = Str_Top(entry);
            StrIter_Advance(iter, Str_Length(ivars->name));
            int32_t code_point = StrIter_Next(iter);
            if (code_point == '-') {
                code_point = StrIter_Next(iter);
                if (code_point != STRITER_DONE && isdigit(code_point)) {
                    while (STRITER_DONE != (code_point = StrIter_Next(iter))) {
                        if (!isdigit(code_point)) { break; }
                    }
                    if (code_point == '.'
                        && StrIter_Starts_With_Utf8(iter, "lock", 4)
                    ) {
                        StrIter_Advance(iter, 4);
                        if (!StrIter_Has_Next(iter)) {
                            DECREF(iter);
                            DECREF(entry);
                            DECREF(dh);
                            return true;
                        }
                    }
                }
            }
            DECREF(iter);
        }
        DECREF(entry);
    }

    DECREF(dh);
    return false;
}


