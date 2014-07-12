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

#define C_LUCY_POSTINGPOOL
#define C_LUCY_RAWPOSTING
#define C_LUCY_MEMORYPOOL
#define C_LUCY_TERMINFO
#define C_LUCY_SKIPSTEPPER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PostingPool.h"
#include "Clownfish/CharBuf.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Index/LexiconReader.h"
#include "Lucy/Index/LexiconWriter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/RawLexicon.h"
#include "Lucy/Index/RawPostingList.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SkipStepper.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Index/TermStepper.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/MemoryPool.h"

// Prepare to read back postings from disk.
static void
S_fresh_flip(PostingPool *self, InStream *lex_temp_in,
             InStream *post_temp_in);

// Main loop.
static void
S_write_terms_and_postings(PostingPool *self, PostingWriter *post_writer,
                           OutStream *skip_stream);

PostingPool*
PostPool_new(Schema *schema, Snapshot *snapshot, Segment *segment,
             PolyReader *polyreader,  String *field,
             LexiconWriter *lex_writer, MemoryPool *mem_pool,
             OutStream *lex_temp_out, OutStream *post_temp_out,
             OutStream *skip_out) {
    PostingPool *self = (PostingPool*)Class_Make_Obj(POSTINGPOOL);
    return PostPool_init(self, schema, snapshot, segment, polyreader, field,
                         lex_writer, mem_pool, lex_temp_out, post_temp_out,
                         skip_out);
}

PostingPool*
PostPool_init(PostingPool *self, Schema *schema, Snapshot *snapshot,
              Segment *segment, PolyReader *polyreader, String *field,
              LexiconWriter *lex_writer, MemoryPool *mem_pool,
              OutStream *lex_temp_out, OutStream *post_temp_out,
              OutStream *skip_out) {
    // Init.
    SortEx_init((SortExternal*)self);
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    ivars->doc_base         = 0;
    ivars->last_doc_id      = 0;
    ivars->doc_map          = NULL;
    ivars->post_count       = 0;
    ivars->lexicon          = NULL;
    ivars->plist            = NULL;
    ivars->lex_temp_in      = NULL;
    ivars->post_temp_in     = NULL;
    ivars->lex_start        = INT64_MAX;
    ivars->post_start       = INT64_MAX;
    ivars->lex_end          = 0;
    ivars->post_end         = 0;
    ivars->skip_stepper     = SkipStepper_new();

    // Assign.
    ivars->schema         = (Schema*)INCREF(schema);
    ivars->snapshot       = (Snapshot*)INCREF(snapshot);
    ivars->segment        = (Segment*)INCREF(segment);
    ivars->polyreader     = (PolyReader*)INCREF(polyreader);
    ivars->lex_writer     = (LexiconWriter*)INCREF(lex_writer);
    ivars->mem_pool       = (MemoryPool*)INCREF(mem_pool);
    ivars->field          = Str_Clone(field);
    ivars->lex_temp_out   = (OutStream*)INCREF(lex_temp_out);
    ivars->post_temp_out  = (OutStream*)INCREF(post_temp_out);
    ivars->skip_out       = (OutStream*)INCREF(skip_out);

    // Derive.
    Similarity *sim = Schema_Fetch_Sim(schema, field);
    ivars->posting   = Sim_Make_Posting(sim);
    ivars->type      = (FieldType*)INCREF(Schema_Fetch_Type(schema, field));
    ivars->field_num = Seg_Field_Num(segment, field);

    return self;
}

void
PostPool_Destroy_IMP(PostingPool *self) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    DECREF(ivars->schema);
    DECREF(ivars->snapshot);
    DECREF(ivars->segment);
    DECREF(ivars->polyreader);
    DECREF(ivars->lex_writer);
    DECREF(ivars->field);
    DECREF(ivars->doc_map);
    DECREF(ivars->lexicon);
    DECREF(ivars->plist);
    DECREF(ivars->lex_temp_out);
    DECREF(ivars->post_temp_out);
    DECREF(ivars->skip_out);
    DECREF(ivars->lex_temp_in);
    DECREF(ivars->post_temp_in);
    DECREF(ivars->posting);
    DECREF(ivars->skip_stepper);
    DECREF(ivars->type);
    MemoryPool *mem_pool = ivars->mem_pool;
    SUPER_DESTROY(self, POSTINGPOOL);

    // The MemoryPool must be kept alive while SUPER_DESTROY runs because if
    // it is destroyed sooner, any RawPosting objects which belong to it will
    // become dangling references.  SortExternal's destructor will then
    // malfunction when it traverses the buffer DECREFing each element.
    DECREF(mem_pool);
}

