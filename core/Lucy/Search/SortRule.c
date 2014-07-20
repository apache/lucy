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
#include "Lucy/Util/Freezer.h"

int32_t SortRule_FIELD  = 0;
int32_t SortRule_SCORE  = 1;
int32_t SortRule_DOC_ID = 2;

SortRule*
SortRule_new(int32_t type, String *field, bool reverse) {
    SortRule *self = (SortRule*)Class_Make_Obj(SORTRULE);
    return SortRule_init(self, type, field, reverse);
}

SortRule*
SortRule_init(SortRule *self, int32_t type, String *field,
              bool reverse) {
    SortRuleIVARS *ivars = SortRule_IVARS(self);
    ivars->field    = field ? Str_Clone(field) : NULL;
    ivars->type     = type;
    ivars->reverse  = reverse;

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
SortRule_Destroy_IMP(SortRule *self) {
    SortRuleIVARS *ivars = SortRule_IVARS(self);
    DECREF(ivars->field);
    SUPER_DESTROY(self, SORTRULE);
}

SortRule*
SortRule_Deserialize_IMP(SortRule *self, InStream *instream) {
    SortRuleIVARS *ivars = SortRule_IVARS(self);
    ivars->type = InStream_Read_C32(instream);
    if (ivars->type == SortRule_FIELD) {
        ivars->field = Freezer_read_string(instream);
    }
    ivars->reverse = !!InStream_Read_C32(instream);
    return self;
}

void
SortRule_Serialize_IMP(SortRule *self, OutStream *target) {
    SortRuleIVARS *ivars = SortRule_IVARS(self);
    OutStream_Write_C32(target, ivars->type);
    if (ivars->type == SortRule_FIELD) {
        Freezer_serialize_string(ivars->field, target);
    }
    OutStream_Write_C32(target, !!ivars->reverse);
}

String*
SortRule_Get_Field_IMP(SortRule *self) {
    return SortRule_IVARS(self)->field;
}

int32_t
SortRule_Get_Type_IMP(SortRule *self) {
    return SortRule_IVARS(self)->type;
}

bool
SortRule_Get_Reverse_IMP(SortRule *self) {
    return SortRule_IVARS(self)->reverse;
}


