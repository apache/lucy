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

#define C_LUCY_LEXICONWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/LexiconWriter.h"
#include "Clownfish/CharBuf.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Posting/MatchPosting.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Index/TermStepper.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

int32_t LexWriter_current_file_format = 3;

LexiconWriter*
LexWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
              PolyReader *polyreader) {
    LexiconWriter *self = (LexiconWriter*)Class_Make_Obj(LEXICONWRITER);
    return LexWriter_init(self, schema, snapshot, segment, polyreader);
}

LexiconWriter*
LexWriter_init(LexiconWriter *self, Schema *schema, Snapshot *snapshot,
               Segment *segment, PolyReader *polyreader) {
    Architecture *arch = Schema_Get_Architecture(schema);

    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);

    // Assign.
    ivars->index_interval = Arch_Index_Interval(arch);
    ivars->skip_interval  = Arch_Skip_Interval(arch);

    // Init.
    ivars->ix_out             = NULL;
    ivars->ixix_out           = NULL;
    ivars->dat_out            = NULL;
    ivars->count              = 0;
    ivars->ix_count           = 0;
    ivars->dat_file           = NULL;
    ivars->ix_file            = NULL;
    ivars->ixix_file          = NULL;
    ivars->counts             = Hash_new(0);
    ivars->ix_counts          = Hash_new(0);
    ivars->temp_mode          = false;
    ivars->term_stepper       = NULL;
    ivars->tinfo_stepper      = (TermStepper*)MatchTInfoStepper_new(schema);

    return self;
}

void
LexWriter_Destroy_IMP(LexiconWriter *self) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    DECREF(ivars->term_stepper);
    DECREF(ivars->tinfo_stepper);
    DECREF(ivars->dat_file);
    DECREF(ivars->ix_file);
    DECREF(ivars->ixix_file);
    DECREF(ivars->dat_out);
    DECREF(ivars->ix_out);
    DECREF(ivars->ixix_out);
    DECREF(ivars->counts);
    DECREF(ivars->ix_counts);
    SUPER_DESTROY(self, LEXICONWRITER);
}

static void
S_add_last_term_to_ix(LexiconWriter *self) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);

    // Write file pointer to index record.
    OutStream_Write_I64(ivars->ixix_out, OutStream_Tell(ivars->ix_out));

    // Write term and file pointer to main record.  Track count of terms added
    // to ix.
    TermStepper_Write_Key_Frame(ivars->term_stepper,
                                ivars->ix_out, TermStepper_Get_Value(ivars->term_stepper));
    TermStepper_Write_Key_Frame(ivars->tinfo_stepper,
                                ivars->ix_out, TermStepper_Get_Value(ivars->tinfo_stepper));
    OutStream_Write_C64(ivars->ix_out, OutStream_Tell(ivars->dat_out));
    ivars->ix_count++;
}

void
LexWriter_Add_Term_IMP(LexiconWriter* self, Obj* term_text, TermInfo* tinfo) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    OutStream *dat_out = ivars->dat_out;

    if ((ivars->count % ivars->index_interval == 0)
        && !ivars->temp_mode
       ) {
        // Write a subset of entries to lexicon.ix.
        S_add_last_term_to_ix(self);
    }

    TermStepper_Write_Delta(ivars->term_stepper, dat_out, term_text);
    TermStepper_Write_Delta(ivars->tinfo_stepper, dat_out, (Obj*)tinfo);

    // Track number of terms.
    ivars->count++;
}

void
LexWriter_Start_Field_IMP(LexiconWriter *self, int32_t field_num) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    Segment   *const segment  = LexWriter_Get_Segment(self);
    Folder    *const folder   = LexWriter_Get_Folder(self);
    Schema    *const schema   = LexWriter_Get_Schema(self);
    String    *const seg_name = Seg_Get_Name(segment);
    String    *const field    = Seg_Field_Name(segment, field_num);
    FieldType *const type     = Schema_Fetch_Type(schema, field);

    // Open outstreams.
    DECREF(ivars->dat_file);
    DECREF(ivars->ix_file);
    DECREF(ivars->ixix_file);
    ivars->dat_file  = Str_newf("%o/lexicon-%i32.dat",  seg_name, field_num);
    ivars->ix_file   = Str_newf("%o/lexicon-%i32.ix",   seg_name, field_num);
    ivars->ixix_file = Str_newf("%o/lexicon-%i32.ixix", seg_name, field_num);
    ivars->dat_out = Folder_Open_Out(folder, ivars->dat_file);
    if (!ivars->dat_out) { RETHROW(INCREF(Err_get_error())); }
    ivars->ix_out = Folder_Open_Out(folder, ivars->ix_file);
    if (!ivars->ix_out) { RETHROW(INCREF(Err_get_error())); }
    ivars->ixix_out = Folder_Open_Out(folder, ivars->ixix_file);
    if (!ivars->ixix_out) { RETHROW(INCREF(Err_get_error())); }

    // Initialize count and ix_count, term stepper and term info stepper.
    ivars->count    = 0;
    ivars->ix_count = 0;
    ivars->term_stepper = FType_Make_Term_Stepper(type);
    TermStepper_Reset(ivars->tinfo_stepper);
}