int
PostPool_Compare_IMP(PostingPool *self, void *va, void *vb) {
    RawPostingIVARS *const a     = RawPost_IVARS(*(RawPosting**)va);
    RawPostingIVARS *const b     = RawPost_IVARS(*(RawPosting**)vb);
    const size_t      a_len = a->content_len;
    const size_t      b_len = b->content_len;
    const size_t      len   = a_len < b_len ? a_len : b_len;
    int comparison = memcmp(a->blob, b->blob, len);
    UNUSED_VAR(self);

    if (comparison == 0) {
        // If a is a substring of b, it's less than b, so return a neg num.
        comparison = a_len - b_len;

        // Break ties by doc id.
        if (comparison == 0) {
            comparison = a->doc_id - b->doc_id;
        }
    }

    return comparison;
}

MemoryPool*
PostPool_Get_Mem_Pool_IMP(PostingPool *self) {
    return PostPool_IVARS(self)->mem_pool;
}

void
PostPool_Flip_IMP(PostingPool *self) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    uint32_t num_runs   = VA_Get_Size(ivars->runs);
    uint32_t sub_thresh = num_runs > 0
                          ? ivars->mem_thresh / num_runs
                          : ivars->mem_thresh;

    if (num_runs) {
        Folder  *folder = PolyReader_Get_Folder(ivars->polyreader);
        String *seg_name = Seg_Get_Name(ivars->segment);
        String *lex_temp_path  = Str_newf("%o/lextemp", seg_name);
        String *post_temp_path = Str_newf("%o/ptemp", seg_name);
        ivars->lex_temp_in = Folder_Open_In(folder, lex_temp_path);
        if (!ivars->lex_temp_in) {
            RETHROW(INCREF(Err_get_error()));
        }
        ivars->post_temp_in = Folder_Open_In(folder, post_temp_path);
        if (!ivars->post_temp_in) {
            RETHROW(INCREF(Err_get_error()));
        }
        DECREF(lex_temp_path);
        DECREF(post_temp_path);
    }

    PostPool_Sort_Buffer(self);
    if (num_runs && (ivars->buf_max - ivars->buf_tick) > 0) {
        uint32_t num_items = PostPool_Buffer_Count(self);
        // Cheap imitation of flush. FIXME.
        PostingPool *run
            = PostPool_new(ivars->schema, ivars->snapshot, ivars->segment,
                           ivars->polyreader, ivars->field, ivars->lex_writer,
                           ivars->mem_pool, ivars->lex_temp_out,
                           ivars->post_temp_out, ivars->skip_out);
        PostPool_Grow_Buffer(run, num_items);
        PostingPoolIVARS *const run_ivars = PostPool_IVARS(run);

        memcpy(run_ivars->buffer, (ivars->buffer) + ivars->buf_tick,
               num_items * sizeof(Obj*));
        run_ivars->buf_max = num_items;
        PostPool_Add_Run(self, (SortExternal*)run);
        ivars->buf_tick = 0;
        ivars->buf_max = 0;
    }

    // Assign.
    for (uint32_t i = 0; i < num_runs; i++) {
        PostingPool *run = (PostingPool*)VA_Fetch(ivars->runs, i);
        if (run != NULL) {
            PostPool_Set_Mem_Thresh(run, sub_thresh);
            if (!PostPool_IVARS(run)->lexicon) {
                S_fresh_flip(run, ivars->lex_temp_in, ivars->post_temp_in);
            }
        }
    }

    ivars->flipped = true;
}

void
PostPool_Add_Segment_IMP(PostingPool *self, SegReader *reader,
                         I32Array *doc_map, int32_t doc_base) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    LexiconReader *lex_reader = (LexiconReader*)SegReader_Fetch(
                                    reader, Class_Get_Name(LEXICONREADER));
    Lexicon *lexicon = lex_reader
                       ? LexReader_Lexicon(lex_reader, ivars->field, NULL)
                       : NULL;

    if (lexicon) {
        PostingListReader *plist_reader
            = (PostingListReader*)SegReader_Fetch(
                  reader, Class_Get_Name(POSTINGLISTREADER));
        PostingList *plist = plist_reader
                             ? PListReader_Posting_List(plist_reader, ivars->field, NULL)
                             : NULL;
        if (!plist) {
            THROW(ERR, "Got a Lexicon but no PostingList for '%o' in '%o'",
                  ivars->field, SegReader_Get_Seg_Name(reader));
        }
        PostingPool *run
            = PostPool_new(ivars->schema, ivars->snapshot, ivars->segment,
                           ivars->polyreader, ivars->field, ivars->lex_writer,
                           ivars->mem_pool, ivars->lex_temp_out,
                           ivars->post_temp_out, ivars->skip_out);
        PostingPoolIVARS *const run_ivars = PostPool_IVARS(run);
        run_ivars->lexicon  = lexicon;
        run_ivars->plist    = plist;
        run_ivars->doc_base = doc_base;
        run_ivars->doc_map  = (I32Array*)INCREF(doc_map);
        PostPool_Add_Run(self, (SortExternal*)run);
    }
}

