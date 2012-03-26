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
#include "Lucy/Util/StringHelper.h"

RawPosting RAWPOSTING_BLANK = {
    RAWPOSTING,
    {1},                   // ref.count
    0,                     // doc_id
    1,                     // freq
    0,                     // content_len
    0,                     // aux_len
    { '\0' }               // blob
};


RawPosting*
RawPost_new(void *pre_allocated_memory, int32_t doc_id, uint32_t freq,
            char *term_text, size_t term_text_len) {
    RawPosting *self    = (RawPosting*)pre_allocated_memory;
    self->vtable        = RAWPOSTING;
    self->ref.count     = 1; // never used
    self->doc_id        = doc_id;
    self->freq          = freq;
    self->content_len   = term_text_len;
    self->aux_len       = 0;
    memcpy(&self->blob, term_text, term_text_len);

    return self;
}

void
RawPost_destroy(RawPosting *self) {
    UNUSED_VAR(self);
    THROW(ERR, "Illegal attempt to destroy RawPosting object");
}

uint32_t
RawPost_get_refcount(RawPosting* self) {
    UNUSED_VAR(self);
    return 1;
}

RawPosting*
RawPost_inc_refcount(RawPosting* self) {
    return self;
}

uint32_t
RawPost_dec_refcount(RawPosting* self) {
    UNUSED_VAR(self);
    return 1;
}

/***************************************************************************/

RawPostingWriter*
RawPostWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                  PolyReader *polyreader, OutStream *outstream) {
    RawPostingWriter *self
        = (RawPostingWriter*)VTable_Make_Obj(RAWPOSTINGWRITER);
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
    self->outstream = (OutStream*)INCREF(outstream);
    self->last_doc_id = 0;
    return self;
}

void
RawPostWriter_start_term(RawPostingWriter *self, TermInfo *tinfo) {
    self->last_doc_id   = 0;
    tinfo->post_filepos = OutStream_Tell(self->outstream);
}

void
RawPostWriter_update_skip_info(RawPostingWriter *self, TermInfo *tinfo) {
    tinfo->post_filepos = OutStream_Tell(self->outstream);
}

void
RawPostWriter_destroy(RawPostingWriter *self) {
    DECREF(self->outstream);
    SUPER_DESTROY(self, RAWPOSTINGWRITER);
}

void
RawPostWriter_write_posting(RawPostingWriter *self, RawPosting *posting) {
    OutStream *const outstream   = self->outstream;
    const int32_t    doc_id      = posting->doc_id;
    const uint32_t   delta_doc   = doc_id - self->last_doc_id;
    char  *const     aux_content = posting->blob + posting->content_len;
    if (posting->freq == 1) {
        const uint32_t doc_code = (delta_doc << 1) | 1;
        OutStream_Write_C32(outstream, doc_code);
    }
    else {
        const uint32_t doc_code = delta_doc << 1;
        OutStream_Write_C32(outstream, doc_code);
        OutStream_Write_C32(outstream, posting->freq);
    }
    OutStream_Write_Bytes(outstream, aux_content, posting->aux_len);
    self->last_doc_id = doc_id;
}


