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
             String *field) {
    LexIndex *self = (LexIndex*)Class_Make_Obj(LEXINDEX);
    return LexIndex_init(self, schema, folder, segment, field);
}

LexIndex*
LexIndex_init(LexIndex *self, Schema *schema, Folder *folder,
              Segment *segment, String *field) {
    int32_t  field_num = Seg_Field_Num(segment, field);
    String  *seg_name  = Seg_Get_Name(segment);
    String  *ixix_file = Str_newf("%o/lexicon-%i32.ixix", seg_name, field_num);
    String  *ix_file   = Str_newf("%o/lexicon-%i32.ix", seg_name, field_num);
    Architecture *arch = Schema_Get_Architecture(schema);

    // Init.
    Lex_init((Lexicon*)self, field);
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    ivars->tinfo        = TInfo_new(0);
    ivars->tick         = 0;

    // Derive
    ivars->field_type = Schema_Fetch_Type(schema, field);
    if (!ivars->field_type) {
        String *mess = MAKE_MESS("Unknown field: '%o'", field);
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        Err_throw_mess(ERR, mess);
    }
    ivars->field_type = (FieldType*)INCREF(ivars->field_type);
    ivars->term_stepper = FType_Make_Term_Stepper(ivars->field_type);
    ivars->ixix_in = Folder_Open_In(folder, ixix_file);
    if (!ivars->ixix_in) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        RETHROW(error);
    }
    ivars->ix_in = Folder_Open_In(folder, ix_file);
    if (!ivars->ix_in) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(ix_file);
        DECREF(ixix_file);
        DECREF(self);
        RETHROW(error);
    }
    ivars->index_interval = Arch_Index_Interval(arch);
    ivars->skip_interval  = Arch_Skip_Interval(arch);
    ivars->size    = (int32_t)(InStream_Length(ivars->ixix_in) / sizeof(int64_t));
    ivars->offsets = (const int64_t*)InStream_Buf(ivars->ixix_in,
            (size_t)InStream_Length(ivars->ixix_in));

    DECREF(ixix_file);
    DECREF(ix_file);

    return self;
}

void
LexIndex_Destroy_IMP(LexIndex *self) {
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    DECREF(ivars->field_type);
    DECREF(ivars->ixix_in);
    DECREF(ivars->ix_in);
    DECREF(ivars->term_stepper);
    DECREF(ivars->tinfo);
    SUPER_DESTROY(self, LEXINDEX);
}

int32_t
LexIndex_Get_Term_Num_IMP(LexIndex *self) {
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    return (ivars->index_interval * ivars->tick) - 1;
}

Obj*
LexIndex_Get_Term_IMP(LexIndex *self) {
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    return TermStepper_Get_Value(ivars->term_stepper);
}

TermInfo*
LexIndex_Get_Term_Info_IMP(LexIndex *self) {
    return LexIndex_IVARS(self)->tinfo;
}

static void
S_read_entry(LexIndex *self) {
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    InStream *ix_in  = ivars->ix_in;
    TermInfo *const tinfo = ivars->tinfo;
    int64_t offset = (int64_t)NumUtil_decode_bigend_u64(ivars->offsets + ivars->tick);
    InStream_Seek(ix_in, offset);
    TermStepper_Read_Key_Frame(ivars->term_stepper, ix_in);
    int32_t doc_freq = InStream_Read_C32(ix_in);
    TInfo_Set_Doc_Freq(tinfo, doc_freq);
    TInfo_Set_Post_FilePos(tinfo, InStream_Read_C64(ix_in));
    int64_t skip_filepos = doc_freq >= ivars->skip_interval
                           ? InStream_Read_C64(ix_in)
                           : 0;
    TInfo_Set_Skip_FilePos(tinfo, skip_filepos);
    TInfo_Set_Lex_FilePos(tinfo, InStream_Read_C64(ix_in));
}

void
LexIndex_Seek_IMP(LexIndex *self, Obj *target) {
    LexIndexIVARS *const ivars = LexIndex_IVARS(self);
    TermStepper *term_stepper = ivars->term_stepper;
    InStream    *ix_in        = ivars->ix_in;
    FieldType   *type         = ivars->field_type;
    int32_t      lo           = 0;
    int32_t      hi           = ivars->size - 1;
    int32_t      result       = -100;

    if (target == NULL || ivars->size == 0) {
        ivars->tick = 0;
        return;
    }
    else {
        if (!Obj_Is_A(target, STRING)) {
            THROW(ERR, "Target is a %o, and not comparable to a %o",
                  Obj_Get_Class_Name(target), Class_Get_Name(STRING));
        }
        /* TODO:
        Obj *first_obj = VA_Fetch(terms, 0);
        if (!Obj_Is_A(target, Obj_Get_Class(first_obj))) {
            THROW(ERR, "Target is a %o, and not comparable to a %o",
                Obj_Get_Class_Name(target), Obj_Get_Class_Name(first_obj));
        }
        */
    }

    // Divide and conquer.
    while (hi >= lo) {
        const int32_t mid = lo + ((hi - lo) / 2);
        const int64_t offset
            = (int64_t)NumUtil_decode_bigend_u64(ivars->offsets + mid);
        InStream_Seek(ix_in, offset);
        TermStepper_Read_Key_Frame(term_stepper, ix_in);

        // Compare values.  There is no need for a NULL-check because the term
        // number is alway between 0 and ivars->size - 1.
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
    ivars->tick = hi == -1 // indicating that target lt first entry
                 ? 0
                 : result == -100 // if result is still -100, it wasn't set
                 ? hi
                 : result;
    S_read_entry(self);
}


