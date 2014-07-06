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
#include "Lucy/Util/Freezer.h"

MatchDoc*
MatchDoc_new(int32_t doc_id, float score, VArray *values) {
    MatchDoc *self = (MatchDoc*)Class_Make_Obj(MATCHDOC);
    return MatchDoc_init(self, doc_id, score, values);
}

MatchDoc*
MatchDoc_init(MatchDoc *self, int32_t doc_id, float score, VArray *values) {
    MatchDocIVARS *const ivars = MatchDoc_IVARS(self);
    ivars->doc_id      = doc_id;
    ivars->score       = score;
    ivars->values      = (VArray*)INCREF(values);
    return self;
}

void
MatchDoc_Destroy_IMP(MatchDoc *self) {
    MatchDocIVARS *const ivars = MatchDoc_IVARS(self);
    DECREF(ivars->values);
    SUPER_DESTROY(self, MATCHDOC);
}

void
MatchDoc_Serialize_IMP(MatchDoc *self, OutStream *outstream) {
    MatchDocIVARS *const ivars = MatchDoc_IVARS(self);
    OutStream_Write_C32(outstream, ivars->doc_id);
    OutStream_Write_F32(outstream, ivars->score);
    OutStream_Write_U8(outstream, ivars->values ? 1 : 0);
    if (ivars->values) { Freezer_serialize_varray(ivars->values, outstream); }
}

MatchDoc*
MatchDoc_Deserialize_IMP(MatchDoc *self, InStream *instream) {
    MatchDocIVARS *const ivars = MatchDoc_IVARS(self);
    ivars->doc_id = InStream_Read_C32(instream);
    ivars->score  = InStream_Read_F32(instream);
    if (InStream_Read_U8(instream)) {
        ivars->values = Freezer_read_varray(instream);
    }
    return self;
}

int32_t
MatchDoc_Get_Doc_ID_IMP(MatchDoc *self) {
    return MatchDoc_IVARS(self)->doc_id;
}

float
MatchDoc_Get_Score_IMP(MatchDoc *self) {
    return MatchDoc_IVARS(self)->score;
}

VArray*
MatchDoc_Get_Values_IMP(MatchDoc *self) {
    return MatchDoc_IVARS(self)->values;
}

void
MatchDoc_Set_Doc_ID_IMP(MatchDoc *self, int32_t doc_id) {
    MatchDoc_IVARS(self)->doc_id = doc_id;
}

void
MatchDoc_Set_Score_IMP(MatchDoc *self, float score) {
    MatchDoc_IVARS(self)->score = score;
}

void
MatchDoc_Set_Values_IMP(MatchDoc *self, VArray *values) {
    MatchDocIVARS *const ivars = MatchDoc_IVARS(self);
    DECREF(ivars->values);
    ivars->values = (VArray*)INCREF(values);
}


