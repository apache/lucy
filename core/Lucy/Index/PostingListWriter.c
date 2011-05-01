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

#define C_LUCY_POSTINGLISTWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PostingListWriter.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/PostingPool.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/LexiconWriter.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/MemoryPool.h"

static size_t default_mem_thresh = 0x1000000;

int32_t PListWriter_current_file_format = 1;

// Open streams only if content gets added.
static void
S_lazy_init(PostingListWriter *self);

// Return the PostingPool for this field, creating one if necessary.
static PostingPool*
S_lazy_init_posting_pool(PostingListWriter *self, int32_t field_num);

PostingListWriter*
PListWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                PolyReader *polyreader, LexiconWriter *lex_writer) {
    PostingListWriter *self
        = (PostingListWriter*)VTable_Make_Obj(POSTINGLISTWRITER);
    return PListWriter_init(self, schema, snapshot, segment, polyreader,
                            lex_writer);
}

PostingListWriter*
PListWriter_init(PostingListWriter *self, Schema *schema, Snapshot *snapshot,
                 Segment *segment, PolyReader *polyreader,
                 LexiconWriter *lex_writer) {
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);

    // Assign.
    self->lex_writer = (LexiconWriter*)INCREF(lex_writer);

    // Init.
    self->pools          = VA_new(Schema_Num_Fields(schema));
    self->mem_thresh     = default_mem_thresh;
    self->mem_pool       = MemPool_new(0);
    self->lex_temp_out   = NULL;
    self->post_temp_out  = NULL;

    return self;
}

static void
S_lazy_init(PostingListWriter *self) {
    if (!self->lex_temp_out) {
        Folder  *folder         = self->folder;
        CharBuf *seg_name       = Seg_Get_Name(self->segment);
        CharBuf *lex_temp_path  = CB_newf("%o/lextemp", seg_name);
        CharBuf *post_temp_path = CB_newf("%o/ptemp", seg_name);
        CharBuf *skip_path      = CB_newf("%o/postings.skip", seg_name);

        // Open temp streams and final skip stream.
        self->lex_temp_out  = Folder_Open_Out(folder, lex_temp_path);
        if (!self->lex_temp_out) { RETHROW(INCREF(Err_get_error())); }
        self->post_temp_out = Folder_Open_Out(folder, post_temp_path);
        if (!self->post_temp_out) { RETHROW(INCREF(Err_get_error())); }
        self->skip_out = Folder_Open_Out(folder, skip_path);
        if (!self->skip_out) { RETHROW(INCREF(Err_get_error())); }

        DECREF(skip_path);
        DECREF(post_temp_path);
        DECREF(lex_temp_path);
    }
}

static PostingPool*
S_lazy_init_posting_pool(PostingListWriter *self, int32_t field_num) {
    PostingPool *pool = (PostingPool*)VA_Fetch(self->pools, field_num);
    if (!pool && field_num != 0) {
        CharBuf *field = Seg_Field_Name(self->segment, field_num);
        pool = PostPool_new(self->schema, self->snapshot, self->segment,
                            self->polyreader, field, self->lex_writer,
                            self->mem_pool, self->lex_temp_out,
                            self->post_temp_out, self->skip_out);
        VA_Store(self->pools, field_num, (Obj*)pool);
    }
    return pool;
}

void
PListWriter_destroy(PostingListWriter *self) {
    DECREF(self->lex_writer);
    DECREF(self->mem_pool);
    DECREF(self->pools);
    DECREF(self->lex_temp_out);
    DECREF(self->post_temp_out);
    DECREF(self->skip_out);
    SUPER_DESTROY(self, POSTINGLISTWRITER);
}

void
PListWriter_set_default_mem_thresh(size_t mem_thresh) {
    default_mem_thresh = mem_thresh;
}

int32_t
PListWriter_format(PostingListWriter *self) {
    UNUSED_VAR(self);
    return PListWriter_current_file_format;
}

