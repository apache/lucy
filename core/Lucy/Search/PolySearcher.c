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

#define C_LUCY_POLYSEARCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/PolySearcher.h"

#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Collector.h"
#include "Lucy/Search/HitQueue.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/TopDocs.h"
#include "Lucy/Search/Compiler.h"

PolySearcher*
PolySearcher_init(PolySearcher *self, Schema *schema, VArray *searchers) {
    const uint32_t num_searchers = VA_Get_Size(searchers);
    int32_t *starts_array = (int32_t*)MALLOCATE(num_searchers * sizeof(int32_t));
    int32_t  doc_max      = 0;

    Searcher_init((Searcher*)self, schema);
    self->searchers = (VArray*)INCREF(searchers);
    self->starts = NULL; // Safe cleanup.

    for (uint32_t i = 0; i < num_searchers; i++) {
        Searcher *searcher
            = (Searcher*)CERTIFY(VA_Fetch(searchers, i), SEARCHER);
        Schema *candidate    = Searcher_Get_Schema(searcher);
        VTable *orig_vt      = Schema_Get_VTable(schema);
        VTable *candidate_vt = Schema_Get_VTable(candidate);

        // Confirm that searchers all use the same schema.
        if (orig_vt != candidate_vt) {
            THROW(ERR, "Conflicting schemas: '%o', '%o'",
                  Schema_Get_Class_Name(schema),
                  Schema_Get_Class_Name(candidate));
        }

        // Derive doc_max and relative start offsets.
        starts_array[i] = (int32_t)doc_max;
        doc_max += Searcher_Doc_Max(searcher);
    }

    self->doc_max = doc_max;
    self->starts  = I32Arr_new_steal(starts_array, num_searchers);

    return self;
}

void
PolySearcher_destroy(PolySearcher *self) {
    DECREF(self->searchers);
    DECREF(self->starts);
    SUPER_DESTROY(self, POLYSEARCHER);
}

HitDoc*
PolySearcher_fetch_doc(PolySearcher *self, int32_t doc_id) {
    uint32_t  tick     = PolyReader_sub_tick(self->starts, doc_id);
    Searcher *searcher = (Searcher*)VA_Fetch(self->searchers, tick);
    int32_t   offset   = I32Arr_Get(self->starts, tick);
    if (!searcher) { THROW(ERR, "Invalid doc id: %i32", doc_id); }
    HitDoc *hit_doc = Searcher_Fetch_Doc(searcher, doc_id - offset);
    HitDoc_Set_Doc_ID(hit_doc, doc_id);
    return hit_doc;
}

DocVector*
PolySearcher_fetch_doc_vec(PolySearcher *self, int32_t doc_id) {
    uint32_t  tick     = PolyReader_sub_tick(self->starts, doc_id);
    Searcher *searcher = (Searcher*)VA_Fetch(self->searchers, tick);
    int32_t   start    = I32Arr_Get(self->starts, tick);
    if (!searcher) { THROW(ERR, "Invalid doc id: %i32", doc_id); }
    return Searcher_Fetch_Doc_Vec(searcher, doc_id - start);
}

int32_t
PolySearcher_doc_max(PolySearcher *self) {
    return self->doc_max;
}

uint32_t
PolySearcher_doc_freq(PolySearcher *self, const CharBuf *field, Obj *term) {
    uint32_t doc_freq = 0;
    for (uint32_t i = 0, max = VA_Get_Size(self->searchers); i < max; i++) {
        Searcher *searcher = (Searcher*)VA_Fetch(self->searchers, i);
        doc_freq += Searcher_Doc_Freq(searcher, field, term);
    }
    return doc_freq;
}

static void
S_modify_doc_ids(VArray *match_docs, int32_t base) {
    for (uint32_t i = 0, max = VA_Get_Size(match_docs); i < max; i++) {
        MatchDoc *match_doc = (MatchDoc*)VA_Fetch(match_docs, i);
        int32_t  new_doc_id = MatchDoc_Get_Doc_ID(match_doc) + base;
        MatchDoc_Set_Doc_ID(match_doc, new_doc_id);
    }
}

TopDocs*
PolySearcher_top_docs(PolySearcher *self, Query *query, uint32_t num_wanted,
                      SortSpec *sort_spec) {
    Schema   *schema      = PolySearcher_Get_Schema(self);
    VArray   *searchers   = self->searchers;
    I32Array *starts      = self->starts;
    HitQueue *hit_q       = sort_spec
                            ? HitQ_new(schema, sort_spec, num_wanted)
                            : HitQ_new(NULL, NULL, num_wanted);
    uint32_t  total_hits  = 0;
    Compiler *compiler    = Query_Is_A(query, COMPILER)
                            ? ((Compiler*)INCREF(query))
                            : Query_Make_Compiler(query, (Searcher*)self,
                                                  Query_Get_Boost(query),
                                                  false);

    for (uint32_t i = 0, max = VA_Get_Size(searchers); i < max; i++) {
        Searcher   *searcher   = (Searcher*)VA_Fetch(searchers, i);
        int32_t     base       = I32Arr_Get(starts, i);
        TopDocs    *top_docs   = Searcher_Top_Docs(searcher, (Query*)compiler,
                                                   num_wanted, sort_spec);
        VArray     *sub_match_docs = TopDocs_Get_Match_Docs(top_docs);

        total_hits += TopDocs_Get_Total_Hits(top_docs);

        S_modify_doc_ids(sub_match_docs, base);
        for (uint32_t j = 0, jmax = VA_Get_Size(sub_match_docs); j < jmax; j++) {
            MatchDoc *match_doc = (MatchDoc*)VA_Fetch(sub_match_docs, j);
            if (!HitQ_Insert(hit_q, INCREF(match_doc))) { break; }
        }

        DECREF(top_docs);
    }

    VArray  *match_docs = HitQ_Pop_All(hit_q);
    TopDocs *retval     = TopDocs_new(match_docs, total_hits);

    DECREF(match_docs);
    DECREF(compiler);
    DECREF(hit_q);
    return retval;
}


void
PolySearcher_collect(PolySearcher *self, Query *query,
                     Collector *collector) {
    VArray *const searchers = self->searchers;
    I32Array *starts = self->starts;

    for (uint32_t i = 0, max = VA_Get_Size(searchers); i < max; i++) {
        int32_t start = I32Arr_Get(starts, i);
        Searcher *searcher = (Searcher*)VA_Fetch(searchers, i);
        OffsetCollector *offset_coll = OffsetColl_new(collector, start);
        Searcher_Collect(searcher, query, (Collector*)offset_coll);
        DECREF(offset_coll);
    }
}


