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
FType_init2(FieldType *self, float boost, bool_t indexed, bool_t stored,
            bool_t sortable) {
    self->boost              = boost;
    self->indexed            = indexed;
    self->stored             = stored;
    self->sortable           = sortable;
    ABSTRACT_CLASS_CHECK(self, FIELDTYPE);
    return self;
}

void
FType_set_boost(FieldType *self, float boost) {
    self->boost = boost;
}

void
FType_set_indexed(FieldType *self, bool_t indexed) {
    self->indexed = !!indexed;
}

void
FType_set_stored(FieldType *self, bool_t stored) {
    self->stored = !!stored;
}

void
FType_set_sortable(FieldType *self, bool_t sortable) {
    self->sortable = !!sortable;
}

float
FType_get_boost(FieldType *self) {
    return self->boost;
}

bool_t
FType_indexed(FieldType *self) {
    return self->indexed;
}

bool_t
FType_stored(FieldType *self) {
    return self->stored;
}

bool_t
FType_sortable(FieldType *self) {
    return self->sortable;
}

bool_t
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

bool_t
FType_equals(FieldType *self, Obj *other) {
    FieldType *twin = (FieldType*)other;
    if (twin == self)                                     { return true; }
    if (FType_Get_VTable(self) != FType_Get_VTable(twin)) { return false; }
    if (self->boost != twin->boost)                       { return false; }
    if (!!self->indexed    != !!twin->indexed)            { return false; }
    if (!!self->stored     != !!twin->stored)             { return false; }
    if (!!self->sortable   != !!twin->sortable)           { return false; }
    if (!!FType_Binary(self) != !!FType_Binary(twin))     { return false; }
    return true;
}


