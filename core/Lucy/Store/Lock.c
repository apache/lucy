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

#include <stdio.h>
#include <ctype.h>

#include "Lucy/Store/Lock.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Util/ProcessID.h"
#include "Lucy/Util/Sleep.h"

Lock*
Lock_init(Lock *self, Folder *folder, String *name,
          String *host, int32_t timeout, int32_t interval) {
    LockIVARS *const ivars = Lock_IVARS(self);

    // Validate.
    if (interval <= 0) {
        DECREF(self);
        THROW(ERR, "Invalid value for 'interval': %i32", interval);
    }
    StringIterator *iter = Str_Top(name);
    int32_t code_point;
    while (STR_OOB != (code_point = StrIter_Next(iter))) {
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
    DECREF(iter);

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

bool
Lock_Obtain_Shared_IMP(Lock *self) {
    LockIVARS *const ivars = Lock_IVARS(self);
    int32_t time_left = ivars->interval == 0 ? 0 : ivars->timeout;
    bool locked = Lock_Request_Shared(self);

    while (!locked) {
        time_left -= ivars->interval;
        if (time_left <= 0) { break; }
        Sleep_millisleep((uint32_t)ivars->interval);
        locked = Lock_Request_Shared(self);
    }

    if (!locked) { ERR_ADD_FRAME(Err_get_error()); }
    return locked;
}

bool
Lock_Obtain_Exclusive_IMP(Lock *self) {
    LockIVARS *const ivars = Lock_IVARS(self);
    int32_t time_left = ivars->interval == 0 ? 0 : ivars->timeout;
    bool locked = Lock_Request_Exclusive(self);

    while (!locked) {
        time_left -= ivars->interval;
        if (time_left <= 0) { break; }
        Sleep_millisleep((uint32_t)ivars->interval);
        locked = Lock_Request_Exclusive(self);
    }

    if (!locked) { ERR_ADD_FRAME(Err_get_error()); }
    return locked;
}

/***************************************************************************/

#define LFLOCK_STATE_UNLOCKED          0
#define LFLOCK_STATE_LOCKED_SHARED     1
#define LFLOCK_STATE_LOCKED_EXCLUSIVE  2

static bool
S_request(LockFileLockIVARS *ivars, String *lock_path);

static bool
S_is_locked_exclusive(LockFileLockIVARS *ivars);

static bool
S_is_locked(LockFileLockIVARS *ivars);

static bool
S_is_shared_lock_file(LockFileLockIVARS *ivars, String *entry);

static bool
S_maybe_delete_file(LockFileLockIVARS *ivars, String *path,
                    bool delete_mine, bool delete_other);

LockFileLock*
LFLock_new(Folder *folder, String *name, String *host, int32_t timeout,
           int32_t interval, bool exclusive_only) {
    LockFileLock *self = (LockFileLock*)Class_Make_Obj(LOCKFILELOCK);
    return LFLock_init(self, folder, name, host, timeout, interval,
                       exclusive_only);
}

LockFileLock*
LFLock_init(LockFileLock *self, Folder *folder, String *name, String *host,
            int32_t timeout, int32_t interval, bool exclusive_only) {
    int pid = PID_getpid();
    Lock_init((Lock*)self, folder, name, host, timeout, interval);
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    ivars->link_path = Str_newf("%o.%o.%i64", ivars->lock_path, host,
                                (int64_t)pid);
    ivars->exclusive_only = exclusive_only;
    return self;
}

struct lockfile_context {
    OutStream *outstream;
    String *json;
};

static void
S_write_lockfile_json(void *context) {
    struct lockfile_context *stuff = (struct lockfile_context*)context;
    size_t size = Str_Get_Size(stuff->json);
    OutStream_Write_Bytes(stuff->outstream, Str_Get_Ptr8(stuff->json), size);
    OutStream_Close(stuff->outstream);
}

bool
LFLock_Request_Shared_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);

    if (ivars->exclusive_only) {
        THROW(ERR, "Can't request shared lock if exclusive_only is set");
    }

    if (ivars->state != LFLOCK_STATE_UNLOCKED) {
        THROW(ERR, "Lock already acquired");
    }

    // TODO: The is_locked test and subsequent file creation is prone to a
    // race condition. We could protect the whole process with an internal
    // exclusive lock.

    if (S_is_locked_exclusive(ivars)) {
        String *msg = Str_newf("'%o.lock' is locked", ivars->name);
        Err_set_error((Err*)LockErr_new(msg));
        return false;
    }

    String *path = NULL;

    uint32_t i = 0;
    do {
        DECREF(path);
        path = Str_newf("locks/%o-%u32.lock", ivars->name, ++i);
    } while (Folder_Exists(ivars->folder, path));

    if (S_request(ivars, path)) {
        ivars->shared_lock_path = path;
        ivars->state = LFLOCK_STATE_LOCKED_SHARED;
        return true;
    }
    else {
        DECREF(path);
        return false;
    }
}

