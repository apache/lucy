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

#define C_LUCY_DELETIONSWRITER
#define C_LUCY_DEFAULTDELETIONSWRITER
#include "Lucy/Util/ToolSet.h"

#include <math.h>

#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/BitVecMatcher.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"

DeletionsWriter*
DelWriter_init(DeletionsWriter *self, Schema *schema, Snapshot *snapshot,
               Segment *segment, PolyReader *polyreader) {
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    ABSTRACT_CLASS_CHECK(self, DELETIONSWRITER);
    return self;
}

I32Array*
DelWriter_Generate_Doc_Map_IMP(DeletionsWriter *self, Matcher *deletions,
                               int32_t doc_max, int32_t offset) {
    int32_t *doc_map = (int32_t*)CALLOCATE(doc_max + 1, sizeof(int32_t));
    int32_t  next_deletion = deletions ? Matcher_Next(deletions) : INT32_MAX;
    UNUSED_VAR(self);

    // 0 for a deleted doc, a new number otherwise
    for (int32_t i = 1, new_doc_id = 1; i <= doc_max; i++) {
        if (i == next_deletion) {
            next_deletion = Matcher_Next(deletions);
        }
        else {
            doc_map[i] = offset + new_doc_id++;
        }
    }

    return I32Arr_new_steal(doc_map, doc_max + 1);
}

int32_t DefDelWriter_current_file_format = 1;

DefaultDeletionsWriter*
DefDelWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                 PolyReader *polyreader) {
    DefaultDeletionsWriter *self
        = (DefaultDeletionsWriter*)Class_Make_Obj(DEFAULTDELETIONSWRITER);
    return DefDelWriter_init(self, schema, snapshot, segment, polyreader);
}

DefaultDeletionsWriter*
DefDelWriter_init(DefaultDeletionsWriter *self, Schema *schema,
                  Snapshot *snapshot, Segment *segment,
                  PolyReader *polyreader) {

    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    ivars->seg_readers          = PolyReader_Seg_Readers(polyreader);
    uint32_t num_seg_readers    = VA_Get_Size(ivars->seg_readers);
    ivars->seg_starts           = PolyReader_Offsets(polyreader);
    ivars->bit_vecs             = VA_new(num_seg_readers);
    ivars->updated              = (bool*)CALLOCATE(num_seg_readers, sizeof(bool));
    ivars->searcher             = IxSearcher_new((Obj*)polyreader);
    ivars->name_to_tick         = Hash_new(num_seg_readers);

    // Materialize a BitVector of deletions for each segment.
    for (uint32_t i = 0; i < num_seg_readers; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->seg_readers, i);
        BitVector *bit_vec    = BitVec_new(SegReader_Doc_Max(seg_reader));
        DeletionsReader *del_reader
            = (DeletionsReader*)SegReader_Fetch(
                  seg_reader, Class_Get_Name(DELETIONSREADER));
        Matcher *seg_dels = del_reader
                            ? DelReader_Iterator(del_reader)
                            : NULL;

        if (seg_dels) {
            int32_t del;
            while (0 != (del = Matcher_Next(seg_dels))) {
                BitVec_Set(bit_vec, del);
            }
            DECREF(seg_dels);
        }
        VA_Store(ivars->bit_vecs, i, (Obj*)bit_vec);
        Hash_Store(ivars->name_to_tick,
                   (Obj*)SegReader_Get_Seg_Name(seg_reader),
                   (Obj*)Int32_new(i));
    }

    return self;
}

void
DefDelWriter_Destroy_IMP(DefaultDeletionsWriter *self) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    DECREF(ivars->seg_readers);
    DECREF(ivars->seg_starts);
    DECREF(ivars->bit_vecs);
    DECREF(ivars->searcher);
    DECREF(ivars->name_to_tick);
    FREEMEM(ivars->updated);
    SUPER_DESTROY(self, DEFAULTDELETIONSWRITER);
}

static String*
S_del_filename(DefaultDeletionsWriter *self, SegReader *target_reader) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    Segment *target_seg = SegReader_Get_Segment(target_reader);
    return Str_newf("%o/deletions-%o.bv", Seg_Get_Name(ivars->segment),
                    Seg_Get_Name(target_seg));
}