void
PostPool_Flush_IMP(PostingPool *self) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);

    // Don't add a run unless we have data to put in it.
    if (PostPool_Buffer_Count(self) == 0) { return; }

    PostingPool *run
        = PostPool_new(ivars->schema, ivars->snapshot, ivars->segment,
                       ivars->polyreader, ivars->field, ivars->lex_writer,
                       ivars->mem_pool, ivars->lex_temp_out,
                       ivars->post_temp_out, ivars->skip_out);
    PostingPoolIVARS *const run_ivars = PostPool_IVARS(run);
    PostingWriter *post_writer
        = (PostingWriter*)RawPostWriter_new(ivars->schema, ivars->snapshot,
                                            ivars->segment, ivars->polyreader,
                                            ivars->post_temp_out);

    // Borrow the buffer.
    run_ivars->buffer   = ivars->buffer;
    run_ivars->buf_tick = ivars->buf_tick;
    run_ivars->buf_max  = ivars->buf_max;
    run_ivars->buf_cap  = ivars->buf_cap;

    // Write to temp files.
    LexWriter_Enter_Temp_Mode(ivars->lex_writer, ivars->field,
                              ivars->lex_temp_out);
    run_ivars->lex_start  = OutStream_Tell(ivars->lex_temp_out);
    run_ivars->post_start = OutStream_Tell(ivars->post_temp_out);
    PostPool_Sort_Buffer(self);
    S_write_terms_and_postings(run, post_writer, NULL);

    run_ivars->lex_end  = OutStream_Tell(ivars->lex_temp_out);
    run_ivars->post_end = OutStream_Tell(ivars->post_temp_out);
    LexWriter_Leave_Temp_Mode(ivars->lex_writer);

    // Return the buffer and empty it.
    run_ivars->buffer   = NULL;
    run_ivars->buf_tick = 0;
    run_ivars->buf_max  = 0;
    run_ivars->buf_cap  = 0;
    PostPool_Clear_Buffer(self);

    // Add the run to the array.
    PostPool_Add_Run(self, (SortExternal*)run);

    DECREF(post_writer);
}

void
PostPool_Finish_IMP(PostingPool *self) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);

    // Bail if there's no data.
    if (!PostPool_Peek(self)) { return; }

    Similarity *sim = Schema_Fetch_Sim(ivars->schema, ivars->field);
    PostingWriter *post_writer
        = Sim_Make_Posting_Writer(sim, ivars->schema, ivars->snapshot,
                                  ivars->segment, ivars->polyreader,
                                  ivars->field_num);
    LexWriter_Start_Field(ivars->lex_writer, ivars->field_num);
    S_write_terms_and_postings(self, post_writer, ivars->skip_out);
    LexWriter_Finish_Field(ivars->lex_writer, ivars->field_num);
    DECREF(post_writer);
}

