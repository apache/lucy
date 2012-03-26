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

#define C_LUCY_MATCHPOSTING
#define C_LUCY_MATCHPOSTINGMATCHER
#define C_LUCY_MATCHPOSTINGWRITER
#define C_LUCY_MATCHTERMINFOSTEPPER
#define C_LUCY_RAWPOSTING
#define C_LUCY_TERMINFO
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Posting/MatchPosting.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingPool.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/TermInfo.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/MemoryPool.h"

#define MAX_RAW_POSTING_LEN(_text_len) \
    (              sizeof(RawPosting) \
                   + _text_len + 1            /* term text content */ \
    )

MatchPosting*
MatchPost_new(Similarity *sim) {
    MatchPosting *self = (MatchPosting*)VTable_Make_Obj(MATCHPOSTING);
    return MatchPost_init(self, sim);
}

MatchPosting*
MatchPost_init(MatchPosting *self, Similarity *sim) {
    self->sim = (Similarity*)INCREF(sim);
    return (MatchPosting*)Post_init((Posting*)self);
}

void
MatchPost_destroy(MatchPosting *self) {
    DECREF(self->sim);
    SUPER_DESTROY(self, MATCHPOSTING);
}

int32_t
MatchPost_get_freq(MatchPosting *self) {
    return self->freq;
}

void
MatchPost_reset(MatchPosting *self) {
    self->doc_id = 0;
}

void
MatchPost_read_record(MatchPosting *self, InStream *instream) {
    const uint32_t doc_code = InStream_Read_C32(instream);
    const uint32_t doc_delta = doc_code >> 1;

    // Apply delta doc and retrieve freq.
    self->doc_id   += doc_delta;
    if (doc_code & 1) {
        self->freq = 1;
    }
    else {
        self->freq = InStream_Read_C32(instream);
    }
}

RawPosting*
MatchPost_read_raw(MatchPosting *self, InStream *instream, int32_t last_doc_id,
                   CharBuf *term_text, MemoryPool *mem_pool) {
    char *const    text_buf  = (char*)CB_Get_Ptr8(term_text);
    const size_t   text_size = CB_Get_Size(term_text);
    const uint32_t doc_code  = InStream_Read_C32(instream);
    const uint32_t delta_doc = doc_code >> 1;
    const int32_t  doc_id    = last_doc_id + delta_doc;
    const uint32_t freq      = (doc_code & 1)
                               ? 1
                               : InStream_Read_C32(instream);
    size_t raw_post_bytes    = MAX_RAW_POSTING_LEN(text_size);
    void *const allocation   = MemPool_Grab(mem_pool, raw_post_bytes);
    UNUSED_VAR(self);

    return RawPost_new(allocation, doc_id, freq, text_buf, text_size);
}

void
MatchPost_add_inversion_to_pool(MatchPosting *self, PostingPool *post_pool,
                                Inversion *inversion, FieldType *type,
                                int32_t doc_id, float doc_boost,
                                float length_norm) {
    MemoryPool  *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    Token      **tokens;
    uint32_t     freq;

    UNUSED_VAR(self);
    UNUSED_VAR(type);
    UNUSED_VAR(doc_boost);
    UNUSED_VAR(length_norm);

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        Token   *token          = *tokens;
        uint32_t raw_post_bytes = MAX_RAW_POSTING_LEN(token->len);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token->text, token->len);
        PostPool_Feed(post_pool, &raw_posting);
    }
}

MatchPostingMatcher*
MatchPost_make_matcher(MatchPosting *self, Similarity *sim,
                       PostingList *plist, Compiler *compiler,
                       bool_t need_score) {
    MatchPostingMatcher *matcher
        = (MatchPostingMatcher*)VTable_Make_Obj(MATCHPOSTINGMATCHER);
    UNUSED_VAR(self);
    UNUSED_VAR(need_score);
    return MatchPostMatcher_init(matcher, sim, plist, compiler);
}

/***************************************************************************/

MatchPostingMatcher*
MatchPostMatcher_init(MatchPostingMatcher *self, Similarity *sim,
                      PostingList *plist, Compiler *compiler) {
    TermMatcher_init((TermMatcher*)self, sim, plist, compiler);
    return self;
}

float
MatchPostMatcher_score(MatchPostingMatcher* self) {
    return self->weight;
}

/***************************************************************************/

MatchPostingWriter*
MatchPostWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                    PolyReader *polyreader, int32_t field_num) {
    MatchPostingWriter *self
        = (MatchPostingWriter*)VTable_Make_Obj(MATCHPOSTINGWRITER);
    return MatchPostWriter_init(self, schema, snapshot, segment, polyreader,
                                field_num);
}

