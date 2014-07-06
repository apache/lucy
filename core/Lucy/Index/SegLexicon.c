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

#define C_LUCY_SEGLEXICON
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SegLexicon.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Index/LexIndex.h"
#include "Lucy/Index/LexiconWriter.h"
#include "Lucy/Index/Posting/MatchPosting.h"
#include "Lucy/Index/SegPostingList.h"
#include "Lucy/Index/TermStepper.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"

// Iterate until the state is greater than or equal to the target.
static void
S_scan_to(SegLexicon *self, Obj *target);

SegLexicon*
SegLex_new(Schema *schema, Folder *folder, Segment *segment,
           String *field) {
    SegLexicon *self = (SegLexicon*)Class_Make_Obj(SEGLEXICON);
    return SegLex_init(self, schema, folder, segment, field);
}

SegLexicon*
SegLex_init(SegLexicon *self, Schema *schema, Folder *folder,
            Segment *segment, String *field) {
    Hash *metadata = (Hash*)CERTIFY(
                         Seg_Fetch_Metadata_Utf8(segment, "lexicon", 7),
                         HASH);
    Architecture *arch      = Schema_Get_Architecture(schema);
    Hash         *counts    = (Hash*)Hash_Fetch_Utf8(metadata, "counts", 6);
    Obj          *format    = Hash_Fetch_Utf8(metadata, "format", 6);
    String       *seg_name  = Seg_Get_Name(segment);
    int32_t       field_num = Seg_Field_Num(segment, field);
    FieldType    *type      = Schema_Fetch_Type(schema, field);
    String *filename = Str_newf("%o/lexicon-%i32.dat", seg_name, field_num);

    Lex_init((Lexicon*)self, field);
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);

    // Check format.
    if (!format) { THROW(ERR, "Missing 'format'"); }
    else {
        if (Obj_To_I64(format) > LexWriter_current_file_format) {
            THROW(ERR, "Unsupported lexicon format: %i64",
                  Obj_To_I64(format));
        }
    }

    // Extract count from metadata.
    if (!counts) { THROW(ERR, "Failed to extract 'counts'"); }
    else {
        Obj *count = CERTIFY(Hash_Fetch(counts, (Obj*)field), OBJ);
        ivars->size = (int32_t)Obj_To_I64(count);
    }

    // Assign.
    ivars->segment        = (Segment*)INCREF(segment);

    // Derive.
    ivars->lex_index      = LexIndex_new(schema, folder, segment, field);
    ivars->field_num      = field_num;
    ivars->index_interval = Arch_Index_Interval(arch);
    ivars->skip_interval  = Arch_Skip_Interval(arch);
    ivars->instream       = Folder_Open_In(folder, filename);
    if (!ivars->instream) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(filename);
        DECREF(self);
        RETHROW(error);
    }
    DECREF(filename);

    // Define the term_num as "not yet started".
    ivars->term_num = -1;

    // Get steppers.
    ivars->term_stepper  = FType_Make_Term_Stepper(type);
    ivars->tinfo_stepper = (TermStepper*)MatchTInfoStepper_new(schema);

    return self;
}

void
SegLex_Destroy_IMP(SegLexicon *self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    DECREF(ivars->segment);
    DECREF(ivars->term_stepper);
    DECREF(ivars->tinfo_stepper);
    DECREF(ivars->lex_index);
    DECREF(ivars->instream);
    SUPER_DESTROY(self, SEGLEXICON);
}

void
SegLex_Seek_IMP(SegLexicon *self, Obj *target) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    LexIndex *const lex_index = ivars->lex_index;

    // Reset upon null term.
    if (target == NULL) {
        SegLex_Reset(self);
        return;
    }

    // Use the LexIndex to get in the ballpark.
    LexIndex_Seek(lex_index, target);
    TermInfo *target_tinfo = LexIndex_Get_Term_Info(lex_index);
    TermInfo *my_tinfo
        = (TermInfo*)TermStepper_Get_Value(ivars->tinfo_stepper);
    Obj *lex_index_term = Obj_Clone(LexIndex_Get_Term(lex_index));
    TInfo_Mimic(my_tinfo, (Obj*)target_tinfo);
    TermStepper_Set_Value(ivars->term_stepper, lex_index_term);
    DECREF(lex_index_term);
    InStream_Seek(ivars->instream, TInfo_Get_Lex_FilePos(target_tinfo));
    ivars->term_num = LexIndex_Get_Term_Num(lex_index);

    // Scan to the precise location.
    S_scan_to(self, target);
}

void
SegLex_Reset_IMP(SegLexicon* self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    ivars->term_num = -1;
    InStream_Seek(ivars->instream, 0);
    TermStepper_Reset(ivars->term_stepper);
    TermStepper_Reset(ivars->tinfo_stepper);
}

int32_t
SegLex_Get_Field_Num_IMP(SegLexicon *self) {
    return SegLex_IVARS(self)->field_num;
}

Obj*
SegLex_Get_Term_IMP(SegLexicon *self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    return TermStepper_Get_Value(ivars->term_stepper);
}

int32_t
SegLex_Doc_Freq_IMP(SegLexicon *self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    TermInfo *tinfo = (TermInfo*)TermStepper_Get_Value(ivars->tinfo_stepper);
    return tinfo ? TInfo_Get_Doc_Freq(tinfo) : 0;
}

TermInfo*
SegLex_Get_Term_Info_IMP(SegLexicon *self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);
    return (TermInfo*)TermStepper_Get_Value(ivars->tinfo_stepper);
}

Segment*
SegLex_Get_Segment_IMP(SegLexicon *self) {
    return SegLex_IVARS(self)->segment;
}

bool
SegLex_Next_IMP(SegLexicon *self) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);

    // If we've run out of terms, null out and return.
    if (++ivars->term_num >= ivars->size) {
        ivars->term_num = ivars->size; // don't keep growing
        TermStepper_Reset(ivars->term_stepper);
        TermStepper_Reset(ivars->tinfo_stepper);
        return false;
    }

    // Read next term/terminfo.
    TermStepper_Read_Delta(ivars->term_stepper, ivars->instream);
    TermStepper_Read_Delta(ivars->tinfo_stepper, ivars->instream);

    return true;
}

static void
S_scan_to(SegLexicon *self, Obj *target) {
    SegLexiconIVARS *const ivars = SegLex_IVARS(self);

    // Keep looping until the term text is ge target.
    do {
        // (mildly evil encapsulation violation, since value can be null)
        Obj *current = TermStepper_Get_Value(ivars->term_stepper);
        const int32_t comparison = Obj_Compare_To(current, target);
        if (comparison >= 0 && ivars->term_num != -1) { break; }
    } while (SegLex_Next(self));
}


