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
    RangeMatcher *self = (RangeMatcher*)VTable_Make_Obj(RANGEMATCHER);
    return RangeMatcher_init(self, lower_bound, upper_bound, sort_cache,
                             doc_max);
}

RangeMatcher*
RangeMatcher_init(RangeMatcher *self, int32_t lower_bound, int32_t upper_bound,
                  SortCache *sort_cache, int32_t doc_max) {
    Matcher_init((Matcher*)self);

    // Init.
    self->doc_id       = 0;

    // Assign.
    self->lower_bound  = lower_bound;
    self->upper_bound  = upper_bound;
    self->sort_cache   = (SortCache*)INCREF(sort_cache);
    self->doc_max      = doc_max;

    // Derive.

    return self;
}

void
RangeMatcher_destroy(RangeMatcher *self) {
    DECREF(self->sort_cache);
    SUPER_DESTROY(self, RANGEMATCHER);
}

int32_t
RangeMatcher_next(RangeMatcher* self) {
    while (1) {
        if (++self->doc_id > self->doc_max) {
            self->doc_id--;
            return 0;
        }
        else {
            // Check if ord for this document is within the specied range.
            // TODO: Unroll? i.e. use SortCache_Get_Ords at constructor time
            // and save ourselves some method call overhead.
            const int32_t ord
                = SortCache_Ordinal(self->sort_cache, self->doc_id);
            if (ord >= self->lower_bound && ord <= self->upper_bound) {
                break;
            }
        }
    }
    return self->doc_id;
}

int32_t
RangeMatcher_advance(RangeMatcher* self, int32_t target) {
    self->doc_id = target - 1;
    return RangeMatcher_next(self);
}

float
RangeMatcher_score(RangeMatcher* self) {
    UNUSED_VAR(self);
    return 0.0f;
}

int32_t
RangeMatcher_get_doc_id(RangeMatcher* self) {
    return self->doc_id;
}


