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

TermVector*
TV_new(const CharBuf *field, const CharBuf *text, I32Array *positions,
       I32Array *start_offsets, I32Array *end_offsets) {
    TermVector *self = (TermVector*)VTable_Make_Obj(TERMVECTOR);
    return TV_init(self, field, text, positions, start_offsets, end_offsets);
}

TermVector*
TV_init(TermVector *self, const CharBuf *field, const CharBuf *text,
        I32Array *positions, I32Array *start_offsets, I32Array *end_offsets) {
    // Assign.
    self->field          = CB_Clone(field);
    self->text           = CB_Clone(text);
    self->num_pos        = I32Arr_Get_Size(positions);
    self->positions      = (I32Array*)INCREF(positions);
    self->start_offsets  = (I32Array*)INCREF(start_offsets);
    self->end_offsets    = (I32Array*)INCREF(end_offsets);

    if (I32Arr_Get_Size(start_offsets) != self->num_pos
        || I32Arr_Get_Size(end_offsets) != self->num_pos
       ) {
        THROW(ERR, "Unbalanced arrays: %u32 %u32 %u32", self->num_pos,
              I32Arr_Get_Size(start_offsets), I32Arr_Get_Size(end_offsets));
    }

    return self;
}

void
TV_destroy(TermVector *self) {
    DECREF(self->field);
    DECREF(self->text);
    DECREF(self->positions);
    DECREF(self->start_offsets);
    DECREF(self->end_offsets);
    SUPER_DESTROY(self, TERMVECTOR);
}

I32Array*
TV_get_positions(TermVector *self) {
    return self->positions;
}

I32Array*
TV_get_start_offsets(TermVector *self) {
    return self->start_offsets;
}

I32Array*
TV_get_end_offsets(TermVector *self) {
    return self->end_offsets;
}

void
TV_serialize(TermVector *self, OutStream *target) {
    uint32_t i;
    int32_t *posits = self->positions->ints;
    int32_t *starts = self->start_offsets->ints;
    int32_t *ends   = self->start_offsets->ints;

    CB_Serialize(self->field, target);
    CB_Serialize(self->text, target);
    OutStream_Write_C32(target, self->num_pos);

    for (i = 0; i < self->num_pos; i++) {
        OutStream_Write_C32(target, posits[i]);
        OutStream_Write_C32(target, starts[i]);
        OutStream_Write_C32(target, ends[i]);
    }
}

TermVector*
TV_deserialize(TermVector *self, InStream *instream) {
    uint32_t  i;
    CharBuf  *field  = (CharBuf*)CB_deserialize(NULL, instream);
    CharBuf  *text   = (CharBuf*)CB_deserialize(NULL, instream);
    uint32_t num_pos = InStream_Read_C32(instream);
    int32_t  *posits, *starts, *ends;
    I32Array *positions, *start_offsets, *end_offsets;

    // Read positional data.
    posits = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    starts = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    ends   = (int32_t*)MALLOCATE(num_pos * sizeof(int32_t));
    for (i = 0; i < num_pos; i++) {
        posits[i] = InStream_Read_C32(instream);
        starts[i] = InStream_Read_C32(instream);
        ends[i]   = InStream_Read_C32(instream);
    }
    positions     = I32Arr_new_steal(posits, num_pos);
    start_offsets = I32Arr_new_steal(starts, num_pos);
    end_offsets   = I32Arr_new_steal(ends, num_pos);

    self = self ? self : (TermVector*)VTable_Make_Obj(TERMVECTOR);
    self = TV_init(self, field, text, positions, start_offsets, end_offsets);

    DECREF(positions);
    DECREF(start_offsets);
    DECREF(end_offsets);
    DECREF(text);
    DECREF(field);

    return self;
}

bool_t
TV_equals(TermVector *self, Obj *other) {
    TermVector *const twin = (TermVector*)other;
    uint32_t i;
    int32_t *const posits       = self->positions->ints;
    int32_t *const starts       = self->start_offsets->ints;
    int32_t *const ends         = self->start_offsets->ints;
    int32_t *const other_posits = twin->positions->ints;
    int32_t *const other_starts = twin->start_offsets->ints;
    int32_t *const other_ends   = twin->start_offsets->ints;

    if (twin == self) { return true; }

    if (!CB_Equals(self->field, (Obj*)twin->field)) { return false; }
    if (!CB_Equals(self->text, (Obj*)twin->text))   { return false; }
    if (self->num_pos != twin->num_pos)             { return false; }

    for (i = 0; i < self->num_pos; i++) {
        if (posits[i] != other_posits[i]) { return false; }
        if (starts[i] != other_starts[i]) { return false; }
        if (ends[i]   != other_ends[i])   { return false; }
    }

    return true;
}


