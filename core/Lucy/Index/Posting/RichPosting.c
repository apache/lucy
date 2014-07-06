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

#define C_LUCY_RICHPOSTING
#define C_LUCY_RICHPOSTINGMATCHER
#define C_LUCY_RAWPOSTING
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Posting/RichPosting.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingPool.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/MemoryPool.h"

#define FREQ_MAX_LEN     C32_MAX_BYTES
#define MAX_RAW_POSTING_LEN(_raw_posting_size, _text_len, _freq) \
    (              _raw_posting_size \
                   + _text_len                /* term text content */ \
                   + FREQ_MAX_LEN             /* freq c32 */ \
                   + (C32_MAX_BYTES * _freq)  /* positions deltas */ \
                   + _freq                    /* per-pos boost byte */ \
    )

RichPosting*
RichPost_new(Similarity *sim) {
    RichPosting *self = (RichPosting*)Class_Make_Obj(RICHPOSTING);
    return RichPost_init(self, sim);
}

RichPosting*
RichPost_init(RichPosting *self, Similarity *sim) {
    ScorePost_init((ScorePosting*)self, sim);
    RichPostingIVARS *const ivars = RichPost_IVARS(self);
    ivars->prox_boosts     = NULL;
    return self;
}

void
RichPost_Destroy_IMP(RichPosting *self) {
    RichPostingIVARS *const ivars = RichPost_IVARS(self);
    FREEMEM(ivars->prox_boosts);
    SUPER_DESTROY(self, RICHPOSTING);
}

void
RichPost_Read_Record_IMP(RichPosting *self, InStream *instream) {
    RichPostingIVARS *const ivars = RichPost_IVARS(self);
    float *const norm_decoder = ivars->norm_decoder;
    uint32_t  num_prox = 0;
    uint32_t  position = 0;
    float     aggregate_weight = 0.0;

    // Decode delta doc.
    uint32_t doc_code = InStream_Read_C32(instream);
    ivars->doc_id += doc_code >> 1;

    // If the stored num was odd, the freq is 1.
    if (doc_code & 1) {
        ivars->freq = 1;
    }
    // Otherwise, freq was stored as a C32.
    else {
        ivars->freq = InStream_Read_C32(instream);
    }

    // Read positions, aggregate per-position boost byte into weight.
    num_prox = ivars->freq;
    if (num_prox > ivars->prox_cap) {
        ivars->prox
            = (uint32_t*)REALLOCATE(ivars->prox, num_prox * sizeof(uint32_t));
        ivars->prox_boosts
            = (float*)REALLOCATE(ivars->prox_boosts, num_prox * sizeof(float));
    }
    uint32_t *positions    = ivars->prox;
    float    *prox_boosts  = ivars->prox_boosts;

    while (num_prox--) {
        position += InStream_Read_C32(instream);
        *positions++ = position;
        *prox_boosts = norm_decoder[InStream_Read_U8(instream)];
        aggregate_weight += *prox_boosts;
        prox_boosts++;
    }
    ivars->weight = aggregate_weight / ivars->freq;
}

void
RichPost_Add_Inversion_To_Pool_IMP(RichPosting *self, PostingPool *post_pool,
                                   Inversion *inversion, FieldType *type,
                                   int32_t doc_id, float doc_boost,
                                   float length_norm) {
    RichPostingIVARS *const ivars = RichPost_IVARS(self);
    MemoryPool *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    Similarity *sim = ivars->sim;
    float       field_boost = doc_boost * FType_Get_Boost(type) * length_norm;
    const size_t base_size = Class_Get_Obj_Alloc_Size(RAWPOSTING);
    Token     **tokens;
    uint32_t    freq;

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        TokenIVARS *const token_ivars = Token_IVARS(*tokens);
        uint32_t raw_post_bytes
            = MAX_RAW_POSTING_LEN(base_size, token_ivars->len, freq);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token_ivars->text, token_ivars->len);
        RawPostingIVARS *const raw_post_ivars = RawPost_IVARS(raw_posting);
        char *const start = raw_post_ivars->blob + token_ivars->len;
        char *dest = start;
        uint32_t last_prox = 0;

        // Positions and boosts.
        for (uint32_t i = 0; i < freq; i++) {
            TokenIVARS *const t_ivars = Token_IVARS(tokens[i]);
            const uint32_t prox_delta = t_ivars->pos - last_prox;
            const float boost = field_boost * t_ivars->boost;

            NumUtil_encode_c32(prox_delta, &dest);
            last_prox = t_ivars->pos;

            *((uint8_t*)dest) = Sim_Encode_Norm(sim, boost);
            dest++;
        }

        // Resize raw posting memory allocation.
        raw_post_ivars->aux_len = dest - start;
        raw_post_bytes = dest - (char*)raw_posting;
        MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);
        PostPool_Feed(post_pool, (Obj*)raw_posting);
    }
}

RawPosting*
RichPost_Read_Raw_IMP(RichPosting *self, InStream *instream,
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
    size_t raw_post_bytes  = MAX_RAW_POSTING_LEN(base_size, text_size, freq);
    void *const allocation = MemPool_Grab(mem_pool, raw_post_bytes);
    RawPosting *const raw_posting
        = RawPost_new(allocation, doc_id, freq, text_buf, text_size);
        RawPostingIVARS *const raw_post_ivars = RawPost_IVARS(raw_posting);
    uint32_t num_prox = freq;
    char *const start = raw_post_ivars->blob + text_size;
    char *      dest  = start;
    UNUSED_VAR(self);

    // Read positions and per-position boosts.
    while (num_prox--) {
        dest += InStream_Read_Raw_C64(instream, dest);
        *((uint8_t*)dest) = InStream_Read_U8(instream);
        dest++;
    }

    // Resize raw posting memory allocation.
    raw_post_ivars->aux_len = dest - start;
    raw_post_bytes       = dest - (char*)raw_posting;
    MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);

    return raw_posting;
}

RichPostingMatcher*
RichPost_Make_Matcher_IMP(RichPosting *self, Similarity *sim,
                          PostingList *plist, Compiler *compiler,
                          bool need_score) {
    RichPostingMatcher* matcher
        = (RichPostingMatcher*)Class_Make_Obj(RICHPOSTINGMATCHER);
    UNUSED_VAR(self);
    UNUSED_VAR(need_score);
    return RichPostMatcher_init(matcher, sim, plist, compiler);
}

RichPostingMatcher*
RichPostMatcher_init(RichPostingMatcher *self, Similarity *sim,
                     PostingList *plist, Compiler *compiler) {
    return (RichPostingMatcher*)ScorePostMatcher_init((ScorePostingMatcher*)self,
                                                      sim, plist, compiler);
}


