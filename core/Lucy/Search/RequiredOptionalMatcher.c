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
        = (RequiredOptionalMatcher*)Class_Make_Obj(REQUIREDOPTIONALMATCHER);
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
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);

    // Assign.
    ivars->req_matcher      = (Matcher*)INCREF(required_matcher);
    ivars->opt_matcher      = (Matcher*)INCREF(optional_matcher);

    // Init.
    ivars->opt_matcher_first_time = true;

    DECREF(children);
    return self;
}

void
ReqOptMatcher_Destroy_IMP(RequiredOptionalMatcher *self) {
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);
    DECREF(ivars->req_matcher);
    DECREF(ivars->opt_matcher);
    SUPER_DESTROY(self, REQUIREDOPTIONALMATCHER);
}

int32_t
ReqOptMatcher_Next_IMP(RequiredOptionalMatcher *self) {
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);
    return Matcher_Next(ivars->req_matcher);
}

int32_t
ReqOptMatcher_Advance_IMP(RequiredOptionalMatcher *self, int32_t target) {
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);
    return Matcher_Advance(ivars->req_matcher, target);
}

int32_t
ReqOptMatcher_Get_Doc_ID_IMP(RequiredOptionalMatcher *self) {
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);
    return Matcher_Get_Doc_ID(ivars->req_matcher);
}

float
ReqOptMatcher_Score_IMP(RequiredOptionalMatcher *self) {
    RequiredOptionalMatcherIVARS *const ivars = ReqOptMatcher_IVARS(self);
    int32_t const current_doc = Matcher_Get_Doc_ID(ivars->req_matcher);

    if (ivars->opt_matcher_first_time) {
        ivars->opt_matcher_first_time = false;
        if (ivars->opt_matcher != NULL
            && !Matcher_Advance(ivars->opt_matcher, current_doc)) {
            DECREF(ivars->opt_matcher);
            ivars->opt_matcher = NULL;
        }
    }

    if (ivars->opt_matcher == NULL) {
        return Matcher_Score(ivars->req_matcher) * ivars->coord_factors[1];
    }
    else {
        int32_t opt_matcher_doc = Matcher_Get_Doc_ID(ivars->opt_matcher);

        if (opt_matcher_doc < current_doc) {
            opt_matcher_doc = Matcher_Advance(ivars->opt_matcher, current_doc);
            if (!opt_matcher_doc) {
                DECREF(ivars->opt_matcher);
                ivars->opt_matcher = NULL;
                float req_score = Matcher_Score(ivars->req_matcher);
                return req_score * ivars->coord_factors[1];
            }
        }

        if (opt_matcher_doc == current_doc) {
            float score = Matcher_Score(ivars->req_matcher)
                          + Matcher_Score(ivars->opt_matcher);
            score *= ivars->coord_factors[2];
            return score;
        }
        else {
            return Matcher_Score(ivars->req_matcher) * ivars->coord_factors[1];
        }
    }
}


