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
    HitDoc *self = (HitDoc*)Class_Make_Obj(HITDOC);
    return HitDoc_init(self, fields, doc_id, score);
}

HitDoc*
HitDoc_init(HitDoc *self, void *fields, int32_t doc_id, float score) {
    Doc_init((Doc*)self, fields, doc_id);
    HitDocIVARS *const ivars = HitDoc_IVARS(self);
    ivars->score = score;
    return self;
}

void
HitDoc_Set_Score_IMP(HitDoc *self, float score) {
    HitDoc_IVARS(self)->score = score;
}

float
HitDoc_Get_Score_IMP(HitDoc *self) {
    return HitDoc_IVARS(self)->score;
}

void
HitDoc_Serialize_IMP(HitDoc *self, OutStream *outstream) {
    HitDocIVARS *const ivars = HitDoc_IVARS(self);
    HitDoc_Serialize_t super_serialize
        = SUPER_METHOD_PTR(HITDOC, LUCY_HitDoc_Serialize);
    super_serialize(self, outstream);
    OutStream_Write_F32(outstream, ivars->score);
}

HitDoc*
HitDoc_Deserialize_IMP(HitDoc *self, InStream *instream) {
    HitDoc_Deserialize_t super_deserialize
        = SUPER_METHOD_PTR(HITDOC, LUCY_HitDoc_Deserialize);
    self = super_deserialize(self, instream);
    HitDocIVARS *const ivars = HitDoc_IVARS(self);
    ivars->score = InStream_Read_F32(instream);
    return self;
}

Hash*
HitDoc_Dump_IMP(HitDoc *self) {
    HitDocIVARS *const ivars = HitDoc_IVARS(self);
    HitDoc_Dump_t super_dump
        = SUPER_METHOD_PTR(HITDOC, LUCY_HitDoc_Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Utf8(dump, "score", 5, (Obj*)Str_newf("%f64", ivars->score));
    return dump;
}

HitDoc*
HitDoc_Load_IMP(HitDoc *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    HitDoc_Load_t super_load
        = SUPER_METHOD_PTR(HITDOC, LUCY_HitDoc_Load);
    HitDoc *loaded = super_load(self, dump);
    HitDocIVARS *const loaded_ivars = HitDoc_IVARS(loaded);
    Obj *score = CERTIFY(Hash_Fetch_Utf8(source, "score", 5), OBJ);
    loaded_ivars->score = (float)Obj_To_F64(score);
    return loaded;
}

bool
HitDoc_Equals_IMP(HitDoc *self, Obj *other) {
    if ((HitDoc*)other == self)           { return true;  }
    if (!Obj_Is_A(other, HITDOC))         { return false; }
    HitDoc_Equals_t super_equals
        = (HitDoc_Equals_t)SUPER_METHOD_PTR(HITDOC, LUCY_HitDoc_Equals);
    if (!super_equals(self, other))       { return false; }
    HitDocIVARS *const ivars = HitDoc_IVARS(self);
    HitDocIVARS *const ovars = HitDoc_IVARS((HitDoc*)other);
    if (ivars->score != ovars->score)     { return false; }
    return true;
}


