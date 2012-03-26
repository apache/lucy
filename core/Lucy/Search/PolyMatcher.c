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

#define C_LUCY_POLYMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/PolyMatcher.h"
#include "Lucy/Index/Similarity.h"

PolyMatcher*
PolyMatcher_new(VArray *children, Similarity *sim) {
    PolyMatcher *self = (PolyMatcher*)VTable_Make_Obj(POLYMATCHER);
    return PolyMatcher_init(self, children, sim);
}

PolyMatcher*
PolyMatcher_init(PolyMatcher *self, VArray *children, Similarity *similarity) {
    uint32_t i;

    Matcher_init((Matcher*)self);
    self->num_kids = VA_Get_Size(children);
    self->sim      = (Similarity*)INCREF(similarity);
    self->children = (VArray*)INCREF(children);
    self->coord_factors = (float*)MALLOCATE((self->num_kids + 1) * sizeof(float));
    for (i = 0; i <= self->num_kids; i++) {
        self->coord_factors[i] = similarity
                                 ? Sim_Coord(similarity, i, self->num_kids)
                                 : 1.0f;
    }
    return self;
}

void
PolyMatcher_destroy(PolyMatcher *self) {
    DECREF(self->children);
    DECREF(self->sim);
    FREEMEM(self->coord_factors);
    SUPER_DESTROY(self, POLYMATCHER);
}


