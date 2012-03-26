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
Lock_init(Lock *self, Folder *folder, const CharBuf *name,
          const CharBuf *host, int32_t timeout, int32_t interval) {
    // Validate.
    if (interval <= 0) {
        DECREF(self);
        THROW(ERR, "Invalid value for 'interval': %i32", interval);
    }
    {
        ZombieCharBuf *scratch = ZCB_WRAP(name);
        uint32_t code_point;
        while (0 != (code_point = ZCB_Nip_One(scratch))) {
            if (isalnum(code_point)
                || code_point == '.'
                || code_point == '-'
                || code_point == '_'
               ) {
                continue;
            }
            DECREF(self);
            THROW(ERR, "Lock name contains disallowed characters: '%o'",
                  name);
        }
    }

    // Assign.
    self->folder       = (Folder*)INCREF(folder);
    self->timeout      = timeout;
    self->name         = CB_Clone(name);
    self->host         = CB_Clone(host);
    self->interval     = interval;

    // Derive.
    self->lock_path = CB_newf("locks/%o.lock", name);

    return self;
}

void
Lock_destroy(Lock *self) {
    DECREF(self->folder);
    DECREF(self->host);
    DECREF(self->name);
    DECREF(self->lock_path);
    SUPER_DESTROY(self, LOCK);
}

CharBuf*
Lock_get_name(Lock *self) {
    return self->name;
}

CharBuf*
Lock_get_lock_path(Lock *self) {
    return self->lock_path;
}

CharBuf*
Lock_get_host(Lock *self) {
    return self->host;
}

bool_t
Lock_obtain(Lock *self) {
    int32_t time_left = self->interval == 0 ? 0 : self->timeout;
    bool_t locked = Lock_Request(self);

    while (!locked) {
        time_left -= self->interval;
        if (time_left <= 0) { break; }
        Sleep_millisleep(self->interval);
        locked = Lock_Request(self);
    }

    if (!locked) { ERR_ADD_FRAME(Err_get_error()); }
    return locked;
}

/***************************************************************************/

LockFileLock*
LFLock_new(Folder *folder, const CharBuf *name, const CharBuf *host,
           int32_t timeout, int32_t interval) {
    LockFileLock *self = (LockFileLock*)VTable_Make_Obj(LOCKFILELOCK);
    return LFLock_init(self, folder, name, host, timeout, interval);
}

LockFileLock*
LFLock_init(LockFileLock *self, Folder *folder, const CharBuf *name,
            const CharBuf *host, int32_t timeout, int32_t interval) {
    int pid = PID_getpid();
    Lock_init((Lock*)self, folder, name, host, timeout, interval);
    self->link_path = CB_newf("%o.%o.%i64", self->lock_path, host, pid);
    return self;
}

bool_t
LFLock_shared(LockFileLock *self) {
    UNUSED_VAR(self); return false;
}