static void
S_write_terms_and_postings(PostingPool *self, PostingWriter *post_writer,
                           OutStream *skip_stream) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    TermInfo      *const tinfo            = TInfo_new(0);
    TermInfo      *const skip_tinfo       = TInfo_new(0);
    TermInfoIVARS *const tinfo_ivars      = TInfo_IVARS(tinfo);
    TermInfoIVARS *const skip_tinfo_ivars = TInfo_IVARS(skip_tinfo);
    LexiconWriter *const lex_writer       = ivars->lex_writer;
    SkipStepper   *const skip_stepper     = ivars->skip_stepper;
    SkipStepperIVARS *const skip_stepper_ivars
        = SkipStepper_IVARS(skip_stepper);
    int32_t        last_skip_doc          = 0;
    int64_t        last_skip_filepos      = 0;
    const int32_t  skip_interval
        = Arch_Skip_Interval(Schema_Get_Architecture(ivars->schema));

    // Prime heldover variables.
    RawPosting *posting
        = (RawPosting*)CERTIFY(PostPool_Fetch(self), RAWPOSTING);
    RawPostingIVARS *post_ivars = RawPost_IVARS(posting);
    CharBuf *last_term_text
        = CB_new_from_trusted_utf8(post_ivars->blob, post_ivars->content_len);
    const char *last_text_buf  = CB_Get_Ptr8(last_term_text);
    uint32_t    last_text_size = CB_Get_Size(last_term_text);
    SkipStepper_Set_ID_And_Filepos(skip_stepper, 0, 0);

    // Initialize sentinel to be used on the last iter, using an empty string
    // in order to make LexiconWriter Do The Right Thing.
    size_t sentinel_size = Class_Get_Obj_Alloc_Size(RAWPOSTING)
                           + 20;  // blob length + cushion
    char empty_string[] = "";
    RawPosting *sentinel = RawPost_new(alloca(sentinel_size), 0, 1,
                                       empty_string, 0);

    while (1) {
        bool same_text_as_last = true;

        if (posting == NULL) {
            // On the last iter, use an empty string to make LexiconWriter
            // DTRT.
            posting = sentinel;
            post_ivars = RawPost_IVARS(posting);
            same_text_as_last = false;
        }
        else {
            // Compare once.
            if (post_ivars->content_len != last_text_size
                || memcmp(&post_ivars->blob, last_text_buf, last_text_size) != 0
               ) {
                same_text_as_last = false;
            }
        }

        // If the term text changes, process the last term.
        if (!same_text_as_last) {
            // Hand off to LexiconWriter.
            LexWriter_Add_Term(lex_writer, (Obj*)last_term_text, tinfo);

            // Start each term afresh.
            TInfo_Reset(tinfo);
            PostWriter_Start_Term(post_writer, tinfo);

            // Init skip data in preparation for the next term.
            skip_stepper_ivars->doc_id  = 0;
            skip_stepper_ivars->filepos = tinfo_ivars->post_filepos;
            last_skip_doc         = 0;
            last_skip_filepos     = tinfo_ivars->post_filepos;

            // Remember the term_text so we can write string diffs.
            CB_Mimic_Utf8(last_term_text, post_ivars->blob,
                          post_ivars->content_len);
            last_text_buf  = CB_Get_Ptr8(last_term_text);
            last_text_size = CB_Get_Size(last_term_text);
        }

        // Bail on last iter before writing invalid posting data.
        if (posting == sentinel) { break; }

        // Write posting data.
        PostWriter_Write_Posting(post_writer, posting);

        // Doc freq lags by one iter.
        tinfo_ivars->doc_freq++;

        //  Write skip data.
        if (skip_stream != NULL
            && same_text_as_last
            && tinfo_ivars->doc_freq % skip_interval == 0
            && tinfo_ivars->doc_freq != 0
           ) {
            // If first skip group, save skip stream pos for term info.
            if (tinfo_ivars->doc_freq == skip_interval) {
                tinfo_ivars->skip_filepos = OutStream_Tell(skip_stream);
            }
            // Write deltas.
            last_skip_doc               = skip_stepper_ivars->doc_id;
            last_skip_filepos           = skip_stepper_ivars->filepos;
            skip_stepper_ivars->doc_id  = post_ivars->doc_id;
            PostWriter_Update_Skip_Info(post_writer, skip_tinfo);
            skip_stepper_ivars->filepos = skip_tinfo_ivars->post_filepos;
            SkipStepper_Write_Record(skip_stepper, skip_stream,
                                     last_skip_doc, last_skip_filepos);
        }

        // Retrieve the next posting from the sort pool.
        // DECREF(posting);  // No!!  DON'T destroy!!!

        posting = (RawPosting*)PostPool_Fetch(self);
        post_ivars = RawPost_IVARS(posting);
    }

    // Clean up.
    DECREF(last_term_text);
    DECREF(skip_tinfo);
    DECREF(tinfo);
}

