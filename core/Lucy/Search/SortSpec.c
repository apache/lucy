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

#define C_LUCY_SORTSPEC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/SortSpec.h"
#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/SortUtils.h"

SortSpec*
SortSpec_new(VArray *rules) {
    SortSpec *self = (SortSpec*)VTable_Make_Obj(SORTSPEC);
    return SortSpec_init(self, rules);
}

SortSpec*
SortSpec_init(SortSpec *self, VArray *rules) {
    int32_t i, max;
    self->rules = VA_Shallow_Copy(rules);
    for (i = 0, max = VA_Get_Size(rules); i < max; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(rules, i);
        CERTIFY(rule, SORTRULE);
    }
    return self;
}

void
SortSpec_destroy(SortSpec *self) {
    DECREF(self->rules);
    SUPER_DESTROY(self, SORTSPEC);
}

SortSpec*
SortSpec_deserialize(SortSpec *self, InStream *instream) {
    uint32_t num_rules = InStream_Read_C32(instream);
    VArray *rules = VA_new(num_rules);
    uint32_t i;

    // Create base object.
    self = self ? self : (SortSpec*)VTable_Make_Obj(SORTSPEC);

    // Add rules.
    for (i = 0; i < num_rules; i++) {
        VA_Push(rules, (Obj*)SortRule_deserialize(NULL, instream));
    }
    SortSpec_init(self, rules);
    DECREF(rules);

    return self;
}

VArray*
SortSpec_get_rules(SortSpec *self) {
    return self->rules;
}

void
SortSpec_serialize(SortSpec *self, OutStream *target) {
    uint32_t num_rules = VA_Get_Size(self->rules);
    uint32_t i;
    OutStream_Write_C32(target, num_rules);
    for (i = 0; i < num_rules; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(self->rules, i);
        SortRule_Serialize(rule, target);
    }
}


