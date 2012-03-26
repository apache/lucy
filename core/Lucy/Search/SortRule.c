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

#define C_LUCY_SORTRULE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/SortRule.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

int32_t SortRule_FIELD  = 0;
int32_t SortRule_SCORE  = 1;
int32_t SortRule_DOC_ID = 2;

SortRule*
SortRule_new(int32_t type, const CharBuf *field, bool_t reverse) {
    SortRule *self = (SortRule*)VTable_Make_Obj(SORTRULE);
    return SortRule_init(self, type, field, reverse);
}

SortRule*
SortRule_init(SortRule *self, int32_t type, const CharBuf *field,
              bool_t reverse) {
    self->field    = field ? CB_Clone(field) : NULL;
    self->type     = type;
    self->reverse  = reverse;

    // Validate.
    if (type == SortRule_FIELD) {
        if (!field) {
            THROW(ERR, "When sorting by field, param 'field' is required");
        }
    }
    else if (type == SortRule_SCORE)  { }
    else if (type == SortRule_DOC_ID) { }
    else { THROW(ERR, "Unknown type: %i32", type); }

    return self;
}

void
SortRule_destroy(SortRule *self) {
    DECREF(self->field);
    SUPER_DESTROY(self, SORTRULE);
}

SortRule*
SortRule_deserialize(SortRule *self, InStream *instream) {
    self = self ? self : (SortRule*)VTable_Make_Obj(SORTRULE);
    self->type = InStream_Read_C32(instream);
    if (self->type == SortRule_FIELD) {
        self->field = CB_deserialize(NULL, instream);
    }
    self->reverse = InStream_Read_C32(instream);
    return self;
}

void
SortRule_serialize(SortRule *self, OutStream *target) {
    OutStream_Write_C32(target, self->type);
    if (self->type == SortRule_FIELD) {
        CB_Serialize(self->field, target);
    }
    OutStream_Write_C32(target, !!self->reverse);
}

CharBuf*
SortRule_get_field(SortRule *self) {
    return self->field;
}

int32_t
SortRule_get_type(SortRule *self) {
    return self->type;
}

bool_t
SortRule_get_reverse(SortRule *self) {
    return self->reverse;
}