void
LexWriter_Finish_Field_IMP(LexiconWriter *self, int32_t field_num) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    String *field = Seg_Field_Name(ivars->segment, field_num);

    // Store count of terms for this field as metadata.
    Hash_Store(ivars->counts, (Obj*)field,
               (Obj*)Str_newf("%i32", ivars->count));
    Hash_Store(ivars->ix_counts, (Obj*)field,
               (Obj*)Str_newf("%i32", ivars->ix_count));

    // Close streams.
    OutStream_Close(ivars->dat_out);
    OutStream_Close(ivars->ix_out);
    OutStream_Close(ivars->ixix_out);
    DECREF(ivars->dat_out);
    DECREF(ivars->ix_out);
    DECREF(ivars->ixix_out);
    ivars->dat_out  = NULL;
    ivars->ix_out   = NULL;
    ivars->ixix_out = NULL;

    // Close term stepper.
    DECREF(ivars->term_stepper);
    ivars->term_stepper = NULL;
}

void
LexWriter_Enter_Temp_Mode_IMP(LexiconWriter *self, String *field,
                              OutStream *temp_outstream) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    Schema    *schema = LexWriter_Get_Schema(self);
    FieldType *type   = Schema_Fetch_Type(schema, field);

    // Assign outstream.
    if (ivars->dat_out != NULL) {
        THROW(ERR, "Can't enter temp mode (filename: %o) ", ivars->dat_file);
    }
    ivars->dat_out = (OutStream*)INCREF(temp_outstream);

    // Initialize count and ix_count, term stepper and term info stepper.
    ivars->count    = 0;
    ivars->ix_count = 0;
    ivars->term_stepper = FType_Make_Term_Stepper(type);
    TermStepper_Reset(ivars->tinfo_stepper);

    // Remember that we're in temp mode.
    ivars->temp_mode = true;
}

void
LexWriter_Leave_Temp_Mode_IMP(LexiconWriter *self) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    DECREF(ivars->term_stepper);
    ivars->term_stepper = NULL;
    DECREF(ivars->dat_out);
    ivars->dat_out   = NULL;
    ivars->temp_mode = false;
}

void
LexWriter_Finish_IMP(LexiconWriter *self) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);

    // Ensure that streams were closed (by calling Finish_Field or
    // Leave_Temp_Mode).
    if (ivars->dat_out != NULL) {
        THROW(ERR, "File '%o' never closed", ivars->dat_file);
    }
    else if (ivars->ix_out != NULL) {
        THROW(ERR, "File '%o' never closed", ivars->ix_file);
    }
    else if (ivars->ix_out != NULL) {
        THROW(ERR, "File '%o' never closed", ivars->ix_file);
    }

    // Store metadata.
    Seg_Store_Metadata_Utf8(ivars->segment, "lexicon", 7,
                            (Obj*)LexWriter_Metadata(self));
}

Hash*
LexWriter_Metadata_IMP(LexiconWriter *self) {
    LexiconWriterIVARS *const ivars = LexWriter_IVARS(self);
    LexWriter_Metadata_t super_meta
        = (LexWriter_Metadata_t)SUPER_METHOD_PTR(LEXICONWRITER,
                                                 LUCY_LexWriter_Metadata);
    Hash *const metadata  = super_meta(self);
    Hash *const counts    = (Hash*)INCREF(ivars->counts);
    Hash *const ix_counts = (Hash*)INCREF(ivars->ix_counts);

    // Placeholders.
    if (Hash_Get_Size(counts) == 0) {
        Hash_Store_Utf8(counts, "none", 4, (Obj*)Str_newf("%i32", (int32_t)0));
        Hash_Store_Utf8(ix_counts, "none", 4,
                        (Obj*)Str_newf("%i32", (int32_t)0));
    }

    Hash_Store_Utf8(metadata, "counts", 6, (Obj*)counts);
    Hash_Store_Utf8(metadata, "index_counts", 12, (Obj*)ix_counts);

    return metadata;
}

void
LexWriter_Add_Segment_IMP(LexiconWriter *self, SegReader *reader,
                          I32Array *doc_map) {
    // No-op, since the data gets added via PostingListWriter.
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(doc_map);
}

int32_t
LexWriter_Format_IMP(LexiconWriter *self) {
    UNUSED_VAR(self);
    return LexWriter_current_file_format;
}