uint32_t
PostPool_Refill_IMP(PostingPool *self) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    Lexicon *const     lexicon     = ivars->lexicon;
    PostingList *const plist       = ivars->plist;
    I32Array    *const doc_map     = ivars->doc_map;
    const uint32_t     mem_thresh  = ivars->mem_thresh;
    const int32_t      doc_base    = ivars->doc_base;
    uint32_t           num_elems   = 0; // number of items recovered
    String            *term_text   = NULL;

    if (ivars->lexicon == NULL) { return 0; }
    else { term_text = (String*)Lex_Get_Term(lexicon); }

    // Make sure buffer is empty.
    if (ivars->buf_max - ivars->buf_tick > 0) {
        THROW(ERR, "Refill called but buffer contains %u32 items",
              ivars->buf_max - ivars->buf_tick);
    }
    ivars->buf_max  = 0;
    ivars->buf_tick = 0;

    // Ditch old MemoryPool and get another.
    DECREF(ivars->mem_pool);
    ivars->mem_pool = MemPool_new(0);
    MemoryPool *const mem_pool = ivars->mem_pool;
    MemoryPoolIVARS *const mem_pool_ivars = MemPool_IVARS(mem_pool);


    while (1) {
        if (ivars->post_count == 0) {
            // Read a term.
            if (Lex_Next(lexicon)) {
                ivars->post_count = Lex_Doc_Freq(lexicon);
                term_text = (String*)Lex_Get_Term(lexicon);
                if (term_text && !Obj_Is_A((Obj*)term_text, STRING)) {
                    THROW(ERR, "Only String terms are supported for now");
                }
                Posting *posting = PList_Get_Posting(plist);
                Post_Set_Doc_ID(posting, doc_base);
                ivars->last_doc_id = doc_base;
            }
            // Bail if we've read everything in this run.
            else {
                break;
            }
        }

        // Bail if we've hit the ceiling for this run's buffer.
        if (mem_pool_ivars->consumed >= mem_thresh && num_elems > 0) {
            break;
        }

        // Read a posting from the input stream.
        RawPosting *rawpost
            = PList_Read_Raw(plist, ivars->last_doc_id, term_text, mem_pool);
        RawPostingIVARS *const rawpost_ivars = RawPost_IVARS(rawpost);
        ivars->last_doc_id = rawpost_ivars->doc_id;
        ivars->post_count--;

        // Skip deletions.
        if (doc_map != NULL) {
            const int32_t remapped
                = I32Arr_Get(doc_map, rawpost_ivars->doc_id - doc_base);
            if (!remapped) {
                continue;
            }
            rawpost_ivars->doc_id = remapped;
        }

        // Add to the run's buffer.
        if (num_elems >= ivars->buf_cap) {
            size_t new_cap = Memory_oversize(num_elems + 1, sizeof(Obj*));
            PostPool_Grow_Buffer(self, new_cap);
        }
        ivars->buffer[num_elems] = (Obj*)rawpost;
        num_elems++;
    }

    // Reset the buffer array position and length; remember file pos.
    ivars->buf_max   = num_elems;
    ivars->buf_tick  = 0;

    return num_elems;
}

void
PostPool_Add_Inversion_IMP(PostingPool *self, Inversion *inversion,
                           int32_t doc_id, float doc_boost,
                           float length_norm) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    Post_Add_Inversion_To_Pool(ivars->posting, self, inversion, ivars->type,
                               doc_id, doc_boost, length_norm);
}

static void
S_fresh_flip(PostingPool *self, InStream *lex_temp_in,
             InStream *post_temp_in) {
    PostingPoolIVARS *const ivars = PostPool_IVARS(self);
    if (ivars->flipped) { THROW(ERR, "Can't Flip twice"); }
    ivars->flipped = true;

    // Sort RawPostings in buffer, if any.
    PostPool_Sort_Buffer(self);

    // Bail if never flushed.
    if (ivars->lex_end == 0) { return; }

    // Get a Lexicon.
    String *lex_alias = Str_newf("%o-%i64-to-%i64",
                                 InStream_Get_Filename(lex_temp_in),
                                 ivars->lex_start, ivars->lex_end);
    InStream *lex_temp_in_dupe = InStream_Reopen(
                                     lex_temp_in, lex_alias, ivars->lex_start,
                                     ivars->lex_end - ivars->lex_start);
    ivars->lexicon = (Lexicon*)RawLex_new(
                        ivars->schema, ivars->field, lex_temp_in_dupe, 0,
                        ivars->lex_end - ivars->lex_start);
    DECREF(lex_alias);
    DECREF(lex_temp_in_dupe);

    // Get a PostingList.
    String *post_alias
        = Str_newf("%o-%i64-to-%i64", InStream_Get_Filename(post_temp_in),
                  ivars->post_start, ivars->post_end);
    InStream *post_temp_in_dupe
        = InStream_Reopen(post_temp_in, post_alias, ivars->post_start,
                          ivars->post_end - ivars->post_start);
    ivars->plist
        = (PostingList*)RawPList_new(ivars->schema, ivars->field,
                                     post_temp_in_dupe, 0,
                                     ivars->post_end - ivars->post_start);
    DECREF(post_alias);
    DECREF(post_temp_in_dupe);
}


