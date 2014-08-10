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

#define C_LUCY_SCOREPOSTING
#define C_LUCY_SCOREPOSTINGMATCHER
#define C_LUCY_RAWPOSTING
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Index/Posting/RawPosting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingPool.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/MemoryPool.h"

#define FIELD_BOOST_LEN  1
#define FREQ_MAX_LEN     C32_MAX_BYTES
#define MAX_RAW_POSTING_LEN(_raw_post_size, _text_len, _freq) \
    (              _raw_post_size \
                   + _text_len                /* term text content */ \
                   + FIELD_BOOST_LEN          /* field boost byte */ \
                   + FREQ_MAX_LEN             /* freq c32 */ \
                   + (C32_MAX_BYTES * _freq)  /* positions deltas */ \
    )

ScorePosting*
ScorePost_new(Similarity *sim) {
    ScorePosting *self = (ScorePosting*)Class_Make_Obj(SCOREPOSTING);
    return ScorePost_init(self, sim);
}

ScorePosting*
ScorePost_init(ScorePosting *self, Similarity *sim) {
    MatchPost_init((MatchPosting*)self, sim);
    ScorePostingIVARS *const ivars = ScorePost_IVARS(self);
    ivars->norm_decoder = Sim_Get_Norm_Decoder(sim);
    ivars->freq         = 0;
    ivars->weight       = 0.0;
    ivars->prox         = NULL;
    ivars->prox_cap     = 0;
    return self;
}

void
ScorePost_Destroy_IMP(ScorePosting *self) {
    ScorePostingIVARS *const ivars = ScorePost_IVARS(self);
    FREEMEM(ivars->prox);
    SUPER_DESTROY(self, SCOREPOSTING);
}

uint32_t*
ScorePost_Get_Prox_IMP(ScorePosting *self) {
    return ScorePost_IVARS(self)->prox;
}

void
ScorePost_Add_Inversion_To_Pool_IMP(ScorePosting *self,
                                    PostingPool *post_pool,
                                    Inversion *inversion, FieldType *type,
                                    int32_t doc_id, float doc_boost,
                                    float length_norm) {
    ScorePostingIVARS *const ivars = ScorePost_IVARS(self);
    MemoryPool     *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    Similarity     *sim = ivars->sim;
    float           field_boost = doc_boost * FType_Get_Boost(type) * length_norm;
    const uint8_t   field_boost_byte  = Sim_Encode_Norm(sim, field_boost);
    const size_t    base_size = Class_Get_Obj_Alloc_Size(RAWPOSTING);
    Token         **tokens;
    uint32_t        freq;

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        TokenIVARS *const token_ivars = Token_IVARS(*tokens);
        uint32_t raw_post_bytes
            = MAX_RAW_POSTING_LEN(base_size, token_ivars->len, freq);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token_ivars->text, token_ivars->len);
        RawPostingIVARS *const raw_post_ivars = RawPost_IVARS(raw_posting);
        char *const start  = raw_post_ivars->blob + token_ivars->len;
        char *dest         = start;
        uint32_t last_prox = 0;

        // Field_boost.
        *((uint8_t*)dest) = field_boost_byte;
        dest++;

        // Positions.
        for (uint32_t i = 0; i < freq; i++) {
            TokenIVARS *const t_ivars = Token_IVARS(tokens[i]);
            const uint32_t prox_delta = t_ivars->pos - last_prox;
            NumUtil_encode_c32(prox_delta, &dest);
            last_prox = t_ivars->pos;
        }

        // Resize raw posting memory allocation.
        raw_post_ivars->aux_len = dest - start;
        raw_post_bytes = dest - (char*)raw_posting;
        MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);
        PostPool_Feed(post_pool, (Obj*)raw_posting);
    }
}

void
ScorePost_Reset_IMP(ScorePosting *self) {
    ScorePostingIVARS *const ivars = ScorePost_IVARS(self);
    ivars->doc_id = 0;
    ivars->freq   = 0;
    ivars->weight = 0.0;
}

