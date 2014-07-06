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

#define C_LUCY_SEGPOSTINGLIST
#define C_LUCY_POSTING
#define C_LUCY_SKIPSTEPPER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SegPostingList.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SkipStepper.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Index/SegLexicon.h"
#include "Lucy/Index/LexiconReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/MemoryPool.h"

// Low level seek call.
static void
S_seek_tinfo(SegPostingList *self, TermInfo *tinfo);

SegPostingList*
SegPList_new(PostingListReader *plist_reader, String *field) {
    SegPostingList *self = (SegPostingList*)Class_Make_Obj(SEGPOSTINGLIST);
    return SegPList_init(self, plist_reader, field);
}

SegPostingList*
SegPList_init(SegPostingList *self, PostingListReader *plist_reader,
              String *field) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    Schema       *const schema   = PListReader_Get_Schema(plist_reader);
    Folder       *const folder   = PListReader_Get_Folder(plist_reader);
    Segment      *const segment  = PListReader_Get_Segment(plist_reader);
    Architecture *const arch     = Schema_Get_Architecture(schema);
    String       *const seg_name = Seg_Get_Name(segment);
    int32_t       field_num      = Seg_Field_Num(segment, field);
    String       *post_file      = Str_newf("%o/postings-%i32.dat",
                                           seg_name, field_num);
    String       *skip_file      = Str_newf("%o/postings.skip", seg_name);

    // Init.
    ivars->doc_freq        = 0;
    ivars->count           = 0;

    // Init skipping vars.
    ivars->skip_stepper    = SkipStepper_new();
    ivars->skip_count      = 0;
    ivars->num_skips       = 0;

    // Assign.
    ivars->plist_reader    = (PostingListReader*)INCREF(plist_reader);
    ivars->field           = Str_Clone(field);
    ivars->skip_interval   = Arch_Skip_Interval(arch);

    // Derive.
    Similarity *sim  = Schema_Fetch_Sim(schema, field);
    ivars->posting   = Sim_Make_Posting(sim);
    ivars->field_num = field_num;

    // Open both a main stream and a skip stream if the field exists.
    if (Folder_Exists(folder, post_file)) {
        ivars->post_stream = Folder_Open_In(folder, post_file);
        if (!ivars->post_stream) {
            Err *error = (Err*)INCREF(Err_get_error());
            DECREF(post_file);
            DECREF(skip_file);
            DECREF(self);
            RETHROW(error);
        }
        ivars->skip_stream = Folder_Open_In(folder, skip_file);
        if (!ivars->skip_stream) {
            Err *error = (Err*)INCREF(Err_get_error());
            DECREF(post_file);
            DECREF(skip_file);
            DECREF(self);
            RETHROW(error);
        }
    }
    else {
        //  Empty, so don't bother with these.
        ivars->post_stream = NULL;
        ivars->skip_stream = NULL;
    }
    DECREF(post_file);
    DECREF(skip_file);

    return self;
}

void
SegPList_Destroy_IMP(SegPostingList *self) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    DECREF(ivars->plist_reader);
    DECREF(ivars->posting);
    DECREF(ivars->skip_stepper);
    DECREF(ivars->field);

    if (ivars->post_stream != NULL) {
        InStream_Close(ivars->post_stream);
        InStream_Close(ivars->skip_stream);
        DECREF(ivars->post_stream);
        DECREF(ivars->skip_stream);
    }

    SUPER_DESTROY(self, SEGPOSTINGLIST);
}

Posting*
SegPList_Get_Posting_IMP(SegPostingList *self) {
    return SegPList_IVARS(self)->posting;
}

uint32_t
SegPList_Get_Doc_Freq_IMP(SegPostingList *self) {
    return SegPList_IVARS(self)->doc_freq;
}

int32_t
SegPList_Get_Doc_ID_IMP(SegPostingList *self) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    return Post_IVARS(ivars->posting)->doc_id;
}

uint32_t
SegPList_Get_Count_IMP(SegPostingList *self) {
    return SegPList_IVARS(self)->count;
}

InStream*
SegPList_Get_Post_Stream_IMP(SegPostingList *self) {
    return SegPList_IVARS(self)->post_stream;
}

int32_t
SegPList_Next_IMP(SegPostingList *self) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    InStream *const post_stream = ivars->post_stream;
    Posting  *const posting     = ivars->posting;

    // Bail if we're out of docs.
    if (ivars->count >= ivars->doc_freq) {
        Post_Reset(posting);
        return 0;
    }
    ivars->count++;

    Post_Read_Record(posting, post_stream);

    return Post_IVARS(posting)->doc_id;
}