void
DefDelWriter_Finish_IMP(DefaultDeletionsWriter *self) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    Folder *const folder = ivars->folder;

    for (uint32_t i = 0, max = VA_Get_Size(ivars->seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->seg_readers, i);
        if (ivars->updated[i]) {
            BitVector *deldocs   = (BitVector*)VA_Fetch(ivars->bit_vecs, i);
            int32_t    doc_max   = SegReader_Doc_Max(seg_reader);
            double     used      = (doc_max + 1) / 8.0;
            uint32_t   byte_size = (uint32_t)ceil(used);
            uint32_t   new_max   = byte_size * 8 - 1;
            String    *filename  = S_del_filename(self, seg_reader);
            OutStream *outstream = Folder_Open_Out(folder, filename);
            if (!outstream) { RETHROW(INCREF(Err_get_error())); }

            // Ensure that we have 1 bit for each doc in segment.
            BitVec_Grow(deldocs, new_max);

            // Write deletions data and clean up.
            OutStream_Write_Bytes(outstream,
                                  (char*)BitVec_Get_Raw_Bits(deldocs),
                                  byte_size);
            OutStream_Close(outstream);
            DECREF(outstream);
            DECREF(filename);
        }
    }

    Seg_Store_Metadata_Utf8(ivars->segment, "deletions", 9,
                            (Obj*)DefDelWriter_Metadata(self));
}

Hash*
DefDelWriter_Metadata_IMP(DefaultDeletionsWriter *self) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    DefDelWriter_Metadata_t super_meta
        = (DefDelWriter_Metadata_t)SUPER_METHOD_PTR(DEFAULTDELETIONSWRITER,
                                                    LUCY_DefDelWriter_Metadata);
    Hash    *const metadata = super_meta(self);
    Hash    *const files    = Hash_new(0);

    for (uint32_t i = 0, max = VA_Get_Size(ivars->seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->seg_readers, i);
        if (ivars->updated[i]) {
            BitVector *deldocs   = (BitVector*)VA_Fetch(ivars->bit_vecs, i);
            Segment   *segment   = SegReader_Get_Segment(seg_reader);
            Hash      *mini_meta = Hash_new(2);
            Hash_Store_Utf8(mini_meta, "count", 5,
                            (Obj*)Str_newf("%u32", (uint32_t)BitVec_Count(deldocs)));
            Hash_Store_Utf8(mini_meta, "filename", 8,
                            (Obj*)S_del_filename(self, seg_reader));
            Hash_Store(files, (Obj*)Seg_Get_Name(segment), (Obj*)mini_meta);
        }
    }
    Hash_Store_Utf8(metadata, "files", 5, (Obj*)files);

    return metadata;
}

int32_t
DefDelWriter_Format_IMP(DefaultDeletionsWriter *self) {
    UNUSED_VAR(self);
    return DefDelWriter_current_file_format;
}

Matcher*
DefDelWriter_Seg_Deletions_IMP(DefaultDeletionsWriter *self,
                               SegReader *seg_reader) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    Matcher *deletions    = NULL;
    Segment *segment      = SegReader_Get_Segment(seg_reader);
    String  *seg_name     = Seg_Get_Name(segment);
    Integer32 *tick_obj   = (Integer32*)Hash_Fetch(ivars->name_to_tick,
                                                   (Obj*)seg_name);
    int32_t tick          = tick_obj ? Int32_Get_Value(tick_obj) : 0;
    SegReader *candidate  = tick_obj
                            ? (SegReader*)VA_Fetch(ivars->seg_readers, tick)
                            : NULL;

    if (tick_obj) {
        DeletionsReader *del_reader
            = (DeletionsReader*)SegReader_Obtain(
                  candidate, Class_Get_Name(DELETIONSREADER));
        if (ivars->updated[tick] || DelReader_Del_Count(del_reader)) {
            BitVector *deldocs = (BitVector*)VA_Fetch(ivars->bit_vecs, tick);
            deletions = (Matcher*)BitVecMatcher_new(deldocs);
        }
    }
    else { // Sanity check.
        THROW(ERR, "Couldn't find SegReader %o", seg_reader);
    }

    return deletions;
}

int32_t
DefDelWriter_Seg_Del_Count_IMP(DefaultDeletionsWriter *self,
                               String *seg_name) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    Integer32 *tick
        = (Integer32*)Hash_Fetch(ivars->name_to_tick, (Obj*)seg_name);
    BitVector *deldocs = tick
                         ? (BitVector*)VA_Fetch(ivars->bit_vecs, Int32_Get_Value(tick))
                         : NULL;
    return deldocs ? BitVec_Count(deldocs) : 0;
}

