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

#define C_KINO_NOTSCORER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/NOTScorer.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"

NOTScorer*
NOTScorer_new(Matcher *negated_matcher, int32_t doc_max) 
{
    NOTScorer *self = (NOTScorer*)VTable_Make_Obj(NOTSCORER);
    return NOTScorer_init(self, negated_matcher, doc_max);
}

NOTScorer*
NOTScorer_init(NOTScorer *self, Matcher *negated_matcher, int32_t doc_max)
{
    VArray *children = VA_new(1);
    VA_Push(children, INCREF(negated_matcher));
    PolyMatcher_init((PolyMatcher*)self, children, NULL);

    // Init. 
    self->doc_id           = 0;
    self->next_negation    = 0;

    // Assign. 
    self->negated_matcher   = (Matcher*)INCREF(negated_matcher);
    self->doc_max          = doc_max;

    DECREF(children);

    return self;
}

void
NOTScorer_destroy(NOTScorer *self) 
{
    DECREF(self->negated_matcher);
    SUPER_DESTROY(self, NOTSCORER);
}

int32_t
NOTScorer_next(NOTScorer *self)
{
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
NOTScorer_advance(NOTScorer *self, int32_t target)
{
    self->doc_id = target - 1;
    return NOTScorer_next(self);
}

int32_t
NOTScorer_get_doc_id(NOTScorer *self)
{
    return self->doc_id;
}

float
NOTScorer_score(NOTScorer *self)
{
    UNUSED_VAR(self);
    return 0.0f;
}


