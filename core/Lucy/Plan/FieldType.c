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

#define C_LUCY_FIELDTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/FieldType.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/Similarity.h"

FieldType*
FType_init(FieldType *self) {
    return FType_init2(self, 1.0f, false, false, false);
}

FieldType*
FType_init2(FieldType *self, float boost, bool indexed, bool stored,
            bool sortable) {
    FieldTypeIVARS *const ivars = FType_IVARS(self);
    ivars->boost             = boost;
    ivars->indexed           = indexed;
    ivars->stored            = stored;
    ivars->sortable          = sortable;
    ABSTRACT_CLASS_CHECK(self, FIELDTYPE);
    return self;
}

void
FType_Set_Boost_IMP(FieldType *self, float boost) {
    FType_IVARS(self)->boost = boost;
}

void
FType_Set_Indexed_IMP(FieldType *self, bool indexed) {
    FType_IVARS(self)->indexed = !!indexed;
}

void
FType_Set_Stored_IMP(FieldType *self, bool stored) {
    FType_IVARS(self)->stored = !!stored;
}

void
FType_Set_Sortable_IMP(FieldType *self, bool sortable) {
    FType_IVARS(self)->sortable = !!sortable;
}

float
FType_Get_Boost_IMP(FieldType *self) {
    return FType_IVARS(self)->boost;
}

bool
FType_Indexed_IMP(FieldType *self) {
    return FType_IVARS(self)->indexed;
}

bool
FType_Stored_IMP(FieldType *self) {
    return FType_IVARS(self)->stored;
}

bool
FType_Sortable_IMP(FieldType *self) {
    return FType_IVARS(self)->sortable;
}

bool
FType_Binary_IMP(FieldType *self) {
    UNUSED_VAR(self);
    return false;
}

Similarity*
FType_Similarity_IMP(FieldType *self) {
    UNUSED_VAR(self);
    return NULL;
}

int32_t
FType_Compare_Values_IMP(FieldType *self, Obj *a, Obj *b) {
    UNUSED_VAR(self);
    return Obj_Compare_To(a, b);
}

bool
FType_Equals_IMP(FieldType *self, Obj *other) {
    if ((FieldType*)other == self)                     { return true; }
    if (FType_Get_Class(self) != Obj_Get_Class(other)) { return false; }
    FieldTypeIVARS *const ivars = FType_IVARS(self);
    FieldTypeIVARS *const ovars = FType_IVARS((FieldType*)other);
    if (ivars->boost != ovars->boost)                  { return false; }
    if (!!ivars->indexed    != !!ovars->indexed)       { return false; }
    if (!!ivars->stored     != !!ovars->stored)        { return false; }
    if (!!ivars->sortable   != !!ovars->sortable)      { return false; }
    if (!!FType_Binary(self) != !!FType_Binary((FieldType*)other)) {
        return false;
    }
    return true;
}


