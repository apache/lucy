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
RawPList_new(Schema *schema, const CharBuf *field, InStream *instream,
             int64_t start, int64_t end) {
    RawPostingList *self = (RawPostingList*)VTable_Make_Obj(RAWPOSTINGLIST);
    return RawPList_init(self, schema, field, instream, start, end);
}

RawPostingList*
RawPList_init(RawPostingList *self, Schema *schema, const CharBuf *field,
              InStream *instream, int64_t start, int64_t end) {
    PList_init((PostingList*)self);
    self->start     = start;
    self->end       = end;
    self->len       = end - start;
    self->instream  = (InStream*)INCREF(instream);
    Similarity *sim = Schema_Fetch_Sim(schema, field);
    self->posting   = Sim_Make_Posting(sim);
    InStream_Seek(self->instream, self->start);
    return self;
}

void
RawPList_destroy(RawPostingList *self) {
    DECREF(self->instream);
    DECREF(self->posting);
    SUPER_DESTROY(self, RAWPOSTINGLIST);
}

Posting*
RawPList_get_posting(RawPostingList *self) {
    return self->posting;
}

RawPosting*
RawPList_read_raw(RawPostingList *self, int32_t last_doc_id, CharBuf *term_text,
                  MemoryPool *mem_pool) {
    return Post_Read_Raw(self->posting, self->instream,
                         last_doc_id, term_text, mem_pool);
}


