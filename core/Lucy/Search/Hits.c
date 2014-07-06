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

#define C_LUCY_HITS
#define C_LUCY_MATCHDOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Hits.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/TopDocs.h"

Hits*
Hits_new(Searcher *searcher, TopDocs *top_docs, uint32_t offset) {
    Hits *self = (Hits*)Class_Make_Obj(HITS);
    return Hits_init(self, searcher, top_docs, offset);
}

Hits*
Hits_init(Hits *self, Searcher *searcher, TopDocs *top_docs, uint32_t offset) {
    HitsIVARS *const ivars = Hits_IVARS(self);
    ivars->searcher   = (Searcher*)INCREF(searcher);
    ivars->top_docs   = (TopDocs*)INCREF(top_docs);
    ivars->match_docs = (VArray*)INCREF(TopDocs_Get_Match_Docs(top_docs));
    ivars->offset     = offset;
    return self;
}

void
Hits_Destroy_IMP(Hits *self) {
    HitsIVARS *const ivars = Hits_IVARS(self);
    DECREF(ivars->searcher);
    DECREF(ivars->top_docs);
    DECREF(ivars->match_docs);
    SUPER_DESTROY(self, HITS);
}

HitDoc*
Hits_Next_IMP(Hits *self) {
    HitsIVARS *const ivars = Hits_IVARS(self);
    MatchDoc *match_doc = (MatchDoc*)VA_Fetch(ivars->match_docs, ivars->offset);
    ivars->offset++;

    if (!match_doc) {
        /** Bail if there aren't any more *captured* hits. (There may be more
         * total hits.) */
        return NULL;
    }
    else {
        // Lazily fetch HitDoc, set score.
        MatchDocIVARS *match_doc_ivars = MatchDoc_IVARS(match_doc);
        HitDoc *hit_doc = Searcher_Fetch_Doc(ivars->searcher,
                                             match_doc_ivars->doc_id);
        HitDoc_Set_Score(hit_doc, match_doc_ivars->score);
        return hit_doc;
    }
}

uint32_t
Hits_Total_Hits_IMP(Hits *self) {
    HitsIVARS *const ivars = Hits_IVARS(self);
    return TopDocs_Get_Total_Hits(ivars->top_docs);
}


