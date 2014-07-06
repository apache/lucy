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

#define C_LUCY_TERMVECTOR
#define C_LUCY_I32ARRAY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/TermVector.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

TermVector*
TV_new(String *field, String *text, I32Array *positions,
       I32Array *start_offsets, I32Array *end_offsets) {
    TermVector *self = (TermVector*)Class_Make_Obj(TERMVECTOR);
    return TV_init(self, field, text, positions, start_offsets, end_offsets);
}

TermVector*
TV_init(TermVector *self, String *field, String *text,
        I32Array *positions, I32Array *start_offsets, I32Array *end_offsets) {
    TermVectorIVARS *const ivars = TV_IVARS(self);

    // Assign.
    ivars->field          = Str_Clone(field);
    ivars->text           = Str_Clone(text);
    ivars->num_pos        = I32Arr_Get_Size(positions);
    ivars->positions      = (I32Array*)INCREF(positions);
    ivars->start_offsets  = (I32Array*)INCREF(start_offsets);
    ivars->end_offsets    = (I32Array*)INCREF(end_offsets);

    if (I32Arr_Get_Size(start_offsets) != ivars->num_pos
        || I32Arr_Get_Size(end_offsets) != ivars->num_pos
       ) {
        THROW(ERR, "Unbalanced arrays: %u32 %u32 %u32", ivars->num_pos,
              I32Arr_Get_Size(start_offsets), I32Arr_Get_Size(end_offsets));
    }

    return self;
}

void
TV_Destroy_IMP(TermVector *self) {
    TermVectorIVARS *const ivars = TV_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->text);
    DECREF(ivars->positions);
    DECREF(ivars->start_offsets);
    DECREF(ivars->end_offsets);
    SUPER_DESTROY(self, TERMVECTOR);
}

I32Array*
TV_Get_Positions_IMP(TermVector *self) {
    return TV_IVARS(self)->positions;
}

I32Array*
TV_Get_Start_Offsets_IMP(TermVector *self) {
    return TV_IVARS(self)->start_offsets;
}

I32Array*
TV_Get_End_Offsets_IMP(TermVector *self) {
    return TV_IVARS(self)->end_offsets;
}

void
TV_Serialize_IMP(TermVector *self, OutStream *target) {
    TermVectorIVARS *const ivars = TV_IVARS(self);
    int32_t *posits = I32Arr_IVARS(ivars->positions)->ints;
    int32_t *starts = I32Arr_IVARS(ivars->start_offsets)->ints;
    int32_t *ends   = I32Arr_IVARS(ivars->start_offsets)->ints;

    Freezer_serialize_string(ivars->field, target);
    Freezer_serialize_string(ivars->text, target);
    OutStream_Write_C32(target, ivars->num_pos);

    for (uint32_t i = 0; i < ivars->num_pos; i++) {
        OutStream_Write_C32(target, posits[i]);
        OutStream_Write_C32(target, starts[i]);
        OutStream_Write_C32(target, ends[i]);
    }
}

TermVector*
TV_Deserialize_IMP(TermVector *self, InStream *instream) {
    String *field = Freezer_read_string(instream);
    String *text  = Freezer_read_string(instream);
    uint32_t num_pos = InStream_Read_C32(instream);

    // Read positional data.
    int32_t *posits = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    int32_t *starts = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    int32_t *ends   = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    for (uint32_t i = 0; i < num_pos; i++) {
        posits[i] = InStream_Read_C32(instream);
        starts[i] = InStream_Read_C32(instream);
        ends[i]   = InStream_Read_C32(instream);
    }
    I32Array *positions     = I32Arr_new_steal(posits, num_pos);
    I32Array *start_offsets = I32Arr_new_steal(starts, num_pos);
    I32Array *end_offsets   = I32Arr_new_steal(ends, num_pos);

    TV_init(self, field, text, positions, start_offsets, end_offsets);

    DECREF(positions);
    DECREF(start_offsets);
    DECREF(end_offsets);
    DECREF(text);
    DECREF(field);

    return self;
}

bool
TV_Equals_IMP(TermVector *self, Obj *other) {
    if ((TermVector*)other == self) { return true; }
    TermVectorIVARS *const ivars = TV_IVARS(self);
    TermVectorIVARS *const ovars = TV_IVARS((TermVector*)other);
    if (!Str_Equals(ivars->field, (Obj*)ovars->field)) { return false; }
    if (!Str_Equals(ivars->text, (Obj*)ovars->text))   { return false; }
    if (ivars->num_pos != ovars->num_pos)              { return false; }

    int32_t *const posits       = I32Arr_IVARS(ivars->positions)->ints;
    int32_t *const starts       = I32Arr_IVARS(ivars->start_offsets)->ints;
    int32_t *const ends         = I32Arr_IVARS(ivars->start_offsets)->ints;
    int32_t *const other_posits = I32Arr_IVARS(ovars->positions)->ints;
    int32_t *const other_starts = I32Arr_IVARS(ovars->start_offsets)->ints;
    int32_t *const other_ends   = I32Arr_IVARS(ovars->start_offsets)->ints;
    for (uint32_t i = 0; i < ivars->num_pos; i++) {
        if (posits[i] != other_posits[i]) { return false; }
        if (starts[i] != other_starts[i]) { return false; }
        if (ends[i]   != other_ends[i])   { return false; }
    }

    return true;
}


