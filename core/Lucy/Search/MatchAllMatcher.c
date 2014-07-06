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

#define C_LUCY_MATCHALLMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/MatchAllMatcher.h"

MatchAllMatcher*
MatchAllMatcher_new(float score, int32_t doc_max) {
    MatchAllMatcher *self
        = (MatchAllMatcher*)Class_Make_Obj(MATCHALLMATCHER);
    return MatchAllMatcher_init(self, score, doc_max);
}

MatchAllMatcher*
MatchAllMatcher_init(MatchAllMatcher *self, float score, int32_t doc_max) {
    MatchAllMatcherIVARS *const ivars = MatchAllMatcher_IVARS(self);
    Matcher_init((Matcher*)self);
    ivars->doc_id        = 0;
    ivars->score         = score;
    ivars->doc_max       = doc_max;
    return self;
}

int32_t
MatchAllMatcher_Next_IMP(MatchAllMatcher* self) {
    MatchAllMatcherIVARS *const ivars = MatchAllMatcher_IVARS(self);
    if (++ivars->doc_id <= ivars->doc_max) {
        return ivars->doc_id;
    }
    else {
        ivars->doc_id--;
        return 0;
    }
}

int32_t
MatchAllMatcher_Advance_IMP(MatchAllMatcher* self, int32_t target) {
    MatchAllMatcher_IVARS(self)->doc_id = target - 1;
    return MatchAllMatcher_Next_IMP(self);
}

float
MatchAllMatcher_Score_IMP(MatchAllMatcher* self) {
    return MatchAllMatcher_IVARS(self)->score;
}

int32_t
MatchAllMatcher_Get_Doc_ID_IMP(MatchAllMatcher* self) {
    return MatchAllMatcher_IVARS(self)->doc_id;
}


