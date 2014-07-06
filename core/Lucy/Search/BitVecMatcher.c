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

#define C_LUCY_BITVECMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/BitVecMatcher.h"

BitVecMatcher*
BitVecMatcher_new(BitVector *bit_vector) {
    BitVecMatcher *self = (BitVecMatcher*)Class_Make_Obj(BITVECMATCHER);
    return BitVecMatcher_init(self, bit_vector);
}

BitVecMatcher*
BitVecMatcher_init(BitVecMatcher *self, BitVector *bit_vector) {
    BitVecMatcherIVARS *const ivars = BitVecMatcher_IVARS(self);
    Matcher_init((Matcher*)self);
    ivars->bit_vec = (BitVector*)INCREF(bit_vector);
    ivars->doc_id = 0;
    return self;
}

void
BitVecMatcher_Destroy_IMP(BitVecMatcher *self) {
    BitVecMatcherIVARS *const ivars = BitVecMatcher_IVARS(self);
    DECREF(ivars->bit_vec);
    SUPER_DESTROY(self, BITVECMATCHER);
}

int32_t
BitVecMatcher_Next_IMP(BitVecMatcher *self) {
    BitVecMatcherIVARS *const ivars = BitVecMatcher_IVARS(self);
    ivars->doc_id = BitVec_Next_Hit(ivars->bit_vec, ivars->doc_id + 1);
    return ivars->doc_id == -1 ? 0 : ivars->doc_id;
}

int32_t
BitVecMatcher_Advance_IMP(BitVecMatcher *self, int32_t target) {
    BitVecMatcherIVARS *const ivars = BitVecMatcher_IVARS(self);
    ivars->doc_id = BitVec_Next_Hit(ivars->bit_vec, target);
    return ivars->doc_id == -1 ? 0 : ivars->doc_id;
}

int32_t
BitVecMatcher_Get_Doc_ID_IMP(BitVecMatcher *self) {
    return BitVecMatcher_IVARS(self)->doc_id;
}


