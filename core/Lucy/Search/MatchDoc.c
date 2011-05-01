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

#define C_LUCY_MATCHDOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

MatchDoc*
MatchDoc_new(int32_t doc_id, float score, VArray *values) {
    MatchDoc *self = (MatchDoc*)VTable_Make_Obj(MATCHDOC);
    return MatchDoc_init(self, doc_id, score, values);
}

MatchDoc*
MatchDoc_init(MatchDoc *self, int32_t doc_id, float score, VArray *values) {
    self->doc_id      = doc_id;
    self->score       = score;
    self->values      = (VArray*)INCREF(values);
    return self;
}

void
MatchDoc_destroy(MatchDoc *self) {
    DECREF(self->values);
    SUPER_DESTROY(self, MATCHDOC);
}

void
MatchDoc_serialize(MatchDoc *self, OutStream *outstream) {
    OutStream_Write_C32(outstream, self->doc_id);
    OutStream_Write_F32(outstream, self->score);
    OutStream_Write_U8(outstream, self->values ? 1 : 0);
    if (self->values) { VA_Serialize(self->values, outstream); }
}

MatchDoc*
MatchDoc_deserialize(MatchDoc *self, InStream *instream) {
    self = self ? self : (MatchDoc*)VTable_Make_Obj(MATCHDOC);
    self->doc_id = InStream_Read_C32(instream);
    self->score  = InStream_Read_F32(instream);
    if (InStream_Read_U8(instream)) {
        self->values = VA_deserialize(NULL, instream);
    }
    return self;
}

int32_t
MatchDoc_get_doc_id(MatchDoc *self) {
    return self->doc_id;
}

float
MatchDoc_get_score(MatchDoc *self) {
    return self->score;
}

VArray*
MatchDoc_get_values(MatchDoc *self) {
    return self->values;
}

void
MatchDoc_set_doc_id(MatchDoc *self, int32_t doc_id) {
    self->doc_id = doc_id;
}

void
MatchDoc_set_score(MatchDoc *self, float score) {
    self->score = score;
}

void
MatchDoc_set_values(MatchDoc *self, VArray *values) {
    DECREF(self->values);
    self->values = (VArray*)INCREF(values);
}


