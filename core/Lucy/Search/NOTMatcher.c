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

#define C_LUCY_NOTMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/NOTMatcher.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"

NOTMatcher*
NOTMatcher_new(Matcher *negated_matcher, int32_t doc_max) {
    NOTMatcher *self = (NOTMatcher*)Class_Make_Obj(NOTMATCHER);
    return NOTMatcher_init(self, negated_matcher, doc_max);
}

NOTMatcher*
NOTMatcher_init(NOTMatcher *self, Matcher *negated_matcher, int32_t doc_max) {
    NOTMatcherIVARS *const ivars = NOTMatcher_IVARS(self);
    VArray *children = VA_new(1);
    VA_Push(children, INCREF(negated_matcher));
    PolyMatcher_init((PolyMatcher*)self, children, NULL);

    // Init.
    ivars->doc_id           = 0;
    ivars->next_negation    = 0;

    // Assign.
    ivars->negated_matcher  = (Matcher*)INCREF(negated_matcher);
    ivars->doc_max          = doc_max;

    DECREF(children);

    return self;
}

void
NOTMatcher_Destroy_IMP(NOTMatcher *self) {
    NOTMatcherIVARS *const ivars = NOTMatcher_IVARS(self);
    DECREF(ivars->negated_matcher);
    SUPER_DESTROY(self, NOTMATCHER);
}

int32_t
NOTMatcher_Next_IMP(NOTMatcher *self) {
    NOTMatcherIVARS *const ivars = NOTMatcher_IVARS(self);
    while (1) {
        ivars->doc_id++;

        // Get next negated doc id.
        if (ivars->next_negation < ivars->doc_id) {
            ivars->next_negation
                = Matcher_Advance(ivars->negated_matcher, ivars->doc_id);
            if (ivars->next_negation == 0) {
                DECREF(ivars->negated_matcher);
                ivars->negated_matcher = NULL;
                ivars->next_negation = ivars->doc_max + 1;
            }
        }

        if (ivars->doc_id > ivars->doc_max) {
            ivars->doc_id = ivars->doc_max; // halt advance
            return 0;
        }
        else if (ivars->doc_id != ivars->next_negation) {
            // Success!
            return ivars->doc_id;
        }
    }
}

int32_t
NOTMatcher_Advance_IMP(NOTMatcher *self, int32_t target) {
    NOTMatcher_IVARS(self)->doc_id = target - 1;
    return NOTMatcher_Next_IMP(self);
}

int32_t
NOTMatcher_Get_Doc_ID_IMP(NOTMatcher *self) {
    return NOTMatcher_IVARS(self)->doc_id;
}

float
NOTMatcher_Score_IMP(NOTMatcher *self) {
    UNUSED_VAR(self);
    return 0.0f;
}


