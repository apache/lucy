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

#define C_LUCY_SERIESMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/SeriesMatcher.h"

SeriesMatcher*
SeriesMatcher_new(VArray *matchers, I32Array *offsets) {
    SeriesMatcher *self = (SeriesMatcher*)Class_Make_Obj(SERIESMATCHER);
    return SeriesMatcher_init(self, matchers, offsets);
}

SeriesMatcher*
SeriesMatcher_init(SeriesMatcher *self, VArray *matchers, I32Array *offsets) {
    Matcher_init((Matcher*)self);
    SeriesMatcherIVARS *const ivars = SeriesMatcher_IVARS(self);

    // Init.
    ivars->current_matcher = NULL;
    ivars->current_offset  = 0;
    ivars->next_offset     = 0;
    ivars->doc_id          = 0;
    ivars->tick            = 0;

    // Assign.
    ivars->matchers        = (VArray*)INCREF(matchers);
    ivars->offsets         = (I32Array*)INCREF(offsets);

    // Derive.
    ivars->num_matchers    = (int32_t)I32Arr_Get_Size(offsets);

    return self;
}

void
SeriesMatcher_Destroy_IMP(SeriesMatcher *self) {
    SeriesMatcherIVARS *const ivars = SeriesMatcher_IVARS(self);
    DECREF(ivars->matchers);
    DECREF(ivars->offsets);
    SUPER_DESTROY(self, SERIESMATCHER);
}

int32_t
SeriesMatcher_Next_IMP(SeriesMatcher *self) {
    SeriesMatcherIVARS *const ivars = SeriesMatcher_IVARS(self);
    return SeriesMatcher_Advance_IMP(self, ivars->doc_id + 1);
}

int32_t
SeriesMatcher_Advance_IMP(SeriesMatcher *self, int32_t target) {
    SeriesMatcherIVARS *const ivars = SeriesMatcher_IVARS(self);
    if (target >= ivars->next_offset) {
        // Proceed to next matcher or bail.
        if (ivars->tick < ivars->num_matchers) {
            while (1) {
                uint32_t next_offset
                    = ivars->tick + 1 == ivars->num_matchers
                      ? INT32_MAX
                      : I32Arr_Get(ivars->offsets, ivars->tick + 1);
                ivars->current_matcher = (Matcher*)VA_Fetch(ivars->matchers,
                                                           ivars->tick);
                ivars->current_offset = ivars->next_offset;
                ivars->next_offset = next_offset;
                ivars->doc_id = next_offset - 1;
                ivars->tick++;
                if (ivars->current_matcher != NULL
                    || ivars->tick >= ivars->num_matchers
                   ) {
                    break;
                }
            }
            return SeriesMatcher_Advance(self, target); // Recurse.
        }
        else {
            // We're done.
            ivars->doc_id = 0;
            return 0;
        }
    }
    else {
        int32_t target_minus_offset = target - ivars->current_offset;
        int32_t found
            = Matcher_Advance(ivars->current_matcher, target_minus_offset);
        if (found) {
            ivars->doc_id = found + ivars->current_offset;
            return ivars->doc_id;
        }
        else {
            // Recurse.
            return SeriesMatcher_Advance(self, ivars->next_offset);
        }
    }
}

int32_t
SeriesMatcher_Get_Doc_ID_IMP(SeriesMatcher *self) {
    return SeriesMatcher_IVARS(self)->doc_id;
}


