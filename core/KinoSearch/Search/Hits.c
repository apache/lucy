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

#define C_KINO_HITS
#define C_KINO_MATCHDOC
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/Hits.h"
#include "KinoSearch/Document/HitDoc.h"
#include "KinoSearch/Search/Query.h"
#include "KinoSearch/Search/MatchDoc.h"
#include "KinoSearch/Search/Searcher.h"
#include "KinoSearch/Search/TopDocs.h"

Hits*
Hits_new(Searcher *searcher, TopDocs *top_docs, uint32_t offset)
{
    Hits *self = (Hits*)VTable_Make_Obj(HITS);
    return Hits_init(self, searcher, top_docs, offset);
}

Hits*
Hits_init(Hits *self, Searcher *searcher, TopDocs *top_docs, uint32_t offset)
{
    self->searcher = (Searcher*)INCREF(searcher);
    self->top_docs   = (TopDocs*)INCREF(top_docs);
    self->match_docs = (VArray*)INCREF(TopDocs_Get_Match_Docs(top_docs));
    self->offset     = offset;
    return self;
}

void
Hits_destroy(Hits *self)
{
    DECREF(self->searcher);
    DECREF(self->top_docs);
    DECREF(self->match_docs);
    SUPER_DESTROY(self, HITS);
}

HitDoc*
Hits_next(Hits *self)
{
    MatchDoc *match_doc = (MatchDoc*)VA_Fetch(self->match_docs, self->offset);
    self->offset++;

    if (!match_doc) {
        /** Bail if there aren't any more *captured* hits. (There may be more
         * total hits.) */
        return NULL;
    }
    else {
        // Lazily fetch HitDoc, set score. 
        HitDoc *doc = Searcher_Fetch_Doc(self->searcher,
            match_doc->doc_id, match_doc->score);

        return doc;
    }
}

uint32_t
Hits_total_hits(Hits *self)
{
    return TopDocs_Get_Total_Hits(self->top_docs);
}


