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
    BitVecMatcher *self = (BitVecMatcher*)VTable_Make_Obj(BITVECMATCHER);
    return BitVecMatcher_init(self, bit_vector);
}

BitVecMatcher*
BitVecMatcher_init(BitVecMatcher *self, BitVector *bit_vector) {
    Matcher_init((Matcher*)self);
    self->bit_vec = (BitVector*)INCREF(bit_vector);
    self->doc_id = 0;
    return self;
}

void
BitVecMatcher_destroy(BitVecMatcher *self) {
    DECREF(self->bit_vec);
    SUPER_DESTROY(self, BITVECMATCHER);
}

int32_t
BitVecMatcher_next(BitVecMatcher *self) {
    self->doc_id = BitVec_Next_Hit(self->bit_vec, self->doc_id + 1);
    return self->doc_id == -1 ? 0 : self->doc_id;
}

int32_t
BitVecMatcher_advance(BitVecMatcher *self, int32_t target) {
    self->doc_id = BitVec_Next_Hit(self->bit_vec, target);
    return self->doc_id == -1 ? 0 : self->doc_id;
}

int32_t
BitVecMatcher_get_doc_id(BitVecMatcher *self) {
    return self->doc_id;
}


