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

#define C_LUCY_RAWPOSTING
#define C_LUCY_RAWPOSTINGWRITER
#define C_LUCY_TERMINFO
#include "Lucy/Util/ToolSet.h"

#include <string.h>

#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/OutStream.h"
#include "Clownfish/Util/StringHelper.h"

RawPosting*
RawPost_new(void *pre_allocated_memory, int32_t doc_id, uint32_t freq,
            const char *term_text, size_t term_text_len) {
    RawPosting *self
        = (RawPosting*)Class_Init_Obj(RAWPOSTING, pre_allocated_memory);
    RawPostingIVARS *const ivars = RawPost_IVARS(self);
    ivars->doc_id      = doc_id;
    ivars->freq        = freq;
    ivars->content_len = term_text_len;
    ivars->aux_len     = 0;
    memcpy(&ivars->blob, term_text, term_text_len);

    return self;
}

void
RawPost_Destroy_IMP(RawPosting *self) {
    UNUSED_VAR(self);
    THROW(ERR, "Illegal attempt to destroy RawPosting object");
}

int32_t
RawPost_Compare_To_IMP(RawPosting *self, Obj *other) {
    RawPostingIVARS *const ivars = RawPost_IVARS(self);
    RawPostingIVARS *const ovars = RawPost_IVARS((RawPosting*)other);
    const size_t my_len    = ivars->content_len;
    const size_t other_len = ovars->content_len;
    const size_t len       = my_len < other_len ? my_len : other_len;
    int32_t comparison = memcmp(ivars->blob, ovars->blob, len);

    if (comparison == 0) {
        // If a is a substring of b, it's less than b, so return a neg num.
        comparison = (int32_t)((int64_t)my_len - (int64_t)other_len);

        // Break ties by doc id.
        if (comparison == 0) {
            comparison = ivars->doc_id - ovars->doc_id;
        }
    }

    return comparison;
}

uint32_t
RawPost_Get_RefCount_IMP(RawPosting* self) {
    UNUSED_VAR(self);
    return 1;
}

RawPosting*
RawPost_Inc_RefCount_IMP(RawPosting* self) {
    return self;
}

uint32_t
RawPost_Dec_RefCount_IMP(RawPosting* self) {
    UNUSED_VAR(self);
    return 1;
}

/***************************************************************************/

RawPostingWriter*
RawPostWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                  PolyReader *polyreader, OutStream *outstream) {
    RawPostingWriter *self
        = (RawPostingWriter*)Class_Make_Obj(RAWPOSTINGWRITER);
    return RawPostWriter_init(self, schema, snapshot, segment, polyreader,
                              outstream);
}

RawPostingWriter*
RawPostWriter_init(RawPostingWriter *self, Schema *schema,
                   Snapshot *snapshot, Segment *segment,
                   PolyReader *polyreader, OutStream *outstream) {
    const int32_t invalid_field_num = 0;
    PostWriter_init((PostingWriter*)self, schema, snapshot, segment,
                    polyreader, invalid_field_num);
    RawPostingWriterIVARS *const ivars = RawPostWriter_IVARS(self);
    ivars->outstream = (OutStream*)INCREF(outstream);
    ivars->last_doc_id = 0;
    return self;
}

void
RawPostWriter_Start_Term_IMP(RawPostingWriter *self, TermInfo *tinfo) {
    RawPostingWriterIVARS *const ivars = RawPostWriter_IVARS(self);
    ivars->last_doc_id   = 0;
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS(tinfo);
    tinfo_ivars->post_filepos = OutStream_Tell(ivars->outstream);
}

void
RawPostWriter_Update_Skip_Info_IMP(RawPostingWriter *self, TermInfo *tinfo) {
    RawPostingWriterIVARS *const ivars = RawPostWriter_IVARS(self);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS(tinfo);
    tinfo_ivars->post_filepos = OutStream_Tell(ivars->outstream);
}

void
RawPostWriter_Destroy_IMP(RawPostingWriter *self) {
    RawPostingWriterIVARS *const ivars = RawPostWriter_IVARS(self);
    DECREF(ivars->outstream);
    SUPER_DESTROY(self, RAWPOSTINGWRITER);
}

void
RawPostWriter_Write_Posting_IMP(RawPostingWriter *self, RawPosting *posting) {
    RawPostingWriterIVARS *const ivars = RawPostWriter_IVARS(self);
    RawPostingIVARS *const posting_ivars = RawPost_IVARS(posting);
    OutStream *const outstream   = ivars->outstream;
    const int32_t    doc_id      = posting_ivars->doc_id;
    const uint32_t   delta_doc   = doc_id - ivars->last_doc_id;
    char  *const     aux_content = posting_ivars->blob
                                   + posting_ivars->content_len;
    if (posting_ivars->freq == 1) {
        const uint32_t doc_code = (delta_doc << 1) | 1;
        OutStream_Write_C32(outstream, doc_code);
    }
    else {
        const uint32_t doc_code = delta_doc << 1;
        OutStream_Write_C32(outstream, doc_code);
        OutStream_Write_C32(outstream, posting_ivars->freq);
    }
    OutStream_Write_Bytes(outstream, aux_content, posting_ivars->aux_len);
    ivars->last_doc_id = doc_id;
}


