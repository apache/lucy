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
    PolyMatcher *self = (PolyMatcher*)Class_Make_Obj(POLYMATCHER);
    return PolyMatcher_init(self, children, sim);
}

PolyMatcher*
PolyMatcher_init(PolyMatcher *self, VArray *children, Similarity *similarity) {
    Matcher_init((Matcher*)self);
    PolyMatcherIVARS *const ivars = PolyMatcher_IVARS(self);
    ivars->num_kids = VA_Get_Size(children);
    ivars->sim      = (Similarity*)INCREF(similarity);
    ivars->children = (VArray*)INCREF(children);
    ivars->coord_factors = (float*)MALLOCATE((ivars->num_kids + 1) * sizeof(float));
    for (uint32_t i = 0; i <= ivars->num_kids; i++) {
        ivars->coord_factors[i] = similarity
                                 ? Sim_Coord(similarity, i, ivars->num_kids)
                                 : 1.0f;
    }
    return self;
}

void
PolyMatcher_Destroy_IMP(PolyMatcher *self) {
    PolyMatcherIVARS *const ivars = PolyMatcher_IVARS(self);
    DECREF(ivars->children);
    DECREF(ivars->sim);
    FREEMEM(ivars->coord_factors);
    SUPER_DESTROY(self, POLYMATCHER);
}


