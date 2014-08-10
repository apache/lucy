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
    MockMatcher *self = (MockMatcher*)Class_Make_Obj(MOCKMATCHER);
    return MockMatcher_init(self, doc_ids, scores);
}

MockMatcher*
MockMatcher_init(MockMatcher *self, I32Array *doc_ids, ByteBuf *scores) {
    Matcher_init((Matcher*)self);
    MockMatcherIVARS *const ivars = MockMatcher_IVARS(self);
    ivars->tick    = -1;
    ivars->size    = I32Arr_Get_Size(doc_ids);
    ivars->doc_ids = (I32Array*)INCREF(doc_ids);
    ivars->scores  = (ByteBuf*)INCREF(scores);
    return self;
}

void
MockMatcher_Destroy_IMP(MockMatcher *self) {
    MockMatcherIVARS *const ivars = MockMatcher_IVARS(self);
    DECREF(ivars->doc_ids);
    DECREF(ivars->scores);
    SUPER_DESTROY(self, MOCKMATCHER);
}

int32_t
MockMatcher_Next_IMP(MockMatcher* self) {
    MockMatcherIVARS *const ivars = MockMatcher_IVARS(self);
    if (++ivars->tick >= (int32_t)ivars->size) {
        ivars->tick--;
        return 0;
    }
    return I32Arr_Get(ivars->doc_ids, ivars->tick);
}

float
MockMatcher_Score_IMP(MockMatcher* self) {
    MockMatcherIVARS *const ivars = MockMatcher_IVARS(self);
    if (!ivars->scores) {
        THROW(ERR, "Can't call Score() unless scores supplied");
    }
    const float *raw_scores = (const float*)BB_Get_Buf(ivars->scores);
    return raw_scores[ivars->tick];
}

int32_t
MockMatcher_Get_Doc_ID_IMP(MockMatcher* self) {
    MockMatcherIVARS *const ivars = MockMatcher_IVARS(self);
    return I32Arr_Get(ivars->doc_ids, ivars->tick);
}


