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
             PolyReader *polyreader,  const CharBuf *field,
             LexiconWriter *lex_writer, MemoryPool *mem_pool,
             OutStream *lex_temp_out, OutStream *post_temp_out,
             OutStream *skip_out) {
    PostingPool *self = (PostingPool*)VTable_Make_Obj(POSTINGPOOL);
    return PostPool_init(self, schema, snapshot, segment, polyreader, field,
                         lex_writer, mem_pool, lex_temp_out, post_temp_out,
                         skip_out);
}

PostingPool*
PostPool_init(PostingPool *self, Schema *schema, Snapshot *snapshot,
              Segment *segment, PolyReader *polyreader, const CharBuf *field,
              LexiconWriter *lex_writer, MemoryPool *mem_pool,
              OutStream *lex_temp_out, OutStream *post_temp_out,
              OutStream *skip_out) {
    // Init.
    SortEx_init((SortExternal*)self, sizeof(Obj*));
    self->doc_base         = 0;
    self->last_doc_id      = 0;
    self->doc_map          = NULL;
    self->post_count       = 0;
    self->lexicon          = NULL;
    self->plist            = NULL;
    self->lex_temp_in      = NULL;
    self->post_temp_in     = NULL;
    self->lex_start        = I64_MAX;
    self->post_start       = I64_MAX;
    self->lex_end          = 0;
    self->post_end         = 0;
    self->skip_stepper     = SkipStepper_new();

    // Assign.
    self->schema         = (Schema*)INCREF(schema);
    self->snapshot       = (Snapshot*)INCREF(snapshot);
    self->segment        = (Segment*)INCREF(segment);
    self->polyreader     = (PolyReader*)INCREF(polyreader);
    self->lex_writer     = (LexiconWriter*)INCREF(lex_writer);
    self->mem_pool       = (MemoryPool*)INCREF(mem_pool);
    self->field          = CB_Clone(field);
    self->lex_temp_out   = (OutStream*)INCREF(lex_temp_out);
    self->post_temp_out  = (OutStream*)INCREF(post_temp_out);
    self->skip_out       = (OutStream*)INCREF(skip_out);

    // Derive.
    Similarity *sim = Schema_Fetch_Sim(schema, field);
    self->posting   = Sim_Make_Posting(sim);
    self->type      = (FieldType*)INCREF(Schema_Fetch_Type(schema, field));
    self->field_num = Seg_Field_Num(segment, field);

    return self;
}

void
PostPool_destroy(PostingPool *self) {
    DECREF(self->schema);
    DECREF(self->snapshot);
    DECREF(self->segment);
    DECREF(self->polyreader);
    DECREF(self->lex_writer);
    DECREF(self->mem_pool);
    DECREF(self->field);
    DECREF(self->doc_map);
    DECREF(self->lexicon);
    DECREF(self->plist);
    DECREF(self->lex_temp_out);
    DECREF(self->post_temp_out);
    DECREF(self->skip_out);
    DECREF(self->lex_temp_in);
    DECREF(self->post_temp_in);
    DECREF(self->posting);
    DECREF(self->skip_stepper);
    DECREF(self->type);
    SUPER_DESTROY(self, POSTINGPOOL);
}

