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

#define C_LUCY_HITDOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Document/HitDoc.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

HitDoc*
HitDoc_new(void *fields, int32_t doc_id, float score) {
    HitDoc *self = (HitDoc*)VTable_Make_Obj(HITDOC);
    return HitDoc_init(self, fields, doc_id, score);
}

HitDoc*
HitDoc_init(HitDoc *self, void *fields, int32_t doc_id, float score) {
    Doc_init((Doc*)self, fields, doc_id);
    self->score = score;
    return self;
}

void
HitDoc_set_score(HitDoc *self, float score) {
    self->score = score;
}

float
HitDoc_get_score(HitDoc *self) {
    return self->score;
}

void
HitDoc_serialize(HitDoc *self, OutStream *outstream) {
    Doc_serialize((Doc*)self, outstream);
    OutStream_Write_F32(outstream, self->score);
}

HitDoc*
HitDoc_deserialize(HitDoc *self, InStream *instream) {
    self = self ? self : (HitDoc*)VTable_Make_Obj(HITDOC);
    Doc_deserialize((Doc*)self, instream);
    self->score = InStream_Read_F32(instream);
    return self;
}

Hash*
HitDoc_dump(HitDoc *self) {
    HitDoc_dump_t super_dump
        = (HitDoc_dump_t)SUPER_METHOD(HITDOC, HitDoc, Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Str(dump, "score", 5, (Obj*)CB_newf("%f64", self->score));
    return dump;
}

HitDoc*
HitDoc_load(HitDoc *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    HitDoc_load_t super_load
        = (HitDoc_load_t)SUPER_METHOD(HITDOC, HitDoc, Load);
    HitDoc *loaded = super_load(self, dump);
    Obj *score = CERTIFY(Hash_Fetch_Str(source, "score", 5), OBJ);
    loaded->score = (float)Obj_To_F64(score);
    return loaded;
}

bool_t
HitDoc_equals(HitDoc *self, Obj *other) {
    HitDoc *twin = (HitDoc*)other;
    if (twin == self)                     { return true;  }
    if (!Obj_Is_A(other, HITDOC))         { return false; }
    if (!Doc_equals((Doc*)self, other))   { return false; }
    if (self->score != twin->score)       { return false; }
    return true;
}