void
DefDelWriter_Delete_By_Term_IMP(DefaultDeletionsWriter *self,
                                String *field, Obj *term) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->seg_readers, i);
        PostingListReader *plist_reader
            = (PostingListReader*)SegReader_Fetch(
                  seg_reader, Class_Get_Name(POSTINGLISTREADER));
        BitVector *bit_vec = (BitVector*)VA_Fetch(ivars->bit_vecs, i);
        PostingList *plist = plist_reader
                             ? PListReader_Posting_List(plist_reader, field, term)
                             : NULL;
        int32_t doc_id;
        int32_t num_zapped = 0;

        // Iterate through postings, marking each doc as deleted.
        if (plist) {
            while (0 != (doc_id = PList_Next(plist))) {
                num_zapped += !BitVec_Get(bit_vec, doc_id);
                BitVec_Set(bit_vec, doc_id);
            }
            if (num_zapped) { ivars->updated[i] = true; }
            DECREF(plist);
        }
    }
}

void
DefDelWriter_Delete_By_Query_IMP(DefaultDeletionsWriter *self, Query *query) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    Compiler *compiler = Query_Make_Compiler(query, (Searcher*)ivars->searcher,
                                             Query_Get_Boost(query), false);

    for (uint32_t i = 0, max = VA_Get_Size(ivars->seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->seg_readers, i);
        BitVector *bit_vec = (BitVector*)VA_Fetch(ivars->bit_vecs, i);
        Matcher *matcher = Compiler_Make_Matcher(compiler, seg_reader, false);

        if (matcher) {
            int32_t doc_id;
            int32_t num_zapped = 0;

            // Iterate through matches, marking each doc as deleted.
            while (0 != (doc_id = Matcher_Next(matcher))) {
                num_zapped += !BitVec_Get(bit_vec, doc_id);
                BitVec_Set(bit_vec, doc_id);
            }
            if (num_zapped) { ivars->updated[i] = true; }

            DECREF(matcher);
        }
    }

    DECREF(compiler);
}

void
DefDelWriter_Delete_By_Doc_ID_IMP(DefaultDeletionsWriter *self, int32_t doc_id) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    uint32_t   sub_tick   = PolyReader_sub_tick(ivars->seg_starts, doc_id);
    BitVector *bit_vec    = (BitVector*)VA_Fetch(ivars->bit_vecs, sub_tick);
    uint32_t   offset     = I32Arr_Get(ivars->seg_starts, sub_tick);
    int32_t    seg_doc_id = doc_id - offset;

    if (!BitVec_Get(bit_vec, seg_doc_id)) {
        ivars->updated[sub_tick] = true;
        BitVec_Set(bit_vec, seg_doc_id);
    }
}

bool
DefDelWriter_Updated_IMP(DefaultDeletionsWriter *self) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->seg_readers); i < max; i++) {
        if (ivars->updated[i]) { return true; }
    }
    return false;
}

void
DefDelWriter_Add_Segment_IMP(DefaultDeletionsWriter *self, SegReader *reader,
                             I32Array *doc_map) {
    // This method is a no-op, because the only reason it would be called is
    // if we are adding an entire index.  If that's the case, all deletes are
    // already being applied.
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(doc_map);
}

void
DefDelWriter_Merge_Segment_IMP(DefaultDeletionsWriter *self,
                               SegReader *reader, I32Array *doc_map) {
    DefaultDeletionsWriterIVARS *const ivars = DefDelWriter_IVARS(self);
    UNUSED_VAR(doc_map);
    Segment *segment = SegReader_Get_Segment(reader);
    Hash *del_meta = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "deletions", 9);

    if (del_meta) {
        VArray *seg_readers = ivars->seg_readers;
        Hash   *files = (Hash*)Hash_Fetch_Utf8(del_meta, "files", 5);
        if (files) {
            String *seg;
            Hash *mini_meta;
            Hash_Iterate(files);
            while (Hash_Next(files, (Obj**)&seg, (Obj**)&mini_meta)) {

                /* Find the segment the deletions from the SegReader
                 * we're adding correspond to.  If it's gone, we don't
                 * need to worry about losing deletions files that point
                 * at it. */
                for (uint32_t i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
                    SegReader *candidate
                        = (SegReader*)VA_Fetch(seg_readers, i);
                    String *candidate_name
                        = Seg_Get_Name(SegReader_Get_Segment(candidate));

                    if (Str_Equals(seg, (Obj*)candidate_name)) {
                        /* If the count hasn't changed, we're about to
                         * merge away the most recent deletions file
                         * pointing at this target segment -- so force a
                         * new file to be written out. */
                        int32_t count = (int32_t)Obj_To_I64(Hash_Fetch_Utf8(mini_meta, "count", 5));
                        DeletionsReader *del_reader
                            = (DeletionsReader*)SegReader_Obtain(
                                  candidate, Class_Get_Name(DELETIONSREADER));
                        if (count == DelReader_Del_Count(del_reader)) {
                            ivars->updated[i] = true;
                        }
                        break;
                    }
                }
            }
        }
    }
}


