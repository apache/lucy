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

#define C_LUCY_LOCK
#define C_LUCY_LOCKFILELOCK
#include "Lucy/Util/ToolSet.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "Lucy/Store/Lock.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Util/ProcessID.h"
#include "Lucy/Util/Sleep.h"

Lock*
Lock_init(Lock *self, Folder *folder, const String *name,
          const String *host, int32_t timeout, int32_t interval) {
    LockIVARS *const ivars = Lock_IVARS(self);

    // Validate.
    if (interval <= 0) {
        DECREF(self);
        THROW(ERR, "Invalid value for 'interval': %i32", interval);
    }
    StackString *scratch = SSTR_WRAP(name);
    uint32_t code_point;
    while (0 != (code_point = SStr_Nibble(scratch))) {
        if (isalnum(code_point)
            || code_point == '.'
            || code_point == '-'
            || code_point == '_'
           ) {
            continue;
        }
        DECREF(self);
        THROW(ERR, "Lock name contains disallowed characters: '%o'", name);
    }

    // Assign.
    ivars->folder       = (Folder*)INCREF(folder);
    ivars->timeout      = timeout;
    ivars->name         = Str_Clone(name);
    ivars->host         = Str_Clone(host);
    ivars->interval     = interval;

    // Derive.
    ivars->lock_path = Str_newf("locks/%o.lock", name);

    return self;
}

void
Lock_Destroy_IMP(Lock *self) {
    LockIVARS *const ivars = Lock_IVARS(self);
    DECREF(ivars->folder);
    DECREF(ivars->host);
    DECREF(ivars->name);
    DECREF(ivars->lock_path);
    SUPER_DESTROY(self, LOCK);
}

String*
Lock_Get_Name_IMP(Lock *self) {
    return Lock_IVARS(self)->name;
}

String*
Lock_Get_Lock_Path_IMP(Lock *self) {
    return Lock_IVARS(self)->lock_path;
}

String*
Lock_Get_Host_IMP(Lock *self) {
    return Lock_IVARS(self)->host;
}

bool
Lock_Obtain_IMP(Lock *self) {
    LockIVARS *const ivars = Lock_IVARS(self);
    int32_t time_left = ivars->interval == 0 ? 0 : ivars->timeout;
    bool locked = Lock_Request(self);

    while (!locked) {
        time_left -= ivars->interval;
        if (time_left <= 0) { break; }
        Sleep_millisleep(ivars->interval);
        locked = Lock_Request(self);
    }

    if (!locked) { ERR_ADD_FRAME(Err_get_error()); }
    return locked;
}

/***************************************************************************/

LockFileLock*
LFLock_new(Folder *folder, const String *name, const String *host,
           int32_t timeout, int32_t interval) {
    LockFileLock *self = (LockFileLock*)VTable_Make_Obj(LOCKFILELOCK);
    return LFLock_init(self, folder, name, host, timeout, interval);
}

LockFileLock*
LFLock_init(LockFileLock *self, Folder *folder, const String *name,
            const String *host, int32_t timeout, int32_t interval) {
    int pid = PID_getpid();
    Lock_init((Lock*)self, folder, name, host, timeout, interval);
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    ivars->link_path = Str_newf("%o.%o.%i64", ivars->lock_path, host, pid);
    return self;
}

bool
LFLock_Shared_IMP(LockFileLock *self) {
    UNUSED_VAR(self); return false;
}

