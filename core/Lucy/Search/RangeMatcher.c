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

#define C_LUCY_RANGEMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/RangeMatcher.h"
#include "Lucy/Index/SortCache.h"

RangeMatcher*
RangeMatcher_new(int32_t lower_bound, int32_t upper_bound, SortCache *sort_cache,
                 int32_t doc_max) {
    RangeMatcher *self = (RangeMatcher*)Class_Make_Obj(RANGEMATCHER);
    return RangeMatcher_init(self, lower_bound, upper_bound, sort_cache,
                             doc_max);
}

RangeMatcher*
RangeMatcher_init(RangeMatcher *self, int32_t lower_bound, int32_t upper_bound,
                  SortCache *sort_cache, int32_t doc_max) {
    Matcher_init((Matcher*)self);
    RangeMatcherIVARS *const ivars = RangeMatcher_IVARS(self);

    // Init.
    ivars->doc_id       = 0;

    // Assign.
    ivars->lower_bound  = lower_bound;
    ivars->upper_bound  = upper_bound;
    ivars->sort_cache   = (SortCache*)INCREF(sort_cache);
    ivars->doc_max      = doc_max;

    // Derive.

    return self;
}

void
RangeMatcher_Destroy_IMP(RangeMatcher *self) {
    RangeMatcherIVARS *const ivars = RangeMatcher_IVARS(self);
    DECREF(ivars->sort_cache);
    SUPER_DESTROY(self, RANGEMATCHER);
}

int32_t
RangeMatcher_Next_IMP(RangeMatcher* self) {
    RangeMatcherIVARS *const ivars = RangeMatcher_IVARS(self);
    while (1) {
        if (++ivars->doc_id > ivars->doc_max) {
            ivars->doc_id--;
            return 0;
        }
        else {
            // Check if ord for this document is within the specied range.
            // TODO: Unroll? i.e. use SortCache_Get_Ords at constructor time
            // and save ourselves some method call overhead.
            const int32_t ord
                = SortCache_Ordinal(ivars->sort_cache, ivars->doc_id);
            if (ord >= ivars->lower_bound && ord <= ivars->upper_bound) {
                break;
            }
        }
    }
    return ivars->doc_id;
}

int32_t
RangeMatcher_Advance_IMP(RangeMatcher* self, int32_t target) {
    RangeMatcher_IVARS(self)->doc_id = target - 1;
    return RangeMatcher_Next_IMP(self);
}

float
RangeMatcher_Score_IMP(RangeMatcher* self) {
    UNUSED_VAR(self);
    return 0.0f;
}

int32_t
RangeMatcher_Get_Doc_ID_IMP(RangeMatcher* self) {
    return RangeMatcher_IVARS(self)->doc_id;
}


