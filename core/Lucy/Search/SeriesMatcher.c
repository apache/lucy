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
    SeriesMatcher *self = (SeriesMatcher*)VTable_Make_Obj(SERIESMATCHER);
    return SeriesMatcher_init(self, matchers, offsets);
}

SeriesMatcher*
SeriesMatcher_init(SeriesMatcher *self, VArray *matchers, I32Array *offsets) {
    Matcher_init((Matcher*)self);

    // Init.
    self->current_matcher = NULL;
    self->current_offset  = 0;
    self->next_offset     = 0;
    self->doc_id          = 0;
    self->tick            = 0;

    // Assign.
    self->matchers        = (VArray*)INCREF(matchers);
    self->offsets         = (I32Array*)INCREF(offsets);

    // Derive.
    self->num_matchers    = (int32_t)I32Arr_Get_Size(offsets);

    return self;
}

void
SeriesMatcher_destroy(SeriesMatcher *self) {
    DECREF(self->matchers);
    DECREF(self->offsets);
    SUPER_DESTROY(self, SERIESMATCHER);
}

int32_t
SeriesMatcher_next(SeriesMatcher *self) {
    return SeriesMatcher_advance(self, self->doc_id + 1);
}

int32_t
SeriesMatcher_advance(SeriesMatcher *self, int32_t target) {
    if (target >= self->next_offset) {
        // Proceed to next matcher or bail.
        if (self->tick < self->num_matchers) {
            while (1) {
                uint32_t next_offset
                    = self->tick + 1 == self->num_matchers
                      ? I32_MAX
                      : I32Arr_Get(self->offsets, self->tick + 1);
                self->current_matcher = (Matcher*)VA_Fetch(self->matchers,
                                                           self->tick);
                self->current_offset = self->next_offset;
                self->next_offset = next_offset;
                self->doc_id = next_offset - 1;
                self->tick++;
                if (self->current_matcher != NULL
                    || self->tick >= self->num_matchers
                   ) {
                    break;
                }
            }
            return SeriesMatcher_advance(self, target); // Recurse.
        }
        else {
            // We're done.
            self->doc_id = 0;
            return 0;
        }
    }
    else {
        int32_t target_minus_offset = target - self->current_offset;
        int32_t found
            = Matcher_Advance(self->current_matcher, target_minus_offset);
        if (found) {
            self->doc_id = found + self->current_offset;
            return self->doc_id;
        }
        else {
            // Recurse.
            return SeriesMatcher_advance(self, self->next_offset);
        }
    }
}

int32_t
SeriesMatcher_get_doc_id(SeriesMatcher *self) {
    return self->doc_id;
}


