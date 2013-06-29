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
FType_set_boost(FieldType *self, float boost) {
    FType_IVARS(self)->boost = boost;
}

void
FType_set_indexed(FieldType *self, bool indexed) {
    FType_IVARS(self)->indexed = !!indexed;
}

void
FType_set_stored(FieldType *self, bool stored) {
    FType_IVARS(self)->stored = !!stored;
}

void
FType_set_sortable(FieldType *self, bool sortable) {
    FType_IVARS(self)->sortable = !!sortable;
}

float
FType_get_boost(FieldType *self) {
    return FType_IVARS(self)->boost;
}

bool
FType_indexed(FieldType *self) {
    return FType_IVARS(self)->indexed;
}

bool
FType_stored(FieldType *self) {
    return FType_IVARS(self)->stored;
}

bool
FType_sortable(FieldType *self) {
    return FType_IVARS(self)->sortable;
}

bool
FType_binary(FieldType *self) {
    UNUSED_VAR(self);
    return false;
}

Similarity*
FType_similarity(FieldType *self) {
    UNUSED_VAR(self);
    return NULL;
}

int32_t
FType_compare_values(FieldType *self, Obj *a, Obj *b) {
    UNUSED_VAR(self);
    return Obj_Compare_To(a, b);
}

bool
FType_equals(FieldType *self, Obj *other) {
    if ((FieldType*)other == self)                       { return true; }
    if (FType_Get_VTable(self) != Obj_Get_VTable(other)) { return false; }
    FieldTypeIVARS *const ivars = FType_IVARS(self);
    FieldTypeIVARS *const ovars = FType_IVARS((FieldType*)other);
    if (ivars->boost != ovars->boost)                    { return false; }
    if (!!ivars->indexed    != !!ovars->indexed)         { return false; }
    if (!!ivars->stored     != !!ovars->stored)          { return false; }
    if (!!ivars->sortable   != !!ovars->sortable)        { return false; }
    if (!!FType_Binary(self) != !!FType_Binary((FieldType*)other)) {
        return false;
    }
    return true;
}


