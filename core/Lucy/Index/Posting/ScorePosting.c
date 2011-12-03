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
#define MAX_RAW_POSTING_LEN(_text_len, _freq) \
    (              sizeof(RawPosting) \
                   + _text_len                /* term text content */ \
                   + FIELD_BOOST_LEN          /* field boost byte */ \
                   + FREQ_MAX_LEN             /* freq c32 */ \
                   + (C32_MAX_BYTES * _freq)  /* positions deltas */ \
    )

ScorePosting*
ScorePost_new(Similarity *sim) {
    ScorePosting *self = (ScorePosting*)VTable_Make_Obj(SCOREPOSTING);
    return ScorePost_init(self, sim);
}

ScorePosting*
ScorePost_init(ScorePosting *self, Similarity *sim) {
    MatchPost_init((MatchPosting*)self, sim);
    self->norm_decoder = Sim_Get_Norm_Decoder(sim);
    self->freq         = 0;
    self->weight       = 0.0;
    self->prox         = NULL;
    self->prox_cap     = 0;
    return self;
}

void
ScorePost_destroy(ScorePosting *self) {
    FREEMEM(self->prox);
    SUPER_DESTROY(self, SCOREPOSTING);
}

uint32_t*
ScorePost_get_prox(ScorePosting *self) {
    return self->prox;
}

void
ScorePost_add_inversion_to_pool(ScorePosting *self, PostingPool *post_pool,
                                Inversion *inversion, FieldType *type,
                                int32_t doc_id, float doc_boost,
                                float length_norm) {
    MemoryPool     *mem_pool = PostPool_Get_Mem_Pool(post_pool);
    Similarity     *sim = self->sim;
    float           field_boost = doc_boost * FType_Get_Boost(type) * length_norm;
    const uint8_t   field_boost_byte  = Sim_Encode_Norm(sim, field_boost);
    Token         **tokens;
    uint32_t        freq;

    Inversion_Reset(inversion);
    while ((tokens = Inversion_Next_Cluster(inversion, &freq)) != NULL) {
        Token   *token          = *tokens;
        uint32_t raw_post_bytes = MAX_RAW_POSTING_LEN(token->len, freq);
        RawPosting *raw_posting
            = RawPost_new(MemPool_Grab(mem_pool, raw_post_bytes), doc_id,
                          freq, token->text, token->len);
        char *const start  = raw_posting->blob + token->len;
        char *dest         = start;
        uint32_t last_prox = 0;

        // Field_boost.
        *((uint8_t*)dest) = field_boost_byte;
        dest++;

        // Positions.
        for (uint32_t i = 0; i < freq; i++) {
            Token *const t = tokens[i];
            const uint32_t prox_delta = t->pos - last_prox;
            NumUtil_encode_c32(prox_delta, &dest);
            last_prox = t->pos;
        }

        // Resize raw posting memory allocation.
        raw_posting->aux_len = dest - start;
        raw_post_bytes = dest - (char*)raw_posting;
        MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);
        PostPool_Feed(post_pool, &raw_posting);
    }
}

void
ScorePost_reset(ScorePosting *self) {
    self->doc_id = 0;
    self->freq   = 0;
    self->weight = 0.0;
}

void
ScorePost_read_record(ScorePosting *self, InStream *instream) {
    uint32_t  position = 0;
    const size_t max_start_bytes = (C32_MAX_BYTES * 2) + 1;
    char *buf = InStream_Buf(instream, max_start_bytes);
    const uint32_t doc_code = NumUtil_decode_c32(&buf);
    const uint32_t doc_delta = doc_code >> 1;

    // Apply delta doc and retrieve freq.
    self->doc_id   += doc_delta;
    if (doc_code & 1) {
        self->freq = 1;
    }
    else {
        self->freq = NumUtil_decode_c32(&buf);
    }

    // Decode boost/norm byte.
    self->weight = self->norm_decoder[*(uint8_t*)buf];
    buf++;

    // Read positions.
    uint32_t num_prox = self->freq;
    if (num_prox > self->prox_cap) {
        self->prox = (uint32_t*)REALLOCATE(
                         self->prox, num_prox * sizeof(uint32_t));
        self->prox_cap = num_prox;
    }
    uint32_t *positions = self->prox;

    InStream_Advance_Buf(instream, buf);
    buf = InStream_Buf(instream, num_prox * C32_MAX_BYTES);
    while (num_prox--) {
        position += NumUtil_decode_c32(&buf);
        *positions++ = position;
    }

    InStream_Advance_Buf(instream, buf);
}

RawPosting*
ScorePost_read_raw(ScorePosting *self, InStream *instream,
                   int32_t last_doc_id, CharBuf *term_text,
                   MemoryPool *mem_pool) {
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
    raw_posting->aux_len = dest - start;
    raw_post_bytes       = dest - (char*)raw_posting;
    MemPool_Resize(mem_pool, raw_posting, raw_post_bytes);

    return raw_posting;
}

ScorePostingMatcher*
ScorePost_make_matcher(ScorePosting *self, Similarity *sim,
                       PostingList *plist, Compiler *compiler,
                       bool_t need_score) {
    ScorePostingMatcher *matcher
        = (ScorePostingMatcher*)VTable_Make_Obj(SCOREPOSTINGMATCHER);
    UNUSED_VAR(self);
    UNUSED_VAR(need_score);
    return ScorePostMatcher_init(matcher, sim, plist, compiler);
}

ScorePostingMatcher*
ScorePostMatcher_init(ScorePostingMatcher *self, Similarity *sim,
                      PostingList *plist, Compiler *compiler) {
    // Init.
    TermMatcher_init((TermMatcher*)self, sim, plist, compiler);

    // Fill score cache.
    self->score_cache = (float*)MALLOCATE(TERMMATCHER_SCORE_CACHE_SIZE * sizeof(float));
    for (uint32_t i = 0; i < TERMMATCHER_SCORE_CACHE_SIZE; i++) {
        self->score_cache[i] = Sim_TF(sim, (float)i) * self->weight;
    }

    return self;
}

float
ScorePostMatcher_score(ScorePostingMatcher* self) {
    ScorePosting *const posting = (ScorePosting*)self->posting;
    const uint32_t freq = posting->freq;

    // Calculate initial score based on frequency of term.
    float score = (freq < TERMMATCHER_SCORE_CACHE_SIZE)
                  ? self->score_cache[freq] // cache hit
                  : Sim_TF(self->sim, (float)freq) * self->weight;

    // Factor in field-length normalization and doc/field/prox boost.
    score *= posting->weight;

    return score;
}

void
ScorePostMatcher_destroy(ScorePostingMatcher *self) {
    FREEMEM(self->score_cache);
    SUPER_DESTROY(self, SCOREPOSTINGMATCHER);
}


