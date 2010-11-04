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

#define C_LUCY_MATCHALLSCORER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/MatchAllScorer.h"

MatchAllScorer*
MatchAllScorer_new(float score, int32_t doc_max)
{
    MatchAllScorer *self 
        = (MatchAllScorer*)VTable_Make_Obj(MATCHALLSCORER);
    return MatchAllScorer_init(self, score, doc_max);
}

MatchAllScorer*
MatchAllScorer_init(MatchAllScorer *self, float score, int32_t doc_max)
{
    Matcher_init((Matcher*)self);
    self->doc_id        = 0;
    self->score         = score;
    self->doc_max       = doc_max;
    return self;
}

int32_t
MatchAllScorer_next(MatchAllScorer* self) 
{
    if (++self->doc_id <= self->doc_max) {
        return self->doc_id;
    }
    else {
        self->doc_id--;
        return 0;
    }
}

int32_t
MatchAllScorer_advance(MatchAllScorer* self, int32_t target) 
{
    self->doc_id = target - 1;
    return MatchAllScorer_next(self);
}

float
MatchAllScorer_score(MatchAllScorer* self) 
{
    return self->score;
}

int32_t 
MatchAllScorer_get_doc_id(MatchAllScorer* self) 
{
    return self->doc_id;
}


