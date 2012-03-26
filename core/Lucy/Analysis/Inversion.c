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
#include "Lucy/Util/SortUtils.h"

#ifndef SIZE_MAX
  #define SIZE_MAX ((size_t)-1)
#endif

// After inversion, record how many like tokens occur in each group.
static void
S_count_clusters(Inversion *self);

Inversion*
Inversion_new(Token *seed_token) {
    Inversion *self = (Inversion*)VTable_Make_Obj(INVERSION);

    // Init.
    self->cap                 = 16;
    self->size                = 0;
    self->tokens              = (Token**)CALLOCATE(self->cap, sizeof(Token*));
    self->cur                 = 0;
    self->inverted            = false;
    self->cluster_counts      = NULL;
    self->cluster_counts_size = 0;

    // Process the seed token.
    if (seed_token != NULL) {
        Inversion_append(self, (Token*)INCREF(seed_token));
    }

    return self;
}

void
Inversion_destroy(Inversion *self) {
    if (self->tokens) {
        Token **tokens       = self->tokens;
        Token **const limit  = tokens + self->size;
        for (; tokens < limit; tokens++) {
            DECREF(*tokens);
        }
        FREEMEM(self->tokens);
    }
    FREEMEM(self->cluster_counts);
    SUPER_DESTROY(self, INVERSION);
}

uint32_t
Inversion_get_size(Inversion *self) {
    return self->size;
}

Token*
Inversion_next(Inversion *self) {
    // Kill the iteration if we're out of tokens.
    if (self->cur == self->size) {
        return NULL;
    }
    return self->tokens[self->cur++];
}

void
Inversion_reset(Inversion *self) {
    self->cur = 0;
}

static void
S_grow(Inversion *self, size_t size) {
    if (size > self->cap) {
        uint64_t amount = size * sizeof(Token*);
        // Clip rather than wrap.
        if (amount > SIZE_MAX || amount < size) { amount = SIZE_MAX; }
        self->tokens = (Token**)REALLOCATE(self->tokens, (size_t)amount);
        self->cap    = size;
        memset(self->tokens + self->size, 0,
               (size - self->size) * sizeof(Token*));
    }
}

void
Inversion_append(Inversion *self, Token *token) {
    if (self->inverted) {
        THROW(ERR, "Can't append tokens after inversion");
    }
    if (self->size >= self->cap) {
        size_t new_capacity = Memory_oversize(self->size + 1, sizeof(Token*));
        S_grow(self, new_capacity);
    }
    self->tokens[self->size] = token;
    self->size++;
}

Token**
Inversion_next_cluster(Inversion *self, uint32_t *count) {
    Token **cluster = self->tokens + self->cur;

    if (self->cur == self->size) {
        *count = 0;
        return NULL;
    }

    // Don't read past the end of the cluster counts array.
    if (!self->inverted) {
        THROW(ERR, "Inversion not yet inverted");
    }
    if (self->cur > self->cluster_counts_size) {
        THROW(ERR, "Tokens were added after inversion");
    }

    // Place cluster count in passed-in var, advance bookmark.
    *count = self->cluster_counts[self->cur];
    self->cur += *count;

    return cluster;
}

void
Inversion_invert(Inversion *self) {
    Token   **tokens = self->tokens;
    Token   **limit  = tokens + self->size;
    int32_t   token_pos = 0;

    // Thwart future attempts to append.
    if (self->inverted) {
        THROW(ERR, "Inversion has already been inverted");
    }
    self->inverted = true;

    // Assign token positions.
    for (; tokens < limit; tokens++) {
        Token *const cur_token = *tokens;
        cur_token->pos = token_pos;
        token_pos += cur_token->pos_inc;
        if (token_pos < cur_token->pos) {
            THROW(ERR, "Token positions out of order: %i32 %i32",
                  cur_token->pos, token_pos);
        }
    }

    // Sort the tokens lexically, and hand off to cluster counting routine.
    Sort_quicksort(self->tokens, self->size, sizeof(Token*), Token_compare,
                   NULL);
    S_count_clusters(self);
}

static void
S_count_clusters(Inversion *self) {
    Token **tokens = self->tokens;
    uint32_t *counts
        = (uint32_t*)CALLOCATE(self->size + 1, sizeof(uint32_t));

    // Save the cluster counts.
    self->cluster_counts_size = self->size;
    self->cluster_counts = counts;

    for (uint32_t i = 0; i < self->size;) {
        Token *const base_token = tokens[i];
        char  *const base_text  = base_token->text;
        const size_t base_len   = base_token->len;
        uint32_t     j          = i + 1;

        // Iterate through tokens until text doesn't match.
        while (j < self->size) {
            Token *const candidate = tokens[j];

            if ((candidate->len == base_len)
                && (memcmp(candidate->text, base_text, base_len) == 0)
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


