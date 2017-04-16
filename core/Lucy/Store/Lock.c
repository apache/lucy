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
#include "Lucy/Util/ToolSet.h"

#include <ctype.h>

#include "Lucy/Store/Lock.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/Sleep.h"

Lock*
Lock_init(Lock *self, Folder *folder, String *name, int32_t timeout,
          int32_t interval) {
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
    ivars->interval     = interval;

    // Derive.
    ivars->lock_path = Str_newf("locks/%o.lock", name);

    return self;
}

void
Lock_Destroy_IMP(Lock *self) {
    LockIVARS *const ivars = Lock_IVARS(self);
    DECREF(ivars->folder);
    DECREF(ivars->name);
    DECREF(ivars->lock_path);
    SUPER_DESTROY(self, LOCK);
}

bool
Lock_make_lock_dir(Folder *folder) {
    String *lock_dir_name = SSTR_WRAP_C("locks");

    if (!Folder_MkDir(folder, lock_dir_name)) {
        Err *err = (Err*)INCREF(Err_get_error());
        // Maybe our attempt failed because another process succeeded.
        if (Folder_Find_Folder(folder, lock_dir_name)) {
            DECREF(err);
        }
        else {
            // Nope, everything failed, so bail out.
            Err_set_error(err);
            return false;
        }
    }

    return true;
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