bool
LFLock_Request_Exclusive_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);

    if (ivars->state != LFLOCK_STATE_UNLOCKED) {
        THROW(ERR, "Lock already acquired");
    }

    // TODO: The is_locked test and subsequent file creation is prone to a
    // race condition. We could protect the whole process with an internal
    // exclusive lock.

    if (ivars->exclusive_only
        ? S_is_locked_exclusive(ivars)
        : S_is_locked(ivars)
       ) {
        String *msg = Str_newf("'%o.lock' is locked", ivars->name);
        Err_set_error((Err*)LockErr_new(msg));
        return false;
    }

    if (S_request(ivars, ivars->lock_path)) {
        ivars->state = LFLOCK_STATE_LOCKED_EXCLUSIVE;
        return true;
    }
    else {
        return false;
    }
}

static bool
S_request(LockFileLockIVARS *ivars, String *lock_path) {
    bool success = false;

    // Create the "locks" subdirectory if necessary.
    String *lock_dir_name = SSTR_WRAP_C("locks");
    if (!Folder_Exists(ivars->folder, lock_dir_name)) {
        if (!Folder_MkDir(ivars->folder, lock_dir_name)) {
            Err *err = (Err*)INCREF(Err_get_error());
            // Maybe our attempt failed because another process succeeded.
            if (Folder_Find_Folder(ivars->folder, lock_dir_name)) {
                DECREF(err);
            }
            else {
                // Nope, everything failed, so bail out.
                Err_set_error(err);
                return false;
            }
        }
    }

    // Prepare to write pid, lock name, and host to the lock file as JSON.
    Hash *file_data = Hash_new(3);
    Hash_Store_Utf8(file_data, "pid", 3,
                    (Obj*)Str_newf("%i32", (int32_t)PID_getpid()));
    Hash_Store_Utf8(file_data, "host", 4, INCREF(ivars->host));
    Hash_Store_Utf8(file_data, "name", 4, INCREF(ivars->name));
    String *json = Json_to_json((Obj*)file_data);
    DECREF(file_data);

    // Write to a temporary file, then use the creation of a hard link to
    // ensure atomic but non-destructive creation of the lockfile with its
    // complete contents.

    OutStream *outstream = Folder_Open_Out(ivars->folder, ivars->link_path);
    if (!outstream) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(json);
        return false;
    }

    struct lockfile_context context;
    context.outstream = outstream;
    context.json = json;
    Err *json_error = Err_trap(S_write_lockfile_json, &context);
    DECREF(outstream);
    DECREF(json);
    if (json_error) {
        Err_set_error(json_error);
    }
    else {
        success = Folder_Hard_Link(ivars->folder, ivars->link_path,
                                   lock_path);
        if (!success) {
            // TODO: Only return a LockErr if errno == EEXIST, otherwise
            // return a normal Err.
            Err *hard_link_err = (Err*)CERTIFY(Err_get_error(), ERR);
            String *msg = Str_newf("Failed to obtain lock at '%o': %o",
                                   lock_path, Err_Get_Mess(hard_link_err));
            Err_set_error((Err*)LockErr_new(msg));
        }
    }

    // Verify that our temporary file got zapped.
    bool deletion_failed = !Folder_Delete(ivars->folder, ivars->link_path);
    if (deletion_failed) {
        String *mess = MAKE_MESS("Failed to delete '%o'", ivars->link_path);
        Err_throw_mess(ERR, mess);
    }

    return success;
}

