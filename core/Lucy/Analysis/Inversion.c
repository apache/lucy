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

#define C_LUCY_INVERSION
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/Token.h"
#include "Clownfish/Util/SortUtils.h"

// After inversion, record how many like tokens occur in each group.
static void
S_count_clusters(Inversion *self, InversionIVARS *ivars);

Inversion*
Inversion_new(Token *seed_token) {
    Inversion *self = (Inversion*)Class_Make_Obj(INVERSION);
    InversionIVARS *const ivars = Inversion_IVARS(self);

    // Init.
    ivars->cap                 = 16;
    ivars->size                = 0;
    ivars->tokens              = (Token**)CALLOCATE(ivars->cap, sizeof(Token*));
    ivars->cur                 = 0;
    ivars->inverted            = false;
    ivars->cluster_counts      = NULL;
    ivars->cluster_counts_size = 0;

    // Process the seed token.
    if (seed_token != NULL) {
        Inversion_Append(self, (Token*)INCREF(seed_token));
    }

    return self;
}

void
Inversion_Destroy_IMP(Inversion *self) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    if (ivars->tokens) {
        Token **tokens       = ivars->tokens;
        Token **const limit  = tokens + ivars->size;
        for (; tokens < limit; tokens++) {
            DECREF(*tokens);
        }
        FREEMEM(ivars->tokens);
    }
    FREEMEM(ivars->cluster_counts);
    SUPER_DESTROY(self, INVERSION);
}

uint32_t
Inversion_Get_Size_IMP(Inversion *self) {
    return Inversion_IVARS(self)->size;
}

Token*
Inversion_Next_IMP(Inversion *self) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    // Kill the iteration if we're out of tokens.
    if (ivars->cur == ivars->size) {
        return NULL;
    }
    return ivars->tokens[ivars->cur++];
}

void
Inversion_Reset_IMP(Inversion *self) {
    Inversion_IVARS(self)->cur = 0;
}

static void
S_grow(Inversion *self, size_t size) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    if (size > ivars->cap) {
        uint64_t amount = size * sizeof(Token*);
        // Clip rather than wrap.
        if (amount > SIZE_MAX || amount < size) { amount = SIZE_MAX; }
        ivars->tokens = (Token**)REALLOCATE(ivars->tokens, (size_t)amount);
        ivars->cap    = size;
        memset(ivars->tokens + ivars->size, 0,
               (size - ivars->size) * sizeof(Token*));
    }
}

void
Inversion_Append_IMP(Inversion *self, Token *token) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    if (ivars->inverted) {
        THROW(ERR, "Can't append tokens after inversion");
    }
    if (ivars->size >= ivars->cap) {
        size_t new_capacity = Memory_oversize(ivars->size + 1, sizeof(Token*));
        S_grow(self, new_capacity);
    }
    ivars->tokens[ivars->size] = token;
    ivars->size++;
}

Token**
Inversion_Next_Cluster_IMP(Inversion *self, uint32_t *count) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    Token **cluster = ivars->tokens + ivars->cur;

    if (ivars->cur == ivars->size) {
        *count = 0;
        return NULL;
    }

    // Don't read past the end of the cluster counts array.
    if (!ivars->inverted) {
        THROW(ERR, "Inversion not yet inverted");
    }
    if (ivars->cur > ivars->cluster_counts_size) {
        THROW(ERR, "Tokens were added after inversion");
    }

    // Place cluster count in passed-in var, advance bookmark.
    *count = ivars->cluster_counts[ivars->cur];
    ivars->cur += *count;

    return cluster;
}

void
Inversion_Invert_IMP(Inversion *self) {
    InversionIVARS *const ivars = Inversion_IVARS(self);
    Token   **tokens = ivars->tokens;
    Token   **limit  = tokens + ivars->size;
    int32_t   token_pos = 0;

    // Thwart future attempts to append.
    if (ivars->inverted) {
        THROW(ERR, "Inversion has already been inverted");
    }
    ivars->inverted = true;

    // Assign token positions.
    for (; tokens < limit; tokens++) {
        TokenIVARS *const cur_token_ivars = Token_IVARS(*tokens);
        cur_token_ivars->pos = token_pos;
        token_pos = (int32_t)((uint32_t)token_pos
                              + (uint32_t)cur_token_ivars->pos_inc);
        if (token_pos < cur_token_ivars->pos) {
            THROW(ERR, "Token positions out of order: %i32 %i32",
                  cur_token_ivars->pos, token_pos);
        }
    }

    // Sort the tokens lexically, and hand off to cluster counting routine.
    Sort_quicksort(ivars->tokens, ivars->size, sizeof(Token*), Token_compare,
                   NULL);
    S_count_clusters(self, ivars);
}

static void
S_count_clusters(Inversion *self, InversionIVARS *ivars) {
    UNUSED_VAR(self);
    Token **tokens = ivars->tokens;
    uint32_t *counts
        = (uint32_t*)CALLOCATE(ivars->size + 1, sizeof(uint32_t));

    // Save the cluster counts.
    ivars->cluster_counts_size = ivars->size;
    ivars->cluster_counts = counts;

    for (uint32_t i = 0; i < ivars->size;) {
        TokenIVARS *const base_token_ivars = Token_IVARS(tokens[i]);
        char  *const base_text  = base_token_ivars->text;
        const size_t base_len   = base_token_ivars->len;
        uint32_t     j          = i + 1;

        // Iterate through tokens until text doesn't match.
        while (j < ivars->size) {
            TokenIVARS *const candidate_ivars = Token_IVARS(tokens[j]);
            if ((candidate_ivars->len == base_len)
                && (memcmp(candidate_ivars->text, base_text, base_len) == 0)
               ) {
                j++;
            }
            else {
                break;
            }
        }

        // Record count at the position of the first token in the cluster.
        counts[i] = j - i;

        // Start the next loop at the next token we haven't seen.
        i = j;
    }
}


