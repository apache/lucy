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

#define C_LUCY_SEARCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Searcher.h"

#include "Lucy/Index/DocVector.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Collector.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/NoMatchQuery.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/QueryParser.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/TopDocs.h"
#include "Lucy/Search/Compiler.h"

Searcher*
Searcher_init(Searcher *self, Schema *schema) {
    SearcherIVARS *const ivars = Searcher_IVARS(self);
    ivars->schema  = (Schema*)INCREF(schema);
    ivars->qparser = NULL;
    ABSTRACT_CLASS_CHECK(self, SEARCHER);
    return self;
}

void
Searcher_Destroy_IMP(Searcher *self) {
    SearcherIVARS *const ivars = Searcher_IVARS(self);
    DECREF(ivars->schema);
    DECREF(ivars->qparser);
    SUPER_DESTROY(self, SEARCHER);
}

Hits*
Searcher_Hits_IMP(Searcher *self, Obj *query, uint32_t offset,
                  uint32_t num_wanted, SortSpec *sort_spec) {
    Query   *real_query = Searcher_Glean_Query(self, query);
    uint32_t doc_max    = Searcher_Doc_Max(self);
    uint32_t wanted     = offset + num_wanted > doc_max
                          ? doc_max
                          : offset + num_wanted;
    TopDocs *top_docs   = Searcher_Top_Docs(self, real_query, wanted,
                                            sort_spec);
    Hits    *hits       = Hits_new(self, top_docs, offset);
    DECREF(top_docs);
    DECREF(real_query);
    return hits;
}

Query*
Searcher_Glean_Query_IMP(Searcher *self, Obj *query) {
    SearcherIVARS *const ivars = Searcher_IVARS(self);
    Query *real_query = NULL;

    if (!query) {
        real_query = (Query*)NoMatchQuery_new();
    }
    else if (Obj_Is_A(query, QUERY)) {
        real_query = (Query*)INCREF(query);
    }
    else if (Obj_Is_A(query, STRING)) {
        if (!ivars->qparser) {
            ivars->qparser = QParser_new(ivars->schema, NULL, NULL, NULL);
        }
        real_query = QParser_Parse(ivars->qparser, (String*)query);
    }
    else {
        THROW(ERR, "Invalid type for 'query' param: %o",
              Obj_Get_Class_Name(query));
    }

    return real_query;
}

Schema*
Searcher_Get_Schema_IMP(Searcher *self) {
    return Searcher_IVARS(self)->schema;
}

void
Searcher_Close_IMP(Searcher *self) {
    UNUSED_VAR(self);
}


