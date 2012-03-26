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

#define C_LUCY_LEXINDEX
#define C_LUCY_TERMINFO
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/LexIndex.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Index/TermStepper.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"

// Read the data we've arrived at after a seek operation.
static void
S_read_entry(LexIndex *self);

LexIndex*
LexIndex_new(Schema *schema, Folder *folder, Segment *segment,
             const CharBuf *field) {
    LexIndex *self = (LexIndex*)VTable_Make_Obj(LEXINDEX);
    return LexIndex_init(self, schema, folder, segment, field);
}

LexIndex*
LexIndex_init(LexIndex *self, Schema *schema, Folder *folder,
              Segment *segment, const CharBuf *field) {
    int32_t  field_num = Seg_Field_Num(segment, field);
    CharBuf *seg_name  = Seg_Get_Name(segment);
    CharBuf *ixix_file = CB_newf("%o/lexicon-%i32.ixix", seg_name, field_num);
    CharBuf *ix_file   = CB_newf("%o/lexicon-%i32.ix", seg_name, field_num);
    Architecture *arch = Schema_Get_Architecture(schema);

    // Init.
    Lex_init((Lexicon*)self, field);
    self->tinfo        = TInfo_new(0);
    self->tick         = 0;

    // Derive
    self->field_type = Schema_Fetch_Type(schema, field);
    if (!self->field_type) {
        CharBuf *mess = MAKE_MESS("Unknown field: '%o'", field);
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        Err_throw_mess(ERR, mess);
    }
    INCREF(self->field_type);
    self->term_stepper = FType_Make_Term_Stepper(self->field_type);
    self->ixix_in = Folder_Open_In(folder, ixix_file);
    if (!self->ixix_in) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        RETHROW(error);
    }
    self->ix_in = Folder_Open_In(folder, ix_file);
    if (!self->ix_in) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        RETHROW(error);
    }
    self->index_interval = Arch_Index_Interval(arch);
    self->skip_interval  = Arch_Skip_Interval(arch);
    self->size    = (int32_t)(InStream_Length(self->ixix_in) / sizeof(int64_t));
    self->offsets = (int64_t*)InStream_Buf(self->ixix_in,
                                           (size_t)InStream_Length(self->ixix_in));

    DECREF(ixix_file);
    DECREF(ix_file);

    return self;
}

void
LexIndex_destroy(LexIndex *self) {
    DECREF(self->field_type);
    DECREF(self->ixix_in);
    DECREF(self->ix_in);
    DECREF(self->term_stepper);
    DECREF(self->tinfo);
    SUPER_DESTROY(self, LEXINDEX);
}

int32_t
LexIndex_get_term_num(LexIndex *self) {
    return (self->index_interval * self->tick) - 1;
}

Obj*
LexIndex_get_term(LexIndex *self) {
    return TermStepper_Get_Value(self->term_stepper);
}

TermInfo*
LexIndex_get_term_info(LexIndex *self) {
    return self->tinfo;
}

static void
S_read_entry(LexIndex *self) {
    InStream *ix_in  = self->ix_in;
    TermInfo *tinfo  = self->tinfo;
    int64_t offset = (int64_t)NumUtil_decode_bigend_u64(self->offsets + self->tick);
    InStream_Seek(ix_in, offset);
    TermStepper_Read_Key_Frame(self->term_stepper, ix_in);
    tinfo->doc_freq     = InStream_Read_C32(ix_in);
    tinfo->post_filepos = InStream_Read_C64(ix_in);
    tinfo->skip_filepos = tinfo->doc_freq >= self->skip_interval
                          ? InStream_Read_C64(ix_in)
                          : 0;
    tinfo->lex_filepos  = InStream_Read_C64(ix_in);
}

void
LexIndex_seek(LexIndex *self, Obj *target) {
    TermStepper *term_stepper = self->term_stepper;
    InStream    *ix_in        = self->ix_in;
    FieldType   *type         = self->field_type;
    int32_t      lo           = 0;
    int32_t      hi           = self->size - 1;
    int32_t      result       = -100;

    if (target == NULL || self->size == 0) {
        self->tick = 0;
        return;
    }
    else {
        if (!Obj_Is_A(target, CHARBUF)) {
            THROW(ERR, "Target is a %o, and not comparable to a %o",
                  Obj_Get_Class_Name(target), VTable_Get_Name(CHARBUF));
        }
        /* TODO:
        Obj *first_obj = VA_Fetch(terms, 0);
        if (!Obj_Is_A(target, Obj_Get_VTable(first_obj))) {
            THROW(ERR, "Target is a %o, and not comparable to a %o",
                Obj_Get_Class_Name(target), Obj_Get_Class_Name(first_obj));
        }
        */
    }

    // Divide and conquer.
    while (hi >= lo) {
        const int32_t mid = lo + ((hi - lo) / 2);
        const int64_t offset
            = (int64_t)NumUtil_decode_bigend_u64(self->offsets + mid);
        InStream_Seek(ix_in, offset);
        TermStepper_Read_Key_Frame(term_stepper, ix_in);

        // Compare values.  There is no need for a NULL-check because the term
        // number is alway between 0 and self->size - 1.
        Obj *value = TermStepper_Get_Value(term_stepper);
        int32_t comparison = FType_Compare_Values(type, target, value);

        if (comparison < 0) {
            hi = mid - 1;
        }
        else if (comparison > 0) {
            lo = mid + 1;
        }
        else {
            result = mid;
            break;
        }
    }

    // Record the index of the entry we've seeked to, then read entry.
    self->tick = hi == -1 // indicating that target lt first entry
                 ? 0
                 : result == -100 // if result is still -100, it wasn't set
                 ? hi
                 : result;
    S_read_entry(self);
}


