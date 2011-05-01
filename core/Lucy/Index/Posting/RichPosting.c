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
#define MAX_RAW_POSTING_LEN(_text_len, _freq) \
    (              sizeof(RawPosting) \
                   + _text_len                /* term text content */ \
                   + FREQ_MAX_LEN             /* freq c32 */ \
                   + (C32_MAX_BYTES * _freq)  /* positions deltas */ \
                   + _freq                    /* per-pos boost byte */ \
    )

RichPosting*
RichPost_new(Similarity *sim) {
    RichPosting *self = (RichPosting*)VTable_Make_Obj(RICHPOSTING);
    return RichPost_init(self, sim);
}

RichPosting*
RichPost_init(RichPosting *self, Similarity *sim) {
    ScorePost_init((ScorePosting*)self, sim);
    self->prox_boosts     = NULL;
    return self;
}

void
RichPost_destroy(RichPosting *self) {
    FREEMEM(self->prox_boosts);
    SUPER_DESTROY(self, RICHPOSTING);
}

void
RichPost_read_record(RichPosting *self, InStream *instream) {
    float *const norm_decoder = self->norm_decoder;
    uint32_t  doc_code;
    uint32_t  num_prox = 0;
    uint32_t  position = 0;
    uint32_t *positions;
    float    *prox_boosts;
    float     aggregate_weight = 0.0;

    // Decode delta doc.
    doc_code = InStream_Read_C32(instream);
    self->doc_id += doc_code >> 1;

    // If the stored num was odd, the freq is 1.
    if (doc_code & 1) {
        self->freq = 1;
    }
    // Otherwise, freq was stored as a C32.
    else {
        self->freq = InStream_Read_C32(instream);
    }

    // Read positions, aggregate per-position boost byte into weight.
    num_prox = self->freq;
    if (num_prox > self->prox_cap) {
        self->prox
            = (uint32_t*)REALLOCATE(self->prox, num_prox * sizeof(uint32_t));
        self->prox_boosts
            = (float*)REALLOCATE(self->prox_boosts, num_prox * sizeof(float));
    }
    positions   = self->prox;
    prox_boosts = self->prox_boosts;

    while (num_prox--) {
        position += InStream_Read_C32(instream);
        *positions++ = position;
        *prox_boosts = norm_decoder[InStream_Read_U8(instream)];
        aggregate_weight += *prox_boosts;
        prox_boosts++;
    }
    self->weight = aggregate_weight / self->freq;
}

void
RichPost_add_inversion_to_pool(RichPosting *self, PostingPool *post_pool,
                               Inversion *inversion, FieldType *type,
                               int32_t doc_id, float doc_boost,
                               float length_norm) {
    MemoryPool *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    Similarity *sim = self->sim;
    float       field_boost = doc_boost * FType_Get_Boost(type) * length_norm;
    Token     **tokens;
    uint32_t    freq;

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        Token   *token          = *tokens;
        uint32_t raw_post_bytes = MAX_RAW_POSTING_LEN(token->len, freq);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token->text, token->len);
        char *const start = raw_posting->blob + token->len;
        char *dest = start;
        uint32_t last_prox = 0;
        uint32_t i;

        // Positions and boosts.
        for (i = 0; i < freq; i++) {
            Token *const t = tokens[i];
            const uint32_t prox_delta = t->pos - last_prox;
            const float boost = field_boost * t->boost;

            NumUtil_encode_c32(prox_delta, &dest);
            last_prox = t->pos;

            *((uint8_t*)dest) = Sim_Encode_Norm(sim, boost);
            dest++;
        }

        // Resize raw posting memory allocation.
        raw_posting->aux_len = dest - start;
        raw_post_bytes = dest - (char*)raw_posting;
        MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);
        PostPool_Feed(post_pool, &raw_posting);
    }
}

RawPosting*
RichPost_read_raw(RichPosting *self, InStream *instream, int32_t last_doc_id,
                  CharBuf *term_text, MemoryPool *mem_pool) {
    char *const    text_buf       = (char*)CB_Get_Ptr8(term_text);
    const size_t   text_size      = CB_Get_Size(term_text);
    const uint32_t doc_code       = InStream_Read_C32(instream);
    const uint32_t delta_doc      = doc_code >> 1;
    const int32_t  doc_id         = last_doc_id + delta_doc;
    const uint32_t freq           = (doc_code & 1)
                                    ? 1
                                    : InStream_Read_C32(instream);
    size_t raw_post_bytes         = MAX_RAW_POSTING_LEN(text_size, freq);
    void *const allocation        = MemPool_Grab(mem_pool, raw_post_bytes);
    RawPosting *const raw_posting
        = RawPost_new(allocation, doc_id, freq, text_buf, text_size);
    uint32_t num_prox = freq;
    char *const start = raw_posting->blob + text_size;
    char *      dest  = start;
    UNUSED_VAR(self);

    // Read positions and per-position boosts.
    while (num_prox--) {
        dest += InStream_Read_Raw_C64(instream, dest);
        *((uint8_t*)dest) = InStream_Read_U8(instream);
        dest++;
    }

    // Resize raw posting memory allocation.
    raw_posting->aux_len = dest - start;
    raw_post_bytes       = dest - (char*)raw_posting;
    MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);

    return raw_posting;
}

RichPostingMatcher*
RichPost_make_matcher(RichPosting *self, Similarity *sim,
                      PostingList *plist, Compiler *compiler,
                      bool_t need_score) {
    RichPostingMatcher* matcher
        = (RichPostingMatcher*)VTable_Make_Obj(RICHPOSTINGMATCHER);
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


