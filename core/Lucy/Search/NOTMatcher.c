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
    NOTMatcher *self = (NOTMatcher*)VTable_Make_Obj(NOTMATCHER);
    return NOTMatcher_init(self, negated_matcher, doc_max);
}

NOTMatcher*
NOTMatcher_init(NOTMatcher *self, Matcher *negated_matcher, int32_t doc_max) {
    VArray *children = VA_new(1);
    VA_Push(children, INCREF(negated_matcher));
    PolyMatcher_init((PolyMatcher*)self, children, NULL);

    // Init.
    self->doc_id           = 0;
    self->next_negation    = 0;

    // Assign.
    self->negated_matcher  = (Matcher*)INCREF(negated_matcher);
    self->doc_max          = doc_max;

    DECREF(children);

    return self;
}

void
NOTMatcher_destroy(NOTMatcher *self) {
    DECREF(self->negated_matcher);
    SUPER_DESTROY(self, NOTMATCHER);
}

int32_t
NOTMatcher_next(NOTMatcher *self) {
    while (1) {
        self->doc_id++;

        // Get next negated doc id.
        if (self->next_negation < self->doc_id) {
            self->next_negation
                = Matcher_Advance(self->negated_matcher, self->doc_id);
            if (self->next_negation == 0) {
                DECREF(self->negated_matcher);
                self->negated_matcher = NULL;
                self->next_negation = self->doc_max + 1;
            }
        }

        if (self->doc_id > self->doc_max) {
            self->doc_id = self->doc_max; // halt advance
            return 0;
        }
        else if (self->doc_id != self->next_negation) {
            // Success!
            return self->doc_id;
        }
    }
}

int32_t
NOTMatcher_advance(NOTMatcher *self, int32_t target) {
    self->doc_id = target - 1;
    return NOTMatcher_next(self);
}

int32_t
NOTMatcher_get_doc_id(NOTMatcher *self) {
    return self->doc_id;
}

float
NOTMatcher_score(NOTMatcher *self) {
    UNUSED_VAR(self);
    return 0.0f;
}


