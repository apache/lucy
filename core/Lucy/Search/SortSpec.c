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
#include "Clownfish/Util/SortUtils.h"

SortSpec*
SortSpec_new(VArray *rules) {
    SortSpec *self = (SortSpec*)Class_Make_Obj(SORTSPEC);
    return SortSpec_init(self, rules);
}

SortSpec*
SortSpec_init(SortSpec *self, VArray *rules) {
    SortSpecIVARS *const ivars = SortSpec_IVARS(self);
    ivars->rules = VA_Shallow_Copy(rules);
    for (int32_t i = 0, max = VA_Get_Size(rules); i < max; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(rules, i);
        CERTIFY(rule, SORTRULE);
    }
    return self;
}

void
SortSpec_Destroy_IMP(SortSpec *self) {
    SortSpecIVARS *const ivars = SortSpec_IVARS(self);
    DECREF(ivars->rules);
    SUPER_DESTROY(self, SORTSPEC);
}

SortSpec*
SortSpec_Deserialize_IMP(SortSpec *self, InStream *instream) {
    uint32_t num_rules = InStream_Read_C32(instream);
    VArray *rules = VA_new(num_rules);

    // Add rules.
    for (uint32_t i = 0; i < num_rules; i++) {
        SortRule *blank = (SortRule*)Class_Make_Obj(SORTRULE);
        VA_Push(rules, (Obj*)SortRule_Deserialize(blank, instream));
    }
    SortSpec_init(self, rules);
    DECREF(rules);

    return self;
}

VArray*
SortSpec_Get_Rules_IMP(SortSpec *self) {
    return SortSpec_IVARS(self)->rules;
}

void
SortSpec_Serialize_IMP(SortSpec *self, OutStream *target) {
    SortSpecIVARS *const ivars = SortSpec_IVARS(self);
    uint32_t num_rules = VA_Get_Size(ivars->rules);
    OutStream_Write_C32(target, num_rules);
    for (uint32_t i = 0; i < num_rules; i++) {
        SortRule *rule = (SortRule*)VA_Fetch(ivars->rules, i);
        SortRule_Serialize(rule, target);
    }
}


