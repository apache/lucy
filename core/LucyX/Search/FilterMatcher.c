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

#define C_LUCY_FILTERMATCHER
#include "Lucy/Util/ToolSet.h"

#include "LucyX/Search/FilterMatcher.h"

FilterMatcher*
FilterMatcher_new(BitVector *bits, int32_t doc_max) {
    FilterMatcher *self = (FilterMatcher*)Class_Make_Obj(FILTERMATCHER);
    return FilterMatcher_init(self, bits, doc_max);
}

FilterMatcher*
FilterMatcher_init(FilterMatcher *self, BitVector *bits, int32_t doc_max) {
    Matcher_init((Matcher*)self);
    FilterMatcherIVARS *const ivars = FilterMatcher_IVARS(self);

    // Init.
    ivars->doc_id       = 0;

    // Assign.
    ivars->bits         = (BitVector*)INCREF(bits);
    ivars->doc_max      = doc_max;

    return self;
}

void
FilterMatcher_Destroy_IMP(FilterMatcher *self) {
    FilterMatcherIVARS *const ivars = FilterMatcher_IVARS(self);
    DECREF(ivars->bits);
    SUPER_DESTROY(self, FILTERMATCHER);
}

int32_t
FilterMatcher_Next_IMP(FilterMatcher* self) {
    FilterMatcherIVARS *const ivars = FilterMatcher_IVARS(self);
    do {
        if (++ivars->doc_id > ivars->doc_max) {
            ivars->doc_id--;
            return 0;
        }
    } while (!BitVec_Get(ivars->bits, ivars->doc_id));
    return ivars->doc_id;
}

int32_t
FilterMatcher_Skip_To_IMP(FilterMatcher* self, int32_t target) {
    FilterMatcher_IVARS(self)->doc_id = target - 1;
    return FilterMatcher_Next_IMP(self);
}

float
FilterMatcher_Score_IMP(FilterMatcher* self) {
    UNUSED_VAR(self);
    return 0.0f;
}

int32_t
FilterMatcher_Get_Doc_ID_IMP(FilterMatcher* self) {
    return FilterMatcher_IVARS(self)->doc_id;
}


