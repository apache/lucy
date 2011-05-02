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
    ANDMatcher *self = (ANDMatcher*)VTable_Make_Obj(ANDMATCHER);
    return ANDMatcher_init(self, children, sim);
}

ANDMatcher*
ANDMatcher_init(ANDMatcher *self, VArray *children, Similarity *sim) {
    uint32_t i;

    // Init.
    PolyMatcher_init((PolyMatcher*)self, children, sim);
    self->first_time       = true;

    // Assign.
    self->more             = self->num_kids ? true : false;
    self->kids             = (Matcher**)MALLOCATE(self->num_kids * sizeof(Matcher*));
    for (i = 0; i < self->num_kids; i++) {
        Matcher *child = (Matcher*)VA_Fetch(children, i);
        self->kids[i] = child;
        if (!Matcher_Next(child)) { self->more = false; }
    }

    // Derive.
    self->matching_kids = self->num_kids;

    return self;
}

void
ANDMatcher_destroy(ANDMatcher *self) {
    FREEMEM(self->kids);
    SUPER_DESTROY(self, ANDMATCHER);
}

int32_t
ANDMatcher_next(ANDMatcher *self) {
    if (self->first_time) {
        return ANDMatcher_Advance(self, 1);
    }
    if (self->more) {
        const int32_t target = Matcher_Get_Doc_ID(self->kids[0]) + 1;
        return ANDMatcher_Advance(self, target);
    }
    else {
        return 0;
    }
}

int32_t
ANDMatcher_advance(ANDMatcher *self, int32_t target) {
    Matcher **const kids     = self->kids;
    const uint32_t  num_kids = self->num_kids;
    int32_t         highest  = 0;

    if (!self->more) { return 0; }

    // First step: Advance first child and use its doc as a starting point.
    if (self->first_time) {
        self->first_time = false;
    }
    else {
        highest = Matcher_Advance(kids[0], target);
        if (!highest) {
            self->more = false;
            return 0;
        }
    }

    // Second step: reconcile.
    while (1) {
        uint32_t i;
        bool_t agreement = true;

        // Scoot all Matchers up.
        for (i = 0; i < num_kids; i++) {
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
                    self->more = false;
                    return 0;
                }
            }
        }

        // If Matchers don't agree, send back through the loop.
        for (i = 0; i < num_kids; i++) {
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
ANDMatcher_get_doc_id(ANDMatcher *self) {
    return Matcher_Get_Doc_ID(self->kids[0]);
}

float
ANDMatcher_score(ANDMatcher *self) {
    uint32_t i;
    Matcher **const kids = self->kids;
    float score = 0.0f;

    for (i = 0; i < self->num_kids; i++) {
        score += Matcher_Score(kids[i]);
    }

    score *= self->coord_factors[self->matching_kids];

    return score;
}