void
LFLock_Release_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);

    if (ivars->state == LFLOCK_STATE_UNLOCKED) {
        THROW(ERR, "Lock not acquired");
    }

    if (ivars->state == LFLOCK_STATE_LOCKED_EXCLUSIVE) {
        if (Folder_Exists(ivars->folder, ivars->lock_path)) {
            S_maybe_delete_file(ivars, ivars->lock_path, true, false);
        }
    }
    else { // Shared lock.
        if (Folder_Exists(ivars->folder, ivars->shared_lock_path)) {
            S_maybe_delete_file(ivars, ivars->shared_lock_path, true, false);
        }

        // Empty out lock_path.
        DECREF(ivars->shared_lock_path);
        ivars->shared_lock_path = NULL;
    }

    ivars->state = LFLOCK_STATE_UNLOCKED;
}

static bool
S_is_locked_exclusive(LockFileLockIVARS *ivars) {
    return Folder_Exists(ivars->folder, ivars->lock_path)
           && !S_maybe_delete_file(ivars, ivars->lock_path, false, true);
}

static bool
S_is_locked(LockFileLockIVARS *ivars) {
    if (S_is_locked_exclusive(ivars)) { return true; }

    // Check for shared lock.

    String *lock_dir_name = SSTR_WRAP_C("locks");
    if (!Folder_Find_Folder(ivars->folder, lock_dir_name)) {
        return false;
    }

    bool locked = false;
    DirHandle *dh = Folder_Open_Dir(ivars->folder, lock_dir_name);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }

    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (S_is_shared_lock_file(ivars, entry)) {
            String *candidate = Str_newf("%o/%o", lock_dir_name, entry);
            if (!S_maybe_delete_file(ivars, candidate, false, true)) {
                locked = true;
            }
            DECREF(candidate);
        }
        DECREF(entry);
    }

    DECREF(dh);
    return locked;
}

static bool
S_is_shared_lock_file(LockFileLockIVARS *ivars, String *entry) {
    // Translation:  $match = $entry =~ /^\Q$name-\d+\.lock\z/
    bool match = false;

    // $name
    if (Str_Starts_With(entry, ivars->name)) {
        StringIterator *iter = Str_Top(entry);
        StrIter_Advance(iter, Str_Length(ivars->name));

        // Hyphen-minus
        if (StrIter_Next(iter) == '-') {
            int32_t code_point = StrIter_Next(iter);

            // Digit
            if (code_point >= '0' && code_point <= '9') {
                // Optional digits
                do {
                    code_point = StrIter_Next(iter);
                } while (code_point >= '0' && code_point <= '9');

                // ".lock"
                match = code_point == '.'
                        && StrIter_Starts_With_Utf8(iter, "lock", 4)
                        && StrIter_Advance(iter, SIZE_MAX) == 4;
            }
        }

        DECREF(iter);
    }

    return match;
}

static bool
S_maybe_delete_file(LockFileLockIVARS *ivars, String *path,
                    bool delete_mine, bool delete_other) {
    Folder *folder  = ivars->folder;
    bool    success = false;

    Hash *hash = (Hash*)Json_slurp_json(folder, path);
    if (hash != NULL && Obj_is_a((Obj*)hash, HASH)) {
        String *pid_buf = (String*)Hash_Fetch_Utf8(hash, "pid", 3);
        String *host    = (String*)Hash_Fetch_Utf8(hash, "host", 4);
        String *name    = (String*)Hash_Fetch_Utf8(hash, "name", 4);

        // Match hostname and lock name.
        if (host != NULL
            && Str_is_a(host, STRING)
            && Str_Equals(host, (Obj*)ivars->host)
            && name != NULL
            && Str_is_a(name, STRING)
            && Str_Equals(name, (Obj*)ivars->name)
            && pid_buf != NULL
            && Str_is_a(pid_buf, STRING)
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

    return success;
}

void
LFLock_Destroy_IMP(LockFileLock *self) {
    LockFileLockIVARS *const ivars = LFLock_IVARS(self);
    if (ivars->state != LFLOCK_STATE_UNLOCKED) { LFLock_Release(self); }
    DECREF(ivars->link_path);
    SUPER_DESTROY(self, LOCKFILELOCK);
}

/***************************************************************************/

LockErr*
LockErr_new(String *message) {
    LockErr *self = (LockErr*)Class_Make_Obj(LOCKERR);
    return LockErr_init(self, message);
}

LockErr*
LockErr_init(LockErr *self, String *message) {
    Err_init((Err*)self, message);
    return self;
}