bool_t
LFLock_request(LockFileLock *self) {
    Hash   *file_data;
    bool_t wrote_json;
    bool_t success = false;
    bool_t deletion_failed = false;

    if (Folder_Exists(self->folder, self->lock_path)) {
        Err_set_error((Err*)LockErr_new(CB_newf("Can't obtain lock: '%o' exists",
                                                self->lock_path)));
        return false;
    }

    // Create the "locks" subdirectory if necessary.
    CharBuf *lock_dir_name = (CharBuf*)ZCB_WRAP_STR("locks", 5);
    if (!Folder_Exists(self->folder, lock_dir_name)) {
        if (!Folder_MkDir(self->folder, lock_dir_name)) {
            Err *mkdir_err = (Err*)CERTIFY(Err_get_error(), ERR);
            LockErr *err = LockErr_new(CB_newf("Can't create 'locks' directory: %o",
                                               Err_Get_Mess(mkdir_err)));
            // Maybe our attempt failed because another process succeeded.
            if (Folder_Find_Folder(self->folder, lock_dir_name)) {
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
                   (Obj*)CB_newf("%i32", (int32_t)PID_getpid()));
    Hash_Store_Str(file_data, "host", 4, INCREF(self->host));
    Hash_Store_Str(file_data, "name", 4, INCREF(self->name));

    // Write to a temporary file, then use the creation of a hard link to
    // ensure atomic but non-destructive creation of the lockfile with its
    // complete contents.
    wrote_json = Json_spew_json((Obj*)file_data, self->folder, self->link_path);
    if (wrote_json) {
        success = Folder_Hard_Link(self->folder, self->link_path,
                                   self->lock_path);
        if (!success) {
            Err *hard_link_err = (Err*)CERTIFY(Err_get_error(), ERR);
            Err_set_error((Err*)LockErr_new(CB_newf("Failed to obtain lock at '%o': %o",
                                                    self->lock_path,
                                                    Err_Get_Mess(hard_link_err))));
        }
        deletion_failed = !Folder_Delete(self->folder, self->link_path);
    }
    else {
        Err *spew_json_err = (Err*)CERTIFY(Err_get_error(), ERR);
        Err_set_error((Err*)LockErr_new(CB_newf("Failed to obtain lock at '%o': %o",
                                                self->lock_path,
                                                Err_Get_Mess(spew_json_err))));
    }
    DECREF(file_data);

    // Verify that our temporary file got zapped.
    if (wrote_json && deletion_failed) {
        CharBuf *mess = MAKE_MESS("Failed to delete '%o'", self->link_path);
        Err_throw_mess(ERR, mess);
    }

    return success;
}

void
LFLock_release(LockFileLock *self) {
    if (Folder_Exists(self->folder, self->lock_path)) {
        LFLock_Maybe_Delete_File(self, self->lock_path, true, false);
    }
}

bool_t
LFLock_is_locked(LockFileLock *self) {
    return Folder_Exists(self->folder, self->lock_path);
}

void
LFLock_clear_stale(LockFileLock *self) {
    LFLock_Maybe_Delete_File(self, self->lock_path, false, true);
}

bool_t
LFLock_maybe_delete_file(LockFileLock *self, const CharBuf *path,
                         bool_t delete_mine, bool_t delete_other) {
    Folder *folder  = self->folder;
    bool_t  success = false;
    ZombieCharBuf *scratch = ZCB_WRAP(path);

    // Only delete locks that start with our lock name.
    CharBuf *lock_dir_name = (CharBuf*)ZCB_WRAP_STR("locks", 5);
    if (!ZCB_Starts_With(scratch, lock_dir_name)) {
        return false;
    }
    ZCB_Nip(scratch, CB_Get_Size(lock_dir_name) + 1);
    if (!ZCB_Starts_With(scratch, self->name)) {
        return false;
    }

    // Attempt to delete dead lock file.
    if (Folder_Exists(folder, path)) {
        Hash *hash = (Hash*)Json_slurp_json(folder, path);
        if (hash != NULL && Obj_Is_A((Obj*)hash, HASH)) {
            CharBuf *pid_buf = (CharBuf*)Hash_Fetch_Str(hash, "pid", 3);
            CharBuf *host    = (CharBuf*)Hash_Fetch_Str(hash, "host", 4);
            CharBuf *name
                = (CharBuf*)Hash_Fetch_Str(hash, "name", 4);

            // Match hostname and lock name.
            if (host != NULL
                && CB_Equals(host, (Obj*)self->host)
                && name != NULL
                && CB_Equals(name, (Obj*)self->name)
                && pid_buf != NULL
               ) {
                // Verify that pid is either mine or dead.
                int pid = (int)CB_To_I64(pid_buf);
                if ((delete_mine && pid == PID_getpid())  // This process.
                    || (delete_other && !PID_active(pid)) // Dead pid.
                   ) {
                    if (Folder_Delete(folder, path)) {
                        success = true;
                    }
                    else {
                        CharBuf *mess
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
LFLock_destroy(LockFileLock *self) {
    DECREF(self->link_path);
    SUPER_DESTROY(self, LOCKFILELOCK);
}

/***************************************************************************/

LockErr*
LockErr_new(CharBuf *message) {
    LockErr *self = (LockErr*)VTable_Make_Obj(LOCKERR);
    return LockErr_init(self, message);
}

LockErr*
LockErr_init(LockErr *self, CharBuf *message) {
    Err_init((Err*)self, message);
    return self;
}

LockErr*
LockErr_make(LockErr *self) {
    UNUSED_VAR(self);
    return LockErr_new(CB_new(0));
}


