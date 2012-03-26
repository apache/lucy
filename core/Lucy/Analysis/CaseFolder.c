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

#define C_LUCY_CASEFOLDER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/CaseFolder.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

CaseFolder*
CaseFolder_new() {
    CaseFolder *self = (CaseFolder*)VTable_Make_Obj(CASEFOLDER);
    return CaseFolder_init(self);
}

CaseFolder*
CaseFolder_init(CaseFolder *self) {
    Analyzer_init((Analyzer*)self);
    self->work_buf = BB_new(0);
    return self;
}

void
CaseFolder_destroy(CaseFolder *self) {
    DECREF(self->work_buf);
    SUPER_DESTROY(self, CASEFOLDER);
}

bool_t
CaseFolder_equals(CaseFolder *self, Obj *other) {
    CaseFolder *const twin = (CaseFolder*)other;
    if (twin == self)                 { return true; }
    UNUSED_VAR(self);
    if (!Obj_Is_A(other, CASEFOLDER)) { return false; }
    return true;
}

Hash*
CaseFolder_dump(CaseFolder *self) {
    CaseFolder_dump_t super_dump
        = (CaseFolder_dump_t)SUPER_METHOD(CASEFOLDER, CaseFolder, Dump);
    return super_dump(self);
}

CaseFolder*
CaseFolder_load(CaseFolder *self, Obj *dump) {
    CaseFolder_load_t super_load
        = (CaseFolder_load_t)SUPER_METHOD(CASEFOLDER, CaseFolder, Load);
    CaseFolder *loaded = super_load(self, dump);
    return CaseFolder_init(loaded);
}


