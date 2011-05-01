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

#define C_LUCY_MOCKMATCHER
#include "Lucy/Util/ToolSet.h"

#include "LucyX/Search/MockMatcher.h"

MockMatcher*
MockMatcher_new(I32Array *doc_ids, ByteBuf *scores) {
    MockMatcher *self = (MockMatcher*)VTable_Make_Obj(MOCKMATCHER);
    return MockMatcher_init(self, doc_ids, scores);
}

MockMatcher*
MockMatcher_init(MockMatcher *self, I32Array *doc_ids, ByteBuf *scores) {
    Matcher_init((Matcher*)self);
    self->tick    = -1;
    self->size    = I32Arr_Get_Size(doc_ids);
    self->doc_ids = (I32Array*)INCREF(doc_ids);
    self->scores  = (ByteBuf*)INCREF(scores);
    return self;
}

void
MockMatcher_destroy(MockMatcher *self) {
    DECREF(self->doc_ids);
    DECREF(self->scores);
    SUPER_DESTROY(self, MOCKMATCHER);
}

int32_t
MockMatcher_next(MockMatcher* self) {
    if (++self->tick >= (int32_t)self->size) {
        self->tick--;
        return 0;
    }
    return I32Arr_Get(self->doc_ids, self->tick);
}

float
MockMatcher_score(MockMatcher* self) {
    if (!self->scores) {
        THROW(ERR, "Can't call Score() unless scores supplied");
    }
    float *raw_scores = (float*)BB_Get_Buf(self->scores);
    return raw_scores[self->tick];
}

int32_t
MockMatcher_get_doc_id(MockMatcher* self) {
    return I32Arr_Get(self->doc_ids, self->tick);
}