MatchPostingWriter*
MatchPostWriter_init(MatchPostingWriter *self, Schema *schema,
                     Snapshot *snapshot, Segment *segment,
                     PolyReader *polyreader, int32_t field_num) {
    Folder  *folder = PolyReader_Get_Folder(polyreader);
    CharBuf *filename
        = CB_newf("%o/postings-%i32.dat", Seg_Get_Name(segment), field_num);
    PostWriter_init((PostingWriter*)self, schema, snapshot, segment,
                    polyreader, field_num);
    self->outstream = Folder_Open_Out(folder, filename);
    if (!self->outstream) { RETHROW(INCREF(Err_get_error())); }
    DECREF(filename);
    return self;
}

void
MatchPostWriter_destroy(MatchPostingWriter *self) {
    DECREF(self->outstream);
    SUPER_DESTROY(self, MATCHPOSTINGWRITER);
}

void
MatchPostWriter_write_posting(MatchPostingWriter *self, RawPosting *posting) {
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

void
MatchPostWriter_start_term(MatchPostingWriter *self, TermInfo *tinfo) {
    self->last_doc_id   = 0;
    tinfo->post_filepos = OutStream_Tell(self->outstream);
}

void
MatchPostWriter_update_skip_info(MatchPostingWriter *self, TermInfo *tinfo) {
    tinfo->post_filepos = OutStream_Tell(self->outstream);
}

/***************************************************************************/

MatchTermInfoStepper*
MatchTInfoStepper_new(Schema *schema) {
    MatchTermInfoStepper *self
        = (MatchTermInfoStepper*)VTable_Make_Obj(MATCHTERMINFOSTEPPER);
    return MatchTInfoStepper_init(self, schema);
}

MatchTermInfoStepper*
MatchTInfoStepper_init(MatchTermInfoStepper *self, Schema *schema) {
    Architecture *arch = Schema_Get_Architecture(schema);
    TermStepper_init((TermStepper*)self);
    self->skip_interval = Arch_Skip_Interval(arch);
    self->value = (Obj*)TInfo_new(0);
    return self;
}

void
MatchTInfoStepper_reset(MatchTermInfoStepper *self) {
    TInfo_Reset((TermInfo*)self->value);
}

void
MatchTInfoStepper_write_key_frame(MatchTermInfoStepper *self,
                                  OutStream *outstream, Obj *value) {
    TermInfo *tinfo    = (TermInfo*)CERTIFY(value, TERMINFO);
    int32_t   doc_freq = TInfo_Get_Doc_Freq(tinfo);

    // Write doc_freq.
    OutStream_Write_C32(outstream, doc_freq);

    // Write postings file pointer.
    OutStream_Write_C64(outstream, tinfo->post_filepos);

    // Write skip file pointer (maybe).
    if (doc_freq >= self->skip_interval) {
        OutStream_Write_C64(outstream, tinfo->skip_filepos);
    }

    TInfo_Mimic((TermInfo*)self->value, (Obj*)tinfo);
}

void
MatchTInfoStepper_write_delta(MatchTermInfoStepper *self,
                              OutStream *outstream, Obj *value) {
    TermInfo *tinfo      = (TermInfo*)CERTIFY(value, TERMINFO);
    TermInfo *last_tinfo = (TermInfo*)self->value;
    int32_t   doc_freq   = TInfo_Get_Doc_Freq(tinfo);
    int64_t   post_delta = tinfo->post_filepos - last_tinfo->post_filepos;

    // Write doc_freq.
    OutStream_Write_C32(outstream, doc_freq);

    // Write postings file pointer delta.
    OutStream_Write_C64(outstream, post_delta);

    // Write skip file pointer (maybe).
    if (doc_freq >= self->skip_interval) {
        OutStream_Write_C64(outstream, tinfo->skip_filepos);
    }

    TInfo_Mimic((TermInfo*)self->value, (Obj*)tinfo);
}

void
MatchTInfoStepper_read_key_frame(MatchTermInfoStepper *self,
                                 InStream *instream) {
    TermInfo *const tinfo = (TermInfo*)self->value;

    // Read doc freq.
    tinfo->doc_freq = InStream_Read_C32(instream);

    // Read postings file pointer.
    tinfo->post_filepos = InStream_Read_C64(instream);

    // Maybe read skip pointer.
    if (tinfo->doc_freq >= self->skip_interval) {
        tinfo->skip_filepos = InStream_Read_C64(instream);
    }
    else {
        tinfo->skip_filepos = 0;
    }
}

void
MatchTInfoStepper_read_delta(MatchTermInfoStepper *self, InStream *instream) {
    TermInfo *const tinfo = (TermInfo*)self->value;

    // Read doc freq.
    tinfo->doc_freq = InStream_Read_C32(instream);

    // Adjust postings file pointer.
    tinfo->post_filepos += InStream_Read_C64(instream);

    // Maybe read skip pointer.
    if (tinfo->doc_freq >= self->skip_interval) {
        tinfo->skip_filepos = InStream_Read_C64(instream);
    }
    else {
        tinfo->skip_filepos = 0;
    }
}