bool
LFLock_Request_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    Hash   *file_data;
    bool wrote_json;
    bool success = false;
    bool deletion_failed = false;

    if (Folder_Exists(ivars->folder, ivars->lock_path)) {
        Err_set_error((Err*)LockErr_new(Str_newf("Can't obtain lock: '%o' exists",
                                                ivars->lock_path)));
        return false;
    }

    // Create the "locks" subdirectory if necessary.
    String *lock_dir_name = (String*)SSTR_WRAP_STR("locks", 5);
    if (!Folder_Exists(ivars->folder, lock_dir_name)) {
        if (!Folder_MkDir(ivars->folder, lock_dir_name)) {
            Err *mkdir_err = (Err*)CERTIFY(Err_get_error(), ERR);
            LockErr *err = LockErr_new(Str_newf("Can't create 'locks' directory: %o",
                                               Err_Get_Mess(mkdir_err)));
            // Maybe our attempt failed because another process succeeded.
            if (Folder_Find_Folder(ivars->folder, lock_dir_name)) {
                DECREF(err);
            }
            else {
                // Nope, everything failed, so bail out.
                Err_set_error((Err*)err);
                return false;
            }
        }
    }

    // Prepare to write pid, lock name, and host to the lock file as JSON.
    file_data = Hash_new(3);
    Hash_Store_Str(file_data, "pid", 3,
                   (Obj*)Str_newf("%i32", (int32_t)PID_getpid()));
    Hash_Store_Str(file_data, "host", 4, INCREF(ivars->host));
    Hash_Store_Str(file_data, "name", 4, INCREF(ivars->name));

    // Write to a temporary file, then use the creation of a hard link to
    // ensure atomic but non-destructive creation of the lockfile with its
    // complete contents.
    wrote_json = Json_spew_json((Obj*)file_data, ivars->folder, ivars->link_path);
    if (wrote_json) {
        success = Folder_Hard_Link(ivars->folder, ivars->link_path,
                                   ivars->lock_path);
        if (!success) {
            Err *hard_link_err = (Err*)CERTIFY(Err_get_error(), ERR);
            Err_set_error((Err*)LockErr_new(Str_newf("Failed to obtain lock at '%o': %o",
                                                    ivars->lock_path,
                                                    Err_Get_Mess(hard_link_err))));
        }
        deletion_failed = !Folder_Delete(ivars->folder, ivars->link_path);
    }
    else {
        Err *spew_json_err = (Err*)CERTIFY(Err_get_error(), ERR);
        Err_set_error((Err*)LockErr_new(Str_newf("Failed to obtain lock at '%o': %o",
                                                ivars->lock_path,
                                                Err_Get_Mess(spew_json_err))));
    }
    DECREF(file_data);

    // Verify that our temporary file got zapped.
    if (wrote_json && deletion_failed) {
        String *mess = MAKE_MESS("Failed to delete '%o'", ivars->link_path);
        Err_throw_mess(ERR, mess);
    }

    return success;
}

void
LFLock_Release_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    if (Folder_Exists(ivars->folder, ivars->lock_path)) {
        LFLock_Maybe_Delete_File(self, ivars->lock_path, true, false);
    }
}

bool
LFLock_Is_Locked_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    return Folder_Exists(ivars->folder, ivars->lock_path);
}

void
LFLock_Clear_Stale_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    LFLock_Maybe_Delete_File(self, ivars->lock_path, false, true);
}

bool
LFLock_Maybe_Delete_File_IMP(LockFileLock *self, const String *path,
                             bool delete_mine, bool delete_other) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    Folder *folder  = ivars->folder;
    bool    success = false;
    StackString *scratch = SSTR_WRAP(path);

    // Only delete locks that start with our lock name.
    String *lock_dir_name = (String*)SSTR_WRAP_STR("locks", 5);
    if (!SStr_Starts_With(scratch, lock_dir_name)) {
        return false;
    }
    SStr_Nip(scratch, Str_Get_Size(lock_dir_name) + 1);
    if (!SStr_Starts_With(scratch, ivars->name)) {
        return false;
    }

    // Attempt to delete dead lock file.
    if (Folder_Exists(folder, path)) {
        Hash *hash = (Hash*)Json_slurp_json(folder, path);
        if (hash != NULL && Obj_Is_A((Obj*)hash, HASH)) {
            String *pid_buf = (String*)Hash_Fetch_Str(hash, "pid", 3);
            String *host    = (String*)Hash_Fetch_Str(hash, "host", 4);
            String *name
                = (String*)Hash_Fetch_Str(hash, "name", 4);

            // Match hostname and lock name.
            if (host != NULL
                && Str_Equals(host, (Obj*)ivars->host)
                && name != NULL
                && Str_Equals(name, (Obj*)ivars->name)
                && pid_buf != NULL
               ) {
                // Verify that pid is either mine or dead.
                int pid = (int)Str_To_I64(pid_buf);
                if ((delete_mine && pid == PID_getpid())  // This process.
                    || (delete_other && !PID_active(pid)) // Dead pid.
                   ) {
                    if (Folder_Delete(folder, path)) {
                        success = true;
                    }
                    else {
                        String *mess
                            = MAKE_MESS("Can't delete '%o'", path);
                        DECREF(hash);
                        Err_throw_mess(ERR, mess);
                    }
                }
            }
        }
        DECREF(hash);
    }

    return success;
}

void
LFLock_Destroy_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    DECREF(ivars->link_path);
    SUPER_DESTROY(self, LOCKFILELOCK);
}

/***************************************************************************/

LockErr*
LockErr_new(String *message) {
    LockErr *self = (LockErr*)VTable_Make_Obj(LOCKERR);
    return LockErr_init(self, message);
}

LockErr*
LockErr_init(LockErr *self, String *message) {
    Err_init((Err*)self, message);
    return self;
}

