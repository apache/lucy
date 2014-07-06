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

#define MAX_RAW_POSTING_LEN(_raw_post_size, _text_len) \
    (              _raw_post_size \
                   + _text_len + 1            /* term text content */ \
    )

MatchPosting*
MatchPost_new(Similarity *sim) {
    MatchPosting *self = (MatchPosting*)Class_Make_Obj(MATCHPOSTING);
    return MatchPost_init(self, sim);
}

MatchPosting*
MatchPost_init(MatchPosting *self, Similarity *sim) {
    MatchPostingIVARS *const ivars = MatchPost_IVARS(self);
    ivars->sim = (Similarity*)INCREF(sim);
    return (MatchPosting*)Post_init((Posting*)self);
}

void
MatchPost_Destroy_IMP(MatchPosting *self) {
    MatchPostingIVARS *const ivars = MatchPost_IVARS(self);
    DECREF(ivars->sim);
    SUPER_DESTROY(self, MATCHPOSTING);
}

int32_t
MatchPost_Get_Freq_IMP(MatchPosting *self) {
    return MatchPost_IVARS(self)->freq;
}

void
MatchPost_Reset_IMP(MatchPosting *self) {
    MatchPost_IVARS(self)->doc_id = 0;
}

void
MatchPost_Read_Record_IMP(MatchPosting *self, InStream *instream) {
    MatchPostingIVARS *const ivars = MatchPost_IVARS(self);
    const uint32_t doc_code = InStream_Read_C32(instream);
    const uint32_t doc_delta = doc_code >> 1;

    // Apply delta doc and retrieve freq.
    ivars->doc_id   += doc_delta;
    if (doc_code & 1) {
        ivars->freq = 1;
    }
    else {
        ivars->freq = InStream_Read_C32(instream);
    }
}

RawPosting*
MatchPost_Read_Raw_IMP(MatchPosting *self, InStream *instream,
                       int32_t last_doc_id, String *term_text,
                       MemoryPool *mem_pool) {
    const char *const text_buf  = Str_Get_Ptr8(term_text);
    const size_t      text_size = Str_Get_Size(term_text);
    const uint32_t    doc_code  = InStream_Read_C32(instream);
    const uint32_t    delta_doc = doc_code >> 1;
    const int32_t     doc_id    = last_doc_id + delta_doc;
    const uint32_t    freq      = (doc_code & 1)
                                  ? 1
                                  : InStream_Read_C32(instream);
    const size_t base_size = Class_Get_Obj_Alloc_Size(RAWPOSTING);
    size_t raw_post_bytes  = MAX_RAW_POSTING_LEN(base_size, text_size);
    void *const allocation = MemPool_Grab(mem_pool, raw_post_bytes);
    UNUSED_VAR(self);

    return RawPost_new(allocation, doc_id, freq, text_buf, text_size);
}

void
MatchPost_Add_Inversion_To_Pool_IMP(MatchPosting *self,
                                    PostingPool *post_pool,
                                    Inversion *inversion, FieldType *type,
                                    int32_t doc_id, float doc_boost,
                                    float length_norm) {
    MemoryPool  *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    const size_t base_size = Class_Get_Obj_Alloc_Size(RAWPOSTING);
    Token      **tokens;
    uint32_t     freq;

    UNUSED_VAR(self);
    UNUSED_VAR(type);
    UNUSED_VAR(doc_boost);
    UNUSED_VAR(length_norm);

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        TokenIVARS *const token_ivars = Token_IVARS(*tokens);
        uint32_t raw_post_bytes
            = MAX_RAW_POSTING_LEN(base_size, token_ivars->len);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token_ivars->text, token_ivars->len);
        PostPool_Feed(post_pool, (Obj*)raw_posting);
    }
}

MatchPostingMatcher*
MatchPost_Make_Matcher_IMP(MatchPosting *self, Similarity *sim,
                           PostingList *plist, Compiler *compiler,
                           bool need_score) {
    MatchPostingMatcher *matcher
        = (MatchPostingMatcher*)Class_Make_Obj(MATCHPOSTINGMATCHER);
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
MatchPostMatcher_Score_IMP(MatchPostingMatcher* self) {
    return MatchPostMatcher_IVARS(self)->weight;
}

/***************************************************************************/

MatchPostingWriter*
MatchPostWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                    PolyReader *polyreader, int32_t field_num) {
    MatchPostingWriter *self
        = (MatchPostingWriter*)Class_Make_Obj(MATCHPOSTINGWRITER);
    return MatchPostWriter_init(self, schema, snapshot, segment, polyreader,
                                field_num);
}

MatchPostingWriter*
MatchPostWriter_init(MatchPostingWriter *self, Schema *schema,
                     Snapshot *snapshot, Segment *segment,
                     PolyReader *polyreader, int32_t field_num) {
    Folder  *folder = PolyReader_Get_Folder(polyreader);
    String *filename
        = Str_newf("%o/postings-%i32.dat", Seg_Get_Name(segment), field_num);
    PostWriter_init((PostingWriter*)self, schema, snapshot, segment,
                    polyreader, field_num);
    MatchPostingWriterIVARS *const ivars = MatchPostWriter_IVARS(self);
    ivars->outstream = Folder_Open_Out(folder, filename);
    if (!ivars->outstream) { RETHROW(INCREF(Err_get_error())); }
    DECREF(filename);
    return self;
}

