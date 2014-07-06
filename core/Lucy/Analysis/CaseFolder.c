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
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/Inversion.h"

CaseFolder*
CaseFolder_new() {
    CaseFolder *self = (CaseFolder*)Class_Make_Obj(CASEFOLDER);
    return CaseFolder_init(self);
}

CaseFolder*
CaseFolder_init(CaseFolder *self) {
    Analyzer_init((Analyzer*)self);
    CaseFolderIVARS *const ivars = CaseFolder_IVARS(self);
    ivars->normalizer = Normalizer_new(NULL, true, false);
    return self;
}

void
CaseFolder_Destroy_IMP(CaseFolder *self) {
    CaseFolderIVARS *const ivars = CaseFolder_IVARS(self);
    DECREF(ivars->normalizer);
    SUPER_DESTROY(self, CASEFOLDER);
}

Inversion*
CaseFolder_Transform_IMP(CaseFolder *self, Inversion *inversion) {
    CaseFolderIVARS *const ivars = CaseFolder_IVARS(self);
    return Normalizer_Transform(ivars->normalizer, inversion);
}

Inversion*
CaseFolder_Transform_Text_IMP(CaseFolder *self, String *text) {
    CaseFolderIVARS *const ivars = CaseFolder_IVARS(self);
    return Normalizer_Transform_Text(ivars->normalizer, text);
}

bool
CaseFolder_Equals_IMP(CaseFolder *self, Obj *other) {
    if ((CaseFolder*)other == self)   { return true; }
    if (!Obj_Is_A(other, CASEFOLDER)) { return false; }
    return true;
}

Hash*
CaseFolder_Dump_IMP(CaseFolder *self) {
    CaseFolder_Dump_t super_dump
        = SUPER_METHOD_PTR(CASEFOLDER, LUCY_CaseFolder_Dump);
    return super_dump(self);
}

CaseFolder*
CaseFolder_Load_IMP(CaseFolder *self, Obj *dump) {
    CaseFolder_Load_t super_load
        = SUPER_METHOD_PTR(CASEFOLDER, LUCY_CaseFolder_Load);
    CaseFolder *loaded = super_load(self, dump);
    return CaseFolder_init(loaded);
}


