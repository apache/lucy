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
SegPList_new(PostingListReader *plist_reader, const CharBuf *field) {
    SegPostingList *self = (SegPostingList*)VTable_Make_Obj(SEGPOSTINGLIST);
    return SegPList_init(self, plist_reader, field);
}

SegPostingList*
SegPList_init(SegPostingList *self, PostingListReader *plist_reader,
              const CharBuf *field) {
    Schema       *const schema   = PListReader_Get_Schema(plist_reader);
    Folder       *const folder   = PListReader_Get_Folder(plist_reader);
    Segment      *const segment  = PListReader_Get_Segment(plist_reader);
    Architecture *const arch     = Schema_Get_Architecture(schema);
    CharBuf      *const seg_name = Seg_Get_Name(segment);
    int32_t       field_num      = Seg_Field_Num(segment, field);
    CharBuf      *post_file      = CB_newf("%o/postings-%i32.dat",
                                           seg_name, field_num);
    CharBuf      *skip_file      = CB_newf("%o/postings.skip", seg_name);

    // Init.
    self->doc_freq        = 0;
    self->count           = 0;

    // Init skipping vars.
    self->skip_stepper    = SkipStepper_new();
    self->skip_count      = 0;
    self->num_skips       = 0;

    // Assign.
    self->plist_reader    = (PostingListReader*)INCREF(plist_reader);
    self->field           = CB_Clone(field);
    self->skip_interval   = Arch_Skip_Interval(arch);

    // Derive.
    Similarity *sim = Schema_Fetch_Sim(schema, field);
    self->posting   = Sim_Make_Posting(sim);
    self->field_num = field_num;

    // Open both a main stream and a skip stream if the field exists.
    if (Folder_Exists(folder, post_file)) {
        self->post_stream = Folder_Open_In(folder, post_file);
        if (!self->post_stream) {
            Err *error = (Err*)INCREF(Err_get_error());
            DECREF(post_file);
            DECREF(skip_file);
            DECREF(self);
            RETHROW(error);
        }
        self->skip_stream = Folder_Open_In(folder, skip_file);
        if (!self->skip_stream) {
            Err *error = (Err*)INCREF(Err_get_error());
            DECREF(post_file);
            DECREF(skip_file);
            DECREF(self);
            RETHROW(error);
        }
    }
    else {
        //  Empty, so don't bother with these.
        self->post_stream = NULL;
        self->skip_stream = NULL;
    }
    DECREF(post_file);
    DECREF(skip_file);

    return self;
}

void
SegPList_destroy(SegPostingList *self) {
    DECREF(self->plist_reader);
    DECREF(self->posting);
    DECREF(self->skip_stepper);
    DECREF(self->field);

    if (self->post_stream != NULL) {
        InStream_Close(self->post_stream);
        InStream_Close(self->skip_stream);
        DECREF(self->post_stream);
        DECREF(self->skip_stream);
    }

    SUPER_DESTROY(self, SEGPOSTINGLIST);
}

Posting*
SegPList_get_posting(SegPostingList *self) {
    return self->posting;
}

uint32_t
SegPList_get_doc_freq(SegPostingList *self) {
    return self->doc_freq;
}

int32_t
SegPList_get_doc_id(SegPostingList *self) {
    return self->posting->doc_id;
}

uint32_t
SegPList_get_count(SegPostingList *self) {
    return self->count;
}

InStream*
SegPList_get_post_stream(SegPostingList *self) {
    return self->post_stream;
}

int32_t
SegPList_next(SegPostingList *self) {
    InStream *const post_stream = self->post_stream;
    Posting  *const posting     = self->posting;

    // Bail if we're out of docs.
    if (self->count >= self->doc_freq) {
        Post_Reset(posting);
        return 0;
    }
    self->count++;

    Post_Read_Record(posting, post_stream);

    return posting->doc_id;
}