void
ScorePost_Read_Record_IMP(ScorePosting *self, InStream *instream) {
    ScorePostingIVARS *const ivars = ScorePost_IVARS(self);
    uint32_t  position = 0;
    const size_t max_start_bytes = (C32_MAX_BYTES * 2) + 1;
    const char *buf = InStream_Buf(instream, max_start_bytes);
    const uint32_t doc_code = NumUtil_decode_c32(&buf);
    const uint32_t doc_delta = doc_code >> 1;

    // Apply delta doc and retrieve freq.
    ivars->doc_id   += doc_delta;
    if (doc_code & 1) {
        ivars->freq = 1;
    }
    else {
        ivars->freq = NumUtil_decode_c32(&buf);
    }

    // Decode boost/norm byte.
    ivars->weight = ivars->norm_decoder[*(uint8_t*)buf];
    buf++;

    // Read positions.
    uint32_t num_prox = ivars->freq;
    if (num_prox > ivars->prox_cap) {
        ivars->prox = (uint32_t*)REALLOCATE(
                         ivars->prox, num_prox * sizeof(uint32_t));
        ivars->prox_cap = num_prox;
    }
    uint32_t *positions = ivars->prox;

    InStream_Advance_Buf(instream, buf);
    buf = InStream_Buf(instream, num_prox * C32_MAX_BYTES);
    while (num_prox--) {
        position += NumUtil_decode_c32(&buf);
        *positions++ = position;
    }

    InStream_Advance_Buf(instream, buf);
}

RawPosting*
ScorePost_Read_Raw_IMP(ScorePosting *self, InStream *instream,
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
    char *dest        = start;
    UNUSED_VAR(self);

    // Field_boost.
    *((uint8_t*)dest) = InStream_Read_U8(instream);
    dest++;

    // Read positions.
    while (num_prox--) {
        dest += InStream_Read_Raw_C64(instream, dest);
    }

    // Resize raw posting memory allocation.
    raw_post_ivars->aux_len = dest - start;
    raw_post_bytes       = dest - (char*)raw_posting;
    MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);

    return raw_posting;
}

ScorePostingMatcher*
ScorePost_Make_Matcher_IMP(ScorePosting *self, Similarity *sim,
                           PostingList *plist, Compiler *compiler,
                           bool need_score) {
    ScorePostingMatcher *matcher
        = (ScorePostingMatcher*)Class_Make_Obj(SCOREPOSTINGMATCHER);
    UNUSED_VAR(self);
    UNUSED_VAR(need_score);
    return ScorePostMatcher_init(matcher, sim, plist, compiler);
}

ScorePostingMatcher*
ScorePostMatcher_init(ScorePostingMatcher *self, Similarity *sim,
                      PostingList *plist, Compiler *compiler) {
    // Init.
    TermMatcher_init((TermMatcher*)self, sim, plist, compiler);
    ScorePostingMatcherIVARS *const ivars = ScorePostMatcher_IVARS(self);

    // Fill score cache.
    ivars->score_cache = (float*)MALLOCATE(TERMMATCHER_SCORE_CACHE_SIZE * sizeof(float));
    for (uint32_t i = 0; i < TERMMATCHER_SCORE_CACHE_SIZE; i++) {
        ivars->score_cache[i] = Sim_TF(sim, (float)i) * ivars->weight;
    }

    return self;
}

float
ScorePostMatcher_Score_IMP(ScorePostingMatcher* self) {
    ScorePostingMatcherIVARS *const ivars = ScorePostMatcher_IVARS(self);
    ScorePostingIVARS *const posting_ivars
        = ScorePost_IVARS((ScorePosting*)ivars->posting);
    const uint32_t freq = posting_ivars->freq;

    // Calculate initial score based on frequency of term.
    float score = (freq < TERMMATCHER_SCORE_CACHE_SIZE)
                  ? ivars->score_cache[freq] // cache hit
                  : Sim_TF(ivars->sim, (float)freq) * ivars->weight;

    // Factor in field-length normalization and doc/field/prox boost.
    score *= posting_ivars->weight;

    return score;
}

void
ScorePostMatcher_Destroy_IMP(ScorePostingMatcher *self) {
    ScorePostingMatcherIVARS *const ivars = ScorePostMatcher_IVARS(self);
    FREEMEM(ivars->score_cache);
    SUPER_DESTROY(self, SCOREPOSTINGMATCHER);
}