int
PostPool_compare(PostingPool *self, void *va, void *vb) {
    RawPosting *const a     = *(RawPosting**)va;
    RawPosting *const b     = *(RawPosting**)vb;
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
PostPool_get_mem_pool(PostingPool *self) {
    return self->mem_pool;
}

void
PostPool_flip(PostingPool *self) {
    uint32_t num_runs   = VA_Get_Size(self->runs);
    uint32_t sub_thresh = num_runs > 0
                          ? self->mem_thresh / num_runs
                          : self->mem_thresh;

    if (num_runs) {
        Folder  *folder = PolyReader_Get_Folder(self->polyreader);
        CharBuf *seg_name = Seg_Get_Name(self->segment);
        CharBuf *lex_temp_path  = CB_newf("%o/lextemp", seg_name);
        CharBuf *post_temp_path = CB_newf("%o/ptemp", seg_name);
        self->lex_temp_in = Folder_Open_In(folder, lex_temp_path);
        if (!self->lex_temp_in) {
            RETHROW(INCREF(Err_get_error()));
        }
        self->post_temp_in = Folder_Open_In(folder, post_temp_path);
        if (!self->post_temp_in) {
            RETHROW(INCREF(Err_get_error()));
        }
        DECREF(lex_temp_path);
        DECREF(post_temp_path);
    }

    PostPool_Sort_Cache(self);
    if (num_runs && (self->cache_max - self->cache_tick) > 0) {
        uint32_t num_items = PostPool_Cache_Count(self);
        // Cheap imitation of flush. FIXME.
        PostingPool *run
            = PostPool_new(self->schema, self->snapshot, self->segment,
                           self->polyreader, self->field, self->lex_writer,
                           self->mem_pool, self->lex_temp_out,
                           self->post_temp_out, self->skip_out);
        PostPool_Grow_Cache(run, num_items);
        memcpy(run->cache, ((Obj**)self->cache) + self->cache_tick,
               num_items * sizeof(Obj*));
        run->cache_max = num_items;
        PostPool_Add_Run(self, (SortExternal*)run);
        self->cache_tick = 0;
        self->cache_max = 0;
    }

    // Assign.
    for (uint32_t i = 0; i < num_runs; i++) {
        PostingPool *run = (PostingPool*)VA_Fetch(self->runs, i);
        if (run != NULL) {
            PostPool_Set_Mem_Thresh(run, sub_thresh);
            if (!run->lexicon) {
                S_fresh_flip(run, self->lex_temp_in, self->post_temp_in);
            }
        }
    }

    self->flipped = true;
}

void
PostPool_add_segment(PostingPool *self, SegReader *reader, I32Array *doc_map,
                     int32_t doc_base) {
    LexiconReader *lex_reader = (LexiconReader*)SegReader_Fetch(
                                    reader, VTable_Get_Name(LEXICONREADER));
    Lexicon *lexicon = lex_reader
                       ? LexReader_Lexicon(lex_reader, self->field, NULL)
                       : NULL;

    if (lexicon) {
        PostingListReader *plist_reader
            = (PostingListReader*)SegReader_Fetch(
                  reader, VTable_Get_Name(POSTINGLISTREADER));
        PostingList *plist = plist_reader
                             ? PListReader_Posting_List(plist_reader, self->field, NULL)
                             : NULL;
        if (!plist) {
            THROW(ERR, "Got a Lexicon but no PostingList for '%o' in '%o'",
                  self->field, SegReader_Get_Seg_Name(reader));
        }
        PostingPool *run
            = PostPool_new(self->schema, self->snapshot, self->segment,
                           self->polyreader, self->field, self->lex_writer,
                           self->mem_pool, self->lex_temp_out,
                           self->post_temp_out, self->skip_out);
        run->lexicon  = lexicon;
        run->plist    = plist;
        run->doc_base = doc_base;
        run->doc_map  = (I32Array*)INCREF(doc_map);
        PostPool_Add_Run(self, (SortExternal*)run);
    }
}

void
PostPool_shrink(PostingPool *self) {
    if (self->cache_max - self->cache_tick > 0) {
        size_t cache_count = PostPool_Cache_Count(self);
        size_t size        = cache_count * sizeof(Obj*);
        if (self->cache_tick > 0) {
            Obj **start = ((Obj**)self->cache) + self->cache_tick;
            memmove(self->cache, start, size);
        }
        self->cache      = (uint8_t*)REALLOCATE(self->cache, size);
        self->cache_tick = 0;
        self->cache_max  = cache_count;
        self->cache_cap  = cache_count;
    }
    else {
        FREEMEM(self->cache);
        self->cache      = NULL;
        self->cache_tick = 0;
        self->cache_max  = 0;
        self->cache_cap  = 0;
    }
    self->scratch_cap = 0;
    FREEMEM(self->scratch);
    self->scratch = NULL;

    // It's not necessary to iterate over the runs, because they don't have
    // any cache costs until Refill() gets called.
}

void
PostPool_flush(PostingPool *self) {
    // Don't add a run unless we have data to put in it.
    if (PostPool_Cache_Count(self) == 0) { return; }

    PostingPool *run
        = PostPool_new(self->schema, self->snapshot, self->segment,
                       self->polyreader, self->field, self->lex_writer,
                       self->mem_pool, self->lex_temp_out,
                       self->post_temp_out, self->skip_out);
    PostingWriter *post_writer
        = (PostingWriter*)RawPostWriter_new(self->schema, self->snapshot,
                                            self->segment, self->polyreader,
                                            self->post_temp_out);

    // Borrow the cache.
    run->cache      = self->cache;
    run->cache_tick = self->cache_tick;
    run->cache_max  = self->cache_max;
    run->cache_cap  = self->cache_cap;

    // Write to temp files.
    LexWriter_Enter_Temp_Mode(self->lex_writer, self->field,
                              self->lex_temp_out);
    run->lex_start  = OutStream_Tell(self->lex_temp_out);
    run->post_start = OutStream_Tell(self->post_temp_out);
    PostPool_Sort_Cache(self);
    S_write_terms_and_postings(run, post_writer, NULL);

    run->lex_end  = OutStream_Tell(self->lex_temp_out);
    run->post_end = OutStream_Tell(self->post_temp_out);
    LexWriter_Leave_Temp_Mode(self->lex_writer);

    // Return the cache and empty it.
    run->cache      = NULL;
    run->cache_tick = 0;
    run->cache_max  = 0;
    run->cache_cap  = 0;
    PostPool_Clear_Cache(self);

    // Add the run to the array.
    PostPool_Add_Run(self, (SortExternal*)run);

    DECREF(post_writer);
}

void
PostPool_finish(PostingPool *self) {
    // Bail if there's no data.
    if (!PostPool_Peek(self)) { return; }

    Similarity *sim = Schema_Fetch_Sim(self->schema, self->field);
    PostingWriter *post_writer
        = Sim_Make_Posting_Writer(sim, self->schema, self->snapshot,
                                  self->segment, self->polyreader,
                                  self->field_num);
    LexWriter_Start_Field(self->lex_writer, self->field_num);
    S_write_terms_and_postings(self, post_writer, self->skip_out);
    LexWriter_Finish_Field(self->lex_writer, self->field_num);
    DECREF(post_writer);
}

static void
S_write_terms_and_postings(PostingPool *self, PostingWriter *post_writer,
                           OutStream *skip_stream) {
    TermInfo      *const tinfo          = TInfo_new(0);
    TermInfo      *const skip_tinfo     = TInfo_new(0);
    CharBuf       *const last_term_text = CB_new(0);
    LexiconWriter *const lex_writer     = self->lex_writer;
    SkipStepper   *const skip_stepper   = self->skip_stepper;
    int32_t        last_doc_id          = 0;
    int32_t        last_skip_doc        = 0;
    int64_t        last_skip_filepos    = 0;
    const int32_t  skip_interval
        = Arch_Skip_Interval(Schema_Get_Architecture(self->schema));

    // Prime heldover variables.
    RawPosting *posting = (RawPosting*)CERTIFY(
                              (*(RawPosting**)PostPool_Fetch(self)),
                              RAWPOSTING);
    CB_Mimic_Str(last_term_text, posting->blob, posting->content_len);
    char *last_text_buf = (char*)CB_Get_Ptr8(last_term_text);
    uint32_t last_text_size = CB_Get_Size(last_term_text);
    SkipStepper_Set_ID_And_Filepos(skip_stepper, 0, 0);

    while (1) {
        bool_t same_text_as_last = true;

        if (posting == NULL) {
            // On the last iter, use an empty string to make LexiconWriter
            // DTRT.
            posting = &RAWPOSTING_BLANK;
            same_text_as_last = false;
        }
        else {
            // Compare once.
            if (posting->content_len != last_text_size
                || memcmp(&posting->blob, last_text_buf, last_text_size) != 0
               ) {
                same_text_as_last = false;
            }
        }

        // If the term text changes, process the last term.
        if (!same_text_as_last) {
            // Hand off to LexiconWriter.
            LexWriter_Add_Term(lex_writer, last_term_text, tinfo);

            // Start each term afresh.
            TInfo_Reset(tinfo);
            PostWriter_Start_Term(post_writer, tinfo);

            // Init skip data in preparation for the next term.
            skip_stepper->doc_id  = 0;
            skip_stepper->filepos = tinfo->post_filepos;
            last_skip_doc         = 0;
            last_skip_filepos     = tinfo->post_filepos;

            // Remember the term_text so we can write string diffs.
            CB_Mimic_Str(last_term_text, posting->blob,
                         posting->content_len);
            last_text_buf  = (char*)CB_Get_Ptr8(last_term_text);
            last_text_size = CB_Get_Size(last_term_text);

            // Starting a new term, thus a new delta doc sequence at 0.
            last_doc_id = 0;
        }

        // Bail on last iter before writing invalid posting data.
        if (posting == &RAWPOSTING_BLANK) { break; }

        // Write posting data.
        PostWriter_Write_Posting(post_writer, posting);

        // Doc freq lags by one iter.
        tinfo->doc_freq++;

        //  Write skip data.
        if (skip_stream != NULL
            && same_text_as_last
            && tinfo->doc_freq % skip_interval == 0
            && tinfo->doc_freq != 0
           ) {
            // If first skip group, save skip stream pos for term info.
            if (tinfo->doc_freq == skip_interval) {
                tinfo->skip_filepos = OutStream_Tell(skip_stream);
            }
            // Write deltas.
            last_skip_doc         = skip_stepper->doc_id;
            last_skip_filepos     = skip_stepper->filepos;
            skip_stepper->doc_id  = posting->doc_id;
            PostWriter_Update_Skip_Info(post_writer, skip_tinfo);
            skip_stepper->filepos = skip_tinfo->post_filepos;
            SkipStepper_Write_Record(skip_stepper, skip_stream,
                                     last_skip_doc, last_skip_filepos);
        }

        // Remember last doc id because we need it for delta encoding.
        last_doc_id = posting->doc_id;

        // Retrieve the next posting from the sort pool.
        // DECREF(posting);  // No!!  DON'T destroy!!!

        void *address = PostPool_Fetch(self);
        posting = address
                  ? *(RawPosting**)address
                  : NULL;
    }

    // Clean up.
    DECREF(last_term_text);
    DECREF(skip_tinfo);
    DECREF(tinfo);
}

uint32_t
PostPool_refill(PostingPool *self) {
    Lexicon *const     lexicon     = self->lexicon;
    PostingList *const plist       = self->plist;
    I32Array    *const doc_map     = self->doc_map;
    const uint32_t     mem_thresh  = self->mem_thresh;
    const int32_t      doc_base    = self->doc_base;
    uint32_t           num_elems   = 0; // number of items recovered
    MemoryPool        *mem_pool;
    CharBuf           *term_text   = NULL;

    if (self->lexicon == NULL) { return 0; }
    else { term_text = (CharBuf*)Lex_Get_Term(lexicon); }

    // Make sure cache is empty.
    if (self->cache_max - self->cache_tick > 0) {
        THROW(ERR, "Refill called but cache contains %u32 items",
              self->cache_max - self->cache_tick);
    }
    self->cache_max  = 0;
    self->cache_tick = 0;

    // Ditch old MemoryPool and get another.
    DECREF(self->mem_pool);
    self->mem_pool = MemPool_new(0);
    mem_pool       = self->mem_pool;

    while (1) {
        RawPosting *raw_posting;

        if (self->post_count == 0) {
            // Read a term.
            if (Lex_Next(lexicon)) {
                self->post_count = Lex_Doc_Freq(lexicon);
                term_text = (CharBuf*)Lex_Get_Term(lexicon);
                if (term_text && !Obj_Is_A((Obj*)term_text, CHARBUF)) {
                    THROW(ERR, "Only CharBuf terms are supported for now");
                }
                Posting *posting = PList_Get_Posting(plist);
                Post_Set_Doc_ID(posting, doc_base);
                self->last_doc_id = doc_base;
            }
            // Bail if we've read everything in this run.
            else {
                break;
            }
        }

        // Bail if we've hit the ceiling for this run's cache.
        if (mem_pool->consumed >= mem_thresh && num_elems > 0) {
            break;
        }

        // Read a posting from the input stream.
        raw_posting = PList_Read_Raw(plist, self->last_doc_id, term_text,
                                     mem_pool);
        self->last_doc_id = raw_posting->doc_id;
        self->post_count--;

        // Skip deletions.
        if (doc_map != NULL) {
            const int32_t remapped
                = I32Arr_Get(doc_map, raw_posting->doc_id - doc_base);
            if (!remapped) {
                continue;
            }
            raw_posting->doc_id = remapped;
        }

        // Add to the run's cache.
        if (num_elems >= self->cache_cap) {
            size_t new_cap = Memory_oversize(num_elems + 1, sizeof(Obj*));
            PostPool_Grow_Cache(self, new_cap);
        }
        Obj **cache = (Obj**)self->cache;
        cache[num_elems] = (Obj*)raw_posting;
        num_elems++;
    }

    // Reset the cache array position and length; remember file pos.
    self->cache_max   = num_elems;
    self->cache_tick  = 0;

    return num_elems;
}

void
PostPool_add_inversion(PostingPool *self, Inversion *inversion, int32_t doc_id,
                       float doc_boost, float length_norm) {
    Post_Add_Inversion_To_Pool(self->posting, self, inversion, self->type,
                               doc_id, doc_boost, length_norm);
}

static void
S_fresh_flip(PostingPool *self, InStream *lex_temp_in,
             InStream *post_temp_in) {
    if (self->flipped) { THROW(ERR, "Can't Flip twice"); }
    self->flipped = true;

    // Sort RawPostings in cache, if any.
    PostPool_Sort_Cache(self);

    // Bail if never flushed.
    if (self->lex_end == 0) { return; }

    // Get a Lexicon.
    CharBuf *lex_alias = CB_newf("%o-%i64-to-%i64",
                                 InStream_Get_Filename(lex_temp_in),
                                 self->lex_start, self->lex_end);
    InStream *lex_temp_in_dupe = InStream_Reopen(
                                     lex_temp_in, lex_alias, self->lex_start,
                                     self->lex_end - self->lex_start);
    self->lexicon = (Lexicon*)RawLex_new(
                        self->schema, self->field, lex_temp_in_dupe, 0,
                        self->lex_end - self->lex_start);
    DECREF(lex_alias);
    DECREF(lex_temp_in_dupe);

    // Get a PostingList.
    CharBuf *post_alias
        = CB_newf("%o-%i64-to-%i64", InStream_Get_Filename(post_temp_in),
                  self->post_start, self->post_end);
    InStream *post_temp_in_dupe
        = InStream_Reopen(post_temp_in, post_alias, self->post_start,
                          self->post_end - self->post_start);
    self->plist
        = (PostingList*)RawPList_new(self->schema, self->field,
                                     post_temp_in_dupe, 0,
                                     self->post_end - self->post_start);
    DECREF(post_alias);
    DECREF(post_temp_in_dupe);
}


