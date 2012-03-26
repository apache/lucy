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

#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/Token.h"

Token*
Token_new(const char* text, size_t len, uint32_t start_offset,
          uint32_t end_offset, float boost, int32_t pos_inc) {
    Token *self = (Token*)VTable_Make_Obj(TOKEN);
    return Token_init(self, text, len, start_offset, end_offset, boost,
                      pos_inc);
}

Token*
Token_init(Token *self, const char* text, size_t len, uint32_t start_offset,
           uint32_t end_offset, float boost, int32_t pos_inc) {
    // Allocate and assign.
    self->text      = (char*)MALLOCATE(len + 1);
    self->text[len] = '\0';
    memcpy(self->text, text, len);

    // Assign.
    self->len          = len;
    self->start_offset = start_offset;
    self->end_offset   = end_offset;
    self->boost        = boost;
    self->pos_inc      = pos_inc;

    // Init.
    self->pos = -1;

    return self;
}

void
Token_destroy(Token *self) {
    FREEMEM(self->text);
    SUPER_DESTROY(self, TOKEN);
}

int
Token_compare(void *context, const void *va, const void *vb) {
    Token *const a = *((Token**)va);
    Token *const b = *((Token**)vb);
    size_t min_len = a->len < b->len ? a->len : b->len;
    int comparison = memcmp(a->text, b->text, min_len);
    UNUSED_VAR(context);

    if (comparison == 0) {
        if (a->len != b->len) {
            comparison = a->len < b->len ? -1 : 1;
        }
        else {
            comparison = a->pos < b->pos ? -1 : 1;
        }
    }

    return comparison;
}

uint32_t
Token_get_start_offset(Token *self) {
    return self->start_offset;
}

uint32_t
Token_get_end_offset(Token *self) {
    return self->end_offset;
}

float
Token_get_boost(Token *self) {
    return self->boost;
}

int32_t
Token_get_pos_inc(Token *self) {
    return self->pos_inc;
}

char*
Token_get_text(Token *self) {
    return self->text;
}

size_t
Token_get_len(Token *self) {
    return self->len;
}

void
Token_set_text(Token *self, char *text, size_t len) {
    if (len > self->len) {
        FREEMEM(self->text);
        self->text = (char*)MALLOCATE(len + 1);
    }
    memcpy(self->text, text, len);
    self->text[len] = '\0';
    self->len = len;
}


