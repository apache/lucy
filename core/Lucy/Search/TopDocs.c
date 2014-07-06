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
#include "Lucy/Util/Freezer.h"

TopDocs*
TopDocs_new(VArray *match_docs, uint32_t total_hits) {
    TopDocs *self = (TopDocs*)Class_Make_Obj(TOPDOCS);
    return TopDocs_init(self, match_docs, total_hits);
}

TopDocs*
TopDocs_init(TopDocs *self, VArray *match_docs, uint32_t total_hits) {
    TopDocsIVARS *const ivars = TopDocs_IVARS(self);
    ivars->match_docs = (VArray*)INCREF(match_docs);
    ivars->total_hits = total_hits;
    return self;
}

void
TopDocs_Destroy_IMP(TopDocs *self) {
    TopDocsIVARS *const ivars = TopDocs_IVARS(self);
    DECREF(ivars->match_docs);
    SUPER_DESTROY(self, TOPDOCS);
}

void
TopDocs_Serialize_IMP(TopDocs *self, OutStream *outstream) {
    TopDocsIVARS *const ivars = TopDocs_IVARS(self);
    Freezer_serialize_varray(ivars->match_docs, outstream);
    OutStream_Write_C32(outstream, ivars->total_hits);
}

TopDocs*
TopDocs_Deserialize_IMP(TopDocs *self, InStream *instream) {
    TopDocsIVARS *const ivars = TopDocs_IVARS(self);
    ivars->match_docs = Freezer_read_varray(instream);
    ivars->total_hits = InStream_Read_C32(instream);
    return self;
}

VArray*
TopDocs_Get_Match_Docs_IMP(TopDocs *self) {
    return TopDocs_IVARS(self)->match_docs;
}

uint32_t
TopDocs_Get_Total_Hits_IMP(TopDocs *self) {
    return TopDocs_IVARS(self)->total_hits;
}

void
TopDocs_Set_Match_Docs_IMP(TopDocs *self, VArray *match_docs) {
    TopDocsIVARS *const ivars = TopDocs_IVARS(self);
    DECREF(ivars->match_docs);
    ivars->match_docs = (VArray*)INCREF(match_docs);
}
void
TopDocs_Set_Total_Hits_IMP(TopDocs *self, uint32_t total_hits) {
    TopDocs_IVARS(self)->total_hits = total_hits;
}


