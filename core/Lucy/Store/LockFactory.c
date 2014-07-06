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

#define C_LUCY_LOCKFACTORY
#include "Lucy/Util/ToolSet.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "Lucy/Store/LockFactory.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Store/SharedLock.h"

LockFactory*
LockFact_new(Folder *folder, String *host) {
    LockFactory *self = (LockFactory*)Class_Make_Obj(LOCKFACTORY);
    return LockFact_init(self, folder, host);
}

LockFactory*
LockFact_init(LockFactory *self, Folder *folder, String *host) {
    LockFactoryIVARS *const ivars = LockFact_IVARS(self);
    ivars->folder    = (Folder*)INCREF(folder);
    ivars->host      = Str_Clone(host);
    return self;
}

void
LockFact_Destroy_IMP(LockFactory *self) {
    LockFactoryIVARS *const ivars = LockFact_IVARS(self);
    DECREF(ivars->folder);
    DECREF(ivars->host);
    SUPER_DESTROY(self, LOCKFACTORY);
}

Lock*
LockFact_Make_Lock_IMP(LockFactory *self, String *name,
                       int32_t timeout, int32_t interval) {
    LockFactoryIVARS *const ivars = LockFact_IVARS(self);
    return (Lock*)LFLock_new(ivars->folder, name, ivars->host, timeout,
                             interval);
}

Lock*
LockFact_Make_Shared_Lock_IMP(LockFactory *self, String *name,
                              int32_t timeout, int32_t interval) {
    LockFactoryIVARS *const ivars = LockFact_IVARS(self);
    return (Lock*)ShLock_new(ivars->folder, name, ivars->host, timeout,
                             interval);
}