void
PListWriter_add_inverted_doc(PostingListWriter *self, Inverter *inverter,
                             int32_t doc_id) {
    S_lazy_init(self);

    // Iterate over fields in document, adding the content of indexed fields
    // to their respective PostingPools.
    float doc_boost = Inverter_Get_Boost(inverter);
    Inverter_Iterate(inverter);
    int32_t field_num;
    while (0 != (field_num = Inverter_Next(inverter))) {
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Indexed(type)) {
            Inversion   *inversion = Inverter_Get_Inversion(inverter);
            Similarity  *sim  = Inverter_Get_Similarity(inverter);
            PostingPool *pool = S_lazy_init_posting_pool(self, field_num);
            float length_norm
                = Sim_Length_Norm(sim, Inversion_Get_Size(inversion));
            PostPool_Add_Inversion(pool, inversion, doc_id, doc_boost,
                                   length_norm);
        }
    }

    // If our PostingPools have collectively passed the memory threshold,
    // flush all of them, then release all the RawPostings with a single
    // action.
    if (MemPool_Get_Consumed(self->mem_pool) > self->mem_thresh) {
        for (uint32_t i = 0, max = VA_Get_Size(self->pools); i < max; i++) {
            PostingPool *const pool = (PostingPool*)VA_Fetch(self->pools, i);
            if (pool) { PostPool_Flush(pool); }
        }
        MemPool_Release_All(self->mem_pool);
    }
}

void
PListWriter_add_segment(PostingListWriter *self, SegReader *reader,
                        I32Array *doc_map) {
    Segment *other_segment = SegReader_Get_Segment(reader);
    Schema  *schema        = self->schema;
    Segment *segment       = self->segment;
    VArray  *all_fields    = Schema_All_Fields(schema);
    S_lazy_init(self);

    for (uint32_t i = 0, max = VA_Get_Size(all_fields); i < max; i++) {
        CharBuf   *field = (CharBuf*)VA_Fetch(all_fields, i);
        FieldType *type  = Schema_Fetch_Type(schema, field);
        int32_t old_field_num = Seg_Field_Num(other_segment, field);
        int32_t new_field_num = Seg_Field_Num(segment, field);

        if (!FType_Indexed(type)) { continue; }
        if (!old_field_num)       { continue; } // not in old segment
        if (!new_field_num) {
            THROW(ERR, "Unrecognized field: %o", field);
        }

        PostingPool *pool = S_lazy_init_posting_pool(self, new_field_num);
        PostPool_Add_Segment(pool, reader, doc_map,
                             (int32_t)Seg_Get_Count(segment));
    }

    // Clean up.
    DECREF(all_fields);
}

void
PListWriter_finish(PostingListWriter *self) {
    // If S_lazy_init was never called, we have no data, so bail out.
    if (!self->lex_temp_out) { return; }

    Folder  *folder = self->folder;
    CharBuf *seg_name = Seg_Get_Name(self->segment);
    CharBuf *lex_temp_path  = CB_newf("%o/lextemp", seg_name);
    CharBuf *post_temp_path = CB_newf("%o/ptemp", seg_name);

    // Close temp streams.
    OutStream_Close(self->lex_temp_out);
    OutStream_Close(self->post_temp_out);

    // Try to free up some memory.
    for (uint32_t i = 0, max = VA_Get_Size(self->pools); i < max; i++) {
        PostingPool *pool = (PostingPool*)VA_Fetch(self->pools, i);
        if (pool) { PostPool_Shrink(pool); }
    }

    // Write postings for each field.
    for (uint32_t i = 0, max = VA_Get_Size(self->pools); i < max; i++) {
        PostingPool *pool = (PostingPool*)VA_Delete(self->pools, i);
        if (pool) {
            // Write out content for each PostingPool.  Let each PostingPool
            // use more RAM while finishing.  (This is a little dicy, because if
            // Shrink() was ineffective, we may double the RAM footprint.)
            PostPool_Set_Mem_Thresh(pool, self->mem_thresh);
            PostPool_Flip(pool);
            PostPool_Finish(pool);
            DECREF(pool);
        }
    }

    // Store metadata.
    Seg_Store_Metadata_Str(self->segment, "postings", 8,
                           (Obj*)PListWriter_Metadata(self));

    // Close down and clean up.
    OutStream_Close(self->skip_out);
    if (!Folder_Delete(folder, lex_temp_path)) {
        THROW(ERR, "Couldn't delete %o", lex_temp_path);
    }
    if (!Folder_Delete(folder, post_temp_path)) {
        THROW(ERR, "Couldn't delete %o", post_temp_path);
    }
    DECREF(self->skip_out);
    self->skip_out = NULL;
    DECREF(post_temp_path);
    DECREF(lex_temp_path);

    // Dispatch the LexiconWriter.
    LexWriter_Finish(self->lex_writer);
}


