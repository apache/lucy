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

#define C_LUCY_RAWPOSTINGLIST
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/RawPostingList.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/MemoryPool.h"

RawPostingList*
RawPList_new(Schema *schema, String *field, InStream *instream,
             int64_t start, int64_t end) {
    RawPostingList *self = (RawPostingList*)Class_Make_Obj(RAWPOSTINGLIST);
    return RawPList_init(self, schema, field, instream, start, end);
}

RawPostingList*
RawPList_init(RawPostingList *self, Schema *schema, String *field,
              InStream *instream, int64_t start, int64_t end) {
    PList_init((PostingList*)self);
    RawPostingListIVARS *const ivars = RawPList_IVARS(self);
    ivars->start     = start;
    ivars->end       = end;
    ivars->len       = end - start;
    ivars->instream  = (InStream*)INCREF(instream);
    Similarity *sim  = Schema_Fetch_Sim(schema, field);
    ivars->posting   = Sim_Make_Posting(sim);
    InStream_Seek(ivars->instream, ivars->start);
    return self;
}

void
RawPList_Destroy_IMP(RawPostingList *self) {
    RawPostingListIVARS *const ivars = RawPList_IVARS(self);
    DECREF(ivars->instream);
    DECREF(ivars->posting);
    SUPER_DESTROY(self, RAWPOSTINGLIST);
}

Posting*
RawPList_Get_Posting_IMP(RawPostingList *self) {
    return RawPList_IVARS(self)->posting;
}

RawPosting*
RawPList_Read_Raw_IMP(RawPostingList *self, int32_t last_doc_id,
                      String *term_text, MemoryPool *mem_pool) {
    RawPostingListIVARS *const ivars = RawPList_IVARS(self);
    return Post_Read_Raw(ivars->posting, ivars->instream,
                         last_doc_id, term_text, mem_pool);
}