int32_t
SegPList_Advance_IMP(SegPostingList *self, int32_t target) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    PostingIVARS *const posting_ivars = Post_IVARS(ivars->posting);
    const uint32_t skip_interval = ivars->skip_interval;

    if (ivars->doc_freq >= skip_interval) {
        InStream *post_stream           = ivars->post_stream;
        InStream *skip_stream           = ivars->skip_stream;
        SkipStepper *const skip_stepper = ivars->skip_stepper;
        SkipStepperIVARS *const skip_stepper_ivars
            = SkipStepper_IVARS(skip_stepper);
        uint32_t new_doc_id             = skip_stepper_ivars->doc_id;
        int64_t new_filepos             = InStream_Tell(post_stream);

        /* Assuming the default skip_interval of 16...
         *
         * Say we're currently on the 5th doc matching this term, and we get a
         * request to skip to the 18th doc matching it.  We won't have skipped
         * yet, but we'll have already gone past 5 of the 16 skip docs --
         * ergo, the modulus in the following formula.
         */
        int32_t num_skipped = 0 - (ivars->count % skip_interval);
        if (num_skipped == 0 && ivars->count != 0) {
            num_skipped = 0 - skip_interval;
        }

        // See if there's anything to skip.
        while (target > skip_stepper_ivars->doc_id) {
            new_doc_id  = skip_stepper_ivars->doc_id;
            new_filepos = skip_stepper_ivars->filepos;

            if (skip_stepper_ivars->doc_id != 0
                && skip_stepper_ivars->doc_id >= posting_ivars->doc_id
               ) {
                num_skipped += skip_interval;
            }

            if (ivars->skip_count >= ivars->num_skips) {
                break;
            }

            SkipStepper_Read_Record(skip_stepper, skip_stream);
            ivars->skip_count++;
        }

        // If we found something to skip, skip it.
        if (new_filepos > InStream_Tell(post_stream)) {

            // Move the postings filepointer up.
            InStream_Seek(post_stream, new_filepos);

            // Jump to the new doc id.
            posting_ivars->doc_id = new_doc_id;

            // Increase count by the number of docs we skipped over.
            ivars->count += num_skipped;
        }
    }

    // Done skipping, so scan.
    while (1) {
        int32_t doc_id = SegPList_Next(self);
        if (doc_id == 0 || doc_id >= target) {
            return doc_id;
        }
    }
}

void
SegPList_Seek_IMP(SegPostingList *self, Obj *target) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    LexiconReader *lex_reader = PListReader_Get_Lex_Reader(ivars->plist_reader);
    TermInfo      *tinfo      = LexReader_Fetch_Term_Info(lex_reader,
                                                          ivars->field, target);
    S_seek_tinfo(self, tinfo);
    DECREF(tinfo);
}

void
SegPList_Seek_Lex_IMP(SegPostingList *self, Lexicon *lexicon) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);

    // Maybe true, maybe not.
    SegLexicon *const seg_lexicon = (SegLexicon*)lexicon;

    // Optimized case.
    if (Obj_Is_A((Obj*)lexicon, SEGLEXICON)
        && (SegLex_Get_Segment(seg_lexicon)
            == PListReader_Get_Segment(ivars->plist_reader)) // i.e. same segment
       ) {
        S_seek_tinfo(self, SegLex_Get_Term_Info(seg_lexicon));
    }
    // Punt case.  This is more expensive because of the call to
    // LexReader_Fetch_Term_Info() in Seek().
    else {
        Obj *term = Lex_Get_Term(lexicon);
        SegPList_Seek(self, term);
    }
}

static void
S_seek_tinfo(SegPostingList *self, TermInfo *tinfo) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    ivars->count = 0;

    if (tinfo == NULL) {
        // Next will return false; other methods invalid now.
        ivars->doc_freq = 0;
    }
    else {
        // Transfer doc_freq, seek main stream.
        int64_t post_filepos = TInfo_Get_Post_FilePos(tinfo);
        ivars->doc_freq      = TInfo_Get_Doc_Freq(tinfo);
        InStream_Seek(ivars->post_stream, post_filepos);

        // Prepare posting.
        Post_Reset(ivars->posting);

        // Prepare to skip.
        ivars->skip_count = 0;
        ivars->num_skips  = ivars->doc_freq / ivars->skip_interval;
        SkipStepper_Set_ID_And_Filepos(ivars->skip_stepper, 0, post_filepos);
        InStream_Seek(ivars->skip_stream, TInfo_Get_Skip_FilePos(tinfo));
    }
}

Matcher*
SegPList_Make_Matcher_IMP(SegPostingList *self, Similarity *sim,
                          Compiler *compiler, bool need_score) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    return Post_Make_Matcher(ivars->posting, sim, (PostingList*)self, compiler,
                             need_score);
}

RawPosting*
SegPList_Read_Raw_IMP(SegPostingList *self, int32_t last_doc_id,
                      String *term_text, MemoryPool *mem_pool) {
    SegPostingListIVARS *const ivars = SegPList_IVARS(self);
    return Post_Read_Raw(ivars->posting, ivars->post_stream,
                         last_doc_id, term_text, mem_pool);
}



