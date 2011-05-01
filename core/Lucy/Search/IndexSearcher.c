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

#define C_LUCY_INDEXSEARCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/IndexSearcher.h"

#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/LexiconReader.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Index/HighlightReader.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Collector.h"
#include "Lucy/Search/Collector/SortCollector.h"
#include "Lucy/Search/HitQueue.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/TopDocs.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/FSFolder.h"

IndexSearcher*
IxSearcher_new(Obj *index) {
    IndexSearcher *self = (IndexSearcher*)VTable_Make_Obj(INDEXSEARCHER);
    return IxSearcher_init(self, index);
}

IndexSearcher*
IxSearcher_init(IndexSearcher *self, Obj *index) {
    if (Obj_Is_A(index, INDEXREADER)) {
        self->reader = (IndexReader*)INCREF(index);
    }
    else {
        self->reader = IxReader_open(index, NULL, NULL);
    }
    Searcher_init((Searcher*)self, IxReader_Get_Schema(self->reader));
    self->seg_readers = IxReader_Seg_Readers(self->reader);
    self->seg_starts  = IxReader_Offsets(self->reader);
    self->doc_reader = (DocReader*)IxReader_Fetch(
                           self->reader, VTable_Get_Name(DOCREADER));
    self->hl_reader = (HighlightReader*)IxReader_Fetch(
                          self->reader, VTable_Get_Name(HIGHLIGHTREADER));
    if (self->doc_reader) { INCREF(self->doc_reader); }
    if (self->hl_reader)  { INCREF(self->hl_reader); }

    return self;
}

void
IxSearcher_destroy(IndexSearcher *self) {
    DECREF(self->reader);
    DECREF(self->doc_reader);
    DECREF(self->hl_reader);
    DECREF(self->seg_readers);
    DECREF(self->seg_starts);
    SUPER_DESTROY(self, INDEXSEARCHER);
}

HitDoc*
IxSearcher_fetch_doc(IndexSearcher *self, int32_t doc_id) {
    if (!self->doc_reader) { THROW(ERR, "No DocReader"); }
    return DocReader_Fetch_Doc(self->doc_reader, doc_id);
}

DocVector*
IxSearcher_fetch_doc_vec(IndexSearcher *self, int32_t doc_id) {
    if (!self->hl_reader) { THROW(ERR, "No HighlightReader"); }
    return HLReader_Fetch_Doc_Vec(self->hl_reader, doc_id);
}

int32_t
IxSearcher_doc_max(IndexSearcher *self) {
    return IxReader_Doc_Max(self->reader);
}

uint32_t
IxSearcher_doc_freq(IndexSearcher *self, const CharBuf *field, Obj *term) {
    LexiconReader *lex_reader
        = (LexiconReader*)IxReader_Fetch(self->reader,
                                         VTable_Get_Name(LEXICONREADER));
    return lex_reader ? LexReader_Doc_Freq(lex_reader, field, term) : 0;
}

TopDocs*
IxSearcher_top_docs(IndexSearcher *self, Query *query, uint32_t num_wanted,
                    SortSpec *sort_spec) {
    Schema        *schema    = IxSearcher_Get_Schema(self);
    uint32_t       doc_max   = IxSearcher_Doc_Max(self);
    uint32_t       wanted    = num_wanted > doc_max ? doc_max : num_wanted;
    SortCollector *collector = SortColl_new(schema, sort_spec, wanted);
    IxSearcher_Collect(self, query, (Collector*)collector);
    {
        VArray  *match_docs = SortColl_Pop_Match_Docs(collector);
        int32_t  total_hits = SortColl_Get_Total_Hits(collector);
        TopDocs *retval     = TopDocs_new(match_docs, total_hits);
        DECREF(collector);
        DECREF(match_docs);
        return retval;
    }
}

void
IxSearcher_collect(IndexSearcher *self, Query *query, Collector *collector) {
    uint32_t i, max;
    VArray   *const seg_readers = self->seg_readers;
    I32Array *const seg_starts  = self->seg_starts;
    bool_t    need_score        = Coll_Need_Score(collector);
    Compiler *compiler = Query_Is_A(query, COMPILER)
                         ? (Compiler*)INCREF(query)
                         : Query_Make_Compiler(query, (Searcher*)self,
                                               Query_Get_Boost(query));

    // Accumulate hits into the Collector.
    for (i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(seg_readers, i);
        DeletionsReader *del_reader = (DeletionsReader*)SegReader_Fetch(
                                          seg_reader,
                                          VTable_Get_Name(DELETIONSREADER));
        Matcher *matcher
            = Compiler_Make_Matcher(compiler, seg_reader, need_score);
        if (matcher) {
            int32_t  seg_start = I32Arr_Get(seg_starts, i);
            Matcher *deletions = DelReader_Iterator(del_reader);
            Coll_Set_Reader(collector, seg_reader);
            Coll_Set_Base(collector, seg_start);
            Coll_Set_Matcher(collector, matcher);
            Matcher_Collect(matcher, collector, deletions);
            DECREF(deletions);
            DECREF(matcher);
        }
    }

    DECREF(compiler);
}

IndexReader*
IxSearcher_get_reader(IndexSearcher *self) {
    return self->reader;
}

void
IxSearcher_close(IndexSearcher *self) {
    UNUSED_VAR(self);
}


