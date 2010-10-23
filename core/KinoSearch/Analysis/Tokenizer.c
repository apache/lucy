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

#define C_KINO_TOKENIZER
#define C_KINO_TOKEN
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Analysis/Tokenizer.h"
#include "KinoSearch/Analysis/Token.h"
#include "KinoSearch/Analysis/Inversion.h"

Tokenizer*
Tokenizer_new(const CharBuf *pattern)
{
    Tokenizer *self = (Tokenizer*)VTable_Make_Obj(TOKENIZER);
    return Tokenizer_init(self, pattern);
}

Inversion*
Tokenizer_transform(Tokenizer *self, Inversion *inversion)
{
    Inversion *new_inversion = Inversion_new(NULL);
    Token *token;

    while (NULL != (token = Inversion_Next(inversion))) {
        Tokenizer_Tokenize_Str(self, token->text, token->len, new_inversion);
    }

    return new_inversion;
}

Inversion*
Tokenizer_transform_text(Tokenizer *self, CharBuf *text)
{
    Inversion *new_inversion = Inversion_new(NULL);
    Tokenizer_Tokenize_Str(self, (char*)CB_Get_Ptr8(text), CB_Get_Size(text), 
        new_inversion);
    return new_inversion;
}

Obj*
Tokenizer_dump(Tokenizer *self)
{
    Tokenizer_dump_t super_dump
        = (Tokenizer_dump_t)SUPER_METHOD(TOKENIZER, Tokenizer, Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Str(dump, "pattern", 7, CB_Dump(self->pattern));
    return (Obj*)dump;
}

Tokenizer*
Tokenizer_load(Tokenizer *self, Obj *dump)
{
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    Tokenizer_load_t super_load 
        = (Tokenizer_load_t)SUPER_METHOD(TOKENIZER, Tokenizer, Load);
    Tokenizer *loaded = super_load(self, dump);
    CharBuf *pattern = (CharBuf*)CERTIFY(
        Hash_Fetch_Str(source, "pattern", 7), CHARBUF);
    return Tokenizer_init(loaded, pattern);
}

bool_t
Tokenizer_equals(Tokenizer *self, Obj *other)
{
    Tokenizer *const evil_twin = (Tokenizer*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, TOKENIZER)) return false;
    if (!CB_Equals(evil_twin->pattern, (Obj*)self->pattern)) return false;
    return true;
}