int32_t
SegPList_advance(SegPostingList *self, int32_t target) {
    Posting *posting          = self->posting;
    const uint32_t skip_interval = self->skip_interval;

    if (self->doc_freq >= skip_interval) {
        InStream *post_stream           = self->post_stream;
        InStream *skip_stream           = self->skip_stream;
        SkipStepper *const skip_stepper = self->skip_stepper;
        uint32_t new_doc_id             = skip_stepper->doc_id;
        int64_t new_filepos             = InStream_Tell(post_stream);

        /* Assuming the default skip_interval of 16...
         *
         * Say we're currently on the 5th doc matching this term, and we get a
         * request to skip to the 18th doc matching it.  We won't have skipped
         * yet, but we'll have already gone past 5 of the 16 skip docs --
         * ergo, the modulus in the following formula.
         */
        int32_t num_skipped = 0 - (self->count % skip_interval);
        if (num_skipped == 0 && self->count != 0) {
            num_skipped = 0 - skip_interval;
        }

        // See if there's anything to skip.
        while (target > skip_stepper->doc_id) {
            new_doc_id  = skip_stepper->doc_id;
            new_filepos = skip_stepper->filepos;

            if (skip_stepper->doc_id != 0
                && skip_stepper->doc_id >= posting->doc_id
               ) {
                num_skipped += skip_interval;
            }

            if (self->skip_count >= self->num_skips) {
                break;
            }

            SkipStepper_Read_Record(skip_stepper, skip_stream);
            self->skip_count++;
        }

        // If we found something to skip, skip it.
        if (new_filepos > InStream_Tell(post_stream)) {

            // Move the postings filepointer up.
            InStream_Seek(post_stream, new_filepos);

            // Jump to the new doc id.
            posting->doc_id = new_doc_id;

            // Increase count by the number of docs we skipped over.
            self->count += num_skipped;
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
SegPList_seek(SegPostingList *self, Obj *target) {
    LexiconReader *lex_reader = PListReader_Get_Lex_Reader(self->plist_reader);
    TermInfo      *tinfo      = LexReader_Fetch_Term_Info(lex_reader,
                                                          self->field, target);
    S_seek_tinfo(self, tinfo);
    DECREF(tinfo);
}

void
SegPList_seek_lex(SegPostingList *self, Lexicon *lexicon) {
    // Maybe true, maybe not.
    SegLexicon *const seg_lexicon = (SegLexicon*)lexicon;

    // Optimized case.
    if (Obj_Is_A((Obj*)lexicon, SEGLEXICON)
        && (SegLex_Get_Segment(seg_lexicon)
            == PListReader_Get_Segment(self->plist_reader)) // i.e. same segment
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
    self->count = 0;

    if (tinfo == NULL) {
        // Next will return false; other methods invalid now.
        self->doc_freq = 0;
    }
    else {
        // Transfer doc_freq, seek main stream.
        int64_t post_filepos = TInfo_Get_Post_FilePos(tinfo);
        self->doc_freq       = TInfo_Get_Doc_Freq(tinfo);
        InStream_Seek(self->post_stream, post_filepos);

        // Prepare posting.
        Post_Reset(self->posting);

        // Prepare to skip.
        self->skip_count = 0;
        self->num_skips  = self->doc_freq / self->skip_interval;
        SkipStepper_Set_ID_And_Filepos(self->skip_stepper, 0, post_filepos);
        InStream_Seek(self->skip_stream, TInfo_Get_Skip_FilePos(tinfo));
    }
}

Matcher*
SegPList_make_matcher(SegPostingList *self, Similarity *sim,
                      Compiler *compiler, bool_t need_score) {
    return Post_Make_Matcher(self->posting, sim, (PostingList*)self, compiler,
                             need_score);
}

RawPosting*
SegPList_read_raw(SegPostingList *self, int32_t last_doc_id, CharBuf *term_text,
                  MemoryPool *mem_pool) {
    return Post_Read_Raw(self->posting, self->post_stream,
                         last_doc_id, term_text, mem_pool);
}



