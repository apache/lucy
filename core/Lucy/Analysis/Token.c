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
    Token *self = (Token*)Class_Make_Obj(TOKEN);
    return Token_init(self, text, len, start_offset, end_offset, boost,
                      pos_inc);
}

Token*
Token_init(Token *self, const char* text, size_t len, uint32_t start_offset,
           uint32_t end_offset, float boost, int32_t pos_inc) {
    TokenIVARS *const ivars = Token_IVARS(self);

    // Allocate and assign.
    ivars->text      = (char*)MALLOCATE(len + 1);
    ivars->text[len] = '\0';
    memcpy(ivars->text, text, len);

    // Assign.
    ivars->len          = len;
    ivars->start_offset = start_offset;
    ivars->end_offset   = end_offset;
    ivars->boost        = boost;
    ivars->pos_inc      = pos_inc;

    // Init.
    ivars->pos = -1;

    return self;
}

void
Token_Destroy_IMP(Token *self) {
    TokenIVARS *const ivars = Token_IVARS(self);
    FREEMEM(ivars->text);
    SUPER_DESTROY(self, TOKEN);
}

int
Token_compare(void *context, const void *va, const void *vb) {
    Token *const token_a = *((Token**)va);
    Token *const token_b = *((Token**)vb);
    TokenIVARS *const a = Token_IVARS(token_a);
    TokenIVARS *const b = Token_IVARS(token_b);
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
Token_Get_Start_Offset_IMP(Token *self) {
    return Token_IVARS(self)->start_offset;
}

uint32_t
Token_Get_End_Offset_IMP(Token *self) {
    return Token_IVARS(self)->end_offset;
}

float
Token_Get_Boost_IMP(Token *self) {
    return Token_IVARS(self)->boost;
}

int32_t
Token_Get_Pos_Inc_IMP(Token *self) {
    return Token_IVARS(self)->pos_inc;
}

int32_t
Token_Get_Pos_IMP(Token *self) {
    return Token_IVARS(self)->pos;
}

char*
Token_Get_Text_IMP(Token *self) {
    return Token_IVARS(self)->text;
}

size_t
Token_Get_Len_IMP(Token *self) {
    return Token_IVARS(self)->len;
}

void
Token_Set_Text_IMP(Token *self, char *text, size_t len) {
    TokenIVARS *const ivars = Token_IVARS(self);
    if (len > ivars->len) {
        FREEMEM(ivars->text);
        ivars->text = (char*)MALLOCATE(len + 1);
    }
    memcpy(ivars->text, text, len);
    ivars->text[len] = '\0';
    ivars->len = len;
}


