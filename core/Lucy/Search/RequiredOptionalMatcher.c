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

#define C_LUCY_REQUIREDOPTIONALMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/RequiredOptionalMatcher.h"
#include "Lucy/Index/Similarity.h"

RequiredOptionalMatcher*
ReqOptMatcher_new(Similarity *similarity, Matcher *required_matcher,
                  Matcher *optional_matcher) {
    RequiredOptionalMatcher *self
        = (RequiredOptionalMatcher*)VTable_Make_Obj(REQUIREDOPTIONALMATCHER);
    return ReqOptMatcher_init(self, similarity, required_matcher,
                              optional_matcher);
}

RequiredOptionalMatcher*
ReqOptMatcher_init(RequiredOptionalMatcher *self, Similarity *similarity,
                   Matcher *required_matcher, Matcher *optional_matcher) {
    VArray *children = VA_new(2);
    VA_Push(children, INCREF(required_matcher));
    VA_Push(children, INCREF(optional_matcher));
    PolyMatcher_init((PolyMatcher*)self, children, similarity);

    // Assign.
    self->req_matcher       = (Matcher*)INCREF(required_matcher);
    self->opt_matcher       = (Matcher*)INCREF(optional_matcher);

    // Init.
    self->opt_matcher_first_time = true;

    DECREF(children);
    return self;
}

void
ReqOptMatcher_destroy(RequiredOptionalMatcher *self) {
    DECREF(self->req_matcher);
    DECREF(self->opt_matcher);
    SUPER_DESTROY(self, REQUIREDOPTIONALMATCHER);
}

int32_t
ReqOptMatcher_next(RequiredOptionalMatcher *self) {
    return Matcher_Next(self->req_matcher);
}

int32_t
ReqOptMatcher_advance(RequiredOptionalMatcher *self, int32_t target) {
    return Matcher_Advance(self->req_matcher, target);
}

int32_t
ReqOptMatcher_get_doc_id(RequiredOptionalMatcher *self) {
    return Matcher_Get_Doc_ID(self->req_matcher);
}

float
ReqOptMatcher_score(RequiredOptionalMatcher *self) {
    int32_t const current_doc = Matcher_Get_Doc_ID(self->req_matcher);

    if (self->opt_matcher_first_time) {
        self->opt_matcher_first_time = false;
        if (self->opt_matcher != NULL
            && !Matcher_Advance(self->opt_matcher, current_doc)) {
            DECREF(self->opt_matcher);
            self->opt_matcher = NULL;
        }
    }

    if (self->opt_matcher == NULL) {
        return Matcher_Score(self->req_matcher) * self->coord_factors[1];
    }
    else {
        int32_t opt_matcher_doc = Matcher_Get_Doc_ID(self->opt_matcher);

        if (opt_matcher_doc < current_doc) {
            opt_matcher_doc = Matcher_Advance(self->opt_matcher, current_doc);
            if (!opt_matcher_doc) {
                DECREF(self->opt_matcher);
                self->opt_matcher = NULL;
                float req_score = Matcher_Score(self->req_matcher);
                return req_score * self->coord_factors[1];
            }
        }

        if (opt_matcher_doc == current_doc) {
            float score = Matcher_Score(self->req_matcher)
                          + Matcher_Score(self->opt_matcher);
            score *= self->coord_factors[2];
            return score;
        }
        else {
            return Matcher_Score(self->req_matcher) * self->coord_factors[1];
        }
    }
}