void
MatchPostWriter_Destroy_IMP(MatchPostingWriter *self) {
    MatchPostingWriterIVARS *const ivars = MatchPostWriter_IVARS(self);
    DECREF(ivars->outstream);
    SUPER_DESTROY(self, MATCHPOSTINGWRITER);
}

void
MatchPostWriter_Write_Posting_IMP(MatchPostingWriter *self, RawPosting *posting) {
    MatchPostingWriterIVARS *const ivars = MatchPostWriter_IVARS(self);
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

void
MatchPostWriter_Start_Term_IMP(MatchPostingWriter *self, TermInfo *tinfo) {
    MatchPostingWriterIVARS *const ivars = MatchPostWriter_IVARS(self);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS(tinfo);
    ivars->last_doc_id   = 0;
    tinfo_ivars->post_filepos = OutStream_Tell(ivars->outstream);
}

void
MatchPostWriter_Update_Skip_Info_IMP(MatchPostingWriter *self, TermInfo *tinfo) {
    MatchPostingWriterIVARS *const ivars = MatchPostWriter_IVARS(self);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS(tinfo);
    tinfo_ivars->post_filepos = OutStream_Tell(ivars->outstream);
}

/***************************************************************************/

MatchTermInfoStepper*
MatchTInfoStepper_new(Schema *schema) {
    MatchTermInfoStepper *self
        = (MatchTermInfoStepper*)Class_Make_Obj(MATCHTERMINFOSTEPPER);
    return MatchTInfoStepper_init(self, schema);
}

MatchTermInfoStepper*
MatchTInfoStepper_init(MatchTermInfoStepper *self, Schema *schema) {
    Architecture *arch = Schema_Get_Architecture(schema);
    TermStepper_init((TermStepper*)self);
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    ivars->skip_interval = Arch_Skip_Interval(arch);
    ivars->value = (Obj*)TInfo_new(0);
    return self;
}

void
MatchTInfoStepper_Reset_IMP(MatchTermInfoStepper *self) {
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    TInfo_Reset((TermInfo*)ivars->value);
}

void
MatchTInfoStepper_Write_Key_Frame_IMP(MatchTermInfoStepper *self,
                                      OutStream *outstream, Obj *value) {
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    TermInfo *tinfo    = (TermInfo*)CERTIFY(value, TERMINFO);
    int32_t   doc_freq = TInfo_Get_Doc_Freq(tinfo);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS((TermInfo*)value);

    // Write doc_freq.
    OutStream_Write_C32(outstream, doc_freq);

    // Write postings file pointer.
    OutStream_Write_C64(outstream, tinfo_ivars->post_filepos);

    // Write skip file pointer (maybe).
    if (doc_freq >= ivars->skip_interval) {
        OutStream_Write_C64(outstream, tinfo_ivars->skip_filepos);
    }

    TInfo_Mimic((TermInfo*)ivars->value, (Obj*)tinfo);
}

void
MatchTInfoStepper_Write_Delta_IMP(MatchTermInfoStepper *self,
                                  OutStream *outstream, Obj *value) {
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    TermInfo *tinfo      = (TermInfo*)CERTIFY(value, TERMINFO);
    TermInfo *last_tinfo = (TermInfo*)ivars->value;
    int32_t   doc_freq   = TInfo_Get_Doc_Freq(tinfo);
    int64_t   post_delta = TInfo_IVARS(tinfo)->post_filepos
                           - TInfo_IVARS(last_tinfo)->post_filepos;

    // Write doc_freq.
    OutStream_Write_C32(outstream, doc_freq);

    // Write postings file pointer delta.
    OutStream_Write_C64(outstream, post_delta);

    // Write skip file pointer (maybe).
    if (doc_freq >= ivars->skip_interval) {
        OutStream_Write_C64(outstream, TInfo_IVARS(tinfo)->skip_filepos);
    }

    TInfo_Mimic((TermInfo*)ivars->value, (Obj*)tinfo);
}

void
MatchTInfoStepper_Read_Key_Frame_IMP(MatchTermInfoStepper *self,
                                     InStream *instream) {
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS((TermInfo*)ivars->value);

    // Read doc freq.
    tinfo_ivars->doc_freq = InStream_Read_C32(instream);

    // Read postings file pointer.
    tinfo_ivars->post_filepos = InStream_Read_C64(instream);

    // Maybe read skip pointer.
    if (tinfo_ivars->doc_freq >= ivars->skip_interval) {
        tinfo_ivars->skip_filepos = InStream_Read_C64(instream);
    }
    else {
        tinfo_ivars->skip_filepos = 0;
    }
}

void
MatchTInfoStepper_Read_Delta_IMP(MatchTermInfoStepper *self, InStream *instream) {
    MatchTermInfoStepperIVARS *const ivars = MatchTInfoStepper_IVARS(self);
    TermInfoIVARS *const tinfo_ivars = TInfo_IVARS((TermInfo*)ivars->value);

    // Read doc freq.
    tinfo_ivars->doc_freq = InStream_Read_C32(instream);

    // Adjust postings file pointer.
    tinfo_ivars->post_filepos += InStream_Read_C64(instream);

    // Maybe read skip pointer.
    if (tinfo_ivars->doc_freq >= ivars->skip_interval) {
        tinfo_ivars->skip_filepos = InStream_Read_C64(instream);
    }
    else {
        tinfo_ivars->skip_filepos = 0;
    }
}


