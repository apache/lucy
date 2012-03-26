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

#define C_LUCY_TOPDOCS
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/TopDocs.h"
#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/Lexicon.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

TopDocs*
TopDocs_new(VArray *match_docs, uint32_t total_hits) {
    TopDocs *self = (TopDocs*)VTable_Make_Obj(TOPDOCS);
    return TopDocs_init(self, match_docs, total_hits);
}

TopDocs*
TopDocs_init(TopDocs *self, VArray *match_docs, uint32_t total_hits) {
    self->match_docs = (VArray*)INCREF(match_docs);
    self->total_hits = total_hits;
    return self;
}

void
TopDocs_destroy(TopDocs *self) {
    DECREF(self->match_docs);
    SUPER_DESTROY(self, TOPDOCS);
}

void
TopDocs_serialize(TopDocs *self, OutStream *outstream) {
    VA_Serialize(self->match_docs, outstream);
    OutStream_Write_C32(outstream, self->total_hits);
}

TopDocs*
TopDocs_deserialize(TopDocs *self, InStream *instream) {
    self = self ? self : (TopDocs*)VTable_Make_Obj(TOPDOCS);
    self->match_docs = VA_deserialize(NULL, instream);
    self->total_hits = InStream_Read_C32(instream);
    return self;
}

VArray*
TopDocs_get_match_docs(TopDocs *self) {
    return self->match_docs;
}

uint32_t
TopDocs_get_total_hits(TopDocs *self) {
    return self->total_hits;
}

void
TopDocs_set_match_docs(TopDocs *self, VArray *match_docs) {
    DECREF(self->match_docs);
    self->match_docs = (VArray*)INCREF(match_docs);
}
void
TopDocs_set_total_hits(TopDocs *self, uint32_t total_hits) {
    self->total_hits = total_hits;
}


