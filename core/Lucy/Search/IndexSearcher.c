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
    IndexSearcher *self = (IndexSearcher*)Class_Make_Obj(INDEXSEARCHER);
    return IxSearcher_init(self, index);
}

IndexSearcher*
IxSearcher_init(IndexSearcher *self, Obj *index) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    if (Obj_Is_A(index, INDEXREADER)) {
        ivars->reader = (IndexReader*)INCREF(index);
    }
    else {
        ivars->reader = IxReader_open(index, NULL, NULL);
    }
    Searcher_init((Searcher*)self, IxReader_Get_Schema(ivars->reader));
    ivars->seg_readers = IxReader_Seg_Readers(ivars->reader);
    ivars->seg_starts  = IxReader_Offsets(ivars->reader);
    ivars->doc_reader = (DocReader*)IxReader_Fetch(
                           ivars->reader, Class_Get_Name(DOCREADER));
    ivars->hl_reader = (HighlightReader*)IxReader_Fetch(
                          ivars->reader, Class_Get_Name(HIGHLIGHTREADER));
    if (ivars->doc_reader) { INCREF(ivars->doc_reader); }
    if (ivars->hl_reader)  { INCREF(ivars->hl_reader); }

    return self;
}

void
IxSearcher_Destroy_IMP(IndexSearcher *self) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    DECREF(ivars->reader);
    DECREF(ivars->doc_reader);
    DECREF(ivars->hl_reader);
    DECREF(ivars->seg_readers);
    DECREF(ivars->seg_starts);
    SUPER_DESTROY(self, INDEXSEARCHER);
}

HitDoc*
IxSearcher_Fetch_Doc_IMP(IndexSearcher *self, int32_t doc_id) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    if (!ivars->doc_reader) { THROW(ERR, "No DocReader"); }
    return DocReader_Fetch_Doc(ivars->doc_reader, doc_id);
}

DocVector*
IxSearcher_Fetch_Doc_Vec_IMP(IndexSearcher *self, int32_t doc_id) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    if (!ivars->hl_reader) { THROW(ERR, "No HighlightReader"); }
    return HLReader_Fetch_Doc_Vec(ivars->hl_reader, doc_id);
}

int32_t
IxSearcher_Doc_Max_IMP(IndexSearcher *self) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    return IxReader_Doc_Max(ivars->reader);
}

uint32_t
IxSearcher_Doc_Freq_IMP(IndexSearcher *self, String *field, Obj *term) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    LexiconReader *lex_reader
        = (LexiconReader*)IxReader_Fetch(ivars->reader,
                                         Class_Get_Name(LEXICONREADER));
    return lex_reader ? LexReader_Doc_Freq(lex_reader, field, term) : 0;
}

TopDocs*
IxSearcher_Top_Docs_IMP(IndexSearcher *self, Query *query, uint32_t num_wanted,
                        SortSpec *sort_spec) {
    Schema        *schema    = IxSearcher_Get_Schema(self);
    uint32_t       doc_max   = IxSearcher_Doc_Max(self);
    uint32_t       wanted    = num_wanted > doc_max ? doc_max : num_wanted;
    SortCollector *collector = SortColl_new(schema, sort_spec, wanted);
    IxSearcher_Collect(self, query, (Collector*)collector);
    VArray  *match_docs = SortColl_Pop_Match_Docs(collector);
    int32_t  total_hits = SortColl_Get_Total_Hits(collector);
    TopDocs *retval     = TopDocs_new(match_docs, total_hits);
    DECREF(collector);
    DECREF(match_docs);
    return retval;
}

void
IxSearcher_Collect_IMP(IndexSearcher *self, Query *query, Collector *collector) {
    IndexSearcherIVARS *const ivars = IxSearcher_IVARS(self);
    VArray   *const seg_readers = ivars->seg_readers;
    I32Array *const seg_starts  = ivars->seg_starts;
    bool      need_score        = Coll_Need_Score(collector);
    Compiler *compiler = Query_Is_A(query, COMPILER)
                         ? (Compiler*)INCREF(query)
                         : Query_Make_Compiler(query, (Searcher*)self,
                                               Query_Get_Boost(query), false);

    // Accumulate hits into the Collector.
    for (uint32_t i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(seg_readers, i);
        DeletionsReader *del_reader = (DeletionsReader*)SegReader_Fetch(
                                          seg_reader,
                                          Class_Get_Name(DELETIONSREADER));
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
IxSearcher_Get_Reader_IMP(IndexSearcher *self) {
    return IxSearcher_IVARS(self)->reader;
}

void
IxSearcher_Close_IMP(IndexSearcher *self) {
    UNUSED_VAR(self);
}


