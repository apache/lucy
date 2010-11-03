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

#define C_KINO_FILTERSCORER
#include "KinoSearch/Util/ToolSet.h"

#include "LucyX/Search/FilterScorer.h"

FilterScorer*
FilterScorer_new(BitVector *bits, int32_t doc_max)
{
    FilterScorer *self = (FilterScorer*)VTable_Make_Obj(FILTERSCORER);
    return FilterScorer_init(self, bits, doc_max);
}

FilterScorer*
FilterScorer_init(FilterScorer *self, BitVector *bits, int32_t doc_max)
{
    Matcher_init((Matcher*)self);

    // Init. 
    self->doc_id       = 0;

    // Assign. 
    self->bits         = (BitVector*)INCREF(bits);
    self->doc_max      = doc_max;

    return self;
}   

void
FilterScorer_destroy(FilterScorer *self) 
{
    DECREF(self->bits);
    SUPER_DESTROY(self, FILTERSCORER);
}

int32_t
FilterScorer_next(FilterScorer* self) 
{
    do {
        if (++self->doc_id > self->doc_max) {
            self->doc_id--;
            return 0;
        }
    } while ( !BitVec_Get(self->bits, self->doc_id) );
    return self->doc_id;
}

int32_t
FilterScorer_skip_to(FilterScorer* self, int32_t target) 
{
    self->doc_id = target - 1;
    return FilterScorer_next(self);
}

float
FilterScorer_score(FilterScorer* self) 
{
    UNUSED_VAR(self);
    return 0.0f;
}

int32_t 
FilterScorer_get_doc_id(FilterScorer* self) 
{
    return self->doc_id;
}


