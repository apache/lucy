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

#define C_LUCY_ANDMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/ANDMatcher.h"
#include "Lucy/Index/Similarity.h"

ANDMatcher*
ANDMatcher_new(VArray *children, Similarity *sim) {
    ANDMatcher *self = (ANDMatcher*)Class_Make_Obj(ANDMATCHER);
    return ANDMatcher_init(self, children, sim);
}

ANDMatcher*
ANDMatcher_init(ANDMatcher *self, VArray *children, Similarity *sim) {
    ANDMatcherIVARS *const ivars = ANDMatcher_IVARS(self);

    // Init.
    PolyMatcher_init((PolyMatcher*)self, children, sim);
    ivars->first_time   = true;

    // Assign.
    ivars->more         = ivars->num_kids ? true : false;
    ivars->kids         = (Matcher**)MALLOCATE(ivars->num_kids * sizeof(Matcher*));
    for (uint32_t i = 0; i < ivars->num_kids; i++) {
        Matcher *child = (Matcher*)VA_Fetch(children, i);
        ivars->kids[i] = child;
        if (!Matcher_Next(child)) { ivars->more = false; }
    }

    // Derive.
    ivars->matching_kids = ivars->num_kids;

    return self;
}

void
ANDMatcher_Destroy_IMP(ANDMatcher *self) {
    ANDMatcherIVARS *const ivars = ANDMatcher_IVARS(self);
    FREEMEM(ivars->kids);
    SUPER_DESTROY(self, ANDMATCHER);
}

int32_t
ANDMatcher_Next_IMP(ANDMatcher *self) {
    ANDMatcherIVARS *const ivars = ANDMatcher_IVARS(self);
    if (ivars->first_time) {
        return ANDMatcher_Advance(self, 1);
    }
    if (ivars->more) {
        const int32_t target = Matcher_Get_Doc_ID(ivars->kids[0]) + 1;
        return ANDMatcher_Advance(self, target);
    }
    else {
        return 0;
    }
}

int32_t
ANDMatcher_Advance_IMP(ANDMatcher *self, int32_t target) {
    ANDMatcherIVARS *const ivars = ANDMatcher_IVARS(self);
    Matcher **const kids     = ivars->kids;
    const uint32_t  num_kids = ivars->num_kids;
    int32_t         highest  = 0;

    if (!ivars->more) { return 0; }

    // First step: Advance first child and use its doc as a starting point.
    if (ivars->first_time) {
        ivars->first_time = false;
    }
    else {
        highest = Matcher_Advance(kids[0], target);
        if (!highest) {
            ivars->more = false;
            return 0;
        }
    }

    // Second step: reconcile.
    while (1) {
        bool agreement = true;

        // Scoot all Matchers up.
        for (uint32_t i = 0; i < num_kids; i++) {
            Matcher *const child = kids[i];
            int32_t candidate = Matcher_Get_Doc_ID(child);

            // If this child is highest, others will need to catch up.
            if (highest < candidate) {
                highest = candidate;
            }

            // If least doc Matchers can agree on exceeds target, raise bar.
            if (target < highest) {
                target = highest;
            }

            // Scoot this Matcher up if not already at highest.
            if (candidate < target) {
                // This Matcher is definitely the highest right now.
                highest = Matcher_Advance(child, target);
                if (!highest) {
                    ivars->more = false;
                    return 0;
                }
            }
        }

        // If Matchers don't agree, send back through the loop.
        for (uint32_t i = 0; i < num_kids; i++) {
            Matcher *const child = kids[i];
            const int32_t candidate = Matcher_Get_Doc_ID(child);
            if (candidate != highest) {
                agreement = false;
                break;
            }
        }

        if (!agreement) {
            continue;
        }
        if (highest >= target) {
            break;
        }
    }

    return highest;
}

int32_t
ANDMatcher_Get_Doc_ID_IMP(ANDMatcher *self) {
    return Matcher_Get_Doc_ID(ANDMatcher_IVARS(self)->kids[0]);
}

float
ANDMatcher_Score_IMP(ANDMatcher *self) {
    ANDMatcherIVARS *const ivars = ANDMatcher_IVARS(self);
    Matcher **const kids = ivars->kids;
    float score = 0.0f;

    for (uint32_t i = 0; i < ivars->num_kids; i++) {
        score += Matcher_Score(kids[i]);
    }

    score *= ivars->coord_factors[ivars->matching_kids];

    return score;
}

